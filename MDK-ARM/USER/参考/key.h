#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"



#define KEY4  PAin(0)
#define KEY1  PBin(8)
#define KEY2  PBin(9)
#define KEY3  PAin(1)


#define		SEND_UART		1 //���ڷ���
#define		SEND_USB		2 //3USB����

extern u8 work_state;		//����״̬

void KEY1_Init(void);	

uint8_t key_scan(void);


#endif
