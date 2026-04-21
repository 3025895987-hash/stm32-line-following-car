#ifndef __DRIVE_H
#define __DRIVE_H

#include "stm32f10x.h"                  // Device header
#include "PID.h"

/* 偏航角 PID 控制器，在 Drive.c 中定义 */
extern PID_TypeDef pidYaw;

void Drive_Init(void);
void Drive_SetSpeed(uint8_t Speed);
void Drive_Stop(void);
void ControlDirection(float target_yaw, float current_yaw);

#endif
