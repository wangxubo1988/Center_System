#ifndef _MAIN_TASK_H
#define _MAIN_TASK_H

#include "main.h"

typedef struct command_list
{
  INT8U cmd[24];
  struct tcp_pcb *socket;
}CMD_LIST;

typedef struct cmd_analysis
{
  INT8U cmd;
  void (*func_cmd)();
}CMD_ANALYSIS;



extern uint32_t IPaddress;
extern unsigned char macaddress[6];

extern uint32_t eth_error;

extern CMD_ANALYSIS cmd_analysis[0x60];

extern struct tcp_pcb *tcp_dev_new ;

void Analysis_Task(void * pvParameters);
void Save_Cmd(INT8U *cmd_msg);
void Change_System_Mode(void);

void ping(void);
void LwIP_Init(void);
void LwIP_Pkt_Handle(void);
void LwIP_Periodic_Handle(__IO uint32_t localtime);



#endif
