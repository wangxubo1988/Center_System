/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    06-June-2011 
  * @brief   This file contains all the functions prototypes for the main.c 
  *          file.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "stm32f2x7_eth_bsp.h"


#include "FreeRTOS.h"
#include "task.h"


/* Exported types ------------------------------------------------------------*/
typedef unsigned char INT8U;
/* Exported constants --------------------------------------------------------*/

//#define USE_LCD        /* enable LCD  */  
#define USE_DHCP       /* enable DHCP, if disabled static address is used*/
#define USE_KEY
   
/* Uncomment SERIAL_DEBUG to enables retarget of printf to  serial port (COM1 on STM32 evalboard) 
   for debug purpose */   
  
#define USART_DMA
#define My_DEBUG

///* MAC ADDRESS*/
//#define MAC_ADDR0   02
//#define MAC_ADDR1   00
//#define MAC_ADDR2   00
//#define MAC_ADDR3   00
//#define MAC_ADDR4   00
//#define MAC_ADDR5   00
 
/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   103
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1  

/* MII and RMII mode selection, for STM322xG-EVAL Board(MB786) RevB ***********/
#define RMII_MODE  // In this case the System clock frequency is configured
                     // to 100 MHz, for more details refer to system_stm32f2xx.c      
                                     
//#define MII_MODE

/* Uncomment the define below to clock the PHY from external 25MHz crystal (only for MII mode) */
#ifdef 	MII_MODE
 #define PHY_CLOCK_MCO
#endif


#define SYS_RST 0
#define UPDATE_FAIL 1
#define SYS_UART_MIS 2
#define SYS_SEVER_DISCON 3
#define SYS_UPDATE 4
#define SYS_MISSPING 5
#define NET_LOGIN 6
#define SYS_UART_RESET 7
/* STM322xG-EVAL jumpers setting
    +==========================================================================================+
    +  Jumper |       MII mode configuration            |      RMII mode configuration         +
    +==========================================================================================+
    +  JP5    | 2-3 provide 25MHz clock by MCO(PA8)     |  2-3 provide 50MHz clock by MCO(PA8) +
    +         | 1-2 provide 25MHz clock by ext. Crystal |                                      +
    + -----------------------------------------------------------------------------------------+
    +  JP6    |          2-3                            |  1-2                                 +
    + -----------------------------------------------------------------------------------------+
    +  JP8    |          Open                           |  Close                               +
    +==========================================================================================+
  */

/* Exported macro ------------------------------------------------------------*/
extern uint32_t LocalTime;

extern bool shake_happen;

extern int usart_check;
extern int ping_check;
extern int	usart_reset_key;
/* Exported functions ------------------------------------------------------- */  
void Time_Update(void);



#endif /* __MAIN_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

