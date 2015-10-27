/**
  ******************************************************************************
  * @file    server.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   A sample UDP/TCP server application.
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
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "const.h"
#include <string.h>
#include <stdio.h>
#include "usr_cpu.h"
#include "flash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define UDP_SERVER_PORT    2658   /* define the UDP local connection port */
#define UDP_CLIENT_PORT    4   /* define the UDP remote connection port */


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);

extern void Pass_Reset(void);
extern void My_Flash_Write(int type);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the server application.
  * @param  None
  * @retval None
  */

void udp_port_reply(void)
{
   struct udp_pcb *upcb;                                
    
/****************************************************************/
   
   /* Create a new UDP control block  */
   upcb = udp_new();
   
   /* Bind the upcb to the UDP_PORT port */
   /* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
   udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_PORT);
   
   /* Set a receive callback for the upcb */
   udp_recv(upcb, udp_server_callback, NULL);

/****************************************************************/
}

/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
extern void check_sys_id(void);
extern bool center_NoID;
void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{ 
  struct pbuf *p_reply = NULL;
  uint8_t *buff;
  int sucess = 0;
   int t = 0;
  /* We have received the UDP Echo from a client */
  /* read its IP address */
 
  /* Connect to the remote client */

  /* recieve data analysis by wangxubo*/
  buff = p->payload;
 
  if((*(buff) == 0xff) && (*(buff+1) == 0xfe) && (*(buff+2) == 0x15) && (p->len == 0x18))
  {	
    switch (*(buff+4)){
      case 0x53:
	    {
	      *(buff+3) = sys_info[0].serial_num[0] + 2 ;
	      *(buff+4) = 0x54;
	      for(t = 1;t < sys_info[0].serial_num[0] + 1; t++)
	      {
	        *(buff+5+t) = sys_info[0].serial_num[t];
	      }
	    }
	    break;
   	  case 0x55:
	    { 
        int data_len = *(buff+3) - 2;
        
				if(CONFIG_MODE == 1 && (data_len <= 18 && data_len > 0))
        {
					INT8U *serial_ops = &sys_info[0].serial_num[1];
				
          sys_info[0].serial_num[0] = data_len;
          for(t = 0;t < data_len; t++)
          {
  	        *(serial_ops + t) = *(buff+6+t);
          }
		      *(buff+3) = 0x3;
          *(buff+4) = 0x56;
          *(buff+6) = sucess;
					
					Pass_Reset();
					check_sys_id();
					if(!center_NoID) 
					{
						SYSTERM  sys_check[1];
						My_Flash_Write(PASS_TYPE);
						FlashBufferRead((u8*)&sys_check[0], zone1_start, sizeof(struct my_systerm));
						if(0 != memcmp(sys_check[0].serial_num, sys_info[0].serial_num, 19))
						{
							*(buff+6) = 1 - sucess;
						}
					}
				}
				else
				{
				  *(buff+6) = 1 - sucess;
				}
	    }
	    break;
	    case 0x57:
	    {
        int data_len = *(buff+3) - 2;
        if(CONFIG_MODE == 1 && (data_len <= 18 && data_len > 0))
        {
          INT8U *key_ops = &sys_info[0].secret_num[1];
          sys_info[0].secret_num[0] = data_len;
          for(t = 0;t < data_len; t++)
          {
  	        *(key_ops + t) = *(buff+6+t);
          }
  		    
          *(buff+3) = 0x3;
		      *(buff+4) = 0x58;
  
          *(buff+6) = sucess;
					
					check_sys_id();
					if(!center_NoID) {
						SYSTERM  sys_check[1];
						My_Flash_Write(PASS_TYPE);
						FlashBufferRead((u8*)&sys_check[0], zone1_start, sizeof(struct my_systerm));
						if(0 != memcmp(sys_check[0].secret_num, sys_info[0].secret_num, 19))
						{
							*(buff+6) = 1 - sucess;
						}
					}
				}
				else
				{
					*(buff+6) = 1 - sucess;
				}
	    }
	    break;
	    default: pbuf_free(p);
	             return;

    }

	  p_reply = pbuf_alloc(PBUF_RAW, 24, PBUF_RAM);
	  p_reply->payload = buff;
    /* Connect to the remote client */
    udp_connect(upcb, addr, port/*UDP_CLIENT_PORT*/);

    /* Tell the client that we have accepted it */
	  udp_send(upcb, p_reply);

    /* free the UDP connection, so we can accept new clients */
    udp_disconnect(upcb);
	  pbuf_free(p_reply);
  }
  /* Free the p buffer */
  pbuf_free(p);   
}





/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
