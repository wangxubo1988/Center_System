#ifndef NET_SEVER_H
#define NET_SEVER_H

#include "stm32f2xx.h"
#include "arch/cc.h"

#include "main.h"

struct sever_pdu
{
  INT8U head[5];
  INT8U data_len[4];
  INT8U cmd;
  INT8U cmd_data[100];
};

struct name 
{
  int length;
  char bytes[20];
};

//#define SEVER_ADDR 0x5200a8c0
#define SEVER_ADDR 0x0e0f792a


#define SEVER_PORT 4455
#define SEVER_CONFIG_PORT 4456

#define sever_HEAD1 0x73     
#define sever_HEAD2 0x68
#define sever_HEAD3 0x70
#define sever_HEAD4 0x31
#define sever_HEAD5 0x30

#define CMD_RNG 0x12

#define CMD_LOGIN 0x01
#define CMD_LOGIN_R 0x03

#define CMD_CONTROL 0x05
#define CMD_STATUS_R 0x06

#define CMD_LIVING 0x09

#define CMD_PASS_CHANGE 0x04
#define CMD_MASS_CHANGE 0x07

#define CMD_VERSION 0x10
#define CMD_VERSION_R 0x11

#define CMD_IAPVERSION 0x17
#define CMD_IAPVERSION_R 0x18

#define CMD_SENDSECRET 0x19

#define CMD_TIMESTAMP 0x15
#define CMD_TIMESTAMP_R 0x16

#define CMD_SUCESS 0x0;
#define CMD_FAILED 0x1;

#define NET_CONN 0
#define GET_DNS 1
#define SEVER_CONN 2
#define SERVER_Living 3

extern u32_t sever_addr;

extern bool net_connected;
extern bool net_logined;
extern bool wan_connected;
extern struct tcp_pcb * sever_pcb;
extern bool password_change;

extern INT8U soft_version[5];
extern u8 iap_version[4];
extern INT8U soft_md5[17];

void Server_Function(void);
void Net_Sever_Send(INT8U CMD, struct tcp_pcb * tcp);
void Check_Update(void);
void Check_Soft_Version(void);
#endif
