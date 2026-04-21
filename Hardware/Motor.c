#include "stm32f10x.h"                  // Device header
#include "PWM.h"

/*
 * 电机引脚说明
 * -----------------------------------------------------------------------------
 * 左电机方向控制：PA2 / PA3
 * 右电机方向控制：PA4 / PA5
 * 左右电机 PWM   ：由 PWM.c 提供（TIM2 -> PA0 / PA1）
 */

void Motor_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    PWM_Init();
}

/*
 * 设置单个电机速度
 * -----------------------------------------------------------------------------
 * n     : 1 表示左电机，2 表示右电机
 * Speed : 有符号速度值
 *         - Speed >= 0：正转
 *         - Speed < 0 ：反转
 *
 * 这里的“正转/反转”是相对于当前接线定义的逻辑方向；
 * 如果你的小车前后方向相反，只需要交换方向引脚的高低电平即可。
 */
void Motor_SetSpeed(uint8_t n, int8_t Speed)
{
    if (n == 1)
    {
        if (Speed >= 0)
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_2);
            GPIO_ResetBits(GPIOA, GPIO_Pin_3);
            PWM_SetCompare1(Speed);
        }
        else
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_2);
            GPIO_SetBits(GPIOA, GPIO_Pin_3);
            PWM_SetCompare1(-Speed);
        }
    }
    else if (n == 2)
    {
        if (Speed >= 0)
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_4);
            GPIO_ResetBits(GPIOA, GPIO_Pin_5);
            PWM_SetCompare2(Speed);
        }
        else
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_4);
            GPIO_SetBits(GPIOA, GPIO_Pin_5);
            PWM_SetCompare2(-Speed);
        }
    }
}
