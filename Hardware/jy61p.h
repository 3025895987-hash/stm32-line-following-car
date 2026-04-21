#ifndef __JY61P_H
#define __JY61P_H

#include "stm32f10x.h"                  // Device header
#include <stdint.h>

void jy61p_ReceiveData(uint8_t RxData);

void JY61P_Init(void);
uint8_t JY61P_DataReady(void);  // 添加这个函数声明

float JY61P_GetYaw(void);
float JY61P_GetPitch(void);
float JY61P_GetRoll(void);

void JY61P_ResetHome(void);
float JY61P_GetRelativeYaw(void);

extern float Roll,Pitch,Yaw;

#endif
