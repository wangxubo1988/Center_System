/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    06-June-2011
  * @brief   Main program body
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
#include "stm32f2x7_eth.h"
#include "main.h"
#include "usr_net.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tcpip.h"
#include "const.h"
#include "net_server.h"

#include "tcp.h"
#include "usr_cpu.h"
#include "usart_dma.h"
#include "usr_api.h"
#include "usr_sd.h"
#include "usr_net.h"

#include "flash.h"
#include "main_time.h"
#include "usr_bsp.h"
#include "ethernetif.h"
#ifdef USE_DEBUG
#include <usart_debug.h>
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/*--------------- Tasks Priority -------------*/

#define ANALYSIS_TASK_PRIO   ( tskIDLE_PRIORITY + 4 )
#define SERVER_TASK_PRIO   	 ( tskIDLE_PRIORITY + 2 )      
#define TIME_TASK_PRIO  	     ( tskIDLE_PRIORITY + 1 )
#define USART_TASK_PRIO  	     ( tskIDLE_PRIORITY + 5 )
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t LocalTime;

int time_to_CheckUpdate = 0;

bool shake_happen;

int usart_check = 0;
int ping_check = 0;
int	usart_reset_key = 0;

static int toEraseIformation = 0;

extern ETH_InitTypeDef ETH_InitStructure;
/* Private function prototypes -----------------------------------------------*/


extern void Server_Task(void *pdata);
extern void udp_port_reply(void);


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Toggle Led4 task
  * @param  pvParameters not used
  * @retval None
  */
 

 
void Time_Task(void * pvParameters)
{
  static uint32_t NetCheckTimer = 0;
  static uint32_t DisplayTimer = 0;
  static uint32_t DevLiveTimer = 0;
  static uint32_t IWDGTimer = 0;
  static uint32_t NetLinkTimer = 0;
  static uint32_t ShakeTimer = 0;
  static uint32_t EraseDoneTimer = 0;
	
	static uint32_t NetTimeout = 0;
	
  static char key_press_once = 0;
	
	static char i = 0;
	
	
 
  Write_SystermState(SYS_RST);
 
	
  while(1)
  { 
     
    Display_Periodic_Handle(LocalTime);
    LwIP_Periodic_Handle(LocalTime);

	 /* to inform the spi-flash is erased */
	  if(LocalTime >= (EraseDoneTimer + 300) && toEraseIformation == 6)
	  {
	    EraseDoneTimer = LocalTime;
	    STM_EVAL_LEDToggle(LED3);	
	  }
 
     /* 1 s */
    if (LocalTime >= (DisplayTimer + 1000)) 
    {
	    DisplayTimer = LocalTime;

	    STM_EVAL_LEDToggle(LED4);
			
	    if (ping_check == 1)
	    { 
				NetTimeout = 0;
				STM_EVAL_LEDToggle(LED2);
 				if(!center_NoID) Server_Function();
			}else{
			  NetTimeout++;
				if(NetTimeout == 600) {
					Write_SystermState(SYS_MISSPING);
					eth_error = 0;
					NetTimeout = 0;
				}
			}
			
	    if (usart_check == 1)
	    {
	  	  STM_EVAL_LEDToggle(LED1);
	    }
	  

	    if(toEraseIformation > 0 && toEraseIformation < 5) toEraseIformation++;
	    if(toEraseIformation == 5)
	    {
				STM_EVAL_LEDOn(LED3);
				STM_EVAL_LEDOff(LED1);
				STM_EVAL_LEDOff(LED2);
				STM_EVAL_LEDOff(LED4);
				
				Feed_IWDG();
	      My_Clean_Data();	
		    toEraseIformation ++;
	    }
			/*Usart RESET FUNCTION*/
			if(usart_reset_key >= 5){
				usart_check = 0;
	      Usart_ResetFunction();
				usart_check = 0;
      }
			/*
			for(i = 0; i < 10; i++)
			{
				if(pad_info[i].pad_living >0)
				{
					pad_info[i].pad_living --;
				}
			}*/
    }

	/*feed the watch dog 10s*/
	  if (LocalTime >= (IWDGTimer + 10000)) 
    {
	    IWDGTimer = LocalTime;
	    Feed_IWDG();
		}
		
	/*system check funcitions  per 10s */
    if (LocalTime >= (NetCheckTimer + 10000))
    {	
      NetCheckTimer = LocalTime;		
	  /*check network*/
			ping_check = 0 ;
      ping();
			
	  /*check Zigbee*/
		if(usart_reset_key < 5)
		{
			usart_check = 0;
	    usart_reset_key++;
			UsartDMA_Send(cmd_to_usart[1]);
    }
	    Check_Update();
	  }

	/*shake happen LEDON keeping 10s*/
	  if(shake_happen)
	  {
	    if(ShakeTimer == 0) {
				ShakeTimer = LocalTime;
				STM_EVAL_LEDOn(LED5);
	    }
			
			if(LocalTime >= (ShakeTimer + 10000))
	    {
	  	  ShakeTimer = 0;
		    shake_happen = FALSE;
				STM_EVAL_LEDOff(LED5);
      }
	  }
	
	/*check ETH linker status 2min*/
	  if (LocalTime >= (NetLinkTimer + 120000))
	  {    
	    NetLinkTimer = LocalTime;
	   
	    Dev_Check_Living();
	   		
	    if(eth_error == 0)
	    { 
				Feed_IWDG();
	      eth_error = ETH_Init(&ETH_InitStructure, 0x00);
	    }
	  }

	/*system check funcitions  per 5min */
    if (LocalTime >= (DevLiveTimer + 300000))
    {
	    DevLiveTimer = LocalTime;
      
	    if(sys_info_change == 1) My_Flash_Write(PASS_TYPE);
	    if(device_change == 1)	 My_Flash_Write(DEVICE_TYPE);
      if(scene_change == 1) My_Flash_Write(SCENE_TYPE);
      if(warn_dev_change == 1)  My_Flash_Write(WARN_DEV_TYPE);
	    if(dev_timer_change == 1)  My_Flash_Write(DEV_TIMER_TYPE);
			
	    sys_info_change = 0;
	    device_change = 0;
	    scene_change =0;
	    warn_dev_change = 0;
			dev_timer_change = 0;

	    if(time_to_CheckUpdate == 1)
	    {
	      UpdateTime_Send();
	  	  Check_Soft_Version();
		    time_to_CheckUpdate = 2;
	    }
		  else if (time_to_CheckUpdate >= 2)
		  {
        time_to_CheckUpdate++;
			  if(time_to_CheckUpdate ==  14)// to reset alarm time after 60min
			  {
      		time_to_CheckUpdate = 0;
		
		      Set_RTCAlarm_Time();
			  }
	    }
	  }


    /*to Detect the Systerm Mode choosing button */
	  if(!STM_EVAL_PBGetState(BUTTON_TAMPER))
	  {
	    key_press_once ++;
	  }
	  else if (key_press_once > 0)
	  {
	
	    Change_System_Mode();
	    key_press_once = 0;
	  }

	/*to Detect the room dev... information in spi-flash erasing  button */
	  if(!STM_EVAL_PBGetState(BUTTON_WAKEUP))
	  {
	    if(toEraseIformation == 0) toEraseIformation ++;	
	  }
	  else if(toEraseIformation < 5 && toEraseIformation > 0)
	  {
	    toEraseIformation = 0;
			OutPrint_SysState();
	  }
	  else if(toEraseIformation == 6)
	  {
	    toEraseIformation = 0;
	    SystemReset();
	  }

	/*task sleep 1ms */
	  vTaskDelay(1);
  }
}


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */


int main(void)
{ 
  /*when program locate in 0x8008000, set the NVIC_VectTab_FLASH 0x8008000*/
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);
	
	/*start the systerm watch dog*/
  IWDG_Init();
	
  /*get the borad id to set the macaddress*/
  Get_STM32F207_ID();
	
  /*init the rng fuction*/
  RNG_Init();

  /*init the rtc to set local time*/
  Time_Init();
  
  /*init USART DMA function*/	 
//DebugComPort_Init();
	USART_DMA_Config();
#ifdef USE_DEBUG
  Usart3_DebugInit();
#endif  
  /*init the SD card and read the alarm record information*/
  Usr_SD_Init();

  /*Initialize LCD and Leds */ 
  LCD_LED_KEY_Init();
  
  /*Initialize ShakeSensor*/
  ShakeSensorInit();
	
  /* configure ethernet (GPIOs, clocks, MAC, DMA) */
  	
  ETH_BSP_Config();
  
  /* init the spi_Flash*/ 
  My_Flash_Init();
	
	Feed_IWDG();
  /* Initilaize the LwIP stack */
  LwIP_Init();
  
  /*Init the systerm command*/
  Cmd_Analysis_Init();
  
  /* Create task ,to make TCP server, and main analysis task working. */
  Server_Task(NULL);
  
  /*udp reply*/
  udp_port_reply();

  /*init the udp to get the time from internet*/
  UpdateTime_Init();

  /* Start tcp_cmd_anlysis task */
  xTaskCreate(Analysis_Task, ( signed portCHAR * ) "Analysis_Task", configMINIMAL_STACK_SIZE*8, NULL, ANALYSIS_TASK_PRIO, NULL); 
  
  /* Start peiriod time task */
  xTaskCreate(Time_Task, ( signed portCHAR * ) "Time_Task", configMINIMAL_STACK_SIZE, NULL, TIME_TASK_PRIO, NULL);
  
  /*Analysis the usart command*/
  xTaskCreate(Usart_Dma_Analysis, ( signed portCHAR * ) "Usart_Dma_Analysis", configMINIMAL_STACK_SIZE, NULL, USART_TASK_PRIO, NULL);
  /*Recieve New Low-Inputing Data*/
	xTaskCreate(EthernetTask, ( signed portCHAR * ) "EthernetTask", configMINIMAL_STACK_SIZE*8, NULL, USART_TASK_PRIO, NULL);
  
  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  for( ;; );
}




/**
  * @brief  Update the time.
  * @param  None.
  * @retval None
  */
void Time_Update(void)
{
  LocalTime += 1;

}










/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
