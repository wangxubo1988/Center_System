/**
  ******************************************************************************
  * @file    usr_sd.c
  * @author  www
  * @version V1.0.0
  * @date    10/20/2012
  * @brief   sd card read and write
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "usr_sd.h"
#include "stm32f2xx.h"
#include "stm322xg_eval.h"
#include "stm32_eval_sdio_sd.h"
#include "ff.h"
#include "diskio.h"
#include "usr_cpu.h"
#include "md5.h"
#include "net_server.h"
//#include <usart.h>
#include "const.h"
#include <string.h>
#include "usr_api.h"
#include "main_time.h"
#include "usr_net.h"
#include "lwip/inet.h"
#include "flash.h"
/* Private define ------------------------------------------------------------*/
#define NULL 0

/* Private variables ---------------------------------------------------------*/

int block_size = 0;

/* Private function prototypes -----------------------------------------------*/



/* ------------------file------------------------ */
FATFS fs;            // Work area (file system object) for logical drive
FIL warn_fsrc, warn_fdst, sys_fdst, pic_fsrc, soft_fsrc;      // file objects
FRESULT res;         // FatFs function common result code
UINT br, bw;         // File R/W count

/* Private functions  -----------------------------------------------*/
char file_Name[20]; 

/**
  * @brief  write date into a the sdcard file. 
  * @param  
           *data: pointer to the date to be writen.
		    cnt : length of the data.
			*filename: the name of saving file. 
  * @retval None
  */
void Write_Warn_Date(void  *data)
{ 
  int warn_count = 0;
	
	block_size = 8;

	if(f_mount(0, &fs) != FR_OK) return;
  
	if(f_open(&warn_fdst, "warn.dat", /*FA_CREATE_ALWAYS |*/ FA_WRITE) != FR_OK){
	  f_open(&warn_fdst, "warn.dat", FA_CREATE_NEW | FA_WRITE);
		 f_chmod("warn.dat", AM_MASK, AM_HID);
	}else{
	  warn_count = (warn_fdst.fsize)/block_size - 1;
  }
	
	if(warn_count >= ALARM_MESSAGE_CAP){
    f_lseek(&warn_fdst, block_size*(warn_ops + 1));
		warn_ops++;
		if(ALARM_MESSAGE_CAP == warn_ops) warn_ops = 0;
  }else{		
	  f_lseek(&warn_fdst, block_size*(warn_count + 1));
		warn_ops = 0;
	}   
	f_write(&warn_fdst, (void*)(data), block_size, &bw);
	
	f_lseek(&warn_fdst, 0);
	f_write(&warn_fdst, (void*)(&warn_ops), 1, &bw);
	
	f_close(&warn_fdst);
  f_mount(0, NULL); 
}

/**
  * @brief  write date into a the sdcard file. 
  * @param  
           *data: pointer to the date to be writen.
		    cnt : length of the data.
			*filename: the name of saving file. 
  * @retval None
  */

void Wrtie_hardFault_state(char * data)
{
	if(f_mount(0, &fs) != FR_OK) return;
	 if(f_open(&sys_fdst, "systerm.log", /*FA_CREATE_ALWAYS |*/ FA_WRITE | FA_READ) != FR_OK)
  {
    f_open(&sys_fdst, "systerm.log", FA_CREATE_NEW | FA_WRITE | FA_READ);
  }
  else
  {
	  f_lseek(&sys_fdst, sys_fdst.fsize);
  }
	
	 f_write(&sys_fdst, (void *)data, 511, &bw);
	
	f_close(&sys_fdst);
  f_mount(0, NULL);
  
}
void Write_SystermState(int state_n)
{ 
#ifdef My_DEBUG 
	unsigned char state[13];
  unsigned char state_detail = 0;
	
  unsigned char time[16] = {0x0, 0x0, 0x2d, 0x0, 0x0, 0x2d, 0x0, 0x0, 
  							0x3a, 0x0, 0x0, 0x3a, 0x0, 0x0, 0xd, 0xa};
  memset(state, 0,13);
  if(f_mount(0, &fs) != FR_OK) return;
  
  Get_RTCtime();
  time[0] = (my_time.month/10) + 0x30;
  time[1] = (my_time.month%10) + 0x30;
  time[3] = (my_time.day/10) + 0x30;
  time[4] = (my_time.day%10) + 0x30;
  time[6] = (my_time.hour/10) + 0x30;
  time[7] = (my_time.hour%10) + 0x30;
  time[9] = (my_time.min/10) + 0x30;
  time[10] = (my_time.min%10) + 0x30;
  time[12] = (my_time.sec/10) + 0x30;
  time[13] = (my_time.sec%10) + 0x30;

  if(f_open(&sys_fdst, "systerm.log", /*FA_CREATE_ALWAYS |*/ FA_WRITE | FA_READ) != FR_OK)
  {
    f_open(&sys_fdst, "systerm.log", FA_CREATE_NEW | FA_WRITE | FA_READ);
  }
  else
  {
	  f_lseek(&sys_fdst, sys_fdst.fsize);
  }
  
  f_write(&sys_fdst, (void *)time, sizeof(time), &bw);
  f_lseek(&sys_fdst, sys_fdst.fsize);
	
	if(state_n == SYS_RST){
    if(SET == RCC_GetFlagStatus(RCC_FLAG_PORRST)) state_detail += 1;
		if(SET == RCC_GetFlagStatus(RCC_FLAG_SFTRST)) state_detail += 2;
		if(SET == RCC_GetFlagStatus(RCC_FLAG_IWDGRST)) state_detail += 4;
  }
	sprintf(state,"state:%2d-%2d\r\n", state_n, state_detail);
	f_write(&sys_fdst, (void *)state, 13, &bw);
	
  f_close(&sys_fdst);
  f_mount(0, NULL);
#endif  
}

extern int memp_cnt;
extern int mem_cnt;

void OutPrint_SysState(void)
{
	struct ip_addr address;
  char text[30];
	unsigned char time[16] = {0x0, 0x0, 0x2d, 0x0, 0x0, 0x2d, 0x0, 0x0, 
  							0x3a, 0x0, 0x0, 0x3a, 0x0, 0x0, 0xd, 0xa};
	
	sprintf(text, "Center Version:%c%c%c%c\r\n", soft_version[1], soft_version[2],soft_version[3],soft_version[4]);
	Usart3_Send(text, 21);
								
	sprintf(text, "IAP Version:%c%c%c%c\r\n", iap_version[0], iap_version[1],iap_version[2],iap_version[3]);
	Usart3_Send(text, 18);
								
	address.addr = IPaddress;
	sprintf(text, "Center IP:%s\r\n",inet_ntoa(address));
	Usart3_Send(text, strlen(inet_ntoa(address)) + 12);
	
	sprintf(text, "pad count:%d\r\n", pad_devices_cnt);
	Usart3_Send(text,  12 + ((pad_devices_cnt>10)?2:1));
	
  sprintf(text, "mem count:%d\r\n", mem_cnt);	
	Usart3_Send(text,  12 + ((mem_cnt>10)?2:1));
	
	sprintf(text, "mem count:%d\r\n", memp_cnt);	
	Usart3_Send(text,  12 + ((memp_cnt>10)?2:1));
	
  if(center_NoID) printf("no serial num or no secret num or use first!\n");
  if(f_mount(0, &fs) != FR_OK) printf("no sd card or sd card err!\n");
	if(net_logined) printf("remote server logined\n");
		
	Get_RTCtime();
  time[0] = (my_time.month/10) + 0x30;
  time[1] = (my_time.month%10) + 0x30;
  time[3] = (my_time.day/10) + 0x30;
  time[4] = (my_time.day%10) + 0x30;
  time[6] = (my_time.hour/10) + 0x30;
  time[7] = (my_time.hour%10) + 0x30;
  time[9] = (my_time.min/10) + 0x30;
  time[10] = (my_time.min%10) + 0x30;
  time[12] = (my_time.sec/10) + 0x30;
  time[13] = (my_time.sec%10) + 0x30;
	
	Usart3_Send(time,  sizeof(time));
	
}


/**
  * @brief  read date from a the sdcard file. 
  * @param  
           *data: pointer to buff for the reading data saving.
		    cnt : length of the data.
			*filename: the name of saving file. 
  * @retval None
  */
void Read_Warn_Cnt(void)
{ 
	if(f_open(&warn_fsrc, "warn.dat", FA_OPEN_EXISTING | FA_READ) != FR_OK)
	{
    return;		
	}
	f_read(&warn_fsrc, (void*)(&warn_ops), 1, &br);  
	f_close(&warn_fsrc);
}
/**
  * @brief  for user to Setup the sd function and read the data before the systerm work. 
  * @param  
  * @retval None
  */

void Usr_SD_Init(void)
{   
	NVIC_InitTypeDef NVIC_InitStructure;
  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
    
  f_mount(0, &fs);
 
	Read_Warn_Cnt();	//读取报警消息条数
  
  f_mount(0, NULL);  
}


/***********************software update****************************************/
void Write_Soft_Date(void  *data, 
      				int offset, 
					int cnt				
	    			)
{  
  f_lseek(&soft_fsrc, offset);
 
  f_write(&soft_fsrc, data, cnt, &bw);
}


md5_context ctx_rc;	
INT8U md5_check[16];
INT8U temp[512];

extern int soft_len;

int Read_UpdateBin_toCheck(INT8U *md5)
{ 
  int i = 0;
  int len_left = 512;

  {
    md5_starts( &ctx_rc );

    for(i = 0; i < soft_len; i += len_left)
    {	
	    if( i + len_left > soft_len) len_left = soft_len - i;
			FlashBufferRead((u8*)temp, zone_soft +i, len_left);//成功读出512个字节
	    md5_update( &ctx_rc, temp, len_left);	
    }				    
  
    md5_finish( &ctx_rc, md5_check );
    memset( &ctx_rc, 0, sizeof( md5_context ) );
  }
	
  for(i = 0; i < 16; i++)
  {
  	if(md5_check[i] != *(md5 + i))
	  {
	    return 0;
	  }
  }
	
  return 1;
}

void CopyBin_toSd(void){
	int i = 0;
	int len_left = 512;
	
	Feed_IWDG();
	
  if(f_mount(0, &fs) != FR_OK) return;
	f_open(&soft_fsrc, "update.bin", FA_CREATE_ALWAYS | FA_WRITE);
  
	for(i = 0; i < soft_len; i += len_left)
  {	
	    if( i + len_left > soft_len) len_left = soft_len - i;
			FlashBufferRead((u8*)temp, zone_soft +i, len_left);//成功读出512个字节
			Write_Soft_Date(temp,  i, len_left);
  }				    
	
  f_close(&soft_fsrc);
	
	f_open(&soft_fsrc, "md5.dat", FA_CREATE_ALWAYS | FA_WRITE);

	f_write(&soft_fsrc, soft_md5, 17, &bw);
  f_close(&soft_fsrc);
	f_mount(0, NULL);
}

/******************* (C) COPYRIGHT 2012 *****END OF FILE****/
