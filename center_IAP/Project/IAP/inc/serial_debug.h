/**
  ******************************************************************************
  * @file    serial_debug.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    06-June-2011
  * @brief   Header for serial_debug.c file
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SERIAL_DEBUG_H
#define __SERIAL_DEBUG_H


/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Definition for USART2 resources *   pA2 pA3*******************************************/
#define USARTx                           USART2
#define USARTx_CLK                       RCC_APB1Periph_USART2
#define USARTx_CLK_INIT                  RCC_APB1PeriphClockCmd
#define USARTx_IRQn                      USART2_IRQn
#define USARTx_IRQHandler                USART2_IRQHandler

#define USARTx_TX_PIN                    GPIO_Pin_5                
#define USARTx_TX_GPIO_PORT              GPIOD                       
#define USARTx_TX_GPIO_CLK               RCC_AHB1Periph_GPIOD
#define USARTx_TX_SOURCE                 GPIO_PinSource5
#define USARTx_TX_AF                     GPIO_AF_USART2

#define USARTx_RX_PIN                    GPIO_Pin_6                
#define USARTx_RX_GPIO_PORT              GPIOD                    
#define USARTx_RX_GPIO_CLK               RCC_AHB1Periph_GPIOD
#define USARTx_RX_SOURCE                 GPIO_PinSource6
#define USARTx_RX_AF                     GPIO_AF_USART2


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
#ifdef SERIAL_DEBUG
  void DebugComPort_Init(void);
#endif
void Usart_Send(INT8U  *cmd_msg);

#endif /* __SERIAL_DEBUG_H */  


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
