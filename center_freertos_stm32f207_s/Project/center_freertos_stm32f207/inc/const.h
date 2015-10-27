/*
*   文件名: Const.h
*   描  述: 
*   作  者: 毛维波
*   创建日期: 2011-11-5
*   版权信息: Copyright (c) 2011-2012 中国钓具技术标准化（北仑海伯）研究中心 All rights reserved
*   
*/

#ifndef CONST_H_
#define CONST_H_
//#include "NVStorage.h"
//#include "CustomMemory.h"

/*command from remote controller */
#define CMD_ROOM_QUERY    0x01
#define CMD_ROOM_ADD    0x03
#define CMD_ROOM_DEL    0x05
#define CMD_ROOM_CHANGE    0x06
#define CMD_SINGLE_ROOM_QUERY 0x44
#define CMD_DEVICE_ONLINE_REPORT 0x43

#define CMD_DEVICE_ADD    0x07
#define CMD_DEVICE_DEL    0x08
#define CMD_DEVICE_CHANGE    0x09

#define CMD_SCENARIO_ADD    0x0a
#define CMD_SCENARIO_DEL    0x0b
#define CMD_SCENARIO_CHANGE    0x0c
#define CMD_SCENARIO_OPADD    0x0d
#define CMD_SCEARIO_OPDEL    0x0f
#define CMD_SCENARIO_QUERY    0x10
#define CMD_SCENARIO_OPQUERY    0x12
#define CMD_SCENARIO_ACTION    0x2c

#define CMD_WARN_OPADD     0x14
#define CMD_WARN_OPDEL    0x0e
#define CMD_WARN_QUERY   0x15
#define CMD_WARN_SMSENABLE   0x37
#define CMD_WARN_ENABLE  0x2d
#define CMD_WARN_ENABLE_QUERY 0x2f
#define CMD_WARN_RECORD_QUERY 0x35
#define CMD_WARN_RECORD_RESP 0x36
#define CMD_WARN_RECORD_DEL 0x38


#define CMD_TIME_SET    0x17
#define CMD_TIME_QUERY    0x1c

#define CMD_MODE_QUERY    0x1e

#define CMD_LIGHT_CTRL    0x20
#define CMD_ROOM_LIGHT_CTRL    0x22
#define CMD_HOUSE_LIGHT_CTRL    0x23
#define CMD_ADJ_LIGHT_CTRL    0x24
#define CMD_DOOR_CTRL    0x25
#define CMD_WINDOW_CTRL    0x26
#define CMD_CURTAIN_CTRL    0x27
#define CMD_OUTLET_CTRL    0x28
#define CMD_VALVE_CTRL     0x2e
#define CMD_FAN_CTRL      0x48
#define CMD_ROOM_DEVICE_QUERY    0x29
#define CMD_ROOM_DEVICE_STATE_QUERY 0x2b
#define CMD_ROOM_SINGLE_PDU_DEVICE_QUERY 0x46

#define CMD_PHONE_QUERY    0x18
#define CMD_PHONE_RESP    0x19
#define CMD_PHONE_ADD    0x1a
#define CMD_PHONE_DEL    0x1b

#define CMD_NWK_REPORT 0x32
#define CMD_SLEEP_REPORT 0x33

#define CMD_SMS_STATE_QUERY 0x60

#define CMD_AUTH_REQUEST 0x50
#define CMD_CHANGE_PASS 0x51
#define CMD_CHANGE_EMAIL 0x52

#define CMD_FINDER_QUERY 0x53
#define CMD_FINDER_RESP 0x54

#define CMD_IR_LEARN_MODEL 0x3a
#define CMD_IR_LEARN_MODEL_RESP 0x3b
#define CMD_IR_LEARN_REQUEST 0x3c
#define CMD_IR_LEARN_RESP 0x3d
#define CMD_IR_CONTROL 0x3e
#define CMD_IR_QUERY 0x3f

/*command from center controller*/
#define CMD_ROOM_INFO    0x02
#define CMD_SINGLE_ROOM_INFO 0x45
#define CMD_CENTER_RESP    0x04
#define CMD_DEVICE_SET_NOTIFY    0x40
#define CMD_SCENARIO_RESP    0x11
#define CMD_SCENARIO_OPRESP    0x13
#define CMD_WARN_RESP    0x16
#define CMD_TIME_RESP    0x1d
#define CMD_MODE_RESP    0x1f

#define CMD_DEVICE_STATE_RESP    0x21
#define CMD_ROOM_DEVICE_RESP    0x41
#define CMD_ROOM_SINGLE_PDU_DEVICE_RESP 0x47
#define CMD_ROOM_DEVICE_STATE_RESP   0x42
#define CMD_WARN_ALARM    0x30
#define CMD_WARN_ENABLE_RESP 0x34
#define CMD_LOW_POWER_ALARM 0x39
#define CMD_ABNORMAL_REPORT    0x31

#define CMD_MODEL_SET 0xf0
#define CMD_KEEPALIVE 0xf1
#define CMD_NET_STATE 0xf2
#define CMD_DEVICE_ADDR 0xf3
#define CMD_COOR_MAC 0xf4

#define CMD_DEVICE_QUERY 0x2a

#define CMD_REMOTE_SWITCH_SET 0x5a
#define CMD_REMOTE_SWITCH_RESP 0x5b
#define CMD_REMOTE_SWITCH_INIT 0x5c
#define CMD_LIGHT_REVERSE      0x5d
/*pc command*/
#define CMD_SERIAL_SET 0x55
#define CMD_SERIAL_RESP 0x56
#define CMD_SECRET_SET 0x57
#define CMD_SECRET_RESP 0x58

/*sms command*/
#define SMS_CLOSE_LIGHTS "706F51685173"
#define SMS_OPEN_DOOR "5F0095E8"
#define SMS_CLOSE_DOOR "517395E8"
#define SMS_OPEN_WINDOW "5F007A97"
#define SMS_CLOSE_WINDOW "51737A97"
#define SMS_OPEN_CURTAIN "5F007A975E18"
#define SMS_CLOSE_CURTAIN "51737A975E18"
#define SMS_OPEN_OUTLET   "5F0063D25EA7"
#define SMS_CLOSE_OUTLET  "517363D25EA7"
#define SMS_QUERY_STATE "67E58BE272B66001"
/*sms state report*/
#define SMS_COMMA  "FF0C"
#define SMS_LIGHT_OFF "706F51685173"
#define SMS_LIGHT_ON "6709706F672A5173"
#define SMS_OUTLET_OFF "63D25EA751685173"
#define SMS_OUTLET_ON "670963D25EA7672A5173"
#define SMS_WINDOW_CLOSE "7A9751685173"
#define SMS_WINDOW_OPEN "67097A97672A5173"
#define SMS_CURTAIN_CLOSE "7A975E1851685173"
#define SMS_CURTAIN_OPEN "67097A975E18672A5173"
#define SMS_DOOR_OPEN "95E85F00"
#define SMS_DOOR_CLOSE "95E85173"
/*sms warning*/
#define SMS_IR_ALARM "7EA2591662A58B66"
#define SMS_WARTER_ALARM "6C346DF962A58B66"
#define SMS_GAS_ALARM "71646C1462A58B66"
#define SMS_SMOKE_ALARM "70DF96FE62A58B66"
#define SMS_LOW_POWER_ALARM "75356C60753591CF4F4E"
#define SMS_TEMPERATURE_ALARM "6E295EA662A58B66"
#define ALL_PHONE_NUM "ALL"

#define ROOM_TYPE    1
#define DEVICE_TYPE    2
#define PHONE_TYPE   3
#define SCENE_TYPE 4
#define WARN_TYPE 5
#define TIME_TYPE 6
#define SECRET_TYPE 7
#define PASS_TYPE 8
#define SERIAL_TYPE 9
#define EMAIL_TYPE 10
#define WARN_DEV_TYPE 11
#define DEV_TIMER_TYPE 12

#define SD_ERR_TYPE 2
#define FLASH_ERR_TYPE 3

#define ROOMDATA_LEN    16
#define ROOM_NAME_LEN 12
#define DEVICEDATA_LEN    16
#define PHONEDATA_LEN 32
#define PDU_LEN    24
#define SRC_DEVICEID_POS_PDU    3
#define CMDID_POS_PDU    4
#define ROOMID_POS_PDU    6
#define DEVICEID_POS_PDU    7
#define DEVICEADDR_POS_PDU    8
#define DATA_POS_PDU    10
#define DATA_LEN_PDU    14
#define HEAD1_PDU 0xff
#define HEAD2_PDU 0xfe
#define HEAD3_PDU 0x15
#define MAC_ADDR_LEN    8
#define WARN_RECORD_LEN 10
#define MAX_WARN_RECORD_NUM 200

#define REMOTE_PDU_LEN 40
#define IMAGE_PDU_LEN 600

#define ANY_DEVICEID    0xff

#define NONE_ROOMID    0xfe
#define NONE_ROOMNAME "公共房间"

#define MAX_SERIAL_LEN 20
#define MIN_PASS_LEN 6
#define MAX_PASS_LEN 12
#define SECRET_LEN   16
/* Device type */
#define  DEVICE_TYPE_LIGHT_III    0x01
#define  DEVICE_TYPE_REMOTE        0x02
#define  DEVICE_TYPE_SENSOR_TEMPERATURE       0x03
#define  DEVICE_TYPE_SENSOR_WATERLEAK       0x04
#define  DEVICE_TYPE_SENSOR_IR        0x05
#define  DEVICE_TYPE_SENSOR_SMOKE         0x06
#define  DEVICE_TYPE_SENSOR_GAS         0x07
#define  DEVICE_TYPE_ELDERLY_ASSISTANT        0x08
#define  DEVICE_TYPE_CURTAIN        0x09
#define  DEVICE_TYPE_WINDOW        0x0a
#define  DEVICE_TYPE_OUTLET         0x0b
#define  DEVICE_TYPE_LIGHT_ADJ         0x0c
#define  DEVICE_TYPE_DOOR         0x0d
#define  DEVICE_TYPE_SENSOR_WIND         0x0e
#define  DEVICE_TYPE_WATER_VALVE          0x0f
#define  DEVICE_TYPE_FAN       0x10
#define  DEVICE_TYPE_LIGHT_II 0x11
#define  DEVICE_TYPE_LIGHT_I 0x12
#define  DEVICE_TYPE_PAD 0x13
#define  DEVICE_TYPE_GAS_VALVE 0x14
#define  DEVICE_TYPE_SENSOR_LIGHT 0x15
#define  DEVICE_TYPE_IR_RELAY 0x18
#define  DEVICE_TYPE_WEATHER  0x19
#define  DEVICE_TYPE_WARNOUTLET 0x1a
#define  DEVICE_TYPE_REMOTE_SWITCH 0x1c

#define LIGHTS_ALL    0x04

enum EFlagType{ZIGBEE_FLAG,ALL_FLAG,GPRS_FLAG,WIFI_FLAG,MODEL_FLAG,STATE_FLAG,INTERNAL_FLAG, REMOTE_FLAG};
enum EModelState{NORMAL_STATE, CONFIG_STATE, RESTORE_STATE};
//extern bool gRunThread;
//extern bool gSystemReady;
//extern NVStorage *gpStorage;
//extern CustomMemory *gpMemory;
//void ExitProcess();


#define  MAX_EVENTS 20

#define ZIGBEE_COM_PORT 1
#define GPRS_COM_PORT 2

/*gpio define*/
#define GPRS_GPIO "/dev/gppio"
#define GPRS_POWER 11
#define GPRS_PWRKEY 12
#define GPRS_VDD_EXT 14

#define ZIGBEE_GPIO "/dev/gpmio"
#define ZIGBEE_POWER 2

#define RIGHT_STATE 0
#define ERROR_STATE 1

#define ENABLE_WARN 0
#define DISABLE_WARN 1

#define DEVICE_WARN 1
#define DEVICE_UNWARN 0
#define WARN_INTERVAL 3

#define ABNORMAL_STATE_LOW_POWER 0x01
#define ABNORMAL_STATE_ADAPTER_POWER 0x05
#define ABNORMAL_STATE_BATTERY_POWER 0x06
struct IRoom
{
    char cRoomId;
    char acRoomName[ROOMDATA_LEN-1];
};

struct stPDU
{
    char head1;
    char head2;
    char len;
    char srcID;
    char cmd[2];
    char roomID;
    char dstID;
    char dstAddr[2];
    char content[14];
};


#define ACTION_LEN 5
#define WARN_ACTION_LEN 7
#define TEMPERATURE_WARN_VALUE 60
#define UPDATE_TIME_MASK 0x7FFFFFFF
#define MAX_ONLINE_REPORT_INTERVAL 300
#define DEVICE_NOT_ONLINE 0xe8
#endif /* CONST_H_ */
