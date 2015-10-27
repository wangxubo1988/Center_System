#include "usart_dma.h"
#include "usr_cpu.h"
#include "FreeRTOS.h"
#include "task.h"
/* Private typedef -----------------------------------------------------------*/
DMA_InitTypeDef  DMA_InitStructure;
GPIO_InitTypeDef Zigbee_InitStructure;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
bool data_complete = TRUE;

int tx_now_send = 0;
int tx_count = 0;

uint8_t  TxBuffer_dma_save[10][24];
uint8_t  TxBuffer_dma[24];

uint8_t RxBuffer_dma [24];
uint8_t  RxBuffer_dma_save[240];
uint8_t  RxBuffer_rigth_save[24];

uint32_t  rx_head = 0; 
INT8U data_count = 0;

INT8U rx_analysis_head = 0;

/* Private function prototypes -----------------------------------------------*/
static void USART_Config(void);
/*******************************************************************************/

void Usart_Dma_Analysis(void * pvParameters)
{ 
	int t = 0;
  INT8U  *usart_msg = NULL;
  
  INT8U head2;
  INT8U head3;
	
	int dev_time = 0;
  while(1)
  { 
    if(data_count >= 24)
  	{
	    head2	= rx_analysis_head + 1;
	    if(head2 >= 240) head2 -= 240;
	    head3	= rx_analysis_head + 2;
	    if(head3 >= 240) head3 -= 240; 
	  
	    if(RxBuffer_dma_save[rx_analysis_head] != 0xff ||
	  	   RxBuffer_dma_save[head2] != 0xfe ||
		     RxBuffer_dma_save[head3] != 0x15)
	    {
	    	rx_analysis_head++;
		    data_count--;
				usart_reset_key = 5;
	    }
	    else
	    {
	      for(t = 0; t < 24 ; t++)
		    { 
		      RxBuffer_rigth_save[t] = RxBuffer_dma_save[rx_analysis_head];
		      rx_analysis_head++;
		      data_count--; 
		      if(rx_analysis_head == 240 ) rx_analysis_head = 0;
		    }
			
	      usart_msg = &RxBuffer_rigth_save[0];
	  	  Zigbee_Cmd_Analysis(usart_msg);
		
	    }
	  
	    if(rx_analysis_head == 240) rx_analysis_head = 0;	
  	}
		if(dev_time%3750 == 0) TaskTimer_Check();
		dev_time++;
	  vTaskDelay(8);
  }	
} 
/* Private functions ---------------------------------------------------------*/
extern int usart_reset_key;
void UsartDMA_Send(INT8U *data)
{ 
  if(usart_reset_key < 5)
  {		
	  Fill_Buffer(TxBuffer_dma_save[tx_count], 24, data);
    tx_count++;
    if (tx_count > 9) tx_count = 0; 
    if(data_complete) 
    {	
      Fill_Buffer(TxBuffer_dma, 24, TxBuffer_dma_save[tx_now_send]);
      data_complete = FALSE;
      DMA_Cmd(USARTx_TX_DMA_STREAM, ENABLE);
    }
	}
}
/**  
  * 
  * 
  */
void  USART_DMA_Config(void)
{
  
  /* USART configuration -----------------------------------------------------*/
  USART_Config(); 
    
    
   /******************************************************************
    DMA TX   
    
    *******************************************************************/
    /* Prepare the DMA to transfer the transaction command (2bytes) from the
         memory to the USART  */ 
    DMA_DeInit(USARTx_TX_DMA_STREAM);
    DMA_InitStructure.DMA_Channel = USARTx_TX_DMA_CHANNEL;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;  
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)TxBuffer_dma;
    DMA_InitStructure.DMA_BufferSize = (uint16_t)24;
    DMA_Init(USARTx_TX_DMA_STREAM, &DMA_InitStructure); 
    
    DMA_ITConfig(USARTx_TX_DMA_STREAM, DMA_IT_TC , ENABLE);
    
    /* Enable the USART DMA requests */   
    USART_DMACmd(USARTx, USART_DMAReq_Tx, ENABLE);
      
    /* Clear the TC bit in the SR register by writing 0 to it */
    USART_ClearFlag(USARTx, USART_FLAG_TC);         
    
   
	
/******************************************************************
    DMA RX
    
*******************************************************************/       
#if 1
    DMA_DeInit(USARTx_RX_DMA_STREAM);
    DMA_InitStructure.DMA_Channel = USARTx_RX_DMA_CHANNEL;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)RxBuffer_dma;
    DMA_InitStructure.DMA_BufferSize = (uint16_t)24;
    DMA_Init(USARTx_RX_DMA_STREAM, &DMA_InitStructure);
    
    /******        DMA ÷–∂œ≈‰÷√        ************************/
    DMA_ITConfig(USARTx_RX_DMA_STREAM, DMA_IT_TC , ENABLE);
    /* Enable the USART Rx DMA request  */
    USART_DMACmd(USARTx, USART_DMAReq_Rx, ENABLE);  
    /* Enable the DMA RX Stream*/ 
    DMA_Cmd(USARTx_RX_DMA_STREAM, ENABLE);   
    

#endif
  
}



/**
  * @brief  Configures the USART 
  * @param  None
  * @retval None
  */
static void USART_Config(void)
{
  
  NVIC_InitTypeDef NVIC_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
    
  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(USARTx_TX_GPIO_CLK | USARTx_RX_GPIO_CLK, ENABLE);
  
  /* Enable USART clock */
  USARTx_CLK_INIT(USARTx_CLK, ENABLE);
   
  /* Connect USART pins to AF7 */
  GPIO_PinAFConfig(USARTx_TX_GPIO_PORT, USARTx_TX_SOURCE, USARTx_TX_AF);
  GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, USARTx_RX_SOURCE, USARTx_RX_AF);
  
  /* Configure USART Tx and Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
  GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
  GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
  GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure); 



  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  /* When using Parity the word length must be configured to 9 bits */
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTx, &USART_InitStructure);
  
  


  /* Configure DMA controller to manage USART TX and RX DMA request ----------*/
  /* Enable the DMA clock */
  RCC_AHB1PeriphClockCmd(USARTx_DMAx_CLK, ENABLE);  

  DMA_InitStructure.DMA_PeripheralBaseAddr = USARTx_DR_ADDRESS;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;      //DMA_Mode_Circular  DMA_Mode_Normal
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  /* Here only the unchanged parameters of the DMA initialization structure are
     configured. During the program operation, the DMA will be configured with 
     different parameters according to the operation phase */
         
  /* Enable USART */
  USART_Cmd(USARTx, ENABLE);
  
  
    /* Enable the DMA Stream IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);   
  
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  Zigbee_InitStructure.GPIO_Pin = GPIO_Pin_7;
	Zigbee_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	Zigbee_InitStructure.GPIO_OType = GPIO_OType_PP;
  Zigbee_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOD, &Zigbee_InitStructure);
  GPIO_SetBits(GPIOD, GPIO_Pin_7);
  
}


////////////////////////////
void Usart_GPIO_Set(UsartState  status)
{ 
  GPIO_InitTypeDef GPIO_InitStructure;
  if (status == CLOSE)
  {
		DMA_ITConfig(USARTx_RX_DMA_STREAM, DMA_IT_TC , DISABLE);
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
		
		DMA_Cmd(USARTx_RX_DMA_STREAM, DISABLE);
		
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		
    GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
    GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
    GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);
		
		USART_Cmd(USARTx, DISABLE);
		
    GPIO_ResetBits(GPIOD, USARTx_TX_PIN);
    GPIO_ResetBits(GPIOD, USARTx_RX_PIN);
    
		Zigbee_InitStructure.GPIO_Pin = GPIO_Pin_7;
		Zigbee_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    Zigbee_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_Init(GPIOD, &Zigbee_InitStructure);
		GPIO_ResetBits(GPIOD, GPIO_Pin_7);
  }
  else if(status == WAIT)
  {	
    Zigbee_InitStructure.GPIO_Pin = GPIO_Pin_7;
	  Zigbee_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	  Zigbee_InitStructure.GPIO_OType = GPIO_OType_PP;
    Zigbee_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &Zigbee_InitStructure);
    GPIO_SetBits(GPIOD, GPIO_Pin_7);
  }
	else if(status == OPEN)
  {
		Usart_StateClear();
		
		USART_DMA_Config();
  }
    
}


/**
  * 
  * 
  */
 void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLength, const INT8U *data)
{
  uint16_t index = 0;
  
  /* Put in global buffer same values */
  for (index = 0; index < BufferLength; index++ )
  {
    pBuffer[index] = *(data+index);
  }
}


void Usart_StateClear(void)
{
  memset(TxBuffer_dma_save, 0, sizeof(TxBuffer_dma_save));
  memset(TxBuffer_dma, 0, sizeof(TxBuffer_dma));
  tx_now_send = 0;
  tx_count = 0;
  
	memset(RxBuffer_dma, 0, sizeof(RxBuffer_dma));
  memset(RxBuffer_dma_save, 0, sizeof(RxBuffer_dma_save));
	data_complete = TRUE;
	
	rx_head = 0;
  data_count = 0;
	
	rx_analysis_head = 0;
}



