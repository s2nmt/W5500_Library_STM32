/*
 * app_5500.h
 *
 *  Created on: Jul 22, 2023
 *      Author: dangnguyen
 */

#ifndef SRC_LAN_W5500_APP_5500_H_
#define SRC_LAN_W5500_APP_5500_H_

static void PrintPHYConf();

void test5500(void);
char W5500_Mqtt_Publish(uint8_t *topic, uint8_t *msg);

char PHYStatusCheck(char try_time);
char W5500_ConnectServer (void);

//DHCP
char DHCPTry ();
void my_ip_conflict(void);
void my_ip_assign(void);

static void Display_Net_Conf();

static void Net_Conf();

//char W5500_Mqtt_Subcribe(uint8_t *topic);
char W5500_Mqtt_Subcribe(uint8_t *topic);

uint8_t W5500_Mqtt_Yield();
uint8_t W5500_ConnectNetwork();

char PHYStatusCheckPing(char try_time);

uint8_t W5500_ConnectCable();
uint8_t W5500_PingServer();
uint8_t W5500_connectServer();
uint8_t W5500_ConnectMQTT();
uint8_t W5500_ConnectCableTest();
void W5500_Disconnect_MQTT();
void initialize_opts();
#endif /* SRC_LAN_W5500_APP_5500_H_ */
