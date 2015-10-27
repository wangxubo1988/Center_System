#ifndef __USART_DMA_H__
#define __USART_DMA_H__



extern void  USART_DMA_test(void);
#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
//#include "stm32_eval.h"
//#include "stm322xg_eval_ioe.h"

/* Exported typedef ----------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

typedef unsigned char INT8U;

/* Exported define -----------------------------------------------------------*/

/* USER_TIMEOUT value for waiting loops. This timeout is just guarantee that the
   application will not remain stuck if the USART communication is corrupted. 
   You may modify this timeout value depending on CPU frequency and application
   conditions (interrupts routines, number of data to transfer, baudrate, CPU
   frequency...). */ 
#define USER_TIMEOUT                    ((uint32_t)0x64) /* Waiting 1s */

/* Definition for USART2 resources *   pA2 pA3*******************************************/
#define USARTx                           USART2
#define USARTx_CLK                       RCC_APB1Periph_USART2
#define USARTx_CLK_INIT                  RCC_APB1PeriphClockCmd
#define USARTx_IRQn                      USART2_IRQn
#define USARTx_IRQHandler                USART2_IRQHandler

#define USARTx_TX_PIN                    GPIO_Pin_5                
#define USARTx_TX_GPIO_PORT              GPIOD                       
#define USARTx_TX_GPIO_CLK               RCC_AHB1Periph_GPIOD
#define USARTx_TX_SOURCE                 GPIO_PinSource5
#define USARTx_TX_AF                     GPIO_AF_USART2

#define USARTx_RX_PIN                    GPIO_Pin_6                
#define USARTx_RX_GPIO_PORT              GPIOD                   
#define USARTx_RX_GPIO_CLK               RCC_AHB1Periph_GPIOD
#define USARTx_RX_SOURCE                 GPIO_PinSource6
#define USARTx_RX_AF                     GPIO_AF_USART2

/* Definition for DMAx resources **********************************************/
#define USARTx_DR_ADDRESS                ((uint32_t)USART2 + 0x04) 

#define USARTx_DMA                       DMA1
#define USARTx_DMAx_CLK                  RCC_AHB1Periph_DMA1
   
#define USARTx_TX_DMA_CHANNEL            DMA_Channel_4
#define USARTx_TX_DMA_STREAM             DMA1_Stream6
#define USARTx_TX_DMA_FLAG_FEIF          DMA_FLAG_FEIF6
#define USARTx_TX_DMA_FLAG_DMEIF         DMA_FLAG_DMEIF6
#define USARTx_TX_DMA_FLAG_TEIF          DMA_FLAG_TEIF6
#define USARTx_TX_DMA_FLAG_HTIF          DMA_FLAG_HTIF6
#define USARTx_TX_DMA_FLAG_TCIF          DMA_FLAG_TCIF6
            
#define USARTx_RX_DMA_CHANNEL            DMA_Channel_4
#define USARTx_RX_DMA_STREAM             DMA1_Stream5
#define USARTx_RX_DMA_FLAG_FEIF          DMA_FLAG_FEIF5
#define USARTx_RX_DMA_FLAG_DMEIF         DMA_FLAG_DMEIF5
#define USARTx_RX_DMA_FLAG_TEIF          DMA_FLAG_TEIF5
#define USARTx_RX_DMA_FLAG_HTIF          DMA_FLAG_HTIF5
#define USARTx_RX_DMA_FLAG_TCIF          DMA_FLAG_TCIF5

#define USARTx_DMA_TX_IRQn               DMA1_Stream6_IRQn
#define USARTx_DMA_RX_IRQn               DMA1_Stream5_IRQn
#define USARTx_DMA_TX_IRQHandler         DMA1_Stream6_IRQHandler
#define USARTx_DMA_RX_IRQHandler         DMA1_Stream5_IRQHandler   

/* Misc definition ************************************************************/
/* Transmit buffer size */
#define TXBUFFERSIZE                    (countof(TxBuffer_dma) - 1)
/* Receive buffer size */
#define RXBUFFERSIZE                     TXBUFFERSIZE

/* Commands list */
#define CMD_RIGHT                        0x55
#define CMD_LEFT                         0xAA
#define CMD_UP                           0x33
#define CMD_DOWN                         0xCC
#define CMD_SEL                          0xFF 
#define CMD_ACK                          0x66 

/* Number of data bytes for each command */
#define CMD_RIGHT_SIZE                   0x01
#define CMD_LEFT_SIZE                    0x05
#define CMD_UP_SIZE                      0x14
#define CMD_DOWN_SIZE                    0x1E
#define CMD_SEL_SIZE                     TXBUFFERSIZE




/* Exported types ------------------------------------------------------------*/
typedef enum {CLOSE = 0, WAIT = 1, OPEN = 2} UsartState;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
extern bool data_complete;
extern int tx_now_send ;
extern int tx_count ;

extern uint8_t  TxBuffer_dma_save[10][24];
extern uint8_t  TxBuffer_dma[24];

extern uint8_t RxBuffer_dma [24];
extern uint8_t  RxBuffer_dma_save[240];
extern uint8_t  RxBuffer_rigth_save[24];

extern uint32_t  rx_head; 
extern INT8U data_count;

/* Exported functions ------------------------------------------------------- */

void Usart_Dma_Analysis(void * pvParameters);
void UsartDMA_Send(INT8U *data);
void  USART_DMA_Config(void);
void Usart_GPIO_Set(UsartState  status);
void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLength, const INT8U *data);
void Usart_StateClear(void);


#ifdef __cplusplus
}
#endif

#endif
