#include "stm32f10x.h"
#include "Delay.h"
#include "Hcsr04.h"

/*
 * Hcsr04.c
 * -----------------------------------------------------------------------------
 * 使用 HC-SR04 完成距离测量。
 *
 * 当前实现思路：
 * 1. 通过 Trig 发送 >=10us 高电平触发信号
 * 2. 使用 TIM4 周期中断轮询 Echo 引脚电平变化
 * 3. 在 Echo 为高电平期间累计 Time
 * 4. 将 Time 转换为距离（cm）
 */

static uint16_t Time = 0;       // Echo 高电平持续计数
static uint8_t Measuring = 0;   // 1 表示正在测量
static uint8_t Echo_State = 0;  // 保存上一时刻 Echo 电平，用于检测边沿

/* TIM4 初始化：提供 HC-SR04 的测量时基 */
void TIM4_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 7199;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM4, ENABLE);
}

/* TIM4 中断：根据 Echo 引脚高低电平完成计时 */
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        uint8_t current_state = GPIO_ReadInputDataBit(Echo_Port, Echo_Pin);

        /* 上升沿：开始测量 */
        if ((current_state == 1) && (Echo_State == 0))
        {
            Time = 0;
            Measuring = 1;
        }
        /* 下降沿：结束测量 */
        else if ((current_state == 0) && (Echo_State == 1))
        {
            Measuring = 0;
        }

        if (Measuring && (current_state == 1))
        {
            Time++;
            if (Time > 5000)   // 超时保护，避免异常卡死
            {
                Measuring = 0;
                Time = 0;
            }
        }

        Echo_State = current_state;
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}

void HCSR04_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Trig：输出 */
    RCC_APB2PeriphClockCmd(Trig_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = Trig_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Trig_Port, &GPIO_InitStructure);

    /* Echo：输入 */
    RCC_APB2PeriphClockCmd(Echo_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = Echo_Pin;
    GPIO_Init(Echo_Port, &GPIO_InitStructure);

    TIM4_Init();

    /* 上电默认拉低触发脚 */
    GPIO_ResetBits(Trig_Port, Trig_Pin);
}

/*
 * 获取距离值（单位：cm）
 * -----------------------------------------------------------------------------
 * 注意：这里 Time 的时间单位取决于 TIM4 中断频率，当前代码按 0.1ms 做换算。
 * 如果你修改了 TIM4 的分频或周期，这里的距离换算也要一起检查。
 */
uint16_t HCSR04_GetValue(void)
{
    uint32_t timeout = 0;

    /* 发送触发脉冲 */
    GPIO_SetBits(Trig_Port, Trig_Pin);
    Delay_us(15);
    GPIO_ResetBits(Trig_Port, Trig_Pin);

    Time = 0;
    Measuring = 0;

    /* 等待上升沿：开始测量 */
    timeout = 0;
    while ((Measuring == 0) && (timeout < 100))
    {
        Delay_us(100);
        timeout++;
    }

    /* 等待下降沿：结束测量 */
    timeout = 0;
    while ((Measuring == 1) && (timeout < 500))
    {
        Delay_us(100);
        timeout++;
    }

    if (Time > 0)
    {
        uint16_t result = Time * 17 / 10;
        if (result > 400) result = 400;   // 保护上限
        return result;
    }

    return 400;   // 测量失败时返回 400，表示“远处/未测到”
}
