#include "usr_api.h"
#include "main.h"
#include "usr_cpu.h"
#include "const.h"
#include "usr_api.h"
#include "usart_dma.h"

/*sys variables*/
u32 CpuID[3];

/*net variables*/
unsigned char cmd_reply[24] = {0xff, 0xfe, 0x15};
unsigned char usart_reply[24] = {0xff, 0xfe, 0x15};

/*usart variables*/
unsigned char cmd_to_usart[5][24] = {{0xff, 0xfe, 0x15, 0x00, 0xf0},
 									 {0xff, 0xfe, 0x15, 0x00, 0xf1}, 
									 {0xff, 0xfe, 0x15, 0x00, 0xf2},
									 {0xff, 0xfe, 0x15, 0x00, 0xf3},
									 {0xff, 0xfe, 0x15, 0x00, 0xf4}};

 
void Get_STM32F207_ID(void)
{
  CpuID[0]=*(vu32*)(0x1fff7a10);
  CpuID[1]=*(vu32*)(0x1fff7a14);
  CpuID[2]=*(vu32*)(0x1fff7a18);
  macaddress[0] = (uint8_t)(CpuID[1] >> 8) & 0xfe;	  //×îµÍÎ»Îª0
  macaddress[1] = (uint8_t)(CpuID[1] );
  macaddress[2] = (uint8_t)((CpuID[0] >> 24) );
  macaddress[3] = (uint8_t)((CpuID[0] >> 16));
  macaddress[4] = (uint8_t)((CpuID[0] >> 8) );
  macaddress[5] = (uint8_t)(CpuID[0]);
}

void SystemReset(void)
{
  __set_FAULTMASK(1);
  NVIC_SystemReset();
}


/**
  * @brief  change the system mode, CONFIG mode or NORMAL mode.
  * @param  None
  * @retval None
 */

void Change_System_Mode(void)
{ 
  INT8U  *usart_msg;
  CONFIG_MODE = 1 - CONFIG_MODE;
  if(CONFIG_MODE == 1)
  {
    cmd_to_usart[0][10] = 1;
	  usart_msg =cmd_to_usart[0];
		UsartDMA_Send(usart_msg);
	  STM_EVAL_LEDOn(LED3);
  }
  else
  {
    cmd_to_usart[0][10] = 0;
    usart_msg =cmd_to_usart[0];
	  UsartDMA_Send(usart_msg);

	  STM_EVAL_LEDOff(LED3);
		
		My_Flash_Write(PASS_TYPE);	
    My_Flash_Write(ROOM_TYPE);
    My_Flash_Write(DEVICE_TYPE);
    My_Flash_Write(SCENE_TYPE);
    My_Flash_Write(WARN_DEV_TYPE);
    My_Flash_Write(DEV_TIMER_TYPE);
		
		sys_info_change = 0;
	  device_change = 0;
	  scene_change =0;
	
  }
}

/**
  * @brief  get a rngnuber.
  * @param  None.
  * @retval a number from 20000ms to 40000
  */
unsigned int Get_RNG_LivingSec(void)
{
  uint32_t sec = 0;
  uint32_t  random32bit = 0;
  
  while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET)
  {
  }
     
  random32bit = RNG_GetRandomNumber();
  sec =  20 + random32bit/(0xffffffff/20);
  return sec;
}

/**
  * @brief  get a rngnuber.
  * @param  None.
  * @retval a number from 0 to 3600
  */

int Get_RNG_Sec(void)
{ 
  int sec = 0;
  uint32_t  random32bit = 0;
  
  while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET)
  {
  }
     
  random32bit = RNG_GetRandomNumber();
  sec =  random32bit/(0xffffffff/3600);
  return sec;
}

/**
  * @brief  to reset usart.
  * @param  None.
  * @retval None.
  */

void Usart_ResetFunction(void)
{
	if(usart_reset_key == 5){
	      Usart_GPIO_Set(CLOSE);
	    }else if(usart_reset_key == 10){
	      Usart_GPIO_Set(WAIT);
      }else if(usart_reset_key == 15){
				Usart_GPIO_Set(OPEN);
				usart_reset_key = 0;
				Write_SystermState(SYS_UART_RESET);
      }
	usart_reset_key++;
}

