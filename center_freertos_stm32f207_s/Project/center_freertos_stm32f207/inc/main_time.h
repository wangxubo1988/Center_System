#ifndef _MAIN_TIME_H
#define _MAIN_TIME_H

#include <stdint.h>

typedef struct time{
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t year;
  uint8_t month;
  uint8_t day;
	uint8_t week;
}TIME;

typedef struct timer{
  uint8_t enable;
  uint8_t week;
  uint8_t hour;
  uint8_t min;
  uint8_t id;
  uint8_t cmd[2];
	uint8_t cmd_dev[2];
}TIMER;

#define TIMER_CAP 20
extern TIME my_time;
extern TIMER task_timer[TIMER_CAP];

int Time_Init(void);
void UpdateTime_Init(void);
void Set_RTCAlarm_Time(void);
void Get_RTCtime(void);
void Set_RTCTime(uint8_t hour, uint8_t min, uint8_t sec,
				 uint8_t year, uint8_t month, uint8_t day, uint8_t week);
char My_GetWeek(uint8_t hour, uint8_t min, uint8_t sec, 
								 uint8_t year, uint8_t month, uint8_t day);
void UpdateTime_Send(void);


#endif
