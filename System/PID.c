#include "PID.h"

/*
 * PID 控制器初始化
 * -----------------------------------------------------------------------------
 * Kp     : 比例项系数，误差越大，修正越强
 * Ki     : 积分项系数，用于消除长期累计误差
 * Kd     : 微分项系数，用于抑制变化过快导致的震荡
 * maxOut : 输出限幅
 * maxInt : 积分限幅，避免积分饱和
 */
void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float maxOut, float maxInt)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->target = 0;
    pid->error = 0;
    pid->lastError = 0;
    pid->integral = 0;
    pid->output = 0;
    pid->maxOutput = maxOut;
    pid->maxIntegral = maxInt;
}

/*
 * PID 核心计算函数
 * -----------------------------------------------------------------------------
 * target : 目标值
 * actual : 实际值
 * 返回值 : 当前控制量输出
 */
float PID_Calculate(PID_TypeDef *pid, float target, float actual)
{
    float derivative;

    pid->target = target;
    pid->error = target - actual;

    /* 积分项累加，并进行限幅 */
    pid->integral += pid->error;
    if (pid->integral > pid->maxIntegral)  pid->integral = pid->maxIntegral;
    if (pid->integral < -pid->maxIntegral) pid->integral = -pid->maxIntegral;

    /* 微分项：误差变化量 */
    derivative = pid->error - pid->lastError;

    pid->output = pid->Kp * pid->error +
                  pid->Ki * pid->integral +
                  pid->Kd * derivative;

    /* 输出限幅 */
    if (pid->output > pid->maxOutput)  pid->output = pid->maxOutput;
    if (pid->output < -pid->maxOutput) pid->output = -pid->maxOutput;

    pid->lastError = pid->error;
    return pid->output;
}

/* 清空 PID 历史状态，常用于重新开始控制时 */
void PID_Reset(PID_TypeDef *pid)
{
    pid->error = 0;
    pid->lastError = 0;
    pid->integral = 0;
    pid->output = 0;
}

/* 动态调整 PID 参数 */
void PID_SetParam(PID_TypeDef *pid, float Kp, float Ki, float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}
