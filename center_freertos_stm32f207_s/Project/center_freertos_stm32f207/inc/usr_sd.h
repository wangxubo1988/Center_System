#ifndef _USR_SD_H
#define _USR_SD_H

#include "main.h"

void Usr_SD_Init(void);
void Write_Warn_Date(void  *data);
void Write_SystermState(int state_n);
void OutPrint_SysState(void);
void Read_Warn_Cnt(void);
void Write_Soft_Date(void  *data, int offset, int cnt);
void SoftMd5_Store(INT8U *data);
int Read_UpdateBin_toCheck(INT8U *md5);




#endif
