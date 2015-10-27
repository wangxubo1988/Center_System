#ifndef FLASH_H
#define FLASH_H
#include "stm32f2xx.h"
#include "net_server.h"
#include "main_time.h"

#define FLASH_SPI                     SPI2
#define FLASH_SPI_CLK                 RCC_APB1Periph_SPI2
#define FLASH_SPI_SCK_PIN             GPIO_Pin_10              /* PA.05 */
#define FLASH_SPI_SCK_GPIO_PORT       GPIOB                   /* GPIOA */

#define FLASH_SPI_MISO_PIN            GPIO_Pin_2              /* PA.06 */
#define FLASH_SPI_MISO_GPIO_PORT      GPIOC                   /* GPIOA */

#define FLASH_SPI_MOSI_PIN            GPIO_Pin_3              /* PA.07 */
#define FLASH_SPI_MOSI_GPIO_PORT      GPIOC                   /* GPIOA */
#define FLASH_SPI_GPIO_CLK       RCC_AHB1Periph_GPIOC

#define FLASH_CS_PIN                         GPIO_Pin_13             /* PD.11 */
#define FLASH_CS_GPIO_PORT                   GPIOC                   /* GPIOD */
#define FLASH_CS_GPIO_CLK                    RCC_AHB1Periph_GPIOC



#define SPI_FLASH_PageSize      256
#define SPI_FLASH_PerWritePageSize      256
#define WIP_Flag                0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte              0xA5

#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 
/*.*/
#define  zone1_start 0x0 
#define  zone1_size 0x1000
#define  zone2_start  0x1000
#define  zone2_size 0x1000
#define  zone3_start 0x2000
#define  zone3_size 0x1000
#define  zone4_start 0x3000
#define  zone4_size 0x5000
#define  zone5_start  0x8000
#define  zone5_size 0x5000
#define  zone6_start 0xd000
#define  zone6_size 0x1000

#define zone_soft 0x10000
#define zone_softsize 0x20000
#define zone_softlen 0x30000

#define zone_iap_version 0x31000

extern bool center_NoID;
extern bool iap_first;

void My_Clean_Data(void);
void My_Flash_Write(int type);
void My_Flash_Init(void);
void check_sys_id(void);
void My_Flash_Write(int type);



#endif

