
#include "usr_net.h"
#include "usr_api.h"
#include "md5.h"
#include "sha1.h"
#include "aes.h"
#include "usr_cpu.h"
#include "net_server.h"
#include "const.h"

#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "main.h"

#include "stm32f2x7_eth.h"

#include <stdio.h>
#include <string.h>

#define LCD_TIMER_MSECS       250
#define MAX_DHCP_TRIES        4

#define MY_ICMP_UNREACH_DATASIZE 8

#define cmd_cap 200

struct netif netif;
__IO uint32_t TCPTimer = 0;

__IO uint32_t ARPTimer = 0;

#ifdef LWIP_DHCP
__IO uint32_t DHCPfineTimer = 0;
__IO uint32_t DHCPcoarseTimer = 0;
uint32_t IPaddress = 0;
#endif

unsigned char macaddress[6];

__IO uint32_t DisplayTimer = 0;



/* Private variables ---------------------------------------------------------*/

struct tcp_pcb *tcp_dev_new;

CMD_ANALYSIS cmd_analysis[0x60];

CMD_LIST cmd_info[cmd_cap];

int rev_offset = 0;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  the FreeRTOS task to anlysis the tcp command data.
  * @param  
  * @retval None
  */
int trans_offset = 0;


void Analysis_Task(void * pvParameters)
{ 
  while(1)
  { 
    if(cmd_info[trans_offset].cmd[0] == 0xff)
	  {   
	    tcp_device = cmd_info[trans_offset].socket;
	    if(tcp_device == sever_pcb || tcp_device == NULL)
	    {
	      Cmd = cmd_info[trans_offset].cmd;
	      if(cmd_analysis[*(Cmd+4)].cmd == *(Cmd+4))
      	{  
			
					cmd_analysis[*(cmd_info[trans_offset].cmd+4)].func_cmd();
				}
	    }
	    else
	    { 			
	      Tcp_Cmd_Analysis(cmd_info[trans_offset].cmd);
      }

	    cmd_info[trans_offset].cmd[0] = 0;
	    trans_offset++;
	  
	    memset(cmd_reply+3, 0 ,21);
	  }
	
	  if (trans_offset == cmd_cap) trans_offset = 0;
		
	  vTaskDelay(1);
  }
}

/**
  * @brief  when tcp command are recieved, save them.
  * @param  cmd_msg: the pointer of the 24byte command.
  * @retval None
 */

void Save_Cmd(INT8U *cmd_msg)
{  
  if((cmd_msg[0] == 0xff) && (cmd_msg[1] == 0xfe) && (cmd_msg[2] == 0x15))
	{	       
		if(cmd_info[rev_offset].cmd[0] == 0x0)
		{
		  memcpy(cmd_info[rev_offset].cmd, cmd_msg, 24);
		  cmd_info[rev_offset].socket = tcp_dev_new;
		  rev_offset++;	
		}

		if (rev_offset == cmd_cap) rev_offset = 0;
		
	}
}

void ping(void)
{
  struct pbuf *q;
  /* we can use the echo header here */
  struct icmp_echo_hdr *icmphdr;
  struct ip_addr dest_addr;
  struct ip_addr src_addr;
  /* ICMP header + IP header + 8 bytes of data */
  q = pbuf_alloc(PBUF_IP, sizeof(struct icmp_echo_hdr) + IP_HLEN + MY_ICMP_UNREACH_DATASIZE,
                 PBUF_RAM);
	
  dest_addr.addr = netif.gw.addr;
	src_addr.addr = IPaddress;//0x6900a8c0;
  icmphdr = q->payload;
  icmphdr->type = ICMP_ECHO;
  icmphdr->code = 0;
  icmphdr->id = 25;
  icmphdr->seqno = 1;
  q->len = 60;
  /* calculate checksum */
  icmphdr->chksum = 0;

  ip_output(q, &src_addr, &dest_addr, 255, 0, IP_PROTO_ICMP);
  pbuf_free(q);

}

void ping_server(void)
{
  struct pbuf *q;
  /* we can use the echo header here */
  struct icmp_echo_hdr *icmphdr;
  struct ip_addr dest_addr;
  struct ip_addr src_addr;
  /* ICMP header + IP header + 8 bytes of data */
  q = pbuf_alloc(PBUF_IP, sizeof(struct icmp_echo_hdr) + IP_HLEN + MY_ICMP_UNREACH_DATASIZE,
                 PBUF_RAM);
	
  dest_addr.addr = sever_addr;
	src_addr.addr = IPaddress;//0x6900a8c0;
  icmphdr = q->payload;
  icmphdr->type = ICMP_ECHO;
  icmphdr->code = 0;
  icmphdr->id = 26;
  icmphdr->seqno = 1;
  q->len = 60;
  /* calculate checksum */
  icmphdr->chksum = 0;

  ip_output(q, &src_addr, &dest_addr, 255, 0, IP_PROTO_ICMP);
  pbuf_free(q);

}

/**/

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
	
  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();


#if LWIP_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;	
#else
  IP4_ADDR(&ipaddr, 192, 168, 0, 121);
  IP4_ADDR(&netmask, 255, 255, 255, 0);
  IP4_ADDR(&gw, 192, 168, 0, 1);
#endif

  Set_MAC_Address(macaddress);

  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /*  Registers the default network interface.*/
  netif_set_default(&netif);

#if LWIP_DHCP
  dhcp_start(&netif);
#endif

  /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&netif);
}


/**
  * @brief  Called when a frame is received
  * @param  None
  * @retval None
  */
void LwIP_Pkt_Handle(void)
{
  /* Read a received packet from the Ethernet buffers 
                and send it to the lwIP for handling */
  ethernetif_input(&netif);
}

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
void LwIP_Periodic_Handle(__IO uint32_t localtime)
{

  /* TCP periodic process every 25 ms */
  if ((localtime - TCPTimer) >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  localtime;
    tcp_tmr();
  }
  /* ARP periodic process every 5s */
  if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  localtime;
    etharp_tmr();
  }

#if LWIP_DHCP
  /* Fine DHCP periodic process every 500ms */
  if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  localtime;
    dhcp_fine_tmr();
  }
  /* DHCP Coarse periodic process every 60s */
  if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  localtime;
    dhcp_coarse_tmr();
  }
#endif

}

/**
  * @brief  LCD & LEDs periodic handling
  * @param  localtime: the current LocalTime value
  * @retval None
  */
void Display_Periodic_Handle(__IO uint32_t localtime)
{ 
  u16 PHYRegData;
  /* 250 ms */
  if (localtime - DisplayTimer >= LCD_TIMER_MSECS)
  {
    DisplayTimer = localtime;

    /* We have got a new IP address so update the display */
    if (IPaddress != netif.ip_addr.addr)
    {

      /* Read the new IP address */
      IPaddress = netif.ip_addr.addr;
	  
      /* Display the new IP address */
               
      /*Íø¿Ú×´Ì¬*/
      PHYRegData = ETH_ReadPHYRegister(0,17);
      if(PHYRegData & 0x3000){  
        /* Set Ethernet speed to 10M following the autonegotiation */    
      }else{   
          /* Set Ethernet speed to 100M following the autonegotiation */ 
      } 
        
			/* Configure the MAC with the Duplex Mode fixed by the autonegotiation process */
      if((PHYRegData & 0xA000) != (uint32_t)RESET){
          /* Set Ethernet duplex mode to FullDuplex following the autonegotiation */
      }else{
          /* Set Ethernet duplex mode to HalfDuplex following the autonegotiation */
      }
			
       
    }

#if LWIP_DHCP
    
    else if (IPaddress == 0)
    {
      /* We still waiting for the DHCP server */ 
      /* If no response from a DHCP server for MAX_DHCP_TRIES times */
	  /* stop the dhcp client and set a static IP address */
	    if(netif.dhcp->tries > MAX_DHCP_TRIES)
      {
         netif.dhcp->tries = 0;
      }
			
    }
#endif
  } 
}







