#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Timer.h"
#include "Servo.h"
#include "Motor.h"
#include "Drive.h"
#include "Hcsr04.h"
#include "jy61p.h"
#include "Serial.h"
#include "PID.h"

/* USART3 用于连接 JY61P 姿态模块 */
void Usart3Init(unsigned int uiBaud);

/*
 * 全局状态变量说明
 * -----------------------------------------------------------------------------
 * distance      : 超声波测得的距离（cm）
 * Angle         : 预留变量，当前代码未实际使用
 * speed1        : 预留速度变量，当前主流程中未使用
 *
 * mode          : 主状态机编号
 * turn_left     : 左转过程中的阶段计数
 * turn_right    : 右转过程中的阶段计数
 * slow_down     : 收到“菱形减速区”标志
 * suspend       : 收到“人行横道”标志
 * suspend_mode  : 人行横道停车后的恢复标志
 * stop          : 收到“红线停车”标志
 * left_mode     : 收到“左转标志”任务
 * right_mode    : 收到“右转标志”任务
 * L_mode        : 左转执行过程中的阶段标志
 * R_mode        : 右转执行过程中的阶段标志
 * stop_mode     : 最终停车阶段标志
 * R_stop/L_stop : 记录是否已经进入对应方向后的停车流程
 */
uint16_t distance, Angle, speed1 = 50;
uint8_t mode = 0;
uint8_t turn_left = 0, turn_right = 0;
uint8_t slow_down = 0, suspend = 0;
uint8_t jy61_time = 0, suspend_mode = 0;
uint8_t stop = 0, left_mode = 0, right_mode = 0;
uint8_t L_mode = 0, R_mode = 0, stop_mode = 0;
uint8_t R_stop = 0, L_stop = 0;

int main(void)
{
    float CurrentAngle = 0.0f;

    /*
     * 初始化顺序
     * 1. 底盘与舵机
     * 2. 定时器 / OLED / 串口 / 超声波
     * 3. JY61P 所在的 USART3
     * 4. 偏航角 PID
     */
    Drive_Init();
    Servo_SetAngle(90);                // 舵机回中，90° 作为直行基准

    Timer_Init();
    OLED_Init();
    Serial_Init();                     // USART1：视觉/上位机串口
    HCSR04_Init();
    Usart3Init(9600);                  // USART3：JY61P 串口
    JY61P_Init();
    PID_Init(&pidYaw, 2.5f, 0.0f, 0.8f, 30.0f, 100.0f);

    /* OLED 固定显示区域 */
    OLED_ShowString(1, 1, "distance:");
    OLED_ShowString(1, 13, "cm");
    OLED_ShowString(2, 1, "Yaw:");
    OLED_ShowString(3, 1, "Status:");
    Delay_s(1);

    while (1)
    {
        /* 读取 JY61P 的相对偏航角，用于方向闭环控制 */
        if (JY61P_DataReady())
        {
            CurrentAngle = JY61P_GetRelativeYaw();
            OLED_ShowSignedNum(2, 5, (int)CurrentAngle, 3);
        }

        /*
         * 主状态机
         * ---------------------------------------------------------------------
         * mode = 0  : 待命
         * mode = 1  : 右道直行
         * mode = 2  : 左转
         * mode = 3  : 左道直行
         * mode = 4  : 右转
         * mode = 5  : 最后一段右道直行
         * mode = 6  : 菱形减速区
         * mode = 7  : 人行横道停车/恢复
         * mode = 8  : 执行右转标志任务
         * mode = 9  : 执行左转标志任务
         * mode = 10 : 红线停车
         */
        switch (mode)
        {
            case 0:
                /* 等待启动命令 */
                break;

            case 1:     /* 右道直行 */
                ControlDirection(0, CurrentAngle);
                Drive_SetSpeed(50);
                distance = HCSR04_GetValue();
                OLED_ShowNum(1, 10, distance, 3);
                if ((distance >= 30) && (distance <= 40))
                {
                    mode = 2;
                }
                break;

            case 2:     /* 左转 */
                Drive_SetSpeed(50);
                if (turn_left == 0)
                {
                    Servo_SetAngle(55);
                }
                else if (turn_left == 1)
                {
                    Servo_SetAngle(140);
                }
                else if (turn_left == 2)
                {
                    ControlDirection(0, CurrentAngle);
                }
                else if (turn_left >= 3)
                {
                    mode = 3;
                    turn_left = 0;
                }
                break;

            case 3:     /* 左道直行 */
                ControlDirection(0, CurrentAngle);
                distance = HCSR04_GetValue();
                OLED_ShowNum(1, 10, distance, 3);
                Drive_SetSpeed(50);
                if ((distance >= 30) && (distance <= 40))
                {
                    mode = 4;
                }
                break;

            case 4:     /* 右转 */
                Drive_SetSpeed(50);
                if (turn_right == 0)
                {
                    Servo_SetAngle(130);
                }
                else if (turn_right == 1)
                {
                    Servo_SetAngle(70);
                }
                else if (turn_right == 2)
                {
                    ControlDirection(0, CurrentAngle);
                }
                else if (turn_right == 3)
                {
                    mode = 5;
                    turn_right = 0;
                }
                break;

            case 5:     /* 最后一段右道直行 */
                ControlDirection(0, CurrentAngle);
                Drive_SetSpeed(50);
                if (slow_down == 1)
                {
                    mode = 6;
                }
                break;

            case 6:     /* 检测到菱形减速后低速通过 */
                ControlDirection(0, CurrentAngle);
                Drive_SetSpeed(30);
                if (suspend == 1)
                {
                    mode = 7;
                }
                break;

            case 7:     /* 人行横道停车，满足条件后恢复运行 */
                ControlDirection(0, CurrentAngle);
                if (suspend_mode == 0)
                {
                    Drive_Stop();
                }
                else if (suspend_mode == 1)
                {
                    Drive_SetSpeed(50);
                }

                if (right_mode == 1)
                {
                    mode = 8;
                }
                else if (left_mode == 1)
                {
                    mode = 9;
                }
                break;

            case 8:     /* 执行右转标志对应动作 */
                Drive_SetSpeed(50);
                if (R_mode == 0)
                {
                    ControlDirection(0, CurrentAngle);
                }
                else if (R_mode == 1)
                {
                    Servo_SetAngle(155);
                }
                else if (R_mode == 2)
                {
                    Servo_SetAngle(80);
                }
                else if (R_mode == 3)
                {
                    ControlDirection(90, CurrentAngle);
                }

                if (stop == 1)
                {
                    R_stop++;
                    mode = 10;
                }
                break;

            case 9:     /* 执行左转标志对应动作 */
                Drive_SetSpeed(50);
                if (L_mode == 0)
                {
                    ControlDirection(0, CurrentAngle);
                }
                if (L_mode == 1)
                {
                    Servo_SetAngle(30);
                }
                else if (L_mode == 2)
                {
                    Servo_SetAngle(95);
                }
                else if (L_mode == 3)
                {
                    ControlDirection(-90, CurrentAngle);
                }

                if (stop == 1)
                {
                    L_stop++;
                    mode = 10;
                }
                break;

            case 10:    /* 检测到红线后停车 */
                if (R_stop >= 1) R_stop = 1;
                if (L_stop >= 1) L_stop = 1;
                if (stop_mode == 1)
                {
                    Drive_Stop();
                }
                break;
        }

        /*
         * 串口命令处理（USART1）
         * 协议格式：0xAA + CMD + 0xFF
         * 当前只解析 1 字节命令区。
         */
        if (Serial_RxFlag == 1)
        {
            if (Serial_RxPacket[0] == 0x00)
            {
                slow_down = 1;          // 检测到菱形减速区
            }
            else if (Serial_RxPacket[0] == 0x01)
            {
                suspend = 1;            // 检测到人行横道
            }
            else if (Serial_RxPacket[0] == 0x02)
            {
                left_mode = 1;          // 检测到左转标志
            }
            else if (Serial_RxPacket[0] == 0x03)
            {
                right_mode = 1;         // 检测到右转标志
            }
            else if (Serial_RxPacket[0] == 0x04)
            {
                stop = 1;               // 检测到红线，准备停车
            }
            else if (Serial_RxPacket[0] == 0x05)
            {
                mode = 1;               // 开机启动，进入第一段直行
            }

            /* 标记该帧已处理完成，准备接收下一帧 */
            Serial_RxFlag = 0;
        }

        Delay_ms(5);    // 主循环节拍，约 5 ms 一次
    }
}
