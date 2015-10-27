#include "usr_bsp.h"
#include "stm32f2x7_eth.h"
#include "stm32_eval.h"

NVIC_InitTypeDef   Shake_NVIC_InitStructure;


/**
  * @brief  init STM32 rng Function.
  * @param  None.
  * @retval None
  */
void RNG_Init(void)
{
  /* Enable RNG clock source */
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
  /* RNG Peripheral enable */
  RNG_Cmd(ENABLE);
}

/**
  * @brief  iwdg init.
  * @param  None.
  * @retval None
  */
void IWDG_Init(void)
{ 
  RCC_LSICmd(ENABLE);
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);
  
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  IWDG_SetPrescaler(IWDG_Prescaler_256);

  IWDG_SetReload(2000);
  
  IWDG_ReloadCounter();
  
  IWDG_Enable();
}


/**
  * @brief  reset iwdg countet.
  * @param  None.
  * @retval None
  */
void Feed_IWDG(void)
{
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();
}

/**
  * @brief  Initializes the STM322xG-EVAL's LCD and LEDs resources.
  * @param  None
  * @retval None
  */
void LCD_LED_KEY_Init(void)
{
#ifdef USE_LCD
  /* Initialize the STM322xG-EVAL's LCD */
  STM322xG_LCD_Init();
#endif

  /* Initialize STM322xG-EVAL's LEDs */
  STM_EVAL_LEDInit(LED1);
  STM_EVAL_LEDInit(LED2);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  /* leds off */
  STM_EVAL_LEDOff(LED1);
  STM_EVAL_LEDOff(LED2);
  STM_EVAL_LEDOff(LED3);
  STM_EVAL_LEDOff(LED4);
  STM_EVAL_LEDOff(LED5);
#ifdef USE_LCD
  /* Clear the LCD */
  LCD_Clear(Black);

  /* Set the LCD Back Color */
  LCD_SetBackColor(Black);

  /* Set the LCD Text Color */
  LCD_SetTextColor(White);
#endif

#ifdef USE_KEY
  STM_EVAL_PBInit(BUTTON_TAMPER, BUTTON_MODE_GPIO);
  STM_EVAL_PBInit(BUTTON_WAKEUP, BUTTON_MODE_GPIO);
#endif

  
}
/**
  * @brief  init the shakesensor .
  * @param  None.
  * @retval None
  */
void ShakeSensorInit(void)
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
//  NVIC_InitTypeDef   Shake_NVIC_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource5);

  EXTI_InitStructure.EXTI_Line = EXTI_Line5;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  Shake_NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  Shake_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  Shake_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  Shake_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&Shake_NVIC_InitStructure);
}
