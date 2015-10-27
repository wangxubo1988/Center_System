/**
  ******************************************************************************
  * @file    net_server.c
  * @author  wangxubo
  * @version V1.0.0
  * @date    10/20/2012
  * @brief   to communicate with the net sever
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "lwip/tcp.h"
#include "usr_net.h"
#include "net_server.h"
#include "usr_cpu.h"

#include "md5.h"
#include "sha1.h"
#include "aes.h"

#include "const.h"
#include "dns.h"
#include "flash.h"

#include <string.h>
/* Private variables ---------------------------------------------------------*/

bool rnd_recieved = FALSE;

int setChangeSecret_once = 0;

bool net_connected = FALSE;
bool net_logined = FALSE;

int center_living_count = 0; 

int try_do_update = 4;


u32_t sever_addr = 0x0e0f792a;//NULL;
u32_t severSoft_addr = 0x4906782a;//NULL;

struct tcp_pcb * sever_pcb;
struct tcp_pcb * sever_config_pcb;

INT8U soft_version[5] = {0x04, 0x31, 0x30, 0x31, 0x37};
u8 iap_version[4];

u8 soft_switch = 0;

INT8U time_stamp[5];

INT8U soft_md5[17];
INT8U soft_md5_c[17];

INT8U salt[20] = {0x35,0x18,0x02,0x32,0x2e,0x2d,0xb9,0x13, 0xe7,0x9f,
				  0x17,0x3a,0x5b,0x4e,0x01,0xf4,0xa8,0x25,0xd6,0x3a};

INT8U rng_number[100] = {0x0a, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

/*check num*/
struct sever_pdu pdu = {sever_HEAD1, sever_HEAD2, sever_HEAD3, sever_HEAD4, sever_HEAD5};

INT8U data[100];
INT8U data_save[100];
INT8U check_num_c[20];
INT8U check_num[21] = {0x14};
/*Secret_Pass*/
aes_context ctx; 

INT8U secret_pass_c[20];
INT8U  secret_pass[33] = {0x20};
INT8U  iv[16];
INT8U key[16];


/* Private function prototypes -----------------------------------------------*/
void Net_Sever_SetSecret(void);
void Get_SoftData_Return(void);
INT8U Get_ASCtoHex(INT8U data);
void Get_severDns(void);
void Get_severSoftDns(void);

/* Private functions  -----------------------------------------------*/


/**
  * @brief  get the check number
  * @param  B
  * @retval check number
 */

INT8U *Get_Check_Num(INT8U *b, int a_len)
{

  memcpy(&data_save[0], b + 1, *b);
  memcpy(&data_save[*b], &salt[0], sizeof(salt));
  memcpy(&data_save[*b + sizeof(salt)], b + 1, *b);

  sha1(&data_save[0], *b * 2 + sizeof(salt), &check_num_c[0]);

  memcpy(&data_save[0], &pdu, a_len);
  memcpy(&data_save[a_len], &check_num_c[0], sizeof(check_num_c));

  
  sha1(&data_save[0], a_len + sizeof(check_num_c), &check_num[1]);

  return &check_num[0];
}
/**
  * @brief  get the secret pass number
  * @param   D
  * @retval secret pass number
 */
INT8U secret_pass_c[20];
INT8U secret_pass_e[20];
INT8U *Get_Secret_Pass(INT8U *d, INT8U *b)
{ 
  memcpy(&data_save[0], b + 1, *b);
  memcpy(&data_save[*b], &salt[0], sizeof(salt));
  memcpy(&data_save[*b + sizeof(salt)], b + 1, *b);

  sha1(&data_save[0], *b * 2 + sizeof(salt), &secret_pass_c[0]);

  memcpy(&data_save[0], d + 1, *d);
  memcpy(&data_save[*d], &salt[0], sizeof(salt));
  memcpy(&data_save[*d + sizeof(salt)], d + 1, *d);

  sha1(&data_save[0], *d * 2 + sizeof(salt), &secret_pass_e[0]);
  
  memcpy(key, secret_pass_c, 16);
  memset(iv, 0, 16);
  aes_setkey_enc(&ctx, key, 128);
  aes_crypt_cbc(&ctx, AES_ENCRYPT, 20, iv, &secret_pass_e[0], &secret_pass[1]);
  
  return &secret_pass[0];

}


/**
  * @brief  
  * @param  
  * @retval 
 */


void Net_Sever_Send(INT8U CMD, struct tcp_pcb * tcp)
{ 
  
  int serial_len =  sys_info[0].serial_num[0];
  int rng_len =  rng_number[0];     
  
  

  switch (CMD) {
    case CMD_LOGIN:{
	  
	   pdu.data_len[0] = serial_len + 1 + rng_len + 1 +sizeof(check_num) + 1;
	   
	   pdu.cmd = CMD_LOGIN;

	   memcpy(&pdu.cmd_data[0], &sys_info[0].serial_num[0], serial_len + 1);  
	   memcpy(&pdu.cmd_data[serial_len+1], &rng_number[0], rng_len + 1);
	   
	   memcpy(&pdu.cmd_data[serial_len+1 + rng_len + 1], 
	          Get_Check_Num(&sys_info[0].password[0], rng_number[0] + sys_info[0].serial_num[0] + 2 + 10), 
			  sizeof(check_num));
	}
    break;
	case CMD_PASS_CHANGE: {
	  pdu.data_len[0] = serial_len + 1 + rng_len + 1 + sizeof(secret_pass) + sizeof(check_num) + 1;
	  pdu.cmd = CMD_PASS_CHANGE;
	  
	   
	  memcpy(&pdu.cmd_data[0], &sys_info[0].serial_num[0], serial_len + 1);  
	  memcpy(&pdu.cmd_data[serial_len+1], &rng_number[0], rng_len + 1);
	  
	  memcpy(&pdu.cmd_data[serial_len + 1 + rng_len + 1], 
	   	     Get_Secret_Pass(&sys_info[0].password[0], &sys_info[0].secret_num[0]), sizeof(secret_pass));

	  memcpy(&pdu.cmd_data[serial_len+1 + rng_len + 1 + sizeof(secret_pass)], 
	   		 Get_Check_Num(&sys_info[0].secret_num[0], 10 + serial_len + 1 + rng_len + 1 + sizeof(secret_pass)),
			 sizeof(check_num));

	  
	}
	break;
	case CMD_LIVING: {
	  pdu.data_len[0] = 2;
	  pdu.cmd = CMD_LIVING;
	  pdu.cmd_data[0] = 0x0;

	  center_living_count++;
	}
	break;
	case CMD_VERSION: {
	  pdu.data_len[0] = 9; 
	  pdu.cmd = CMD_VERSION;
		pdu.cmd_data[0] = 2;
		pdu.cmd_data[1] = 0x33;
		pdu.cmd_data[2] = 0x30;
		memcpy(&pdu.cmd_data[3], soft_version, 5);
// 	  memcpy(pdu.cmd_data, soft_version, pdu.data_len[0]);

	  try_do_update --;
	}
	break;
	
	case CMD_IAPVERSION: {
	  pdu.data_len[0] = 9; 
	  pdu.cmd = CMD_IAPVERSION;
		pdu.cmd_data[0] = 2;
		pdu.cmd_data[1] = 0x61;
		pdu.cmd_data[2] = 0x30;
		pdu.cmd_data[3] = 4;
		memcpy(&pdu.cmd_data[4], iap_version, 4);

	  try_do_update --;
	}
	break;
	
	case CMD_SENDSECRET: {
	  pdu.data_len[0] = sys_info[0].secret_num[0] + 2; 
	  pdu.cmd = CMD_SENDSECRET;
		pdu.cmd_data[0] = sys_info[0].secret_num[0];
		memcpy(&pdu.cmd_data[1], &sys_info[0].secret_num[1], pdu.cmd_data[0]);
		
	}
	break;
	
	case CMD_TIMESTAMP_R:{
    pdu.data_len[0] = 5;
		pdu.cmd = CMD_TIMESTAMP_R;
		memcpy(pdu.cmd_data, time_stamp, 5);
	}
	break;
  
  }
	
  tcp_write(tcp, (INT8U*)&pdu, pdu.data_len[0] + 9, 1);

}


void Keep_Living(void)
{ 
  Net_Sever_Send(CMD_LIVING, sever_pcb);
}

/**
  * @brief Anlysis the data recieved from the net sever 
  * @param  cmd: the Net sever pdu command
  			cmd_data: the pdu data 
  * @retval None
 */
int port_src = 0;
int setChoose = 0;

void Net_Cmd_Anlysis(INT8U cmd, INT8U *cmd_data)
{

  switch (cmd){
	  case CMD_RNG: {
	    memcpy(rng_number, cmd_data, *cmd_data + 1);
	    rnd_recieved = TRUE;
	    if(port_src == 0) 
	    {	
	      Net_Sever_Send(CMD_LOGIN, sever_pcb);
	    }
	    else
	    {			
				
	      if(setChoose == 0)
	  	  {
	        Net_Sever_Send(CMD_PASS_CHANGE, sever_config_pcb);
		    }
		    else 
	  	  {
		      Net_Sever_Send(CMD_VERSION, sever_config_pcb);
					//Net_Sever_Send(CMD_IAPVERSION, sever_config_pcb);
		    }
	    }
	  }
	  break;

    case CMD_LOGIN_R: {
	    if(*(cmd_data+1) == 0x0) 
	    { 
		    net_logined = TRUE;
				Write_SystermState(NET_LOGIN);
		    
				UpdateTime_Send();
				Net_Sever_Send(CMD_SENDSECRET, sever_pcb);
				
		    setChoose = 1;
	      Net_Sever_SetSecret();	
	      
	    }
	    else 
	    {	
		      net_connected = FALSE;
	
				 if(setChangeSecret_once == 0)
		     {
					setChoose = 0; 
	        Net_Sever_SetSecret();	  
	    
        }
			}
	  }
	  break;
	
	  case CMD_CONTROL:{
	    Save_Cmd(cmd_data + 1);
	  }
	  break;
	
	  case CMD_VERSION_R:{
	  	
	    int t = 0;
	    int t1 = 0;
	   
	    for(t = 6; t < 10; t++)
	    {
	      if(*(cmd_data+t) > soft_version[t - 5])
	      {
		      for(t1 = 0; t1 < 16; t1 ++)
		      {
		        soft_md5[t1] = (Get_ASCtoHex(*(cmd_data+t1*2+11)) << 4)  |  Get_ASCtoHex(*(cmd_data+t1*2+12));
		      }
	       
				  soft_md5[16] = 1;
					
					Flash_SoftInit();
		      Get_SoftData_Return();
					
					try_do_update = 0;
					tcp_close(sever_config_pcb);
		      return;
		    }
	    }
			Net_Sever_Send(CMD_IAPVERSION, sever_config_pcb);
	  }
		break;
		
		case CMD_IAPVERSION_R:{
	  	
	    int t = 0;
	    int t1 = 0;
	   
	    for(t = 6; t < 10; t++)
	    {
	      if(*(cmd_data+t) > iap_version[t - 6])
	      {
		      for(t1 = 0; t1 < 16; t1 ++)
		      {
		        soft_md5[t1] = (Get_ASCtoHex(*(cmd_data+t1*2+11)) << 4)  |  Get_ASCtoHex(*(cmd_data+t1*2+12));
		      }
	       
				  soft_md5[16] = 1;
					
					soft_switch = 1;
					Flash_SoftInit();
		      Get_SoftData_Return();
		      break;
		    }
	    }
	    try_do_update = 0;
      
	    tcp_close(sever_config_pcb);
	  }
		break;
		
		case CMD_TIMESTAMP:{
			memcpy(time_stamp, cmd_data, 5);
			Net_Sever_Send(CMD_TIMESTAMP_R, sever_pcb);
		
		}
		break;
		
  } 
}

INT8U Get_ASCtoHex(INT8U data)
{
  INT8U hex_result = 0;
  if (0x30 <= data && data <= 0x39)
  {
  	hex_result = data - 0x30;
  }
  else if(0x61 <= data  && data <= 0x66)
  {
    hex_result = data - 0x57;
  }
  return hex_result;

}

err_t Net_Sever_Recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  
  
  struct sever_pdu *pdu = NULL;

  struct pbuf *q;
  
  int cmd_len = 0;
  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {    
	/* We call this function to tell the LwIp that we have processed the data */
	/* This lets the stack advertise a larger window, so more data can be received*/
	  tcp_recved(pcb, p->tot_len);
	  tcp_dev_new = pcb;
    
		
		
	  if (pcb == sever_pcb) port_src = 0;
	  else port_src = 1;
    /* Check the name if NULL, no data passed, return withh illegal argument error */

    for(q=p; (q != NULL ) ; q = q->next) 
    { 
			
      while(cmd_len + 10 <= q->len)
			{
				pdu = (struct sever_pdu*)((int)q->payload + cmd_len);
	      if(pdu->head[0] == sever_HEAD1 && pdu->head[1] == sever_HEAD2 
			  															 && pdu->head[2] == sever_HEAD3 
				  														 && pdu->head[3] == sever_HEAD4
					  													 && pdu->head[4] == sever_HEAD5)
	      {
	  	    Net_Cmd_Anlysis(pdu->cmd, pdu->cmd_data); 
	      }
				cmd_len += pdu->data_len[0] + 9;
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
	
    return tcp_close(pcb);
  }
  
  return ERR_OK;
}

err_t Tcp_CheckConnect(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  if(center_living_count > 0 ) center_living_count--;
  return ERR_OK;
}


err_t Net_init(void *arg, struct tcp_pcb *pcb, err_t err)
{
		net_connected = TRUE;
    center_living_count = 0;
    tcp_sent(pcb, Tcp_CheckConnect);
    tcp_recv(pcb, Net_Sever_Recv);
		return ERR_OK;
  
}

/**
  * @brief  connect the net sever port 4455
  * @param  None
  * @retval None
 */

void Net_Sever_Connect(void)
{ 

  struct ip_addr *addr;
  struct ip_addr ip;
   
  ip.addr = sever_addr;
  addr = &ip;
  

	/* Create a new TCP control block  */
  sever_pcb = tcp_new();
 
  /* Assign to the new pcb a local IP address and a port number */
    tcp_bind(sever_pcb, IP_ADDR_ANY, 3000);

		/* Connect to the server: send the SYN */
   /* Connect to the server: send the SYN */
  if(tcp_connect(sever_pcb, addr, SEVER_PORT, Net_init) == ERR_OK)
  { 
  }
}

err_t Tcp_Set_Close(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  tcp_close(sever_config_pcb);
  
	setChangeSecret_once = 1;
  return ERR_OK;
}

err_t Net_Set_init(void *arg, struct tcp_pcb *pcb, err_t err)
{ 

  if(setChoose == 0)
  {
    tcp_sent(pcb, Tcp_Set_Close);
  }
  tcp_recv(pcb, Net_Sever_Recv);
  return ERR_OK;
}

/**
  * @brief  set the secret .
  * @param  None
  * @retval None
 */
void Net_Sever_SetSecret(void)
{ 
  struct ip_addr *addr;
  struct ip_addr ip;
  ip.addr = sever_addr;
  addr = &ip;

  sever_config_pcb = tcp_new();
  tcp_bind(sever_config_pcb, IP_ADDR_ANY, 3001);
  
  
   /* Connect to the server: send the SYN */
  tcp_connect(sever_config_pcb, addr, SEVER_CONFIG_PORT, Net_Set_init);
}


//////////////////////////////////////////////////////////////////////
struct tcp_pcb * downsoft_pcb;

INT8U soft_buff1[] = "GET /centerapp/center_standard HTTP/1.1\r\nHost: ";
INT8U soft_buff2[] = "GET /centerapp/IAP.bin HTTP/1.1\r\nHost: ";


int soft_len = 0;
int soft_len_cnt = 0;

extern void Write_Soft_Date(void  *data, 
      				int offset, 
					int cnt
				
	    			);

void Soft_Cnt_Check(INT8U *buff)
{
   int ops = 0;
   for(ops = 131; ops < 140;ops++)
   {
     if (*(buff+ops) == 0x0d) break;
     soft_len = soft_len * 10 + (*(buff+ops) - 0x30);	 
   }
}
INT8U soft_temp[512];
int soft_temp_cnt = 0;
int soft_temp_offset = 0;

err_t soft_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{

  struct pbuf *q;
  
  struct name *name = (struct name *)arg;
  INT8U *c = NULL;

  my_tcp_process(pcb);

  if (p != NULL) 
  {        
	/* We call this function to tell the LwIp that we have processed the data */
	/* This lets the stack advertise a larger window, so more data can be received*/
	
	  tcp_recved(pcb, p->tot_len);
	
	  q = p;
	
  	while((q != NULL ) && (q->len)>0 && (q->type) == 0x03 )
    {
      c = q->payload;
    
	    if(soft_len == 0)
	    {
	      Soft_Cnt_Check(c);
      }
	    else
	    {	
		    soft_len_cnt += q->len;
				
        if(soft_temp_cnt + q->len > TCP_MSS)
		    {
		      memcpy(soft_temp + soft_temp_cnt, c, TCP_MSS - soft_temp_cnt);
		      Flash_SoftWrite(soft_temp, soft_temp_offset, TCP_MSS);
		      soft_temp_offset += TCP_MSS;

		      memcpy(soft_temp, c + TCP_MSS - soft_temp_cnt, q->len -  (TCP_MSS - soft_temp_cnt));
		      soft_temp_cnt =  q->len -  (TCP_MSS - soft_temp_cnt);
		    }
		    else
		    {
	        memcpy(soft_temp + soft_temp_cnt, c , q->len);
		      soft_temp_cnt += q->len;
		    }
		 
		    if(soft_len_cnt == soft_len)
		    {
		      Flash_SoftWrite(soft_temp, soft_temp_offset, soft_temp_cnt);
          Flash_SoftLen(soft_len);
		      tcp_close(pcb);
		      

		      try_do_update = 5;
		      return ERR_OK;
		    }   

	    }			
	    q = q->next; 

    }
   	
	
	/* End of processing, we free the pbuf */
    pbuf_free(p);
	 
  } else if (err == ERR_OK)
  {
	  mem_free(name);
	  return tcp_close(pcb); 
  }

  return ERR_OK;
}
INT8U http_pdu[71];
err_t soft_init(void *arg, struct tcp_pcb *pcb, err_t err)
{ 
	int i = 0;
	int pdu_len = 0;
  soft_len = 0;
  soft_len_cnt = 0; 
  tcp_recv(pcb, soft_recv);
	
	if(soft_switch == 0){
		sprintf(http_pdu, "%s%d%s%d%s%d%s%d%s", soft_buff1, (INT8U)(severSoft_addr), ".",
																									 (INT8U)(severSoft_addr>>8), ".",
	                                                 (INT8U)(severSoft_addr>>16), ".",
																									 (INT8U)(severSoft_addr>>24), ":9000\r\n\r\n");
	}else{
		sprintf(http_pdu, "%s%d%s%d%s%d%s%d%s", soft_buff2, (INT8U)(severSoft_addr), ".",
																									 (INT8U)(severSoft_addr>>8), ".",
	                                                 (INT8U)(severSoft_addr>>16), ".",
																									 (INT8U)(severSoft_addr>>24), ":9000\r\n\r\n");
	}
	for(i = 0; i < 71 && http_pdu[i] != 0; i++) pdu_len++;
  tcp_write(pcb, (void*)http_pdu, pdu_len, 1);
  return ERR_OK;
}

char get_date_timer = 0;
void Get_SoftData_Return(void)
{ 
  struct ip_addr *addr;
  struct ip_addr ip;
   
  ip.addr = severSoft_addr;//0x6d00a8c0;
  addr = &ip;
	
   /* Create a new TCP control block  */
  downsoft_pcb = tcp_new();
  
  /* Assign to the new pcb a local IP address and a port number */
  tcp_bind(downsoft_pcb, IP_ADDR_ANY, 3002);
  /* Connect to the server: send the SYN */
  tcp_connect(downsoft_pcb, addr, 9000, soft_init);
 
  get_date_timer = 6;

}
/*****************************************/
extern int Read_UpdateBin_toCheck(INT8U *md5);


void Check_Update(void)
{
	if(get_date_timer>0){
    get_date_timer--;
		if(get_date_timer == 0){
			Write_SystermState(UPDATE_FAIL);
			SystemReset();
    }
  }
   if(try_do_update == 5)
   {
		 get_date_timer = 0;
		 if(soft_switch == 1){
			if(Read_UpdateBin_toCheck(soft_md5) == 1){
				STM_EVAL_LEDOn(LED1);
				STM_EVAL_LEDOn(LED2);
				STM_EVAL_LEDOn(LED3);
				Read_Update_toUpdate();
				STM_EVAL_LEDOff(LED1);
				STM_EVAL_LEDOff(LED2);
				STM_EVAL_LEDOff(LED3);
				try_do_update = 0;
				Soft_Clean();
				SystemReset();
			}
		}
		
		if(soft_switch == 0){
			if(Read_UpdateBin_toCheck(soft_md5) == 1)
			{
				STM_EVAL_LEDOn(LED3);
				Write_SystermState(SYS_UPDATE);
				SystemReset();
			}
		}
		
	  Soft_Clean();
		SystemReset();
   }
   else if(try_do_update < 4 && try_do_update >0)
   {
	 setChoose = 1;
	 Net_Sever_SetSecret();
   }
}

void Check_Soft_Version(void)
{
   if(net_logined && sd_error == 0)
   {
     Net_Sever_SetSecret();
   }
}

#if 0
/*********/

struct ip_addr dns_addr;
struct ip_addr dnssoft_addr;

void DnsGateWay_tryAgain(const char *name, struct ip_addr *ipaddr, void *callback_arg)
{
  Get_severDns();
}
void DnsSoft_tryAgain(const char *name, struct ip_addr *ipaddr, void *callback_arg)
{
  Get_severSoftDns();
}

void Get_severSoftDns(void)
{                         //"update.godrig.com"
  if(dns_gethostbyname("update.godrig.com", &(dnssoft_addr), DnsSoft_tryAgain, NULL) == ERR_OK)
	{
    severSoft_addr = dnssoft_addr.addr;
  }
}
void Get_severDns(void)
{                       
	if(dns_gethostbyname("gateway.godrig.com", &(dns_addr), DnsGateWay_tryAgain, NULL) == ERR_OK)
	{
    sever_addr = dns_addr.addr;
		Get_severSoftDns();
  }
}

#endif

/**/
INT8U server_state = GET_DNS;
int server_timeout = 0;
uint32_t SeverLiveTime = 20;
bool wan_connected = FALSE;
bool password_change = FALSE;

void Server_Function(void)
{
	server_timeout++;
  switch(server_state){
    case NET_CONN:{
			if(wan_connected){
	      if(net_connected && net_logined){
					net_connected = FALSE;
					net_logined = FALSE;
				}else{
// 					if(sever_addr == NULL) Get_severDns();
				  server_state = GET_DNS;
				  server_timeout = 0;
        }
			}else if(server_timeout == 1 || server_timeout == 10){ 
			  ping_server();
        server_timeout = 0;				
      }		
    }
	  break;
		case GET_DNS:{
      if(sever_addr != NULL && sever_addr != 0xffffffff){
		    server_state = SEVER_CONN;
			  server_timeout = 0;
		  	Net_Sever_Connect();
		  }else if(server_timeout == 30){
				server_timeout = 0;
      }
		}
		break;
		case SEVER_CONN:{
		  if(net_connected && net_logined){
			  server_state = SERVER_Living;
				server_timeout = 0;
      }else if(server_timeout == 30){
        server_timeout = 0;
				server_state = GET_DNS;
      }
			
	  }
	  break;
		case SERVER_Living:{
      if(server_timeout > SeverLiveTime){
			  Keep_Living();
			  SeverLiveTime = Get_RNG_LivingSec();
				server_timeout = 0;
				if(center_living_count > 1 || password_change) 
	      {
						Write_SystermState(SYS_SEVER_DISCON);
						server_state = GET_DNS;
						server_timeout = 0;
						center_living_count = 0;
					  net_connected = FALSE;
					  net_logined = FALSE;
					  tcp_close(sever_pcb);
					  password_change = FALSE;
				}
		  }
			
		}
		break;
	}
		
}

/**/
