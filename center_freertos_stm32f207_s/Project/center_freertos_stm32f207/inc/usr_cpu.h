#ifndef _USR_CPU_H
#define _USR_CPU_H

#include "main.h"
#include "lwip/ip_addr.h"

#define DEVICES_CAP 256
#define ROOMS_CAP  31 
#define SCENE_CAP 1500 
#define ALARM_DEV_CAP 256

#define ALARM_MESSAGE_CAP 60

typedef enum {SUCESS = 0, FAIL = 1} CommandStatus;

typedef struct my_systerm
{
 INT8U sign[6];
 INT8U serial_num[19];
 INT8U secret_num[19];
 INT8U password[13];
 INT8U time_mode;
 INT8U alarm_dev_enable[DEVICES_CAP];
}SYSTERM;

/* definitions   of   zigbee devices */
typedef struct devices
{
  INT8U dev_type;
  INT8U dev_id;
  INT8U room_id;
  INT8U dev_mac[8];
  INT8U dev_addr[2];
  INT8U dev_status[3];
}DEV;
/* definitions   of   rooms */
typedef struct rooms
{
  INT8U room_id;
  INT8U room_name[12];
  INT8U dev_cnt;
  INT8U reserve[2];
}ROOM;

typedef struct room_dev_id
{
  INT8U dev_id[20];
}ROOM_DEV_ID;

/* definitions   of   Scene mode */
typedef struct scene
{
  INT8U scene_id;
  INT8U dev_id;
  INT8U act_cmd[2];
  INT8U dev_cmd[2];
}SCENE;

/* definitions   of   pad devices */
typedef struct pad_dev
{
  INT8U dev_id;
  struct tcp_pcb *socket;
  bool  user ;
  INT8U pad_living;
}PAD_DEV;


typedef struct alarm_dev
{
  INT8U alarm_dev_id;
  INT8U alarm_message[2];
  INT8U dev_act_id;
  INT8U dev_active[2];
  INT8U dev_action[2];
  INT8U phone_message;
}AlARM_DEV;

typedef struct alarm_record
{
  INT8U alarm_id;
  INT8U alarm_time[7];
}ALARM_RECORD;
/***********************************/

extern int CONFIG_MODE;

extern INT8U  *Cmd;
extern struct tcp_pcb *tcp_device;

extern char sys_info_change;
extern char device_change;
extern char scene_change;
extern char warn_dev_change;
extern char dev_timer_change;

extern int sd_error;

extern SYSTERM  sys_info[1];

extern DEV dev_info[DEVICES_CAP];
extern int devices_cnt;

extern ROOM room_info[ROOMS_CAP];
extern int rooms_cnt;

extern AlARM_DEV alarm_dev_info[ALARM_DEV_CAP];

extern SCENE scene_info[SCENE_CAP];

extern INT8U dev_living[DEVICES_CAP];

extern int warn_ops;

extern PAD_DEV pad_info[11];

extern int pad_devices_cnt;


/*                Functions                    */

void Zigbee_Cmd_Analysis(INT8U  *Cmd);
void Tcp_Cmd_Analysis(INT8U  *Cmd_trans);
void Cmd_Analysis_Init(void);
void Pcb_Write(const void *data);
void Room_Dev_Add(INT8U room_id, INT8U dev_id);

void Pcb_Update(struct tcp_pcb *pcb ,bool state);


#endif
