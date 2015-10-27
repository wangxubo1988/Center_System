#ifdef USE_DEBUG

#include "stm32f2xx.h"
#include "stm32_eval.h"
#include <stdio.h>
#include <string.h>
#include <usart_debug.h>

void Usart3_DebugInit(void)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
  /* USARTx configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx;

  STM_EVAL_COMInit(COM2, &USART_InitStructure);

  /* Enable the EVAL_COM1 Transmoit interrupt: this interrupt is generated when the 
     EVAL_COM1 transmit data register is empty */  
  USART_ITConfig(EVAL_COM2, USART_IT_TXE, ENABLE);


  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
      /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void Usart3_Send(uint8_t  *cmd_msg, int n)
{   
  int s = 0;
  while(s < n)
  {
    while (USART_GetFlagStatus(EVAL_COM2, USART_FLAG_TC) == RESET)
    {
	  
	}
  
  /* write a character to the USART */
    USART_SendData(EVAL_COM2, cmd_msg[s]);
    s++;
  } 
}

void Usart3_Print(uint8_t  *cmd_msg)
{
  int s = 0;
	int n = strlen(cmd_msg);
  while(s < n)
  {
    while (USART_GetFlagStatus(EVAL_COM2, USART_FLAG_TC) == RESET)
    {
	  
	}
  
  /* write a character to the USART */
    USART_SendData(EVAL_COM2, cmd_msg[s]);
    s++;
  } 

}

int fputc(int ch, FILE *f)
{

  Usart3_Print((uint8_t  *)&ch);

  return ch;
}

#endif

