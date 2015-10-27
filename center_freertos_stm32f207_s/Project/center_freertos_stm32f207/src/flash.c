#include "flash.h"
#include "usr_cpu.h"
#include "const.h"
#include <string.h>
#include <stdio.h>
/* Select SPI FLASH: Chip Select pin low  */
#define FLASH_CS_SELECT()      GPIO_ResetBits(FLASH_CS_GPIO_PORT, FLASH_CS_PIN)
/* Deselect SPI FLASH: Chip Select pin high */
#define FLASH_CS_UNSELECT()    GPIO_SetBits(FLASH_CS_GPIO_PORT, FLASH_CS_PIN)  

static void FlashPageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
static u8 FlashSendByte(u8 byte);
static u8 FlashReadByte(void);
static void FlashWaitForWriteEnd(void);
static void FlashWriteEnable(void);

void Flash_SoftInit(void);


void FlashInit(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable SPI2 and GPIO clocks */ 
  RCC_APB1PeriphClockCmd(FLASH_SPI_CLK , ENABLE);
  RCC_AHB1PeriphClockCmd(FLASH_CS_GPIO_CLK|FLASH_SPI_GPIO_CLK,ENABLE);

  GPIO_PinAFConfig(FLASH_SPI_SCK_GPIO_PORT, GPIO_PinSource10, GPIO_AF_SPI2);
  GPIO_PinAFConfig(FLASH_SPI_MISO_GPIO_PORT, GPIO_PinSource2, GPIO_AF_SPI2);
  GPIO_PinAFConfig(FLASH_SPI_MOSI_GPIO_PORT, GPIO_PinSource3, GPIO_AF_SPI2);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;  
  GPIO_InitStructure.GPIO_Pin = FLASH_SPI_SCK_PIN;
  GPIO_Init(FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin =  FLASH_SPI_MOSI_PIN;
  GPIO_Init(FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin =  FLASH_SPI_MISO_PIN;
  GPIO_Init(FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FLASH_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(FLASH_CS_GPIO_PORT, &GPIO_InitStructure);
  

  /* Select the FLASH: Chip Select high */
  FLASH_CS_UNSELECT();

  /* SPI1 configuration */
  // W25X16: data input on the DIO pin is sampled on the rising edge of the CLK. 
  // Data on the DO and DIO pins are clocked out on the falling edge of CLK.
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(FLASH_SPI, &SPI_InitStructure);

  /* Enable SPI1  */
  SPI_Cmd(FLASH_SPI, ENABLE);
}
static void FlashPageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  /* Enable the write access to the FLASH */
  FlashWriteEnable();
 
  /* Select the FLASH: Chip Select low */
  FLASH_CS_SELECT();
  /* Send "Write to Memory " instruction */
  FlashSendByte(W25X_PageProgram);
  /* Send WriteAddr high nibble address byte to write to */
  FlashSendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  FlashSendByte((WriteAddr & 0xFF00) >> 8);
  /* Send WriteAddr low nibble address byte to write to */
  FlashSendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;     
  }

  /* while there is data to be written on the FLASH */
  while (NumByteToWrite--)
  {
    /* Send the current byte */
    FlashSendByte(*pBuffer);
    /* Point on the next byte to be written */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  FLASH_CS_UNSELECT();

  /* Wait the end of Flash writing */
  FlashWaitForWriteEnd();
}

void FlashBufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % SPI_FLASH_PageSize;
  count = SPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

  if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      FlashPageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      while (NumOfPage--)
      {
        FlashPageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      FlashPageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
      {
        temp = NumOfSingle - count;

        FlashPageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        FlashPageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        FlashPageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      FlashPageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        FlashPageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      if (NumOfSingle != 0)
      {
        FlashPageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}
void FlashBufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  FLASH_CS_SELECT();

  /* Send "Read from Memory " instruction */
  FlashSendByte(W25X_ReadData);

  /* Send ReadAddr high nibble address byte to read from */
  FlashSendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  FlashSendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  FlashSendByte(ReadAddr & 0xFF);

  while (NumByteToRead--) /* while there is data to be read */
  {
    /* Read a byte from the FLASH */
    *pBuffer = FlashSendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  FLASH_CS_UNSELECT();
}

static u8 FlashSendByte(u8 byte)
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(FLASH_SPI, byte);

  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(FLASH_SPI);
}

static u8 FlashReadByte(void)
{
  return (FlashSendByte(Dummy_Byte));
}
static void FlashWaitForWriteEnd(void)
{
  u8 FLASH_Status = 0;

  /* Select the FLASH: Chip Select low */
  FLASH_CS_SELECT();

  /* Send "Read Status Register" instruction */
  FlashSendByte(W25X_ReadStatusReg);

  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    FLASH_Status = FlashSendByte(Dummy_Byte);

  }
  while ((FLASH_Status & WIP_Flag) == SET); /* Write in progress */

  /* Deselect the FLASH: Chip Select high */
  FLASH_CS_UNSELECT();
}

static void FlashWriteEnable(void)
{
  /* Select the FLASH: Chip Select low */
  FLASH_CS_SELECT();

  /* Send "Write Enable" instruction */
  FlashSendByte(W25X_WriteEnable);

  /* Deselect the FLASH: Chip Select high */
  FLASH_CS_UNSELECT();
}

void FlashSectorErase(u32 SectorAddr,u32 SectorNum)
{
  while(SectorNum > 0)
  {
  /* Send write enable instruction */
  FlashWriteEnable();
  FLASH_CS_SELECT();

  /* Sector Erase */
  
    /* Send Sector Erase instruction */
    FlashSendByte(W25X_SectorErase);
    /* Send SectorAddr high nibble address byte */
    FlashSendByte((SectorAddr & 0xFF0000) >> 16);
    /* Send SectorAddr medium nibble address byte */
    FlashSendByte((SectorAddr & 0xFF00) >> 8);
    /* Send SectorAddr low nibble address byte */
    FlashSendByte(SectorAddr & 0xFF);
    
		SectorAddr += 0x1000;
		
  /* Deselect the FLASH: Chip Select high */
  FLASH_CS_UNSELECT();

  /* Wait the end of Flash writing */
  FlashWaitForWriteEnd();
  SectorNum--;
  }
}

u32 FlashReadID(void)
{
  u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /* Select the FLASH: Chip Select low */
  FLASH_CS_SELECT();

  /* Send "RDID " instruction */
  FlashSendByte(W25X_JedecDeviceID);

  /* Read a byte from the FLASH */
  Temp0 = FlashSendByte(Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp1 = FlashSendByte(Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp2 = FlashSendByte(Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  FLASH_CS_UNSELECT();

  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

  return Temp;
}
/******************************************************************************/

static char godrig[6] = {0x67, 0x6f, 0x64, 0x72, 0x69, 0x67};
static bool use_first = FALSE; 
bool center_NoID = FALSE;
void check_sys_id(void)
{
  if(sys_info[0].serial_num[0] == 0x0 || sys_info[0].secret_num[0] == 0x0 || use_first)
  {
    center_NoID = TRUE;
  }
	else
  {
    center_NoID = FALSE;
  }
}


void Mem_Check(void)
{
  int t = 0;
	for(t=0;t<ALARM_DEV_CAP; t++){
    if(dev_info[alarm_dev_info[t].dev_act_id].dev_id != alarm_dev_info[t].dev_act_id){
		  alarm_dev_info[t].alarm_dev_id = 0;
		}
		
	}
	
	for(t=0;t<SCENE_CAP; t++){
    if(dev_info[scene_info[t].dev_id].dev_id != scene_info[t].dev_id){
		  scene_info[t].scene_id = 0;
		}
		
	}
	
	for(t=0;t<DEVICES_CAP; t++){
    if(dev_info[t].dev_id != t || !((dev_info[t].room_id >0 && dev_info[t].room_id <= 30) || dev_info[t].room_id == 0xfe)){
		  memset(&dev_info[t].dev_type, 0, sizeof(struct devices));
		}
		
	}
	
	for(t=0;t<TIMER_CAP; t++){
		if(task_timer[t].enable !=0 && task_timer[t].enable != 1){
      memset(&task_timer[t].enable, 0, sizeof(struct timer));
    }
		if(dev_info[task_timer[t].id].dev_id != task_timer[t].id)
		{
		 memset(&task_timer[t].enable, 0, sizeof(TIMER));
		}
  }
	
}

void My_Flash_Init(void)
{  
  int t = 0;
  FlashInit();

  FlashBufferRead((u8*)&sys_info[0], zone1_start, sizeof(struct my_systerm));

  for(t=0; t < 6; t++)
  {
    if(sys_info[0].sign[t]!= godrig[t])
		{
			use_first = TRUE;  
		}
  }
 
  
  if(!use_first)
  { 
    FlashBufferRead((u8*)&dev_info[0], zone2_start, DEVICES_CAP * sizeof(struct devices));
    FlashBufferRead((u8*)&room_info[0], zone3_start, ROOMS_CAP * sizeof(struct rooms));
    FlashBufferRead((u8*)&alarm_dev_info[0], zone4_start, ALARM_DEV_CAP * sizeof(struct alarm_dev));
    FlashBufferRead((u8*)&scene_info[0], zone5_start, SCENE_CAP * sizeof(struct scene));
		FlashBufferRead((u8*)&task_timer[0], zone6_start, TIMER_CAP * sizeof(struct timer));
  }
	
  room_info[0].room_id = 0xfe;
  room_info[0].dev_cnt = 0;
  
	Mem_Check();
	
  for(t = 0; t < ROOMS_CAP; t++)
  {
    if (room_info[t].room_id == t || room_info[t].room_id == 0xfe)
    {
	    rooms_cnt++;
	  }
  }
   

  for(t = 1; t < DEVICES_CAP; t++)
  {
    if (dev_info[t].dev_id == t)
    {
	    devices_cnt++;
	    Room_Dev_Add(dev_info[t].room_id, dev_info[t].dev_id);
	  }
	
	  if (dev_info[t].room_id == 0xfe  && dev_info[t].dev_id == t)
    { 							    
	    room_info[0].dev_cnt++;
	  }

	  dev_info[t].dev_addr[0] = 0x0;
	  dev_info[t].dev_addr[1] = 0x0;
	  
	  if(dev_info[t].dev_type != 0x18 && dev_info[t].dev_type != 0x1c)
	  {
	    dev_info[t].dev_status[0] = 0x0;
	    dev_info[t].dev_status[1] = 0x0;
			dev_info[t].dev_status[2] = 0x0;
    }
	}
  
	memset(dev_living, 0x3, 256);
  	
	sys_info[0].time_mode = 0x1;
	
	check_sys_id();
	
	/*IAP version read*/
	FlashBufferRead(iap_version, zone_iap_version, 4);
	if(iap_version[0] != 0x31){
		u8 version1[4]={0x31, 0x30, 0x30, 0x30};
	  FlashSectorErase(zone_iap_version, 1);
		FlashBufferWrite(version1, zone_iap_version, 4);
		memcpy(iap_version, version1, 4);
	}
	
	if(1)
	{
		unsigned char hard_default_msg[512];
		int t = 0;
		uint32_t Address = 0x080E0000;
		for(t = 0; t < 512; t++)
  {
    hard_default_msg[t] = *(__IO uint32_t*)Address;
		Address = Address + 1;
	}
	
		if(hard_default_msg[0] == 0x31){
			printf(&hard_default_msg[1]);
			hard_default_msg[510] = 0xd;
			hard_default_msg[511] = 0xa;
		  Wrtie_hardFault_state(&hard_default_msg[1]);
			memset(hard_default_msg, 0, 512);
			Write_HardFault_State(hard_default_msg, 0);
		}
	} 
}

 
void HardFault_Handler_copy(unsigned int * hardfault_args)
{
  
	char wsd[512];

	
	unsigned int stacked_r0;
  unsigned int stacked_r1;
  unsigned int stacked_r2;
  unsigned int stacked_r3;
  unsigned int stacked_r12;
  unsigned int stacked_lr;
  unsigned int stacked_pc;
  unsigned int stacked_psr;
 
	memset(wsd, 0, 512);
	wsd[0] =0x31;
	
  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);
 
  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
	 
  stacked_psr = ((unsigned long) hardfault_args[7]);
  sprintf(&wsd[1], "R0=%x\r\n R1=%x\r\n R2=%x\r\n R3=%x\r\n R12=%x\r\n LR=%x\r\n PC=%x\r\n PSR=%x\r\n BFAR=%x\r\n CFSR=%x\r\n HFSR=%x\r\n DFSR=%x\r\n AFSR=%x\r\n SCB_SHCSR=%x\r\n",
									stacked_r0, stacked_r1, stacked_r2, stacked_r3, stacked_r12, stacked_lr, stacked_pc, stacked_psr,
									(*((volatile unsigned long *)(0xE000ED38))),
									(*((volatile unsigned long *)(0xE000ED28))),
									(*((volatile unsigned long *)(0xE000ED2C))),
									(*((volatile unsigned long *)(0xE000ED30))),
									(*((volatile unsigned long *)(0xE000ED3C))),
									SCB->SHCSR)	;
							 
							
									Write_HardFault_State(wsd, 1);
						
}


void My_Flash_Write(int type)
{ 
	int t = 0;
	for(t=0; t < 6; t++)
  {
      if(sys_info[0].sign[t] != godrig[t]) return;		
	}	

  switch(type){
    
	case DEVICE_TYPE: 					  
      FlashSectorErase(zone2_start, zone2_size/0x1000);
      FlashBufferWrite((u8*)&dev_info[0], zone2_start, DEVICES_CAP * sizeof(struct devices));
	  break;
    case ROOM_TYPE: 
      FlashSectorErase(zone3_start, zone3_size/0x1000);
      FlashBufferWrite((u8*)&room_info[0], zone3_start, ROOMS_CAP * sizeof(struct rooms));	
	  break;
    case WARN_DEV_TYPE: 
	    FlashSectorErase(zone4_start, zone4_size/0x1000);
      FlashBufferWrite((u8*)&alarm_dev_info[0], zone4_start, ALARM_DEV_CAP * sizeof(struct alarm_dev));
	   break;				  							
    case SCENE_TYPE:  
  	   FlashSectorErase(zone5_start, zone5_size/0x1000);
       FlashBufferWrite((u8*)&scene_info[0], zone5_start, SCENE_CAP * sizeof(struct scene));
	   break;
    case PASS_TYPE: 
	     FlashSectorErase(zone1_start, zone1_size/0x1000);
       FlashBufferWrite((u8*)&sys_info[0], zone1_start, sizeof(struct my_systerm));
	   break;
		case DEV_TIMER_TYPE: 
	     FlashSectorErase(zone6_start, zone6_size/0x1000);
       FlashBufferWrite((u8*)&task_timer[0], zone6_start, TIMER_CAP*sizeof(struct timer));
	   break;
	default:return;	
  }

}

void My_Clean_Data(void)
{
	if(use_first)
	{ 
		memset(&sys_info[0], 0x0, sizeof(struct my_systerm));
	  memcpy(&sys_info[0].sign, godrig, 6); 
		My_Flash_Write(PASS_TYPE);
		FlashBufferRead((u8*)&sys_info[0], zone1_start, sizeof(struct my_systerm));
		if(memcmp(&sys_info[0].sign[0], godrig, 6)) 
			printf("flash err\n");
	}
	
  memset(&dev_info[0], 0x0, DEVICES_CAP * sizeof(struct devices));
  memset(&room_info[0], 0x0, ROOMS_CAP * sizeof(struct rooms));
  memset(&alarm_dev_info[0], 0x0, ALARM_DEV_CAP * sizeof(struct alarm_dev));
  memset(&scene_info[0], 0x0, SCENE_CAP * sizeof(struct scene));
	memset(&task_timer[0], 0x0, TIMER_CAP * sizeof(struct timer));
  My_Flash_Write(DEVICE_TYPE);
  My_Flash_Write(ROOM_TYPE);
  My_Flash_Write(WARN_DEV_TYPE);
  My_Flash_Write(SCENE_TYPE);
	My_Flash_Write(DEV_TIMER_TYPE);
}

void Soft_Clean(void){
	u8 clean[3] ={0x0,0x0,0x0};
  FlashSectorErase(zone_softlen, 1);
	FlashBufferWrite((u8*)&clean[0], zone_softlen, 3);
}

void Flash_SoftInit(void){
	FlashSectorErase(zone_soft, zone_softsize/0x1000);
}

void Flash_SoftWrite(void  *data, 
										int offset, 
										int cnt)
{  
   FlashBufferWrite((u8*)data, zone_soft + offset, cnt);
}



extern INT8U soft_md5[17];
void Flash_SoftLen(int len)
{
	u8 softLen[3];
	softLen[2] = (u8)(len>>16);
	softLen[1] = (u8)(len>>8);
	softLen[0] = (u8)(len);
	
	FlashSectorErase(zone_softlen, 1);
	FlashBufferWrite((u8*)softLen, zone_softlen, 3);
}






