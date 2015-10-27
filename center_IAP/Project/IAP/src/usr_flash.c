#include "main.h"


md5_context ctx;

void Flash_SoftWrite(void  *data, 
										int offset, 
										int cnt)
{  
   FlashBufferWrite((u8*)data, zone_soft + offset, cnt);
}

void Read_SoftLen(void)
{ 
  FlashBufferRead((u8*)&soft_len, zone_softlen, 3);
}
void Read_Update_toCheck(void)
{
  int i = 0;
  int len_left = 512;


  md5_starts( &ctx );

  for(i = 0; i < soft_len; i += len_left)
  {	
	  if( i + len_left > soft_len) len_left = soft_len - i;
		FlashBufferRead((u8*)temp, zone_soft+i, len_left);
	  md5_update( &ctx, temp, len_left);  		 
  }
  
  md5_finish( &ctx, md5_check );
  memset( &ctx, 0, sizeof( md5_context ) );

///////////////////////////////////////////////////////////////////////////////////////////////
  STM_EVAL_LEDOn(LED1);
}
void Read_Update_toUpdate(void)
{
  int i = 0;
  int len_left = 512;
	
  Do_Update_Erase();

  for(i = 0; i < soft_len; i += len_left)
  {	
   
	  if( i + len_left > soft_len) len_left = soft_len - i;
    FlashBufferRead((u8*)temp, zone_soft+i, len_left);//成功读出512个字节
	  Do_Update_WriteAndCheck(i, len_left); 		 
  }
}

void Soft_Clean(void){
	u8 clean[512] ={0x0,0x0,0x0};
  FlashSectorErase(zone_softlen, 1);
	FlashBufferWrite((u8*)&clean[0], zone_softlen, 3);
}


void Flash_Func(void)
{
  int t = 0;
	
  Read_SoftLen();
	
  if(soft_len == 0 || soft_len >0x19000 || soft_len<0x14000){
	Soft_Clean();
	return;
  }
	
  
  STM_EVAL_LEDOn(LED1);
	STM_EVAL_LEDOn(LED2);
	STM_EVAL_LEDOn(LED3);
	
  Read_Update_toUpdate();
  
  if(check_err > 0)
  {
  	STM_EVAL_LEDOn(LED4);
		while(1){
		}
  }
  
  Soft_Clean();
	
  iap_load_app(FLASH_USER_START_ADDR);
}
