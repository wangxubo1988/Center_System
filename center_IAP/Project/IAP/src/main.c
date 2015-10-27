/**
  ******************************************************************************
  * @file    main.c 
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    04/16/2010
  * @brief   Main program body
  * Modefied
  * @date    03-April-2012
  * @author  Armjishu.com Application Team
  * @version V1.0  
  ******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
typedef  void (*iapfun)(void);
iapfun jump2app;

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
iapfun jump2app;
/* Private variables ---------------------------------------------------------*/

u8 temp[512];
int temp_size = 0;
int check_err = 0;

int soft_len = 0;

u8 md5_read[16];
unsigned char md5_check[16];


/* Private macro -------------------------------------------------------------*/

u8 iap_version[4]={0x31,0x30,0x30,0x32};

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

static __asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0             //set Main Stack value
    BX lr
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */


void Usr_Init(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	
	FlashInit();
	
	/* Initialize LEDs and Key Button mounted on STM3210X-EVAL board */       
  STM_EVAL_LEDInit(LED1);
  STM_EVAL_LEDInit(LED2);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);

  /* Turn off All LEDs */
  STM_EVAL_LEDOff(LED1);
  STM_EVAL_LEDOff(LED2);
  STM_EVAL_LEDOff(LED3);
  STM_EVAL_LEDOff(LED4);
}

void Soft_Version(void)
{
	int t = 0;
	u8 soft_ver1[4];
  FlashBufferRead((u8*)&soft_ver1, zone_iap_version, 4);
	if(0!= memcmp(soft_ver1, iap_version, 4)){
    FlashSectorErase(zone_iap_version, 1);
		FlashBufferWrite(iap_version, zone_iap_version, 4);
	}
}
void IWDG_Init(void)
{ 
  RCC_LSICmd(ENABLE);
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);
  
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  IWDG_SetPrescaler(IWDG_Prescaler_256);

  IWDG_SetReload(4095);
  
  IWDG_ReloadCounter();
	
  IWDG_Enable();
}

int main(void)
{  
	IWDG_Init();
	Usr_Init();
	Soft_Version();
	
	Flash_Func();
	Sd_Func();
}


void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
	//	PSP_PSP(*(vu32*)appxaddr);
		jump2app();								//跳转到APP.								//跳转到APP.
	}
}






/******************* (C) COPYRIGHT 2012 armjishu.com *****END OF FILE****/
