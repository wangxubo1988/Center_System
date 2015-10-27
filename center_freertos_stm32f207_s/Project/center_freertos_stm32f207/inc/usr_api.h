#ifndef _USR_API_H
#define _USR_API_H

extern unsigned char macaddress[6]; 
extern unsigned char cmd_to_usart[5][24];

extern unsigned char cmd_reply[24];
extern unsigned char usart_reply[24];

void Get_STM32F207_ID(void);
void SystemReset(void);
unsigned int Get_RNG_LivingSec(void);
#endif
