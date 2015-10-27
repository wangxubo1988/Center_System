/**
  ******************************************************************************
  * @file    serial_debug.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    06-June-2011
  * @brief   This file provide functions to retarget the C library printf function
  *          to the USART. 
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "stm32_eval.h"
#include "serial_debug.h"
#include <stdio.h>




/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* Private macro -------------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
extern void DMA_Send(INT8U *data);
/* Private functions ---------------------------------------------------------*/
INT8U byte =0;
INT8U zig_msg[24];
int s = 0;


/**
  * @brief  Initialize COM1 interface for serial debug
  * @note   COM1 interface is defined in stm3210g_eval.h file (under Utilities\STM32_EVAL\STM322xG_EVAL)  
  * @param  None
  * @retval None
  */
void DebugComPort_Init(void)
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
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  STM_EVAL_COMInit(COM1, &USART_InitStructure);

  /* Enable the EVAL_COM1 Transmoit interrupt: this interrupt is generated when the 
     EVAL_COM1 transmit data register is empty */  
  USART_ITConfig(EVAL_COM1, USART_IT_TXE, ENABLE);

  /* Enable the EVAL_COM1 Receive interrupt: this interrupt is generated  
     when the EVAL_COM1 receive data register is not empty */
  USART_ITConfig(EVAL_COM1, USART_IT_RXNE, ENABLE);

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
      /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

}


/**
  * @brief send 24byte data to the usart.
  * @param  
  * @retval None
 */

void Usart_Send(INT8U  *cmd_msg)
{   

  int s = 0;
  while(s < 24)
  {
    while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)
    {
	  
	}
  
  /* write a character to the USART */
    USART_SendData(EVAL_COM1, cmd_msg[s]);
    s++;
  }
  
}


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(EVAL_COM1, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)
  {}

  return ch;
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
