#include "stm32f10x.h"                  // Device header
extern uint8_t mode,turn_left,turn_right,slow_down,suspend,suspend_mode,left_mode,right_mode,L_mode,R_mode,stop_mode,R_stop,L_stop;
extern uint16_t speed1;
uint16_t  left_time,right_time,suspend_time,speed_time,L_time,R_time,stop_time;

void Timer_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	TIM_InternalClockConfig(TIM1);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;  //72MHz/72000 = 1000Hz，1ms计数一次
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM1, ENABLE);
}

void TIM1_UP_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
//超声波控制部分延时
		//左转部分延时
			if(mode==2)
			{
				left_time++;
				if(left_time>=900)
				{
					turn_left++;
					left_time=0;
				}
			}
			
			//右转部分延时
			if(mode==4)
			{
				right_time++;
				if(right_time>=750)
				{
					turn_right++;
					right_time=0;
				}
			}
			
//检测到人行道停车6s
			if(mode==7)
			{
				suspend_time++;
				if(suspend_time>=6000)
				{
					suspend_mode=1;
					suspend_time=0;
				}
			}
			
//检测到左转指令
			if(mode==9)
			{
				L_time++;
				if(L_time>=1150)
				{
					L_mode++;
					L_time=0;
				}
				if(L_mode>=3) L_mode=3;
			}
//			
//检测到右转指令
			if(mode==8)
			{ 
				R_time++;
				if(R_time>=1150)
				{
					R_mode++;
					R_time=0;
				}
				if(R_mode>=3) R_mode=3;
			}
//			
//检测到红线停车
			if((mode==10) && (R_stop==1))
			{
				stop_time++;
				if(stop_time>=1000)
				{
					stop_mode=1;
					stop_time=0;
				}
			}
			else if((mode==10) && (L_stop==1))
			{
				stop_time++;
				if(stop_time>=1000)
				{
					stop_mode=1;
					stop_time=0;
				}
			}
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
