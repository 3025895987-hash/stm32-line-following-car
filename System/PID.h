#ifndef __PID_H
#define __PID_H

#include "stm32f10x.h"

/*
 * PID 结构体
 * -----------------------------------------------------------------------------
 * target      : 目标值
 * error       : 当前误差
 * lastError   : 上一次误差
 * integral    : 积分累计值
 * output      : 当前输出值
 * maxOutput   : 输出限幅
 * maxIntegral : 积分限幅
 */
typedef struct {
    float Kp, Ki, Kd;
    float target;
    float error;
    float lastError;
    float integral;
    float output;
    float maxOutput;
    float maxIntegral;
} PID_TypeDef;

void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float maxOut, float maxInt);
float PID_Calculate(PID_TypeDef *pid, float target, float actual);
void PID_Reset(PID_TypeDef *pid);
void PID_SetParam(PID_TypeDef *pid, float Kp, float Ki, float Kd);

#endif
