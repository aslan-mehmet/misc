#include "usart1.h"

void usart1_init(void)
{
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* pa9:tx
	 * pa10:rx
	 */
	GPIO_InitTypeDef g;
	g.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	g.GPIO_Mode = GPIO_Mode_AF;
	g.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &g);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

	USART_InitTypeDef u;
	USART_StructInit(&u);
	u.USART_BaudRate = 38400;

	USART_Init(USART1, &u);

	USART_Cmd(USART1, ENABLE);
}

void usart1_put(uint8_t d)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
                        ;
	USART_SendData(USART1,d);
}

/* hardware dependent stub for printf */
int _write(int file, char *ptr, int len)
{
        file = 0;
        int i;
        for (i = 0; i < len; ++i) {
                while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
                        ;
                USART_SendData(USART1,(uint8_t) ptr[i]);
        }
        return i;
}
