#include "stm32f0xx.h"
#include "delay.h"

#define DELAY_TIM TIM14

void delay_init(uint8_t unit)
{
	/* tim14, apb1:48 */
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM14EN, ENABLE);

	TIM_TimeBaseInitTypeDef t;
	t.TIM_CounterMode = TIM_CounterMode_Up;
	t.TIM_ClockDivision = TIM_CKD_DIV1;
	
	switch (unit) {
	case mSEC:
		t.TIM_Prescaler = 47999;
		break;
	case uSEC:
		t.TIM_Prescaler = 47;
	}

	TIM_TimeBaseInit(DELAY_TIM, &t);
}

void delay(uint16_t dur)
{
	TIM_SetAutoreload(DELAY_TIM, dur);
	TIM_Cmd(DELAY_TIM, ENABLE);

	while (TIM_GetFlagStatus(DELAY_TIM, TIM_FLAG_Update) == RESET)
		;

	TIM_Cmd(DELAY_TIM, DISABLE);
	TIM_ClearFlag(DELAY_TIM, TIM_FLAG_Update);
}
