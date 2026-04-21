#include "stm32f10x.h"                  // Device header
#include "Servo.h"
#include "Motor.h"
#include "PID.h"
#include "Timer.h"

/*
 * Drive.c
 * -----------------------------------------------------------------------------
 * 该文件负责“整车级”的运动控制封装：
 * - 初始化底盘相关模块
 * - 设置双电机统一速度
 * - 停车
 * - 使用偏航角 PID 输出修正舵机角度
 */

/* 偏航角 PID 实例，在 main.c 中完成参数初始化 */
PID_TypeDef pidYaw;

void Drive_Init(void)
{
    Servo_Init();
    Motor_Init();
}

/*
 * 设置左右电机为同一速度，用于直行或整体前进。
 * Speed 由底层 PWM 占空比决定，数值范围请与 Motor/PWM 模块保持一致。
 */
void Drive_SetSpeed(uint8_t Speed)
{
    Motor_SetSpeed(1, Speed);
    Motor_SetSpeed(2, Speed);
}

/* 立即停车 */
void Drive_Stop(void)
{
    Motor_SetSpeed(1, 0);
    Motor_SetSpeed(2, 0);
}

/*
 * 方向控制函数
 * -----------------------------------------------------------------------------
 * target_yaw  : 目标偏航角
 * current_yaw : 当前偏航角（来自 JY61P）
 *
 * 思路：
 * 1. 使用 PID 计算当前偏差对应的修正量
 * 2. 将修正量叠加到 90° 中位角
 * 3. 结果送给舵机，实现方向修正
 */
void ControlDirection(float target_yaw, float current_yaw)
{
    float pid_output = PID_Calculate(&pidYaw, target_yaw, current_yaw);
    float servo_angle = 90 + pid_output;    // 90° 作为直行中位

    /* 舵机角度限幅，避免输出到无效区间 */
    if (servo_angle < 0)   servo_angle = 0;
    if (servo_angle > 180) servo_angle = 180;

    Servo_SetAngle(servo_angle);
}
