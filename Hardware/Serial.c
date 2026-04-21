#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

/*
 * Serial.c
 * -----------------------------------------------------------------------------
 * USART1 用于和视觉/上位机通信。
 * 当前协议使用固定帧格式：
 *
 *     0xAA + CMD + 0xFF
 *
 * 其中 CMD 只有 1 字节，接收成功后存入 Serial_RxPacket[0]。
 */

char Serial_RxPacket[100];      // 接收缓冲区：当前工程只用到第 1 个字节
uint8_t Serial_RxFlag;          // 接收到完整一帧后置 1，由主循环读取后清零

void Serial_Init(void)
{
    /* 开启时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* GPIO 初始化：PA9 -> TX，PA10 -> RX */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART1 初始化 */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);

    /* 打开串口接收中断 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    /* NVIC 配置 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

/* 发送 1 个字节 */
void Serial_SendByte(uint8_t Byte)
{
    USART_SendData(USART1, Byte);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

/* 发送数组 */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    uint16_t i;
    for (i = 0; i < Length; i++)
    {
        Serial_SendByte(Array[i]);
    }
}

/* 发送字符串 */
void Serial_SendString(char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        Serial_SendByte(String[i]);
    }
}

/* 内部工具函数：计算 X 的 Y 次方，仅供数值发送函数使用 */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/* 以十进制形式发送无符号整数 */
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/* printf 重定向 */
int fputc(int ch, FILE *f)
{
    Serial_SendByte(ch);
    return ch;
}

/* 简单封装的串口 printf */
void Serial_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    Serial_SendString(String);
}

/*
 * 读取“收到一帧数据”的标志位。
 * 如果主循环使用这个函数而不是直接访问 Serial_RxFlag，
 * 则该函数会在返回 1 后自动清零标志。
 */
uint8_t Serial_GetRxFlag(void)
{
    if (Serial_RxFlag == 1)
    {
        Serial_RxFlag = 0;
        return 1;
    }
    return 0;
}

/*
 * USART1 接收中断
 * -----------------------------------------------------------------------------
 * 使用一个简单状态机解析协议帧：
 *
 * 状态 0：等待包头 0xAA
 * 状态 1：接收 1 个命令字节
 * 状态 2：等待包尾 0xFF
 */
void USART1_IRQHandler(void)
{
    static uint8_t RxState = 0;      // 当前状态机状态
    static uint8_t pRxPacket = 0;    // 当前写入缓冲区位置

    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t RxData = USART_ReceiveData(USART1);

        if (RxState == 0)
        {
            /* 等待包头 */
            if (RxData == 0xAA)
            {
                RxState = 1;
                pRxPacket = 0;
            }
        }
        else if (RxState == 1)
        {
            /* 接收命令字节 */
            Serial_RxPacket[pRxPacket] = RxData;
            pRxPacket++;

            /* 当前协议只收 1 个字节，因此立即进入包尾状态 */
            if (pRxPacket >= 1)
            {
                RxState = 2;
            }
        }
        else if (RxState == 2)
        {
            /* 校验包尾 */
            if (RxData == 0xFF)
            {
                RxState = 0;
                Serial_RxFlag = 1;   // 成功接收一帧
            }
            else
            {
                /* 包尾错误时回到初始态，等待下一帧重新同步 */
                RxState = 0;
            }
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
