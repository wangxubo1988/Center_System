/**
  ******************************************************************************
  * @file    usr_cpu.c
  * @author  www
  * @version V1.0.0
  * @date    10/20/2012
  * @brief   system main Logic
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_usart.h"
#include "stm322xg_eval.h"
#include "main.h"
#include "stm32_eval.h"
#include "stm32f2xx.h"
#include "tcp.h"
#include "usr_cpu.h"
#include "mem.h"
#include "stdio.h"
#include "const.h"
#include "ff.h"
#include "diskio.h"
#include "net_server.h"
#include "usr_api.h"
#include "usr_net.h"
#include "main_time.h"
#include "usr_sd.h"
#include "usart_dma.h"
#include "flash.h"

 #include <string.h>
/* Private variables ---------------------------------------------------------*/

int CONFIG_MODE = 0;

char sys_info_change = 0;
char device_change  = 0;
char scene_change = 0;
char warn_dev_change = 0;
char dev_timer_change = 0;

int sd_error = 0;

struct tcp_pcb *tcp_device = NULL;

/*系统信息*/
SYSTERM  sys_info[1];
/*设备信息*/
DEV dev_info[DEVICES_CAP];
int devices_cnt = 0;
INT8U dev_living[DEVICES_CAP];

/*房间信息*/
ROOM room_info[ROOMS_CAP];
int rooms_cnt = 0;
ROOM_DEV_ID room_dev_info[43];

/*情景信息*/
SCENE scene_info[SCENE_CAP];

/*报警联动信息*/
AlARM_DEV alarm_dev_info[ALARM_DEV_CAP];

/*报警记录*/
ALARM_RECORD alarm_record_info[1];
int warn_ops = 0;

/*平板连接信息*/
PAD_DEV pad_info[11];
int pad_devices_cnt;
int dev_offset;


static INT8U zigbee_status[10];
static INT8U zigbee_macaddr[8];
static bool get_macaddr_once = TRUE;

/* Private function prototypes -----------------------------------------------*/
static void Room_Add(INT8U  *Cmd);
static void Room_Delete(INT8U *Cmd);
static void Device_Add(INT8U  *Cmd);
static void Device_Delete(INT8U  *Cmd);
static void Room_Dev_Delete(INT8U room_id, INT8U dev_id) ;
void Room_Dev_Add(INT8U room_id, INT8U dev_id);
static INT8U Room_Dev_Search(INT8U room_id, INT8U ops);
static void Pcb_Write_ALL( const void *data);

static void Task_Usart_Send(INT8U  *cmd_msg);
static void My_Control_DevLamp(INT8U dev_id, INT8U status);
static void Get_Dev_Addr(INT8U dev_id);
static void CmdFromUart_ChangeToNet(void);
static CommandStatus Dev_CheckRoom(int devId, int roomId);

void Systerm_ERROR(int type);

/* Private functions ---------------------------------------------------------*/

/*******************************systerm command******************************************************/
/**
  * @brief  to analysis the 24byte data from the usart. 
  * @param  
           *Cmd: the pointer of the 24byte date. 
  * @retval None
  */
INT8U  *Cmd = NULL;
INT8U room_status = 0;

INT8U  *Cmd_usart = NULL;

extern int	usart_reset_key;


const INT8U Cmds_UsartTask[10]={0x21,0x30,0x32,0x39,0x3b,0x3d,0x40,0x43, 0x49, 0x5b};


void Zigbee_Cmd_Analysis(INT8U  *Cmd_usart_trans)
{
   int t = 0;
  Cmd_usart = Cmd_usart_trans;
	
/* check  Zigbee device */
  if(*(Cmd_usart+4) == 0xf1 && usart_reset_key < 5)
  {
    usart_check = 1;
	  usart_reset_key = 0;
    if(get_macaddr_once)
	  {
	   Task_Usart_Send(cmd_to_usart[4]);
	  }
	return;
  }

/*get the net status*/
  if(*(Cmd_usart+4) == 0xf2)
  {	
		
			for(t = 0; t < 10; t++)
	    {
	      zigbee_status[t] = *(Cmd_usart+10+t);
	    }

		return;
  }
/*get devvice short addr*/
  if(*(Cmd_usart+4) == 0xf3)
  {	
    int t1 = 0;
    for(t = 1; t < 256; t++)
	{
	  if(dev_info[t].dev_id == t)
	  {
	    for(t1 = 0; t1 < 8; t1++)
		{
		  if(dev_info[t].dev_mac[t1] != *(Cmd_usart+ 12 + t1))
		  {
		    break;
		  }
		  else if(t1 < 7)
		  {
		    continue;
		  }
		  
		  dev_info[t].dev_addr[0] = *(Cmd_usart+ 10);
		  dev_info[t].dev_addr[1] = *(Cmd_usart+ 11);
	     
		  return;
		}
	  }
	} 
  
	return;
  }
	/*get the zigbee mac addr*/
  if(*(Cmd_usart+4) == 0xf4)
  {	
		if(get_macaddr_once)
		{
			memcpy(zigbee_macaddr, Cmd_usart+10, 8);
	    get_macaddr_once = FALSE;
	  }
		return;
  }
	

	
  for(t=0; t< sizeof(Cmds_UsartTask);t++)
	{ 
		if(*(Cmd_usart+4) == Cmds_UsartTask[t]){
			cmd_analysis[*(Cmd_usart+4)].func_cmd();
			memset(usart_reply+3, 0 ,21);
			return;
    }			
  }
	
	if(CONFIG_MODE == 0 && *(Cmd_usart+4) == cmd_analysis[*(Cmd_usart+4)].cmd)
	{
	  CmdFromUart_ChangeToNet();
	}
}
/*get the device state  0x21*/

void Get_Device_State(void)
{
  if(dev_info[*(Cmd_usart+3)].dev_type != 0x18)
	{
	  dev_info[*(Cmd_usart+3)].dev_status[0] = *(Cmd_usart+10);
		dev_info[*(Cmd_usart+3)].dev_status[1] = *(Cmd_usart+11);

    dev_living[*(Cmd_usart+3)] = 0x03;
 
	  *(Cmd_usart+6) = dev_info[*(Cmd_usart+3)].room_id;

    Pcb_Write_ALL(Cmd_usart);
  } 
  return;
	
}
void Return_HongWaiZhuanfa(void)
{
  INT8U id = *(Cmd_usart + 3);
  INT8U key = *(Cmd_usart + 11) ;
  
	Pcb_Write_ALL(Cmd_usart);
	
	if(*(Cmd_usart + 4) == 0x3b) return;
	
  if(key > 13 || key == 0) return;
  
	if(key <= 8){
    dev_info[id].dev_status[0] |= 0x1 << (key - 1);
  }else{
    dev_info[id].dev_status[1] |= 0x1 << (key - 9);
  }
  
	
	
  device_change = 1;
}

void Retern_YaoKongKaiGuan(void)
{ 
  INT8U id = *(Cmd_usart + 3);
  INT8U key = *(Cmd_usart + 11) ;
  INT8U cmdSuccess = *(Cmd_usart + 10) ;
	
	if(cmdSuccess == SUCESS) 
	{
    if(8 >= key){
			dev_info[id].dev_status[1] |= 0x1 << (key - 1);
		}else if(key <= 10){
			dev_info[id].dev_status[2] |= 0x1 << (key - 9);
		}
		
		device_change = 1;
		
		Pcb_Write_ALL(Cmd_usart);
	}
}

void Init_YaoKongKaiGuan(void)
{ 
  INT8U id = *(Cmd + 7);
  
  
  if(CONFIG_MODE == 0)
  {
    *(Cmd+8) = dev_info[id].dev_addr[0];
	*(Cmd+9) = dev_info[id].dev_addr[1];

	if(dev_info[id].dev_addr[0] == 0x0 && 
	   dev_info[id].dev_addr[1] == 0x0)
	{
	  Get_Dev_Addr(id);
	  return;
	}  
    Task_Usart_Send(Cmd);
	
	dev_info[id].dev_status[1] = 0;
	dev_info[id].dev_status[2] = 0;
	device_change = 1;

    return;
  }

}
/*Alarm information  0x30  0x39*/
uint32_t AlarmTimer[256];
void Alarm_Info(void)
{ 
  int t = 0;
  INT8U id = *(Cmd_usart+3);
	INT8U alarm_type = *(Cmd_usart+10);
  
  if((sys_info[0].alarm_dev_enable[id] != 0x0 && *(Cmd_usart+4) == 0x30) ||
																				(dev_info[id].dev_id != id) ||
								 ((LocalTime - AlarmTimer[id] < 60000) && alarm_type != 0x08))
		
  {
   return;
  }
	
  AlarmTimer[id] = LocalTime;
	
  Pcb_Write_ALL(Cmd_usart);
	
	
	if(*(Cmd_usart+4) == 0x30)
	{
		/*报警联动*/
	 
    for(t = 0; t<100; t++)
    {
      if(alarm_dev_info[t].alarm_dev_id == id)
	    {
	      Cmd_usart = usart_reply;
	      *(Cmd_usart+6) = dev_info[alarm_dev_info[t].dev_act_id].room_id;
	      *(Cmd_usart+7) = alarm_dev_info[t].dev_act_id;
	      *(Cmd_usart+4) = alarm_dev_info[t].dev_active[0];
	      *(Cmd_usart+10) = alarm_dev_info[t].dev_action[0];
	      *(Cmd_usart+11) = alarm_dev_info[t].dev_action[1];
				
				CmdFromUart_ChangeToNet();
	    }  
    }
		
		/*报警记录*/
		Get_RTCtime();
 
    alarm_record_info[0].alarm_id = id;

    alarm_record_info[0].alarm_time[0] = (my_time.year + 0x7d0)%0x100;
    alarm_record_info[0].alarm_time[1] = (my_time.year + 0x7d0)>>8;
    alarm_record_info[0].alarm_time[2] = my_time.month;
    alarm_record_info[0].alarm_time[3] = my_time.day;
    alarm_record_info[0].alarm_time[4] = my_time.hour;
    alarm_record_info[0].alarm_time[5] = my_time.min;
    alarm_record_info[0].alarm_time[6] = my_time.sec;

    Write_Warn_Date(&alarm_record_info[0]);
      

    
  }
} 

/*reply of dev's net_address 0x32 */
void Reply_dev_netaddress(void)
{	
    INT8U reply_save_id = 0;
	reply_save_id = *(Cmd_usart+3);
    if (dev_info[reply_save_id].dev_id == reply_save_id)
	{
	  dev_info[reply_save_id].dev_addr[0] = *(Cmd_usart+10);
	  dev_info[reply_save_id].dev_addr[1] = *(Cmd_usart+11);
	  
//	  device_change  = 0x1;
	  Cmd_usart = usart_reply;
	  *(Cmd_usart+3) = 0x32;
	  *(Cmd_usart+4) = 0x04;
	  *(Cmd_usart+6) = dev_info[reply_save_id].room_id;
	  *(Cmd_usart+7) = reply_save_id;
	  *(Cmd_usart+8) = dev_info[reply_save_id].dev_addr[0];
	  *(Cmd_usart+9) = dev_info[reply_save_id].dev_addr[1];
	  Task_Usart_Send(Cmd_usart);
	}
	return;
} 
      
							
/*device get into......... 0x40 */
void Device_Get_Into(void)
{
  if(CONFIG_MODE == 1)
  {
    Pcb_Write_ALL(Cmd_usart);
	return;
  }

}

/**
  * @brief  to analysis the 24byte data from the net. 
  * @param  
           *Cmd: the pointer of the 24byte date. 
  * @retval None
  */

CommandStatus Command_Sucess = SUCESS;

void Tcp_Cmd_Analysis(INT8U  *Cmd_trans)
{
  int t = 0;
  dev_offset = 0;
  /*find which user*/
  for(t = 0; t < 11; t++)
  {
    if (pad_info[t].socket == tcp_device)
	{
	  dev_offset = t;
	}
  }
  Cmd = Cmd_trans;
  
  if(pad_info[dev_offset].user == TRUE || (CONFIG_MODE == 1))
  {
    if(cmd_analysis[*(Cmd+4)].cmd == *(Cmd+4))
	{
		 
  	  cmd_analysis[*(Cmd+4)].func_cmd();
	    	
    }
  }
  else if (*(Cmd+4) == 0x50) 
  {
  	cmd_analysis[0x50].func_cmd();
  }
  return;
}

/*check the secret   0x50*/
void Check_secret(void)
{ 
	if(0 != memcmp(&sys_info[0].password[0], Cmd+10, sys_info[0].password[0] + 1) && CONFIG_MODE == 0)
	{
    Command_Sucess = FAIL;
  }
	else
	{
		Command_Sucess = SUCESS;
  }
  
  Cmd = cmd_reply;
  *(Cmd+3) = 0x50;
  *(Cmd+4) = 0x04;
  *(Cmd+10) = Command_Sucess;
  Pcb_Write(Cmd);
	
	if(SUCESS == Command_Sucess){
	  pad_info[dev_offset].user = TRUE;
  }else{
	  tcp_close(pad_info[dev_offset].socket); 
  }
	
  return;  
}
/*change the secret in config mode  0x51*/
void Change_Secret(void)
{ 
  INT8U secret_len = *(Cmd+10);

  if (secret_len < 6 || secret_len > 12 || CONFIG_MODE == 0)
  {
    Command_Sucess = FAIL;
  }
	else
	{
    Command_Sucess = SUCESS;
		memcpy(&sys_info[0].password[0], Cmd+10, secret_len + 1);
		password_change = TRUE;
  }
	
  Cmd = cmd_reply;

  *(Cmd+3) = 0x51;
  *(Cmd+4) = 0x04;
  *(Cmd+10) = Command_Sucess;
	
	memcpy(Cmd+11, &sys_info[0].password[0], secret_len+1);
  
  Pcb_Write(Cmd);
  	 
  return;   
   
}

 /*search  the room information  0x01*/
void Serch_Room_Info(void)
{
  int search_room_t = 0;
	int room_ops = 0;

	Cmd = cmd_reply;

	for(search_room_t = 0; search_room_t < ROOMS_CAP; search_room_t++)
	{
	  if ((room_info[search_room_t].room_id == search_room_t) || (room_info[search_room_t].room_id == 0xfe))
	  {
	    room_ops++;
	    
	    *(Cmd+4) = 0x02;
	    *(Cmd+6) = room_info[search_room_t].room_id;
	    *(Cmd+10) = room_ops;
	    *(Cmd+11) = rooms_cnt;
		 
			memcpy(Cmd+12, &room_info[search_room_t].room_name[0], 12);
		
	    Pcb_Write(Cmd);
	  }
	} 
	return;
}
/*add a room information  0x03*/
void Add_Room_Info(void)
{
	int roomID = *(Cmd+6);
	
  if(room_info[roomID].room_id == roomID || roomID >= ROOMS_CAP 
																				 || CONFIG_MODE == 0)
  {	
     Command_Sucess = FAIL;
  }
  else
  {
		Room_Add(Cmd);
		rooms_cnt++;
    Command_Sucess = SUCESS;
  }
  
  	/*reply  to  tcp */ 
	Cmd = cmd_reply;
		   
  *(Cmd+3) = 0x03;
  *(Cmd+4) = 0x04;
  *(Cmd+10) = Command_Sucess;
  Pcb_Write(Cmd);

  return;
}
/*delete a room 0x05*/
void Delete_Room_Info(void)
{ 
  int t = 0;
  INT8U delete_room_save = *(Cmd+6);
  INT8U room_id_ops = *(Cmd+6);
  
	if(delete_room_save == 0xfe)  room_id_ops = 0;

	if(delete_room_save == 0xfe || room_info[room_id_ops].room_id != delete_room_save
															|| CONFIG_MODE == 0)
	{
		Command_Sucess = FAIL;
	}
	else
  {
    Room_Delete(Cmd);

    for(t = 1;t < DEVICES_CAP; t++)
    {
	    if(dev_info[t].room_id == delete_room_save)
	    {
	      dev_info[t].room_id = 0xfe;
				room_info[0].dev_cnt++;
	   
				Room_Dev_Add(0xfe, dev_info[t].dev_id);
				Room_Dev_Delete(delete_room_save, dev_info[t].dev_id);
	    }									
	  }
    Command_Sucess = SUCESS;
  }
  
  	/*reply to tcp   */
	  Cmd = cmd_reply;

	  *(Cmd+3) = 0x05;
	  *(Cmd+4) = 0x04;
	  *(Cmd+6) = delete_room_save;
	  *(Cmd+10) = Command_Sucess;
	  Pcb_Write(Cmd);
	  return;
  
}
/*change a room_name 0x06*/
void Change_Room_Name(void)
{
	int roomID = *(Cmd+6);
    
	if(CONFIG_MODE == 0 || roomID >= ROOMS_CAP
											|| room_info[roomID].room_id != roomID)
	{
	  Command_Sucess = FAIL;
	}
  else
	{
	  memcpy(&room_info[roomID].room_name[0], Cmd+10, 12);
		Command_Sucess = SUCESS;
	}
  
	Cmd = cmd_reply;

	*(Cmd+3) = 0x06;
	*(Cmd+4) = 0x04;
	*(Cmd+10) = SUCESS;
	Pcb_Write(Cmd);
 
}   
/*device add  in  config mode 0x07 */

void Device_Add_Info(void)
{ 
  int t = 0;
  INT8U dev_add_id = *(Cmd+7);
  INT8U add_room_id = *(Cmd+6);
  INT8U add_room_ops = *(Cmd+6);
  INT8U add_room_cap = 20;
	
	if(add_room_ops == 0xfe){
	  add_room_ops = 0;
	  add_room_cap = 255;
	}

  if(CONFIG_MODE == 1 && room_info[add_room_ops].room_id == add_room_id
											&& dev_info[dev_add_id].dev_id == 0
											&& room_info[add_room_ops].dev_cnt < add_room_cap)
  {	
    Command_Sucess = SUCESS;	
	  for(t = 1; t < DEVICES_CAP; t++)
	  {
	    if(dev_info[t].dev_id != 0 && 0 == memcmp(&dev_info[t].dev_mac[0], Cmd+11, 8))
	    {
			
		    Command_Sucess = FAIL;
			  break;
	    }  
	  }
	}
	else
	{
    Command_Sucess = FAIL;
  }
		
    
  if(SUCESS == Command_Sucess)
  {
	  	Device_Add(Cmd);
	}
  
  Cmd = cmd_reply;

  *(Cmd+3) = 0x07;
  *(Cmd+4) = 0x04;
  *(Cmd+6) = add_room_id;
	*(Cmd+10) = Command_Sucess;
	*(Cmd+11) =	dev_add_id;
	Pcb_Write(Cmd);

	return;
  
}
/*device delete  in  config mode  0x08*/
void Device_Delet_Info(void)
{
	INT8U room_id = *(Cmd+6);
	INT8U dev_id = *(Cmd+7);
	INT8U room_ops = room_id;
	
	if(room_ops == 0xfe) room_ops = 0;
  if(CONFIG_MODE == 1 && SUCESS == Dev_CheckRoom(dev_id, room_id))
  {	
    Device_Delete(Cmd);
    Command_Sucess = SUCESS;
  }
  else
  {
		Command_Sucess = FAIL;
	}
	
	Cmd = cmd_reply;

  *(Cmd+3) = 0x08;
  *(Cmd+4) = 0x04;
  *(Cmd+6) = room_id;
  *(Cmd+10) = Command_Sucess;
  *(Cmd+11) = dev_id;
   
  Pcb_Write(Cmd);
	return;
  
}
/*device change room_id  in  config mode  0x09*/
void Device_Change_RoomID(void)
{ 
  INT8U dev_chang_room_id = dev_info[*(Cmd+7)].room_id;
  INT8U dev_dest_room_id =  *(Cmd+6);
  INT8U dev_chang_room_ops = dev_chang_room_id;
  INT8U dev_dest_room_ops =  dev_dest_room_id;

  INT8U dev_change_id = *(Cmd+7);
  INT8U change_room_cap = 20;
  
	if(dev_chang_room_id == 0xfe) dev_chang_room_ops = 0;
	if(dev_dest_room_id == 0xfe){
	   dev_dest_room_ops = 0;
	   change_room_cap = 255;
	 }
	 
  if(CONFIG_MODE == 1 && room_info[dev_dest_room_ops].room_id == dev_dest_room_id 
											&& SUCESS == Dev_CheckRoom(dev_change_id, dev_chang_room_id)
											&& room_info[dev_dest_room_ops].dev_cnt < change_room_cap )
  {
	 room_info[dev_chang_room_ops].dev_cnt--;

   dev_info[dev_change_id].room_id = dev_dest_room_id;
	 
	 room_info[dev_dest_room_ops].dev_cnt++;
	 

	 Room_Dev_Add(dev_dest_room_id, dev_change_id);
	 Room_Dev_Delete(dev_chang_room_id, dev_change_id);

	 Command_Sucess = SUCESS;
  }
  else
  {
		Command_Sucess = FAIL;
	}
  
	Cmd = cmd_reply;
     
	*(Cmd+3) = 0x09;
	*(Cmd+4) = 0x04;
	*(Cmd+6)  = dev_dest_room_id;
	   
	*(Cmd+10) = Command_Sucess;
  *(Cmd+11) = dev_change_id;
	Pcb_Write(Cmd);
	
	return;
  
}
/*to get a picture return 0x0a*/
void Get_Picture(void)
{   

}

/*add into scene mode  0x0d*/
void Add_Scene(void)
{
  int t = 0;
	INT8U cmd_save[5];
	
	Command_Sucess = FAIL;
	
	memcpy(&cmd_save[0], Cmd+10, 5);
	
  for (t = 0; t < SCENE_CAP; t++) 
  {
	  if(scene_info[t].scene_id == 0)
	  {
	    scene_info[t].scene_id = *(Cmd+6);
      memcpy(&scene_info[t].dev_id, &cmd_save[0], 5);
	    Command_Sucess = SUCESS;
	    scene_change = 1;	
	    break;
	  }
  }
  
  Cmd = cmd_reply;

	*(Cmd+4) = 0x04;
	*(Cmd+3) = 0x0d;
	*(Cmd+6) = scene_info[t].scene_id;
	*(Cmd+10) = Command_Sucess;
  memcpy(Cmd+11, &cmd_save[0], 5);
	
	Pcb_Write(Cmd);
	
	return;
}
/*delete a scene mode act 0x0f*/
void Delete_Scene(void)
{  
  int t = 0;
  int de_sce_save = 0;
	INT8U cmd_save[5];
	
  INT8U scene_id_save = *(Cmd+6);
  de_sce_save = *(Cmd+10);
  
	memcpy(&cmd_save[0], Cmd+10, 5);
	
	Command_Sucess = FAIL;
	
	if(0 == de_sce_save)
  {
		for(t = 0 ;t < SCENE_CAP; t++)
    {
      if(scene_info[t].scene_id == scene_id_save)
	    {
		    scene_info[t].scene_id = 0;
				
      }
		}
		Command_Sucess = SUCESS;
  }
	else
	{
    for(t = 0 ;t < SCENE_CAP; t++)
    {
      if(scene_info[t].scene_id == scene_id_save && 0 == memcmp(&scene_info[t].dev_id, &cmd_save[0], 5))
	    {
		    scene_info[t].scene_id = 0;
				Command_Sucess = SUCESS;
				break;
      }
		}
  
  }
	
	Cmd = cmd_reply;

	*(Cmd+4) = 0x04;
	*(Cmd+3) = 0x0f;
  *(Cmd+6) = scene_id_save;
  *(Cmd+10) = Command_Sucess;	
	memcpy(Cmd+11, &cmd_save[0], 5);
		
	Pcb_Write(Cmd);
	
  return;

}

/*search  scene mode information 0x12*/
void Serch_Scene(void)
{
  int t = 0;
  INT8U search_scene_id = *(Cmd+6); 
  int ser_scen_t = 0;
  int ser_total = 0;

  Cmd = cmd_reply;

  for(t = 0 ;t < SCENE_CAP; t++)
  {
	if(scene_info[t].scene_id == search_scene_id)
	{	 
	  ser_total++;
    } 
  }

  if(ser_total == 0)
  {
	*(Cmd+4) = 0x13;
	*(Cmd+10) = SUCESS;
	*(Cmd+11) = 0;
	Pcb_Write(Cmd);
	return;
  }

  for(t = 0 ;t < SCENE_CAP; t++)
  {
	if(scene_info[t].scene_id == search_scene_id)
	{	 
	  ser_scen_t++;
	  *(Cmd+4) = 0x13;
	  *(Cmd+6) = search_scene_id;
	  *(Cmd+10) = ser_scen_t;
	  *(Cmd+11) = ser_total;
	  *(Cmd+12) = scene_info[t].dev_id;
	  *(Cmd+13) = dev_info[scene_info[t].dev_id].room_id;
	  *(Cmd+14) = scene_info[t].act_cmd[0];
	  *(Cmd+15) = scene_info[t].act_cmd[1];
	  *(Cmd+16) = scene_info[t].dev_cmd[0];
	  *(Cmd+17) = scene_info[t].dev_cmd[1];

	  Pcb_Write(Cmd);

	} 
  }
  return;
}
/*search alarm control device 0x15*/
void Search_Alarm_Device(void)
{ 
  int t = 0;
  INT8U alarm_id = *(Cmd+7);
  INT8U alarm_mess0 = *(Cmd+10);
  INT8U alarm_mess1 = *(Cmd+11);
  int totol_PDU = 0;
  int PDU_ops = 1;
  for(t = 0; t < ALARM_DEV_CAP; t++)
  {
  	if(alarm_dev_info[t].alarm_dev_id ==  alarm_id && alarm_dev_info[t].alarm_message[0] == alarm_mess0 
												   && alarm_dev_info[t].alarm_message[1] == alarm_mess1)
	{
	   totol_PDU ++;
	}
  }

  for(t = 0; t < ALARM_DEV_CAP; t++)
  {
  	if(alarm_dev_info[t].alarm_dev_id ==  alarm_id && alarm_dev_info[t].alarm_message[0] == alarm_mess0 
												   && alarm_dev_info[t].alarm_message[1] == alarm_mess1)
	{
	    Cmd = cmd_reply;
		*(Cmd+4) = 0x16;
        *(Cmd+10) = PDU_ops;
        *(Cmd+11) = totol_PDU;
		*(Cmd+12) = alarm_id;
		*(Cmd+13) = alarm_mess0;
		*(Cmd+14) = alarm_mess1;
		*(Cmd+15) = alarm_dev_info[t].phone_message;
		*(Cmd+16) = alarm_dev_info[t].dev_act_id;
		*(Cmd+17) = dev_info[alarm_dev_info[t].dev_act_id].room_id;
		*(Cmd+18) = alarm_dev_info[t].dev_active[0];
		*(Cmd+19) = alarm_dev_info[t].dev_active[1];
		*(Cmd+20) = alarm_dev_info[t].dev_action[0];
		*(Cmd+21) = alarm_dev_info[t].dev_action[1];

		Pcb_Write(Cmd);
		PDU_ops++; 
	}
  }
}

/*add alarm control device 0x14*/
void Add_Alarm_Device(void)
{
  int t = 0;

  INT8U dev_roomid = *(Cmd+6);
  INT8U alarm_id = *(Cmd+7);
  INT8U reply_save[7];
  

  memcpy(&reply_save[0], Cmd+10, 7);
	
	if(SUCESS == Dev_CheckRoom(alarm_id, dev_roomid))
	{
    for(t = 0;t < ALARM_DEV_CAP; t++)
    {
      if(alarm_dev_info[t].alarm_dev_id == 0)
  	  { 
				alarm_dev_info[t].alarm_dev_id = alarm_id;
				memcpy(&alarm_dev_info[t].alarm_message[0], &reply_save[0], 7);
	      warn_dev_change = 1;
	      break;
	    }

    }
   Command_Sucess = SUCESS;		
  }
	else
	{
   Command_Sucess = FAIL;
  }
  Cmd = cmd_reply;
  *(Cmd+3) = 0x14;
  *(Cmd+4) = 0x04;
  *(Cmd+6) = dev_roomid;
  *(Cmd+10) = Command_Sucess;
  *(Cmd+11) = alarm_id;
  for(t = 0; t < 7; t++)
  {
    *(Cmd+12+t) = reply_save[t];
  }
  Pcb_Write(Cmd);

  
}

/*delete alarm control device 0x0e*/
void Del_Alarm_Device(void)
{
  int t = 0;
  
  INT8U dev_roomid = *(Cmd+6);
  INT8U alarm_id = *(Cmd+7);
	INT8U alarm_dev_id = *(Cmd+12);
	
  INT8U reply_save[7];

  memcpy(&reply_save[0], Cmd+10, 7);
	
	if(alarm_dev_id == 0)
	{	
    for(t = 0;t < ALARM_DEV_CAP; t++)
    {
      if(alarm_dev_info[t].alarm_dev_id == alarm_id)
  	  {
	      alarm_dev_info[t].alarm_dev_id = 0;
	    }
			
    }
		warn_dev_change = 1;
		
	}
	else 
	{
    for(t = 0;t < ALARM_DEV_CAP; t++)
    {
      if(alarm_dev_info[t].alarm_dev_id == alarm_id && 0 == memcmp(&alarm_dev_info[t].alarm_message[0], &reply_save[0], 7))
  	  {
	      alarm_dev_info[t].alarm_dev_id = 0;
				warn_dev_change = 1;
				break;
	    }
    }
  }

  Cmd = cmd_reply;
  *(Cmd+3) = 0x0e;
  *(Cmd+4) = 0x04;
  *(Cmd+6) = dev_roomid;
  *(Cmd+10) = SUCESS;
  *(Cmd+11) = alarm_id;
  for(t = 0; t < 7; t++)
  {
    *(Cmd+12+t) = *(reply_save+t);
  }
  Pcb_Write(Cmd);

}
/*set the alarm phone_message 0x0x37*/
void Alarm_Phone_message(void)
{ 
  
}
/*Enable Alarm_dev 0x2d*/
void Enable_Alarm(void)
{
  INT8U alarm_id  = *(Cmd+7);
	INT8U alarm_roomID = *(Cmd+6);
	
  INT8U alarm_enable = *(Cmd+10);
	
	
	
	if(SUCESS == Dev_CheckRoom(alarm_id, alarm_roomID) 
		&& (alarm_enable == 0x1 || alarm_enable == 0x0))
	{
    sys_info[0].alarm_dev_enable[alarm_id] = alarm_enable;

    sys_info_change = 1;
		Command_Sucess = SUCESS;
  }
	else
	{
    Command_Sucess = FAIL;
  }
  Cmd = cmd_reply;
  *(Cmd+3) = 0x2d;
  *(Cmd+4) = 0x04;
  *(Cmd+10) = Command_Sucess;
  *(Cmd+11) = alarm_id;
  *(Cmd+12) = alarm_enable;
  Pcb_Write(Cmd);
}

/*search Alarm_dev_enable  0x2f*/
void Search_Alarm_Enable(void)
{
  INT8U alarm_id  = *(Cmd+7);
	
  Cmd = cmd_reply;
  
  *(Cmd+3) =  alarm_id;
  *(Cmd+4) = 0x34;
  *(Cmd+6) =  dev_info[alarm_id].room_id;
  *(Cmd+10) = sys_info[0].alarm_dev_enable[alarm_id];
  
  Pcb_Write(Cmd);
}
/* search for alarm record 0x35*/
extern FIL warn_fsrc;
extern UINT br;
extern FATFS fs;

void Search_Alarm_Record(void)
{ 
  int ops = 0;
  int t = 0; 
  int cnt = 0; 
  int count_ops = 0;
  int block_size = 8; 
	
  if(f_mount(0, &fs) != FR_OK) return;

  if(f_open(&warn_fsrc, "warn.dat", FA_OPEN_EXISTING | FA_READ)!= FR_OK){
    return;
  }else{
    cnt = (warn_fsrc.fsize - 1)/8;
	}
	
  Cmd = cmd_reply;
  
	t = warn_ops;
  while(cnt > 0)
  {	
    
	 f_lseek(&warn_fsrc, block_size*(t+1));
	  
	 f_read(&warn_fsrc, (void*)(&alarm_record_info[0]), block_size, &br);
    
	
	count_ops++;
	
	*(Cmd+4) = 0x36;
	*(Cmd+10) = count_ops;
    *(Cmd+12) = (warn_fsrc.fsize - 1)/8;
	*(Cmd+14) = alarm_record_info[0].alarm_id;
	*(Cmd+15) = dev_info[alarm_record_info[0].alarm_id].dev_type;
    *(Cmd+16) = dev_info[alarm_record_info[0].alarm_id].room_id;
	for(ops = 0; ops < 7; ops++)
	{
	  *(Cmd+17 + ops) = alarm_record_info[0].alarm_time[ops];
	}

	Pcb_Write(Cmd);

	t++;
	if (t == ALARM_MESSAGE_CAP) t = 0;
    cnt--;
  }	 
  f_close(&warn_fsrc);
  f_mount(0, NULL);

}

void Alarm_Record_Clean(void)
{
  Cmd = cmd_reply;

	*(Cmd+3) = 0x38;
	*(Cmd+4) = 0x04;
	*(Cmd+10) = SUCESS;
    
  if(f_mount(0, &fs) != FR_OK) return;  
  f_unlink("warn.dat");
	f_mount(0, NULL);
	Pcb_Write(Cmd);
}
/*set the local time  0x17*/
void  Time_Set(void)
{ uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t year;
  uint8_t month;
  uint8_t day;
	uint8_t week;
  
  if(*(Cmd+10) != sys_info[0].time_mode)
  {
    sys_info[0].time_mode = *(Cmd+10);
   	sys_info_change = 1; 
  }
  
  if (*(Cmd+10) == 0x0)
  { 
    sys_info[0].time_mode = 0;

    year = (uint8_t)(((uint32_t)*(Cmd+12)<<8) + (uint32_t)*(Cmd+11) - 0x7d0);
    month = *(Cmd+13);
    day = *(Cmd+14);

    hour = *(Cmd+15);
    min = *(Cmd+16);
    sec = *(Cmd+17);
    week = My_GetWeek(hour, min, sec,
			  year, month, day);
    Set_RTCTime(hour, min, sec,
			  year, month, day, week);

   Cmd = cmd_reply;
 
   *(Cmd+3) = 0x17;
   *(Cmd+4) = 0x04;
   *(Cmd+10) =0x0;

   Pcb_Write(Cmd);
  }
  else
  {	
    UpdateTime_Send();
  }
}
/*search systerm time 0x1c*/
void Search_Sys_Time(void)
{
  Cmd = cmd_reply;
  *(Cmd+4) = 0x1d;
  Get_RTCtime();
  *(Cmd+10) = sys_info[0].time_mode;
  *(Cmd+11) = (my_time.year + 0x7d0)%0x100;
  *(Cmd+12) = (my_time.year + 0x7d0)>>8;
  *(Cmd+13) = my_time.month; 
  *(Cmd+14) = my_time.day;
  *(Cmd+15) = my_time.hour;
  *(Cmd+16) = my_time.min;
  *(Cmd+17) = my_time.sec;
  
  Pcb_Write(Cmd);

}

/*search systerm mode*/
void Search_Sys_Mode(void)
{
   Cmd = cmd_reply;
  
  *(Cmd+4) = 0x1f;
  
  *(Cmd+10) = CONFIG_MODE;
  Pcb_Write(Cmd);

}
/*choose one scene mode 0x2c*/
void Choose_Scene(void)
{ 
  int t;
	
  if(CONFIG_MODE == 0)
  {
    INT8U choose_scene_save = *(Cmd+6);
    
		if(choose_scene_save == 0 || choose_scene_save > 6) return;
		
    for(t = 0; t < SCENE_CAP; t++)
	  {
			Cmd = cmd_reply;
  	   if(scene_info[t].scene_id == choose_scene_save)
	    {
	    *(Cmd+8) = dev_info[scene_info[t].dev_id].dev_addr[0];
	    *(Cmd+9) = dev_info[scene_info[t].dev_id].dev_addr[1];
		 
	    *(Cmd+4) = scene_info[t].act_cmd[0];
		  *(Cmd+6) = dev_info[scene_info[t].dev_id].room_id;
		  *(Cmd+7) = dev_info[scene_info[t].dev_id].dev_id;
		  *(Cmd+10) = scene_info[t].dev_cmd[0];
		  *(Cmd+11) = scene_info[t].dev_cmd[1];
				
		  cmd_analysis[*(Cmd+4)].func_cmd();
			memset(cmd_reply+3, 0 ,21);
	  }
	}
    return;
  }
}

  //*get the room device status 0x5e*/
void Get_device_status(void)  
{  
    INT8U devId = *(Cmd+7);
	  
	 if(dev_info[devId].dev_id == devId)
	 {
		 Cmd = cmd_reply;
	  
		 *(Cmd+3) = devId;
  	 *(Cmd+4) = 0x21;
		 *(Cmd+6) = dev_info[devId].room_id;
		 
	   *(Cmd+10) = dev_info[devId].dev_status[0];
	   *(Cmd+11) = dev_info[devId].dev_status[1];
		 *(Cmd+12) = dev_info[devId].dev_status[2];
		 
    
	   Pcb_Write(Cmd);
	 }
}

/*  Synchronous  the device 0x29   0x46  0x2b*/
void Synchronous_Device(void)
{	
    int t = 0; 
    int sy_dev_ops = 0;
	int sy_pair = 0;
	int sy_n = 0;
	int sy_m = 0;
	INT8U save_cmd46_now = *(Cmd+10);
	INT8U save_syn_cmd = *(Cmd+4);
	INT8U save_syn_room = *(Cmd+6);
	INT8U room_ops = 0;
	INT8U room_dev_cap = 20;   /////
	int room_dev_t = 0;

	//
	INT8U reply_cmd = 0x41;

	if(save_syn_cmd == 0x2b)
	{
	  reply_cmd = 0x42;
	  save_syn_cmd = 0x29;
	}
	//
	Cmd = cmd_reply;

    if (save_syn_room == 0xfe)
	{
	  room_ops = 0;
	  room_dev_cap = 255;
	}
	else
	{
	  room_ops = save_syn_room;
	}

	if((room_info[room_ops].dev_cnt == 0) ||  
	 ((save_syn_room == 0xfe) && (room_info[0].dev_cnt == 0)))
	{ 
	  
	   
	  *(Cmd+10) = SUCESS;
	  *(Cmd+11)	= 0;
	  *(Cmd+6)	= save_syn_room;
	  if(save_syn_cmd == 0x29)
	  {
	    *(Cmd+4) = reply_cmd + room_status;
	  }
	  else
	  {
	    *(Cmd+4) = 0x47;
	  }
	  
	  Pcb_Write(Cmd);
	  return;
	}

    for(room_dev_t = 0; room_dev_t < room_dev_cap; room_dev_t++)
	{ 
	  
	  if (Room_Dev_Search(save_syn_room, room_dev_t) != 0)
	  {	 
	     t = Room_Dev_Search(save_syn_room, room_dev_t);
	     sy_dev_ops++;
	     sy_pair++;
		 *(Cmd+4*sy_pair+9) = dev_info[t].dev_id;
		 *(Cmd+4*sy_pair+10) = dev_info[t].dev_type;
		 *(Cmd+4*sy_pair+11) = dev_info[t].dev_status[0];
		 *(Cmd+4*sy_pair+12) = dev_info[t].dev_status[1];
         
		 if(dev_living[t] == 0)
		 {
		   *(Cmd+4*sy_pair+11) = 0xe8;
		   *(Cmd+4*sy_pair+12) = 0xe8;
		 }

	  }
	  if((sy_pair == 2) || (sy_dev_ops == room_info[room_ops].dev_cnt))
	  {	  
	      sy_n++;
		 

		  if((save_syn_cmd == 0x46) && (sy_n == save_cmd46_now))
		  {
		   *(Cmd+10) = sy_n;
		   *(Cmd+11) = (room_info[room_ops].dev_cnt+1)/2;
		   *(Cmd+12) = sy_pair*4;
		   *(Cmd+4) = 0x47;
		   Pcb_Write(Cmd);
		   
		   return; 
		  }
		  else 
		  {
		    *(Cmd+10) = sy_n;
		    *(Cmd+11) = (room_info[room_ops].dev_cnt+1)/2;
			*(Cmd+12) = sy_pair*4;
		    *(Cmd+4) = reply_cmd + room_status;
			*(Cmd+6) = save_syn_room;
			if(save_syn_cmd == 0x29)
		    { 
			   				  
              Pcb_Write(Cmd);
			}
		    
			for(sy_m = 0; sy_m < 8; sy_m++)
		    {
		      *(Cmd+13+sy_m) = 0;
		    }

		    sy_pair = 0;
		    
	   }	   
          
	  }
	  if(sy_dev_ops == room_info[room_ops].dev_cnt)
	  {
	    return ;
	  }

	  
	}
	 return;
}

static void CmdFromUart_ChangeToNet(void)
{
  Save_Cmd(Cmd_usart);
}
/*control  the  single dev*/
void Control_Single_dev(void)
{ 
 
  if(CONFIG_MODE == 0)
  {
    *(Cmd+8) = dev_info[*(Cmd+7)].dev_addr[0];
	*(Cmd+9) = dev_info[*(Cmd+7)].dev_addr[1];

	if(dev_info[*(Cmd+7)].dev_addr[0] == 0x0 && 
	   dev_info[*(Cmd+7)].dev_addr[1] == 0x0)
	{
	  Get_Dev_Addr(*(Cmd+7));
	  return;
	}  
    Task_Usart_Send(Cmd);
    return;
  }
  
}

/*control  the  lamp of  one room  0x22*/
void Control_Lamp_Room(void)
{
  int t = 0;
  INT8U dev_id = 0;
  INT8U room_ops = *(Cmd + 6) - 1;
  INT8U status = *(Cmd + 10);
	
  if(CONFIG_MODE == 0)
  {
    for(t = 0; t < 20; t++)
	{
	   dev_id = room_dev_info[room_ops].dev_id[t];
	   if((dev_info[dev_id].dev_type == 0x01 ||
	       dev_info[dev_id].dev_type == 0x11 ||
		   dev_info[dev_id].dev_type == 0x12 ||
		   dev_info[dev_id].dev_type == 0x1c || 
		   dev_info[dev_id].dev_type == 0x1e ||
		   dev_info[dev_id].dev_type == 0x24 ||
		   dev_info[dev_id].dev_type == 0x27) && (dev_id > 0))
	   {
	      My_Control_DevLamp(dev_id, status);
			  //vTaskDelay(100);
	   }
	}
	
	return; 
  }
}

/*control  the  lamp of  whole house 0x23*/
void Control_Lamp_All(void)
{ 
  int t = 0;
  INT8U status = *(Cmd + 10);

  if(CONFIG_MODE == 0)
  {
    for(t = 1; t < DEVICES_CAP; t++)
	{
	  if((dev_info[t].dev_type == 0x01 ||
	      dev_info[t].dev_type == 0x11 ||
		    dev_info[t].dev_type == 0x12 ||
		    dev_info[t].dev_type == 0x1c || 
		    dev_info[t].dev_type == 0x1e ||
		    dev_info[t].dev_type == 0x24 ||
		    dev_info[t].dev_type == 0x27) && (dev_info[t].dev_id == t))
	  {
	  	  My_Control_DevLamp(t, status);
			  //vTaskDelay(100);
	  }
	}
	
	return; 
  }
}
/*Unusual_state Message 0x31*/
void Unusual_Message(void)
{
  Pcb_Write_ALL(Cmd);
}

/*device sleep state 0x33*/
void Device_Sleep_Message(void)
{


}
/*control hongwai*/
void Control_HongWai(void)
{
  INT8U key_id = *(Cmd+10);
	INT8U dev_id = *(Cmd+7);
	
	if(dev_info[dev_id].dev_type == 0x18 && 
													key_id <= 13 && 
													key_id >0)
  {
     unsigned int a = dev_info[dev_id].dev_status[0] + (dev_info[dev_id].dev_status[1]<<8);
		 unsigned int b = 0x1<<(key_id - 1);
		 if((a&b) == b) Control_Single_dev();
  }
}

/*Device_Reply_Living 0x43*/
void Device_Reply_Living(void)
{
  dev_living[*(Cmd_usart+3)] = 0x03;
  
  dev_info[*(Cmd_usart+3)].dev_addr[0] = *(Cmd_usart+10);
  dev_info[*(Cmd_usart+3)].dev_addr[1] = *(Cmd_usart+11);
}

void LFdevice_Synchronous(void)
{
  INT8U LF_id=*(Cmd_usart+3);
	INT8U LF_addr[2];
	INT8U LF_macaddr[8];
	
	LF_addr[0] = *(Cmd_usart+18);
	LF_addr[1] = *(Cmd_usart+19);
	memcpy(LF_macaddr,Cmd_usart+10, 8);
	
	if(dev_info[LF_id].dev_id == LF_id &&	0 == memcmp(&dev_info[LF_id].dev_mac[0], Cmd_usart+10, 8))
	{
		dev_info[LF_id].dev_addr[0] = *(Cmd_usart+18);
		dev_info[LF_id].dev_addr[1] = *(Cmd_usart+19);
		
		Command_Sucess = SUCESS;
		dev_living[*(Cmd_usart+3)] = 0x03;
  }
	else
	{
		Command_Sucess = FAIL;
	}
    Cmd_usart = usart_reply;
    *(Cmd_usart+4) = 0x4a;
		*(Cmd_usart+7) = LF_id;
		memcpy(Cmd_usart+8, &LF_addr[0], 2);
		*(Cmd_usart+10) = Command_Sucess;
		memcpy(Cmd_usart+11, &LF_macaddr[0], 8);

  
  Task_Usart_Send(Cmd_usart);

}

void Add_DevTimer(void)
{
  uint8_t timer_id = *(Cmd+6) - 1;
	if((dev_info[*(Cmd+14)].dev_id == *(Cmd+14) || *(Cmd+15) == 0x2c) && timer_id < TIMER_CAP){
    memcpy(&task_timer[timer_id].enable, Cmd+10, 9); 
		Command_Sucess = SUCESS;
		dev_timer_change = 1;
  }
	Cmd = cmd_reply;
	*(Cmd+3) = 0x4c;
	*(Cmd+4) = 0x04;
	*(Cmd+10) = Command_Sucess;
	memcpy(Cmd+11, &task_timer[timer_id].enable, 9);
	Pcb_Write(Cmd);
}
void Del_DevTimer(void)
{
	
  uint8_t timer_id = *(Cmd+6) - 1;
	if(timer_id < TIMER_CAP && task_timer[timer_id].id != 0){
		memset(&task_timer[timer_id].enable, 0, sizeof(struct timer));
	  Command_Sucess = SUCESS;
		dev_timer_change = 1;
	}
	Cmd = cmd_reply;
	*(Cmd+3) = 0x4d;
	*(Cmd+4) = 0x04;
	*(Cmd+10) = Command_Sucess;
	memcpy(Cmd+11, &task_timer[timer_id].enable, sizeof(struct timer));
	Pcb_Write(Cmd);
}
void Sys_DevTimer(void)
{
  int list = 0;
	int total = 0;
	int pdu_n = 0;
	for(list=0; list<TIMER_CAP;list++){
    if(task_timer[list].id != 0) total++;
  }
	
	Cmd = cmd_reply;
	if(total == 0){
		 *(Cmd+4) = 0x4f;
	   *(Cmd+10) = total;
		 *(Cmd+11) = total;
	   Pcb_Write(Cmd);
		return;
	}
	
	for(list=0; list<TIMER_CAP;list++){
    if(task_timer[list].id != 0) {
			pdu_n++;
	   *(Cmd+4) = 0x4f;
		 *(Cmd+6) = list + 1;
	   *(Cmd+10) = pdu_n;
		 *(Cmd+11) = total;
	   memcpy(Cmd+12, &task_timer[list].enable, 9);
	   Pcb_Write(Cmd);
		
    };
  }
}

/*email set  0x52*/
void Email_Set(void)
{

}

/*******************************systerm command******************************************************/
/**
  * @brief  function to add a room . 
  * @param  
           *Cmd: the pointer of the 24byte date. 
  * @retval None
  */

static void Room_Add( INT8U  *Cmd)
{
  room_info[*(Cmd+6)].room_id = *(Cmd+6);

	memcpy(&room_info[*(Cmd+6)].room_name[0], Cmd+10, 12);
	
  room_info[*(Cmd+6)].dev_cnt = 0;
  
}


/**
  * @brief  function to delete a room. 
  * @param  
           *Cmd: the pointer of the 24byte date. 
  * @retval None
  */
static void Room_Delete(INT8U *Cmd)
{ 
  INT8U room_id_save = *(Cmd+6);
  room_info[room_id_save].room_id = 0;
  room_info[room_id_save].dev_cnt = 0;

  rooms_cnt--;
}

/**
  * @brief  function to add a device. 
  * @param  
           *Cmd: the pointer of the 24byte date. 
  * @retval None
  */

void Cmd_toLFDevice(const INT8U dev_now_id)
{
	INT8U *LFCmd;
	LFCmd = cmd_reply;
	
	*(LFCmd+4) = 0x07;
	*(LFCmd+6) = dev_info[dev_now_id].room_id;
	*(LFCmd+7) = dev_now_id;
	*(LFCmd+8) =  dev_info[dev_now_id].dev_addr[0];
	*(LFCmd+9) =  dev_info[dev_now_id].dev_addr[1];
  
	memcpy(LFCmd+10, zigbee_macaddr, 8);
	memcpy(LFCmd+18, &dev_info[dev_now_id].dev_mac[4], 4);
	*(LFCmd+22) = dev_info[dev_now_id].dev_type;
	
	Task_Usart_Send(LFCmd);
}


static void Device_Add(INT8U  *Cmd)
{
  INT8U dev_now_id = *(Cmd+7);
  INT8U room_now_id = *(Cmd+6);
  dev_info[dev_now_id].room_id = *(Cmd+6);
  dev_info[dev_now_id].dev_id = *(Cmd+7);
  dev_info[dev_now_id].dev_type = *(Cmd+10);
  
  dev_info[dev_now_id].dev_status[0] = 0x0;
  dev_info[dev_now_id].dev_status[1] = 0x0;
	dev_info[dev_now_id].dev_status[2] = 0x0;
	
  dev_living[dev_now_id] = 0x03;
	sys_info[0].alarm_dev_enable[dev_now_id] = 0;
  
	memcpy(&dev_info[dev_now_id].dev_mac[0], Cmd+11, 10);
 
  if(0xfe == room_now_id)
  {
    room_info[0].dev_cnt++;
  }
  else
  {
    room_info[room_now_id].dev_cnt++;
  }

   Room_Dev_Add(room_now_id, dev_now_id);

/*set the dev into the net*/
	 Cmd = cmd_reply;
	/*check LF device */	
	 if(dev_info[dev_now_id].dev_mac[0] == 0xee && dev_info[dev_now_id].dev_mac[1] ==0xee){
     Cmd_toLFDevice(dev_now_id);
	 }else{
		*(Cmd+4) = 0x07;
		*(Cmd+6) = room_now_id;
		*(Cmd+7) = dev_now_id;
		*(Cmd+8) =  dev_info[dev_now_id].dev_addr[0];
		*(Cmd+9) =  dev_info[dev_now_id].dev_addr[1];
		
		memcpy(Cmd+10, zigbee_macaddr, 8);
		Task_Usart_Send(Cmd);
	}
  
	memset(cmd_reply+3, 0, 21);
/*save device  in sd card*/
  devices_cnt++; 
}

/**
  * @brief  function to delete a device. 
  * @param  
           *Cmd: the pointer of the 24byte date. 
  * @retval None
  */
static void Device_Relate_Delete(INT8U dev_id)
{
  int t = 0;
	
	for(t=0;t < SCENE_CAP ;t++)
	{
		if(scene_info[t].dev_id == dev_id && scene_info[t].scene_id != 0)
		{
			scene_info[t].scene_id = 0;
		}
	}
	for(t=0;t < ALARM_DEV_CAP ;t++)
	{
		if(alarm_dev_info[t].alarm_dev_id == dev_id)
		{
			alarm_dev_info[t].alarm_dev_id = 0;
		}
	}
	
	for(t=0; t < TIMER_CAP ;t++)
	{
		if(task_timer[t].id == dev_id)
		{
			memset(&task_timer[t].enable, 0, sizeof(TIMER));
		}
	}
	

}
static void Device_Delete(INT8U  *Cmd)
{
  INT8U dev_id = *(Cmd+7);
  INT8U room_id = *(Cmd+6);
	
	memset(&dev_info[dev_id].dev_type, 0, sizeof(struct devices));
  devices_cnt--;


  Room_Dev_Delete(room_id, dev_id);
  
  if(room_id == 0xfe)
  {
    room_info[0].dev_cnt--;
  }
  else
  {
    room_info[room_id].dev_cnt--;
  }
	
	Device_Relate_Delete(dev_id);

}

/**
  * @brief  function to add a room's device count. 
  * @param  
            
  * @retval None
  */
void Room_Dev_Add(INT8U room_id, INT8U dev_id)
{
  int t = 0;
  int room_ops = room_id - 1;
  if (room_id == 0xfe) room_ops = 30;

  while(room_ops < 44)
  {
    for (t = 0; t < 20; t++)
    {
      if(room_dev_info[room_ops].dev_id[t] == 0)
      {
  	    room_dev_info[room_ops].dev_id[t] = dev_id;
	    return;
      }
	
    }
	if (room_id == 0xfe) room_ops++;
	
  }

}

/**
  * @brief  function to delete a room's device count. 
  * @param  
           
  * @retval None
  */

static void Room_Dev_Delete(INT8U room_id, INT8U dev_id)
{
  int t = 0;
  int room_ops = room_id - 1;
  if (room_id == 0xfe) room_ops = 30;

  while(room_ops < 44)
  {
    for (t = 0; t < 20; t++)
    {
      if(room_dev_info[room_ops].dev_id[t] == dev_id)
      {
  	    room_dev_info[room_ops].dev_id[t] = 0;
	    return;
      }
	
    }
	if (room_id == 0xfe) room_ops++;

  }
}

/**
  * @brief  to search device. 
  * @param  
           room_id: the id of  a room.
		   ops: the ops of a device. 
  * @retval None
  */

static INT8U Room_Dev_Search(INT8U room_id, INT8U ops)
{
  
  int room_ops = room_id - 1;
  if (room_id == 0xfe) 
  {
    room_ops = ((int)(ops/20))+30;
	ops = ops - (room_ops - 30) * 20;
  }
  
  if(room_dev_info[room_ops].dev_id[ops] != 0)
  {
    return room_dev_info[room_ops].dev_id[ops];
  }
  return 0;
}

/**
  * @brief  To Control whole lamp of a device. 
  * @param  dev_id: the device id.
            status: the lamp status. 
  * @retval None
  */
static void My_Control_DevLamp(INT8U dev_id, INT8U status)
{
  
  INT8U Cmd1[24] = {0xff, 0xfe, 0x15, 0x00, 0x20};
  if(dev_info[dev_id].dev_addr[0] == 0x0 && 
	 dev_info[dev_id].dev_addr[1] == 0x0)
  {
     Get_Dev_Addr(dev_id);
	 return;
  }
  
  Cmd1[6] = dev_info[dev_id].room_id;
  Cmd1[7] = dev_id;
  Cmd1[8] = dev_info[dev_id].dev_addr[0];
  Cmd1[9] = dev_info[dev_id].dev_addr[1];
  Cmd1[10] = 0x04;
  Cmd1[11] = status;

  Task_Usart_Send(Cmd1);
	vTaskDelay(200);
}
/**
  * @brief  update the pcb_list,when new socket is setup or old socket is delete. 
  * @param  *pcb: the poiter of the pcb struct.
            *state: TRUE means socket added, FALSE mead socket deleted. 
  * @retval None
  */

void ForceDelete_PCB(INT8U dev_id){

	int t = 0;
	for(t = 0; t < 10 ; t++)
			 {
	       if(dev_id == pad_info[t].dev_id){
					 tcp_close(pad_info[t].socket);
					 pad_info[t].dev_id = 0;
					 pad_info[t].socket = NULL;
				   pad_info[t].user = FALSE;
					 pad_info[t].pad_living = 0;
					 pad_devices_cnt--;
	       }else if(pad_info[t].dev_id > dev_id){
					 pad_info[t].dev_id--;
					}
	     }

}
void Delete_FreePcb(void)
{ 
	char i=0;
  for(i = 0; i < 10; i++)
	{
		if(pad_info[i].pad_living == 0 && !pad_info[i].user && pad_info[i].dev_id != 0)
		{
			ForceDelete_PCB(pad_info[i].dev_id);
		}
	}
}
			
void Pcb_Update(struct tcp_pcb *pcb ,bool state)
{ 
   int t = 0;
	
   if (state == TRUE)
   {
	   //
		 if(10 == pad_devices_cnt)
		 {
			 ForceDelete_PCB(1);
		 }
		 //
	   for(t = 0; t < 10; t++)
	   {
	     if (pad_info[t].dev_id == 0)
	     {
				 pad_devices_cnt++;
	     pad_info[t].dev_id = pad_devices_cnt;
     	 pad_info[t].socket = pcb;
		   pad_info[t].user = FALSE;
		   pad_info[t].pad_living = 10;
			 
			if(pad_devices_cnt == 10){
				Delete_FreePcb();
			}
		    break;
	     }
	   }
	 
	 	  
   }

   if (state == FALSE)
   {
		 INT8U pad_dev_save = 0;
		 
	   for(t = 0; t < 10 ; t++)
	   {
	    if (pad_info[t].socket == pcb)
	    {
	      if(tcp_device == 	pad_info[t].socket)	tcp_device = NULL;	  
		    pad_dev_save = pad_info[t].dev_id;
		    pad_info[t].dev_id = 0;
     	  pad_info[t].socket = NULL;
		    pad_info[t].user = FALSE;
		    pad_info[t].pad_living = 0;
				
		    break;
	    }

	  }
	 
	   for(t = 0; t < 10 && pad_dev_save > 0; t++)
	   {
	     if(pad_info[t].dev_id > pad_dev_save)
	     {
		     pad_info[t].dev_id--;
	     }
	   
	   }

   }
	
	  pad_devices_cnt = 0;
	  for(t = 0; t < 10 ; t++){
		if( pad_info[t].dev_id != 0){
		pad_devices_cnt++;
		}
	} 

}


void Pcb_Write_Sever(const void *data)
{
  int try_times = 0;
  struct sever_pdu pdu_r = {sever_HEAD1, sever_HEAD2, sever_HEAD3, sever_HEAD4, sever_HEAD5};
  
  if(!net_logined)
  {
    return;
  }
  pdu_r.data_len[0] = 0x19;
  pdu_r.cmd = 0x06;
  pdu_r.cmd_data[0] = 0x18;
  memcpy(&pdu_r.cmd_data[1], (INT8U*)data, 24);
    
  while(tcp_write(sever_pcb, (const void *)&pdu_r, 35, 1)!=ERR_OK && try_times < 500)
  {
    try_times++;
	vTaskDelay(1);
  }
}

/**
  * @brief  reply to the all user who is aliving. 
  * @param 
           *pcb: the poiter of the pcb struct. 
           *Cmd: the pointer of the 24byte date. 
  * @retval None
  */
static void Pcb_Write_ALL(const void *data)
{ 
  int try_times = 0; 
  int t = 0;
    	
  for(t = 0; t < 11; t++)
  {
    if(pad_info[t].user == TRUE && pad_info[t].pad_living>0) 
	{
	  if(pad_info[t].socket != sever_pcb)
	  {
	    while(tcp_write(pad_info[t].socket, data, 24, 1)!= ERR_OK && try_times < 500)
	    {
		    try_times++;
	  	  vTaskDelay(1);
	    }
	  }
    }
  }
  
  Pcb_Write_Sever(data);

}
/**
  * @brief  reply to now user . 
  * @param 
           *pcb: the poiter of the pcb struct. 
           *Cmd: the pointer of the 24byte date. 
  * @retval None
  */
void Pcb_Write(const void *data)
{ 
  int try_times = 0;
	
  if(tcp_device != sever_pcb && tcp_device != NULL)
  {
    while(tcp_write(pad_info[dev_offset].socket, data, 24, 1)!=ERR_OK && try_times < 500)
    {
	  try_times++;
	  vTaskDelay(1);
    }
  }
  else
  {
    Pcb_Write_Sever(data);
  }
}

/**
  * @brief  send 24 byte data  in OS . 
  * @param 
           *cmd_msg: the pointer of the 24byte date. 
  * @retval None
  */

static void Task_Usart_Send(INT8U  *cmd_msg)
{
	UsartDMA_Send(cmd_msg);
  vTaskDelay(80);
}

/**
  * @brief  check the device living. 
  * @param 
  * @retval None
  */

void Dev_Check_Living(void)
{
  int dev_live_ops = 1;
  for(dev_live_ops =1; dev_live_ops < 256; dev_live_ops++)
  {
    if(dev_living[dev_live_ops] > 0x0) 
																
	{
	  dev_living[dev_live_ops] --;
	  
	}

  }
}


static void Get_Dev_Addr(INT8U dev_id)
{
   INT8U cmd[24] = {0xff, 0xfe, 0x15, 0x0, 0xf3};
   memcpy(&cmd[10], &dev_info[dev_id].dev_mac[0], 8);

   Task_Usart_Send(cmd);

}

/**
  * @brief  reset the password . 
  * @param 
          
  * @retval None
  */
void Pass_Reset(void)
{
  INT8U password_set[7]= {0x06, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31};

  memcpy(&sys_info[0].password[0], &password_set[0], 7);

}

/**
  * @brief  Systerm_ERROR reply . 
  * @param 
          
  * @retval None
  */
INT8U err_cmd[24] = {0xff, 0xfe, 0x15, 0x0, 0x31}; 
void Systerm_ERROR(int type)
{ 
  err_cmd[10] = type;
  Pcb_Write_ALL(&err_cmd[0]);
}

void Recieve_UnKnownCmd(void)
{ 
	
}


static CommandStatus Dev_CheckRoom(int devId, int roomId)
{
	int room_ops = roomId;
	if(0xfe == roomId) room_ops = 0;
	
  if(dev_info[devId].dev_id == devId && dev_info[devId].room_id == roomId
																		 && room_info[room_ops].room_id == roomId)
	{
    return SUCESS;
  }else{
	  return FAIL;
	}
}
/**
  * @brief  init the command list . 
  * @param 
  * @retval None
  */
void Cmd_Analysis_Init(void)
{ 
	
	cmd_analysis[0x0].func_cmd = Recieve_UnKnownCmd;
	
  /*查询房间  */
  cmd_analysis[0x01].cmd = 0x01;	   
  cmd_analysis[0x01].func_cmd = Serch_Room_Info;
  /*添加房间  */
  cmd_analysis[0x03].cmd = 0x03;
  cmd_analysis[0x03].func_cmd = Add_Room_Info;
  /*删除房间  */
  cmd_analysis[0x05].cmd = 0x05;
  cmd_analysis[0x05].func_cmd = Delete_Room_Info;
  /*修改房间名  */
  cmd_analysis[0x06].cmd = 0x06;
  cmd_analysis[0x06].func_cmd = Change_Room_Name;
  /*添加设备  */
  cmd_analysis[0x07].cmd = 0x07;
  cmd_analysis[0x07].func_cmd = Device_Add_Info;
  /*删除设备  */
  cmd_analysis[0x08].cmd = 0x08;
  cmd_analysis[0x08].func_cmd = Device_Delet_Info;
  /*修改设备的所在房间 */
  cmd_analysis[0x09].cmd = 0x09;
  cmd_analysis[0x09].func_cmd = Device_Change_RoomID;
  /*获取图片  */
  cmd_analysis[0x0a].cmd = 0x0a;
  cmd_analysis[0x0a].func_cmd = Get_Picture;
  /*添加情景模式  */
  cmd_analysis[0x0d].cmd = 0x0d;
  cmd_analysis[0x0d].func_cmd = Add_Scene;
  /*删除报警设置  */
  cmd_analysis[0x0e].cmd = 0x0e;
  cmd_analysis[0x0e].func_cmd = Del_Alarm_Device;
  /*删除情景模式  */
  cmd_analysis[0x0f].cmd = 0x0f;
  cmd_analysis[0x0f].func_cmd = Delete_Scene;
  /*查询情景模式  */
  cmd_analysis[0x12].cmd = 0x12;
  cmd_analysis[0x12].func_cmd = Serch_Scene;
  /*添加报警设置  */
  cmd_analysis[0x14].cmd = 0x14;
  cmd_analysis[0x14].func_cmd = Add_Alarm_Device;
  /*报警设置查询  */
  cmd_analysis[0x15].cmd = 0x15;
  cmd_analysis[0x15].func_cmd = Search_Alarm_Device;
  /*时间设置 */
  cmd_analysis[0x17].cmd = 0x17;				  
  cmd_analysis[0x17].func_cmd = Time_Set; 
  /*手机授权 查询 */								//未写
  cmd_analysis[0x18].cmd = 0x18;
  cmd_analysis[0x18].func_cmd = Search_Sys_Time;
  /*手机授权 添加 */
  cmd_analysis[0x1a].cmd = 0x1a;					 //未写
  cmd_analysis[0x1a].func_cmd = Search_Sys_Time;
  /*手机授权 删除 */
  cmd_analysis[0x1b].cmd = 0x1b;				  //未写
  cmd_analysis[0x1b].func_cmd = Search_Sys_Time;
  /*时间查询 */
  cmd_analysis[0x1c].cmd = 0x1c;				   
  cmd_analysis[0x1c].func_cmd = Search_Sys_Time;
  /*系统模式查询 */
  cmd_analysis[0x1e].cmd = 0x1e;
  cmd_analysis[0x1e].func_cmd = Search_Sys_Mode;
  /*单个灯控制  */
  cmd_analysis[0x20].cmd = 0x20;
  cmd_analysis[0x20].func_cmd = Control_Single_dev;
  /*设备状态信息反馈  */
  cmd_analysis[0x21].cmd = 0x21;
  cmd_analysis[0x21].func_cmd = Get_Device_State;
  /*房间灯光控制  */
  cmd_analysis[0x22].cmd = 0x22;
  cmd_analysis[0x22].func_cmd = Control_Lamp_Room;
  /*整个屋子灯控制  */
  cmd_analysis[0x23].cmd = 0x23;
  cmd_analysis[0x23].func_cmd = Control_Lamp_All;
  /*调光灯控制  */
  cmd_analysis[0x24].cmd = 0x24;
  cmd_analysis[0x24].func_cmd = Control_Single_dev;
  /*门锁控制  */
  cmd_analysis[0x25].cmd = 0x25;
  cmd_analysis[0x25].func_cmd = Control_Single_dev;
  /*窗控制  */
  cmd_analysis[0x26].cmd = 0x26;
  cmd_analysis[0x26].func_cmd = Control_Single_dev;
  /*窗帘控制  */
  cmd_analysis[0x27].cmd = 0x27;
  cmd_analysis[0x27].func_cmd = Control_Single_dev;
  /*控制插座  */
  cmd_analysis[0x28].cmd = 0x28;
  cmd_analysis[0x28].func_cmd = Control_Single_dev;
  /*同步设备  */
  cmd_analysis[0x29].cmd = 0x29;
  cmd_analysis[0x29].func_cmd = Synchronous_Device;
  /*报警信息  */
  cmd_analysis[0x30].cmd = 0x30;
  cmd_analysis[0x30].func_cmd = Alarm_Info;
	/*从Zigebee获得单个设备状态信息  */
//   cmd_analysis[0x2a].cmd = 0x2a;
//   cmd_analysis[0x2a].func_cmd = Get_device_status;
  /*获得房间设备状态信息  */
  cmd_analysis[0x2b].cmd = 0x2b;
  cmd_analysis[0x2b].func_cmd = Synchronous_Device;
  /*选择情景模式  */
  cmd_analysis[0x2c].cmd = 0x2c;
  cmd_analysis[0x2c].func_cmd = Choose_Scene;
  /*报警使能 */
  cmd_analysis[0x2d].cmd = 0x2d;
  cmd_analysis[0x2d].func_cmd = Enable_Alarm;
  /*阀门控制  */
  cmd_analysis[0x2e].cmd = 0x2e;
  cmd_analysis[0x2e].func_cmd = Control_Single_dev;
  /*报警使能查询*/
  cmd_analysis[0x2f].cmd = 0x2f;
  cmd_analysis[0x2f].func_cmd = Search_Alarm_Enable;
  /*异常状态报告 */
  cmd_analysis[0x31].cmd = 0x31;						  //未写
  cmd_analysis[0x31].func_cmd = Unusual_Message;
  /*设备网络地址报告 */
  cmd_analysis[0x32].cmd = 0x32;
  cmd_analysis[0x32].func_cmd = Reply_dev_netaddress;
//  /*控制设备睡眠报告*/
//  analysis_cmd[0x33].cmd = 0x33;							  //未写
//  analysis_cmd[0x33].func_cmd = Device_Sleep_Message;
  /*查询报警信息 */
  cmd_analysis[0x35].cmd = 0x35;
  cmd_analysis[0x35].func_cmd = Search_Alarm_Record;
  /*短信报警设置 */
  cmd_analysis[0x37].cmd = 0x37;
  cmd_analysis[0x37].func_cmd = Alarm_Phone_message;
   /*清除报警信息 */
  cmd_analysis[0x38].cmd = 0x38;
  cmd_analysis[0x38].func_cmd = Alarm_Record_Clean;
  /*低电压报警信息  */
  cmd_analysis[0x39].cmd = 0x39;
  cmd_analysis[0x39].func_cmd = Alarm_Info;
  /*红外转发 进出学习模式  */
  cmd_analysis[0x3a].cmd = 0x3a;
  cmd_analysis[0x3a].func_cmd = Control_Single_dev;
  /*红外进出学习模式 反馈  */
  cmd_analysis[0x3b].cmd = 0x3b;
  cmd_analysis[0x3b].func_cmd = Return_HongWaiZhuanfa;
  /*红外学习  */
  cmd_analysis[0x3c].cmd = 0x3c;
  cmd_analysis[0x3c].func_cmd = Control_Single_dev;
  /*红外学习反馈  */
  cmd_analysis[0x3d].cmd = 0x3d;
  cmd_analysis[0x3d].func_cmd = Return_HongWaiZhuanfa;
  /*红外控制  */
  cmd_analysis[0x3e].cmd = 0x3e;
  cmd_analysis[0x3e].func_cmd = Control_HongWai;
  /*从Zigbee获得设备信息  */
  cmd_analysis[0x40].cmd = 0x40;
  cmd_analysis[0x40].func_cmd = Device_Get_Into;
  /*设备在线汇报  */
  cmd_analysis[0x43].cmd = 0x43;								  
  cmd_analysis[0x43].func_cmd = Device_Reply_Living;
  /*同步单个PDU设备  */
  cmd_analysis[0x46].cmd = 0x46;
  cmd_analysis[0x46].func_cmd = Synchronous_Device;
  /*排风扇控制  */
  cmd_analysis[0x48].cmd = 0x48;
  cmd_analysis[0x48].func_cmd = Control_Single_dev;
	/*低频转发设备周期性同步*/
	cmd_analysis[0x49].cmd = 0x49;
  cmd_analysis[0x49].func_cmd = LFdevice_Synchronous;
	/*温控器控制*/
	cmd_analysis[0x4b].cmd = 0x4b;
  cmd_analysis[0x4b].func_cmd = Control_Single_dev;
	/*添加定时器设置*/
	cmd_analysis[0x4c].cmd = 0x4c;
  cmd_analysis[0x4c].func_cmd = Add_DevTimer;
	/*删除定时器设置*/
	cmd_analysis[0x4d].cmd = 0x4d;
  cmd_analysis[0x4d].func_cmd = Del_DevTimer;
	/*同步定时器设置*/
	cmd_analysis[0x4e].cmd = 0x4e;
  cmd_analysis[0x4e].func_cmd = Sys_DevTimer;
  /*密码验证  */
  cmd_analysis[0x50].cmd = 0x50;
  cmd_analysis[0x50].func_cmd = Check_secret;
  /*密码修改  */
  cmd_analysis[0x51].cmd = 0x51;
  cmd_analysis[0x51].func_cmd = Change_Secret;
//  /*邮箱设置  */
//  analysis_cmd[0x52].cmd = 0x52;								  //未写
//  analysis_cmd[0x52].func_cmd = Email_Set;

  /*温控器控制*/
  cmd_analysis[0x59].cmd = 0x59;
  cmd_analysis[0x59].func_cmd =Control_Single_dev;
  /*遥控开关设置   */
  cmd_analysis[0x5a].cmd = 0x5a;
  cmd_analysis[0x5a].func_cmd = Control_Single_dev;
  /*遥控开关设置反馈   */
  cmd_analysis[0x5b].cmd = 0x5b;
  cmd_analysis[0x5b].func_cmd = Retern_YaoKongKaiGuan;
  /*初始化 遥控开关*/
  cmd_analysis[0x5c].cmd = 0x5c;				   
  cmd_analysis[0x5c].func_cmd = Init_YaoKongKaiGuan;
  /*反转 遥控开关*/
  cmd_analysis[0x5d].cmd = 0x5d;				   
  cmd_analysis[0x5d].func_cmd = Control_Single_dev;
	
  cmd_analysis[0x5e].cmd = 0x5e;
  cmd_analysis[0x5e].func_cmd = Get_device_status;
}
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/







