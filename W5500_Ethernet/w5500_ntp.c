/*
 * w5500_ntp.c
 *
 *  Created on: Jun 25, 2024
 *      Author: Minh Tuan
 */


#include "w5500_ntp.h"
#include "socket.h"
#include "w5500.h"

#include <string.h>
#include <stdio.h>

uint8_t NTP_SERVER_IP[4]={216, 239, 35, 0};


uint8_t ntptimer = 0;

void func_analysis_ntp_back_msg(uint8_t* buf, uint16_t idx, TSTAMP *tstmp, DATETIME *datetime)
{
    TSTAMP seconds = 0;
    uint8_t i = 0, zone = TIMEZONE8;
    for (i = 0; i < 4; i++)
    {
      seconds = (seconds << 8) | buf[idx + i];
    }
    switch (zone)
    {
      case 0:
        seconds -=  12*3600;
        break;
      case 1:
        seconds -=  11*3600;
        break;
      case 2:
        seconds -=  10*3600;
        break;
      case 3:
        seconds -=  (9*3600+30*60);
        break;
      case 4:
        seconds -=  9*3600;
        break;
      case 5:
      case 6:
        seconds -=  8*3600;
        break;
      case 7:
      case 8:
        seconds -=  7*3600;
        break;
      case 9:
      case 10:
        seconds -=  6*3600;
        break;
      case 11:
      case 12:
      case 13:
        seconds -= 5*3600;
        break;
      case 14:
        seconds -=  (4*3600+30*60);
        break;
      case 15:
      case 16:
        seconds -=  4*3600;
        break;
      case 17:
        seconds -=  (3*3600+30*60);
        break;
      case 18:
        seconds -=  3*3600;
        break;
      case 19:
        seconds -=  2*3600;
        break;
      case 20:
        seconds -=  1*3600;
        break;
      case 21:                            //��
      case 22:
        break;
      case 23:
      case 24:
      case 25:
        seconds +=  1*3600;
        break;
      case 26:
      case 27:
        seconds +=  2*3600;
        break;
      case 28:
      case 29:
        seconds +=  3*3600;
        break;
      case 30:
        seconds +=  (3*3600+30*60);
        break;
      case 31:
        seconds +=  4*3600;
        break;
      case 32:
        seconds +=  (4*3600+30*60);
        break;
      case 33:
        seconds +=  5*3600;
        break;
      case 34:
        seconds +=  (5*3600+30*60);
        break;
      case 35:
        seconds +=  (5*3600+45*60);
        break;
      case 36:
        seconds +=  6*3600;
        break;
      case 37:
        seconds +=  (6*3600+30*60);
        break;
      case 38:
        seconds +=  7*3600;
        break;
      case 39:
        seconds +=  8*3600;
        break;
      case 40:
        seconds +=  9*3600;
        break;
      case 41:
        seconds +=  (9*3600+30*60);
        break;
      case 42:
        seconds +=  10*3600;
        break;
      case 43:
        seconds +=  (10*3600+30*60);
        break;
      case 44:
        seconds +=  11*3600;
        break;
      case 45:
        seconds +=  (11*3600+30*60);
        break;
      case 46:
        seconds +=  12*3600;
        break;
      case 47:
        seconds +=  (12*3600+45*60);
        break;
      case 48:
        seconds +=  13*3600;
        break;
      case 49:
        seconds +=  14*3600;
        break;
    }

    *tstmp = seconds;
    //calculation for date
    calc_date_time(seconds,datetime);
}
void calc_date_time(TSTAMP seconds, DATETIME *datetime)
{
    uint8_t yf=0;
	uint32_t p_year_total_sec;
    uint32_t r_year_total_sec;
    TSTAMP n=0,d=0,total_d=0,rz=0;
    uint16_t y=0,r=0,yr=0;
    signed long long yd=0;

    n = seconds;
    total_d = seconds/(SECS_PERDAY);
    d=0;
    p_year_total_sec=SECS_PERDAY*365;
    r_year_total_sec=SECS_PERDAY*366;
    while(n>=p_year_total_sec)
    {
      if((EPOCH+r)%400==0 || ((EPOCH+r)%100!=0 && (EPOCH+r)%4==0))
      {
        n = n -(r_year_total_sec);
        d = d + 366;
      }
      else
      {
        n = n - (p_year_total_sec);
        d = d + 365;
      }
      r+=1;
      y+=1;

    }

    y += EPOCH;

    datetime->yy = y;

    yd=0;
    yd = total_d - d;

    yf=1;
    while(yd>=28)
    {

        if(yf==1 || yf==3 || yf==5 || yf==7 || yf==8 || yf==10 || yf==12)
        {
          yd -= 31;
          if(yd<0)break;
          rz += 31;
        }

        if (yf==2)
        {
          if (y%400==0 || (y%100!=0 && y%4==0))
          {
            yd -= 29;
            if(yd<0)break;
            rz += 29;
          }
          else
          {
            yd -= 28;
            if(yd<0)break;
            rz += 28;
          }
        }
        if(yf==4 || yf==6 || yf==9 || yf==11 )
        {
          yd -= 30;
          if(yd<0)break;
          rz += 30;
        }
        yf += 1;

    }
    datetime->mo=yf;
    yr = total_d-d-rz;

    yr += 1;

    datetime->dd=yr;

    //calculation for time
    seconds = seconds%SECS_PERDAY;
    datetime->hh = seconds/3600;
    datetime->mm = (seconds%3600)/60;
    datetime->ss = (seconds%3600)%60;

}
uint8_t func_pack_ntp_message(uint8_t *ntp_server_ip, uint8_t * ntp_message)
{
   uint8_t flag;
   NTPFORMAT ntpfmt;

   ntpfmt.dstaddr[0] = ntp_server_ip[0];
   ntpfmt.dstaddr[1] = ntp_server_ip[1];
   ntpfmt.dstaddr[2] = ntp_server_ip[2];
   ntpfmt.dstaddr[3] = ntp_server_ip[3];
   /*NTPformat.dstaddr[0] = ip[0];
   NTPformat.dstaddr[1] = ip[1];
   NTPformat.dstaddr[2] = ip[2];
   NTPformat.dstaddr[3] = ip[3];*/


   ntpfmt.leap = 0;           /* leap indicator */
   ntpfmt.version = 4;        /* version number */
   ntpfmt.mode = 3;           /* mode */
   ntpfmt.stratum = 0;        /* stratum */
   ntpfmt.poll = 0;           /* poll interval */
   ntpfmt.precision = 0;      /* precision */
   ntpfmt.rootdelay = 0;      /* root delay */
   ntpfmt.rootdisp = 0;       /* root dispersion */
   ntpfmt.refid = 0;          /* reference ID */
   ntpfmt.reftime = 0;        /* reference time */
   ntpfmt.org = 0;            /* origin timestamp */
   ntpfmt.rec = 0;            /* receive timestamp */
   ntpfmt.xmt = 1;            /* transmit timestamp */

   flag = (ntpfmt.leap<<6)+(ntpfmt.version<<3)+ntpfmt.mode; //one byte Flag
   ntp_message[0] = flag;
   return 0;
}



uint8_t func_get_ntp_time(uint8_t sock, TSTAMP *tstamp,DATETIME *datetime, uint16_t timeout_ms)
{
	uint16_t cnt_timeout = 0;
	uint8_t ntp_message[48] = {0,};
	uint8_t ntp_back_msg[256] = {0,};
	uint8_t ntp_s_ip[4] = {0,};
	uint16_t ntp_s_port, len;

	if(getSn_SR(sock) == SOCK_CLOSED){
		socket(sock, Sn_MR_UDP, NTP_PORT,0);

	}
	func_pack_ntp_message(NTP_SERVER_IP, ntp_message);
	sendto(sock , ntp_message, sizeof(ntp_message), NTP_SERVER_IP, NTP_PORT);

	for(;;){
		if(getSn_IR(sock) & Sn_IR_RECV){
			setSn_IR(sock, Sn_IR_RECV);
		}
		len = getSn_RX_RSR(sock);

		if(len > 0){
			len = recvfrom(sock, ntp_back_msg, sizeof(ntp_back_msg), ntp_s_ip, &ntp_s_port);

			if(len >= 48 && ntp_s_port == NTP_PORT){
				func_analysis_ntp_back_msg(ntp_back_msg, 40, tstamp, datetime);
				break;
			}
		}
		cnt_timeout ++;
		if(cnt_timeout > timeout_ms){
			close(sock);
			return 1;
		}
		HAL_Delay(1);
	}
	close(sock);
	return 0;
}

