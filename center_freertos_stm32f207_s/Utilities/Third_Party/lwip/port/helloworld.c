/**
  ******************************************************************************
  * @file    helloworld.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   A hello world example based on a Telnet connection
  *          The application works as a server which wait for the client request
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "helloworld.h"
#include "lwip/tcp.h"
#include "stm32f2xx.h"
#include "main.h"
#include <string.h>
#include "usr_net.h"
#include "usr_cpu.h"


extern uint32_t ETH_GetRxPktSize(void);
extern void LwIP_Pkt_Handle(void);


/* Private typedef -----------------------------------------------------------*/
int seq_cnt = 1;
u32_t seq[100];

extern  void my_tcp_process(struct tcp_pcb *pcb);

/* Private define ------------------------------------------------------------*/

#define MAX_NAME_SIZE 32

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

struct name 
{
  int length;
  char bytes[MAX_NAME_SIZE];
};

/* Private function prototypes -----------------------------------------------*/
static err_t HelloWorld_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t HelloWorld_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static void HelloWorld_conn_err(void *arg, err_t err);

/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Called when a data is received on the telnet connection
  * @param  arg	the user argument
  * @param  pcb	the tcp_pcb that has received the data
  * @param  p	the packet buffer
  * @param  err	the error value linked with the received data
  * @retval error value
  */



int list_i = 0;

static err_t HelloWorld_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  struct pbuf *q;

  struct name *name = (struct name *)arg;
  
  char *c = NULL;
     
  int i = 0;

  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {        
  	/* We call this function to tell the LwIp that we have processed the data */
	  /* This lets the stack advertise a larger window, so more data can be received*/
	  tcp_recved(pcb, p->tot_len);
	  tcp_dev_new = pcb;
		///////////////////////////////////////
		for(i = 0; i < 10; i++)
		{
			if(pad_info[i].socket == tcp_dev_new && pad_info[i].user == TRUE)
			{
				pad_info[i].pad_living = 10;
				break;
			}
		}
		////////////////////////////
    /* Check the name if NULL, no data passed, return withh illegal argument error */
 	  if(!name) 
    {
      pbuf_free(p);
      return ERR_ARG;
    }
   
    for(q=p; (q != NULL && (q->len)%0x18 == 0) ; q = q->next) 
    { 
      c = q->payload;

      list_i = (q->len)/0x18;
  	  for(i = 0; i < list_i; i++)
	    {
	    
		    if(c[i*24] == 0xff && c[i*24 + 1] == 0xfe && c[i*24 + 2] == 0x15)
		    {	 
		      Save_Cmd(c + i*24);	 
		    } 
      }
			

    }


	/* End of processing, we free the pbuf */
    pbuf_free(p);
		
    tcp_dev_new = NULL;		
  }  
  else if (err == ERR_OK) 
  {
    /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
    /* We free the allocated memory and we close the connection */
    mem_free(name);
	  Pcb_Update(pcb, FALSE);
    return tcp_close(pcb);
  }
  
  return ERR_OK;
}

/**
  * @brief  This function when the Telnet connection is established
  * @param  arg  user supplied argument 
  * @param  pcb	 the tcp_pcb which accepted the connection
  * @param  err	 error value
  * @retval ERR_OK
  */
static err_t HelloWorld_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{     
  /* Tell LwIP to associate this structure with this connection. */
  tcp_arg(pcb, mem_calloc(sizeof(struct name), 1));	
  
  /* Configure LwIP to use our call back functions. */
  tcp_err(pcb, HelloWorld_conn_err);
  tcp_recv(pcb, HelloWorld_recv);
  
  /* Send out the first message */  
  Pcb_Update(pcb ,TRUE);  

  return ERR_OK;
}

/**
  * @brief  Initialize the hello application  
  * @param  None 
  * @retval None 
  */ 
struct tcp_pcb *server_pcb;
void Server_Task(void *pdata)
{
  /* Create a new TCP control block  */
  server_pcb = tcp_new();

  /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
  tcp_bind(server_pcb, IP_ADDR_ANY, 2657);       

  /* Set the connection to the LISTEN state */
  server_pcb = tcp_listen(server_pcb);				

  /* Specify the function to be called when a connection is established */	
  tcp_accept(server_pcb, HelloWorld_accept);
									
}

/**
  * @brief  This function is called when an error occurs on the connection 
  * @param  arg
  * @parm   err
  * @retval None 
  */
int conn_err_t = 0;
static void HelloWorld_conn_err(void *arg, err_t err)
{
  struct name *name;
  name = (struct name *)arg;

  mem_free(name);
	conn_err_t++;
}



/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


