/**
  ******************************************************************************
  * @file    stm32f2xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    06-June-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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
#include "stm32f2xx_it.h"
#include "main.h"
#include "stm32f2x7_eth.h"
#include "stm32_eval_sdio_sd.h"
#include "usart_dma.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
extern xSemaphoreHandle s_xSemaphore;
/* Private variables ---------------------------------------------------------*/



extern uint32_t ETH_GetRxPktSize_1(void);
extern void Time_Update(void);
/* Private function prototypes -----------------------------------------------*/
extern void xPortSysTickHandler(void);


/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
 
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */

void SysTick_Handler(void)
{
  Time_Update();
  xPortSysTickHandler();
}

/******************************************************************************/
/*                 STM32F2xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f2xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles ethernet DMA interrupt request.
  * @param  None
  * @retval None
  */
void ETH_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  /* Frame received */
  if ( ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET) 
  {
    /* Give the semaphore to wakeup LwIP task */
    xSemaphoreGiveFromISR( s_xSemaphore, &xHigherPriorityTaskWoken );   
  }
	
  /* Clear the interrupt flags. */
  /* Clear the Eth DMA Rx IT pending bits */
  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
	
  /* Switch tasks if necessary. */	
  if( xHigherPriorityTaskWoken != pdFALSE )
  {
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
  if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
  {   
    /* Write one byte to the transmit data register */
    USART_ITConfig(EVAL_COM2, USART_IT_TXE, DISABLE);
  }

}
/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{
   SD_ProcessIRQSrc();
}

/*******************************************************************************
* Function Name  : DMA_URART_TX_IRQHandler
* Description    : This function handles DMA USART TX interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Stream6_IRQHandler(void)
{  
  DMA_Cmd(DMA1_Stream6, DISABLE);
	DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
	tx_now_send++;
	if(tx_now_send > 9) tx_now_send = 0;
	
	if(tx_now_send < tx_count)
	{ 
	  Fill_Buffer(TxBuffer_dma, 24, TxBuffer_dma_save[tx_now_send]);
	  DMA_Cmd(DMA1_Stream6, ENABLE);  
	}
	else 
	{
	  data_complete = TRUE;
	}
			
}

/*******************************************************************************
* Function Name  : DMA_URART_RX_IRQHandler
* Description    : This function handles DMA USART RX interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Stream5_IRQHandler(void)
{  	
	DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
	
	if(data_count+24 <= 240)
	{
		memcpy(&RxBuffer_dma_save[rx_head], &RxBuffer_dma[0], 24);
	  rx_head += 0x18;
		data_count += 0x18;
		if(rx_head == 240) rx_head = 0;
	}
}
/*******************************************************************************
* Function Name  : EXIT9_IRQHandler
* Description    : This function handles EXTI9_5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line5) != RESET)
  { 
    shake_happen = TRUE;
    /* Clear the EXTI line 0 pending bit */
		
		EXTI_ClearITPendingBit(EXTI_Line5);
  }
}
/*******************************************************************************
* Function Name  : RTC ALARM IRQHandler
* Description    : This function handles in ALARM time .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
extern int time_to_CheckUpdate;

void RTC_Alarm_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {
    time_to_CheckUpdate = 1;
	
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    EXTI_ClearITPendingBit(EXTI_Line17);
  } 
}

void SD_SDIO_DMA_IRQHANDLER(void)
{
  SD_ProcessDMAIRQ();
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
