
#include "main.h"

FATFS fs;            // Work area (file system object) for logical drive
FIL fsrc, fdst;      // file objects
BYTE buffer[51];     // file copy buffer
FRESULT res;         // FatFs function common result code
UINT br, bw;         // File R/W count

FIL md5_file1;
bool sd_update = TRUE;

bool Read_MD5(void)
{
  
  if(f_open(&md5_file1, "md5.dat", FA_OPEN_EXISTING | FA_READ) != FR_OK)
  {
     
	 sd_update = FALSE;
	 return FALSE;
  }
  
  f_read(&md5_file1, md5_read, 16, &br);

  f_close(&md5_file1);
  
  STM_EVAL_LEDOn(LED2);
  
  return TRUE;
}

void Read_UpdateBin_toUpdate(void)
{
  int i = 0;
  FIL bin_file;
  int len_left = 512;
	
  if(f_open(&bin_file, "update.bin", FA_OPEN_EXISTING | FA_READ) != FR_OK)
  {	
    sd_update = FALSE;
	  return;
  }
  
  temp_size = bin_file.fsize;

  Do_Update_Erase();

  for(i = 0; i < bin_file.fsize; i += len_left)
  {	
    f_lseek(&bin_file, i);
	  if( i + len_left > bin_file.fsize) len_left = bin_file.fsize - i;
    f_read(&bin_file, temp, len_left, &br);//成功读出512个字节
	  Do_Update_WriteAndCheck(i, len_left); 		 
  }
  f_close(&bin_file);

}

md5_context ctx1;
void Read_UpdateBin_toCheck(void)
{
  int i = 0;
  FIL bin_file;
  int len_left = 512;
  if(f_open(&bin_file, "update.bin", FA_OPEN_EXISTING | FA_READ) != FR_OK)
  {
  	
    sd_update = FALSE;
	  return;
  }
  
  temp_size = bin_file.fsize;
	
  md5_starts( &ctx1 );

  for(i = 0; i < bin_file.fsize; i += len_left)
  {	
    f_lseek(&bin_file, i);
	  if( i + len_left > bin_file.fsize) len_left = bin_file.fsize - i;
    f_read(&bin_file, temp, len_left, &br);//成功读出512个字节
	  md5_update( &ctx1, temp, len_left);  		 
  }
  f_close(&bin_file);
  
  md5_finish( &ctx1, md5_check );
  memset( &ctx1, 0, sizeof( md5_context ) );

///////////////////////////////////////////////////////////////////////////////////////////////
  STM_EVAL_LEDOn(LED1);
}

void Sd_Func(void)
{
	int t = 0;
	if(f_mount(0, &fs) != FR_OK)
  {		
    iap_load_app(FLASH_USER_START_ADDR);
  }
	
	if(!Read_MD5())
  {	
    iap_load_app(FLASH_USER_START_ADDR);
  }
	
  Read_UpdateBin_toCheck();  

  for(t = 0; t < 16; t++)
  {
    if(md5_check[t] != md5_read[t]) 
	  {
	    sd_update = FALSE;
	    STM_EVAL_LEDOn(LED3);
    }
  }

  if(sd_update)
  {
    Read_UpdateBin_toUpdate();
  }
  
	if(check_err > 0)
  {
  	STM_EVAL_LEDOn(LED4);
  }
	
	f_unlink("md5.dat");
	f_unlink("update.bin");

  f_mount(0, NULL);

  iap_load_app(FLASH_USER_START_ADDR);
}
