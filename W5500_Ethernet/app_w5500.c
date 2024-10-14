/*
 * app_w5500.c
 *
 *  Created on: Jul 22, 2023
 *      Author: dangnguyen
 */

#include <stdio.h>
#include <stdlib.h>
#include "w5500_spi.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "w5500_spi.h"
#include "mqtt_interface.h"
#include "MQTTClient.h"
#include "dns.h"
#include "app_5500.h"
#include "string.h"
#include "main.h"
#include "dhcp.h"
#include "PING.h"

wiz_NetInfo gWIZNETINFO = {
		.mac = {0x00,0x00,0x00,0x00,0x00,0xbb},
		.ip  = {192,168,1,252},
		.sn  = {255,255,255,0},
		.gw  = {192,168,1,1},
		.dns = {8,8,8,8},
		.dhcp = NETINFO_DHCP};

Network n;
Client c;

/*Socket number defines*/
#define TCP_SOCKET	0
#define UDP_SOCKET	1

extern uint8_t  DNS_SOCKET;    // SOCKET number for DNS

/*Port number defines*/
#define TCP_PORT 	60000
#define UDP_PORT 	60001

/*Receive Buffer Size define*/
#define BUFFER_SIZE	2048
#define MQTTSUB_SIZE 7

/*Global variables*/
extern uint8_t targetIP[4];
extern uint16_t targetPort;

extern uint8_t DelayLan(uint8_t time);
unsigned char TargetName[100];

unsigned char w5500_buf[100];
int w5500_rc = 0;
extern uint8_t LAN_Rx_Buf[500];
struct opts_struct
{
	char* clientid;
	int nodelimiter;
	char* delimiter;
	enum QoS qos;
	char* username;
	char* password;
	char* host;
	int port;
	int showtopics;
} opts;

void initialize_opts() {
    opts.clientid = (char*)DeviceID;
    opts.nodelimiter = 0;
    opts.delimiter = (char*)"\n";
    opts.qos = QOS0;
    opts.username = (char*)USERS;
    opts.password = (char*)PASSWORDS;
    opts.host = (char*)HOST;
    opts.port = (int)PORT;  // Assuming PORT is defined elsewhere
    opts.showtopics = 0;
}

void messageArrived(MessageData* md)
{
	MQTTMessage* message = md->message;

	if (opts.showtopics)
	{
		memcpy(LAN_Rx_Buf, "MQTTSUB", MQTTSUB_SIZE);
		memcpy(LAN_Rx_Buf + MQTTSUB_SIZE ,(char*)message->payload,(int)message->payloadlen);
		*(LAN_Rx_Buf + (int)message->payloadlen + MQTTSUB_SIZE ) = '\r';
		*(LAN_Rx_Buf + (int)message->payloadlen + MQTTSUB_SIZE + 1) = '\n';
		//printf("%s\r\n",LAN_Rx_Buf);
	}
	//	printf("%12s\r\n", md->topicName->lenstring.len, md->topicName->lenstring.data);
	if (opts.nodelimiter)
		printf("%.*s", (int)message->payloadlen, (char*)message->payload);
	else
		printf("%.*s%s", (int)message->payloadlen, (char*)message->payload, opts.delimiter);
	//fflush(stdout);
}
unsigned char tempBuffer[BUFFER_SIZE] = {};

char PHYStatusCheck(char try_time)
{
	uint8_t tmp;
#ifdef  DEBUGGER
	//printf("Check cable\n");
#endif
	do
	{
		ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);

		if(tmp == PHY_LINK_OFF)
		{
#ifdef DEBUGGER
				//printf("NO CABLE CONTECT\r\n");
#endif
			HAL_Delay(1000);
			try_time--;
			if(try_time == 0) return 0;
		}
	}while(tmp == PHY_LINK_OFF);

		//printf("GOOD, CONNECTED\r\n");

	return 1;
}
uint8_t W5500_ConnectCableTest(){

	if(!PHYStatusCheckPing(5)) return 0; //try 5 time and no cable

	for(char i = 0; i <= 1; i++)
	{
		if(DHCPTry()) break;
		if(i == 1)  return 0; // DHCP fail
	}
	return 1;
}

char PHYStatusCheckPing(char try_time)
{
	uint8_t tmp;
	//printf("Check cable\n");

	do
	{
		ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);

		if(tmp == PHY_LINK_OFF)
		{
			printf("NO CABLE CONTECT\r\n");
			if(DelayLan(1)) return 0;
			try_time--;
			if(try_time == 0) return 0;
		}
	}while(tmp == PHY_LINK_OFF);
	//printf("GOOD, CONNECTED\r\n");
	return 1;
}

static void PrintPHYConf()
{
	wiz_PhyConf phyconf;

	ctlwizchip(CW_GET_PHYCONF, (void*) &phyconf);
	if(phyconf.by == PHY_CONFBY_HW)
	{
		printf("PHY CONFIG BY HARDWARE\r\n");
	}
	else{
		printf("PHY CONFIG BY REGISTER\r\n");
	}
	if(phyconf.mode == PHY_MODE_AUTONEGO)
	{
		printf("Auto Negotiation Mode\r\n");
	}
	else{
		printf("No Auto Negotiation\r\n");
	}
	if(phyconf.duplex == PHY_DUPLEX_FULL)
	{
		printf("Mode Full Duplex\r\n");
	}
	else {
		printf("Mode Half Duplex\r\n");
	}
	if(phyconf.speed == PHY_SPEED_10)
	{
		printf("Speed 10Mbps\r\n");
	}
	else {
		printf("Speed 100Mbps\r\n");
	}
}
uint8_t W5500_ConnectNetwork(){
	if(!PHYStatusCheck(5)) return 0;
	for(char i = 0; i <= 1; i++)
	{
		if(DHCPTry()) break;
		if(i == 1)  return 0; // DHCP fail
	}

	n.my_socket = 0;
	NewNetwork(&n,1);
	uint8_t tmp;
	uint8_t pDestaddr[4] = {103,151,238,68};
	tmp = ping_auto(0,pDestaddr);
	//tmp = ping_count(1,3,pDestaddr);
	if(tmp == SUCCESS){
		printf("PING TEST OK\r\n");
	}
	else{
		printf("ERROR  = %d\r\n",tmp);
	}

	if(!ConnectNetwork(&n, targetIP, targetPort)) {
		printf(" connect network fail \r\n");
		return 0;
	}
	printf(" connect network success\r\n");
	return 1;
}
uint8_t W5500_ConnectCable(){

	if(!PHYStatusCheck(3)) return 0; //try 5 time and no cable

	for(char i = 0; i <= 1; i++)
	{
		if(DHCPTry()) break;
		if(i == 1)  return 0; // DHCP fail
	}
	return 1;
}
uint8_t W5500_PingServer(){
	uint8_t tmp;
	uint8_t pDestaddr[4];
	pDestaddr[0] = targetIP[0];
	pDestaddr[1] = targetIP[1];
	pDestaddr[2] = targetIP[2];
	pDestaddr[3] = targetIP[3];
	tmp = ping_auto(0,pDestaddr);

	if(tmp == SUCCESS){
		printf("PING TEST OK\r\n");
		return 1;
	}
	else{
		printf("ERROR  = %d\r\n",tmp);
		return 0;
	}
}
uint8_t W5500_connectServer(){
	NewNetwork(&n,1);
	if(!ConnectNetwork(&n, targetIP, targetPort)) {
		printf("connect network fail \r\n");
		return 0;
	}
	printf("connect network success\r\n");
	return 1;
}
uint8_t W5500_ConnectMQTT(){
	initialize_opts();
	MQTTClientInit(&c,&n,1000,w5500_buf,2048,tempBuffer,2048);

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = opts.clientid;
	data.username.cstring = opts.username;
	data.password.cstring = opts.password;

	data.keepAliveInterval = 120;
	data.cleansession = 1;
	w5500_rc = MQTTConnect(&c, &data);
	opts.showtopics = 1;
	return w5500_rc;
}
char W5500_ConnectServer ()
{
	if(!PHYStatusCheck(5)) return 0; //try 5 time and no cable
	//PrintPHYConf();
	for(char i = 0; i <= 1; i++)
	{
		if(DHCPTry()) break;
		if(i == 1)  return 0; // DHCP fail
	}

	n.my_socket = 0;
	//DNS_init(1,tempBuffer);

	//while(DNS_run(gWIZNETINFO.dns,TargetName,targetIP) == 0){}
	NewNetwork(&n,1);

	if(!ConnectNetwork(&n, targetIP, targetPort)) return 0;
	MQTTClientInit(&c,&n,1000,w5500_buf,2048,tempBuffer,2048);

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = opts.clientid;
	data.username.cstring = opts.username;
	data.password.cstring = opts.password;

	data.keepAliveInterval = 60;
	data.cleansession = 1;

	w5500_rc = MQTTConnect(&c, &data);
	opts.showtopics = 1;
	return 1;
}

char W5500_Mqtt_Publish(uint8_t *topic, uint8_t *msg )
{
	enum QoS {
	        QOS0,
	    	QOS1,
	    	QOS2
	    };

	MQTTMessage message;
	message.qos = QOS0;
	message.retained = 0;
	message.dup = 0;
	message.id = 1234;
	message.payload = msg;
	message.payloadlen = strlen(message.payload);
	char res = MQTTPublish(&c, (const char *)topic, &message);
	printf("Ket qua pub LAN: %d\r\n", res);

	return res;
}
//DHCP
static void Net_Conf()
{
	/* wizchip netconf */
	ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
}
static void Display_Net_Conf()
{
	uint8_t tmpstr[6] = {0,};

	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);


	// Display Network Information
	ctlwizchip(CW_GET_ID,(void*)tmpstr);

#ifdef DEBUGGER
	if(gWIZNETINFO.dhcp == NETINFO_DHCP)
	{

			printf("\r\n===== %s NET CONF : DHCP =====\r\n",(char*)tmpstr);

	}
	else
	{

			   printf("\r\n===== %s NET CONF : Static =====\r\n",(char*)tmpstr);

	}

		printf(" MAC : %02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], gWIZNETINFO.mac[2], gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
		printf(" IP : %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
		printf(" GW : %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
		printf(" SN : %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
		printf("=======================================\r\n");
#endif
}

void my_ip_assign(void)
{
   getIPfromDHCP(gWIZNETINFO.ip);
   getGWfromDHCP(gWIZNETINFO.gw);
   getSNfromDHCP(gWIZNETINFO.sn);
   getDNSfromDHCP(gWIZNETINFO.dns);
   gWIZNETINFO.dhcp = NETINFO_DHCP;
   /* Network initialization */
   Net_Conf();      // apply from DHCP

   Display_Net_Conf();
#ifdef  DEBUGGER
   printf("DHCP LEASED TIME : %ld Sec.\r\n", getDHCPLeasetime());
#endif
}
uint8_t ipConflict = 0;
void my_ip_conflict(void)
{
	printf("CONFLICT IP from DHCP\r\n");
	ipConflict = 1;
   //halt or reset or any...
//	HAL_NVIC_SystemReset();
//   while(1); // this example is halt.
}

char DHCPTry ()
{
	uint8_t my_dhcp_retry = 0;
	#define DATA_BUF_SIZE   2048
	uint8_t gDATABUF[2048];

	W5500Init();
	gWIZNETINFO.mac[0] = ((uint8_t*)&DeviceID)[5];
	gWIZNETINFO.mac[1] = ((uint8_t*)&DeviceID)[4];
	gWIZNETINFO.mac[2] = ((uint8_t*)&DeviceID)[3];
	gWIZNETINFO.mac[3] = ((uint8_t*)&DeviceID)[2];
	gWIZNETINFO.mac[4] = ((uint8_t*)&DeviceID)[1];
	gWIZNETINFO.mac[5] = ((uint8_t*)&DeviceID)[0];

	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

	wiz_NetTimeout gWIZNETTIME = {.retry_cnt = 1,
	                               .time_100us = 2000};
	ctlnetwork(CN_SET_TIMEOUT,(void*)&gWIZNETTIME);
//	ctlnetwork(CN_GET_TIMEOUT, (void*)&gWIZNETTIME);
//
//	printf("Retry Count: %d\r\n", gWIZNETTIME.retry_cnt);
//	printf("Timeout (100us): %d\r\n", gWIZNETTIME.time_100us);
	PrintPHYConf();
	DHCP_init(6, gDATABUF);
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
	if(ipConflict == 1){
		ipConflict = 0;
		return 0;
	}
	char dhcp_ok = 0;
	uint16_t time_out = 1500;
	uint8_t timeout_phy = 5;
	while(1)
	{
		while(!PHYStatusCheck(1)){
			timeout_phy -- ;
			if(timeout_phy == 0){
				return 0;
			}
		}
		/* DHCP */
		/* DHCP IP allocation and check the DHCP lease time (for IP renewal) */
		if(gWIZNETINFO.dhcp == NETINFO_DHCP)
		{
			switch(DHCP_run())
			{
				case DHCP_IP_ASSIGN:
					break;
				case DHCP_IP_CHANGED:
					break;
				case DHCP_IP_LEASED:
					printf("DHCP LEASED\r\n");
					dhcp_ok = 1;
					break;
				case DHCP_FAILED:
					my_dhcp_retry++;
					if(my_dhcp_retry > 2)
					{
						gWIZNETINFO.dhcp = NETINFO_STATIC;
						DHCP_stop();      // if restart, recall DHCP_init()
						printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
						Net_Conf();
						Display_Net_Conf();   // print out static netinfo to serial
						my_dhcp_retry = 0;
					}
					break;
				default:
					break;
				}
		   }
		if(dhcp_ok)
		{
			return 1;
			break;
		}
		time_out--;
		HAL_Delay(1);
		if(time_out == 0) return 0;
	} // End of Main loop
}

char W5500_Mqtt_Subcribe(uint8_t *topic )
{
	printf("Subscribing to %s\r\n", (const char *)topic);
	w5500_rc = MQTTSubscribe(&c, (const char *)topic, opts.qos, messageArrived);
	printf("Subscribed %d\r\n", w5500_rc);
	return w5500_rc;
}

uint8_t W5500_Mqtt_Yield()
{
	return MQTTYield(&c, 1000);
}
void W5500_Disconnect_MQTT(){
	MQTTDisconnect(&c);
	disconnect(1);
}
