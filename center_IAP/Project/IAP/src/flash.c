#include "flash.h"
#include "main.h"
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




