
/* Includes ------------------------------------------------------------------*/
#include "main_time.h"
#include "stm32f2xx.h"
#include "main.h"
#include "stm32f2xx_pwr.h"
#include "usr_api.h"

#include "usr_cpu.h"
#include "net_server.h"
#include "udp.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Private typedef -----------------------------------------------------------*/

TIME my_time;
/* Private define ------------------------------------------------------------*/

/* SNTP protocol defines */
#define SNTP_MAX_DATA_LEN           48
#define SNTP_RCV_TIME_OFS           32
#define SNTP_LI_NO_WARNING          0x00
#define SNTP_VERSION               (4/* NTP Version 4*/<<3)
#define SNTP_MODE_CLIENT            0x03
#define SNTP_MODE_SERVER            0x04
#define SNTP_MODE_BROADCAST         0x05
#define SNTP_MODE_MASK              0x07

u8_t  sntp_request [SNTP_MAX_DATA_LEN];

typedef struct sntp_data{
  INT8U LI_VN_Mode; INT8U Stratum; INT8U Poll; INT8U Precision;
  INT8U reserve[28];
  INT8U time[8];
  INT8U reserve1[12]; 
}SNTP_DATA;

/* Uncomment the corresponding line to select the RTC Clock source */
#define RTC_CLOCK_SOURCE_LSE   /* LSE used as RTC source clock */
 /*#define RTC_CLOCK_SOURCE_LSI */ /* LSI used as RTC source clock. The RTC Clock
                                      may varies due to LSI frequency dispersion. */ 

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

RTC_TimeTypeDef RTC_TimeStructure;
RTC_DateTypeDef RTC_DateStructure;

RTC_InitTypeDef RTC_InitStructure;
RTC_AlarmTypeDef  RTC_AlarmStructure;

__IO uint32_t AsynchPrediv = 0, SynchPrediv = 0;

struct udp_pcb *UdpPcb; 					   /*同步时间*/
struct ip_addr ipaddr; 
 
struct pbuf *p_time = NULL;
struct tm *now_time;
time_t timer;
time_t timer1;
SNTP_DATA *time_recv = NULL;

/* Private function prototypes -----------------------------------------------*/
static void RTC_Config(void);
static void RTC_TimeRegulate(void);

void Get_RTCtime(void);
void Set_RTCTime(uint8_t hour, uint8_t min, uint8_t sec,
				 uint8_t year, uint8_t month, uint8_t day, uint8_t week);

/* Private functions ---------------------------------------------------------*/

char My_GetWeek(uint8_t hour, uint8_t min, uint8_t sec, 
								 uint8_t year, uint8_t month, uint8_t day)
{
	struct tm set_time;
	struct tm* now_time;
	time_t  timer;
	
	set_time.tm_year = year + 0x64;
	set_time.tm_mon = month - 0x01;
	set_time.tm_mday = day;
	set_time.tm_hour = hour;
	set_time.tm_min = min;
	set_time.tm_sec = sec;
	timer = mktime(&set_time);
  
	now_time = localtime(&timer);
  return now_time->tm_wday;;
}
void time_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{  
  
  INT8U *cmd = NULL;
  if (p->len == 0x30)
  {
			time_recv = p->payload;
  
			timer = ((time_t)time_recv->time[0]) << 24 | ((time_t)time_recv->time[1] << 16) |
							 ((time_t)time_recv->time[2]) << 8 | (time_t)time_recv->time[3];
			timer1 =  0x83aa7e80; //1900年到1970年经过的秒数	  
			if(timer > timer1)
			{  
				timer = timer - timer1 + 0x7080;//时区换算加上的秒数

				now_time = localtime(&timer);
  
				my_time.year = now_time->tm_year - 0x64;
				my_time.month = now_time->tm_mon + 0x01;
				my_time.day = now_time->tm_mday;
				my_time.hour = now_time->tm_hour;
				my_time.min = now_time->tm_min;
				my_time.sec = now_time->tm_sec;
				my_time.week = now_time->tm_wday;
				Set_RTCTime(my_time.hour, my_time.min, my_time.sec,
				my_time.year, my_time.month, my_time.day, my_time.week);
	  
				cmd = cmd_reply;
 
				*(cmd+3) = 0x17;
				*(cmd+4) = 0x04;
				*(cmd+10) =0x0;

				Pcb_Write(cmd);
				memset(cmd_reply+3, 0 ,21);
			}
  }
  

  pbuf_free(p);
}

void UpdateTime_Send(void)
{ 
	p_time = pbuf_alloc(PBUF_RAW,sizeof(sntp_request),PBUF_RAM); 
  p_time->payload = sntp_request;
	
  udp_connect(UdpPcb,&ipaddr,123);//123
 
  udp_send(UdpPcb, p_time);
  udp_disconnect(UdpPcb);	
  		
  pbuf_free(p_time);
}

void UpdateTime_Init(void)
{ 
  sntp_request[0] = SNTP_LI_NO_WARNING | SNTP_VERSION | SNTP_MODE_CLIENT;
  
  IP4_ADDR(&ipaddr, 24, 56, 178, 140);
  
  UdpPcb = udp_new(); 
  
  udp_bind(UdpPcb,IP_ADDR_ANY,1025);                           /*  绑定本地IP 地址        */ 
  
  
  /* Set a receive callback for the upcb */
  udp_recv(UdpPcb, time_server_callback, NULL);
}  

 
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void RTC_AlarmShow(void);

int Time_Init(void)
{
   EXTI_InitTypeDef  EXTI_InitStructure;
   NVIC_InitTypeDef  NVIC_InitStructure;
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f2xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f2xx.c file
     */     
  if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F2)
  {  
    /* RTC configuration  */
    RTC_Config();

    /* Configure the RTC data register and RTC prescaler */
    RTC_InitStructure.RTC_AsynchPrediv = AsynchPrediv;
    RTC_InitStructure.RTC_SynchPrediv = SynchPrediv;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
   
    /* Check on RTC init */
    if (RTC_Init(&RTC_InitStructure) == ERROR)
    {
      /**/
    }

    /* Configure the time register */
    RTC_TimeRegulate(); 
  }
  else
  {
    /* Check if the Power On Reset flag is set */
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {
      //printf("\r\n Power On Reset occurred....\n\r");
    }
    /* Check if the Pin Reset flag is set */
    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {
      //printf("\r\n External Reset occurred....\n\r");
    }

    
    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_BackupAccessCmd(ENABLE);

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    /* Clear the RTC Alarm Flag */
    RTC_ClearFlag(RTC_FLAG_ALRAF);


  }

  /* RTC Alarm A Interrupt Configuration */
  /* EXTI configuration *********************************************************/
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  /* Enable the RTC Alarm Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  RTC_AlarmShow();
  return 0; 

}

/**
  * @brief  Configure the RTC peripheral by selecting the clock source.
  * @param  None
  * @retval None
  */
static void RTC_Config(void)
{
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to RTC */
  PWR_BackupAccessCmd(ENABLE);
    
#if defined (RTC_CLOCK_SOURCE_LSI)  /* LSI used as RTC source clock*/
/* The RTC Clock may varies due to LSI frequency dispersion. */   
  /* Enable the LSI OSC */ 
  RCC_LSICmd(ENABLE);

  /* Wait till LSI is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
  
  SynchPrediv = 0xFF;
  AsynchPrediv = 0x7F;

#elif defined (RTC_CLOCK_SOURCE_LSE) /* LSE used as RTC source clock */
  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);

  /* Wait till LSE is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  
  SynchPrediv = 0xFF;
  AsynchPrediv = 0x7F;

#else
  #error Please select the RTC Clock source inside the main.c file
#endif /* RTC_CLOCK_SOURCE_LSI */
  
  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();
}

/**
  * @brief  Returns the time entered by user, using Hyperterminal.
  * @param  None
  * @retval None
  */
static void RTC_TimeRegulate(void)
{
  uint32_t tmp_hh = 23, tmp_mm = 59, tmp_ss = 30;

//  printf("\n\r==============Time Settings=====================================\n\r");
  RTC_TimeStructure.RTC_H12     = RTC_H12_AM;

  RTC_TimeStructure.RTC_Hours = tmp_hh;


  RTC_TimeStructure.RTC_Minutes = tmp_mm;
  
 
  RTC_TimeStructure.RTC_Seconds = tmp_ss;
 
  /* Configure the RTC time register */
  if(RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure) == ERROR)
  {
    //printf("\n\r>> !! RTC Set Time failed. !! <<\n\r");
  } 
  else
  {
    //printf("\n\r>> !! RTC Set Time success. !! <<\n\r");
    
    /* Indicator for the RTC configuration */
    RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
  }

   //////////////////////////////////////////////////////////////////////////////
   
  Set_RTCAlarm_Time();
}


/**
  * @}
  */ 
void Set_RTCTime(uint8_t hour, uint8_t min, uint8_t sec,
				 uint8_t year, uint8_t month, uint8_t day, uint8_t week)
{
  RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
  RTC_TimeStructure.RTC_Hours = hour;
  RTC_TimeStructure.RTC_Minutes = min;
  RTC_TimeStructure.RTC_Seconds = sec;

  RTC_DateStructure.RTC_Date = day;
  RTC_DateStructure.RTC_Month = month;
  RTC_DateStructure.RTC_Year = year;
	RTC_DateStructure.RTC_WeekDay = week;
	if(week == 0) RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Sunday;
  
  RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
  
  RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
 
  /* Indicator for the RTC configuration */
  RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);


}

void Get_RTCtime(void)
{ 
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
  RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
  
  my_time.hour = RTC_TimeStructure.RTC_Hours;
  my_time.min = RTC_TimeStructure.RTC_Minutes;
  my_time.sec = RTC_TimeStructure.RTC_Seconds;
  
  my_time.year = RTC_DateStructure.RTC_Year;
  my_time.month = RTC_DateStructure.RTC_Month;
  my_time.day = RTC_DateStructure.RTC_Date;
	my_time.week = RTC_DateStructure.RTC_WeekDay;
}
/**
  * @}
  */
void Set_RTCAlarm_Time(void)
{
  INT8U min = 0;
  INT8U sec = 0;
  int total_sec = 0;

  total_sec = Get_RNG_Sec();
  min = total_sec/60;
  sec = total_sec%60;
   /* Disable the Alarm A */
  RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

//  RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_PM;
 
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = 0;

  RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = min;
   
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = sec;

  /* Set the Alarm A */
  RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
  RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
  RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;

  /* Configure the RTC Alarm A register */
  RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);

  /* Enable the RTC Alarm A Interrupt */
  RTC_ITConfig(RTC_IT_ALRA, ENABLE);
   
  /* Enable the alarm  A */
  RTC_AlarmCmd(RTC_Alarm_A, ENABLE);


}
INT8U alarm_hl;
INT8U alarm_hm;
INT8U alarm_hs;
void RTC_AlarmShow(void)
{
  /* Get the current Alarm */
  RTC_GetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
  alarm_hl = RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours;
  alarm_hm = RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes;
  alarm_hs = RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds;
}


/*timer task set*/
TIMER task_timer[TIMER_CAP];

void TaskTimer_Check(void){
	char week = 0xff;
	char hour = 0xff;
	char min = 0xff;
	int list = 0;
	
  Get_RTCtime();
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
  RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	
	week = RTC_DateStructure.RTC_WeekDay - 1;
  hour = RTC_TimeStructure.RTC_Hours;
	min  = RTC_TimeStructure.RTC_Minutes;
	for(list=0; list<TIMER_CAP;list++){
    if(task_timer[list].enable == 0 || task_timer[list].id == 0) continue;
		if(((task_timer[list].week & (0x01<<week)) != 0x0 || task_timer[list].week == 0) &&
			 task_timer[list].hour == hour && task_timer[list].min == min){
			unsigned char cmd_stu[24]={0xff,0xfe,0x15};
			 cmd_stu[4] = task_timer[list].cmd[0];
			 cmd_stu[6] = dev_info[task_timer[list].id].room_id;
			if(cmd_stu[4] == 0x2c) cmd_stu[6] = task_timer[list].id;
			 else{
				 cmd_stu[7] = task_timer[list].id;
			   cmd_stu[10] = task_timer[list].cmd_dev[0];
			   cmd_stu[11] = task_timer[list].cmd_dev[1];
			 }
			 Save_Cmd(&cmd_stu[0]);
			 if(task_timer[list].week == 0) {
        task_timer[list].enable = 0;
	       
      }
		}
		
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
