#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

void Motor_Init(void);

/*
 * n = 1：左电机
 * n = 2：右电机
 * Speed 为有符号速度值，正负号决定方向
 */
void Motor_SetSpeed(uint8_t n, int8_t Speed);

#endif
