#include "stm32f0xx.h"
#include "usart1.h"
#include "board.h"
#include "delay.h"
#include "sp.h"

float f = 0;

void main()
{
	usart1_init();
	board_led_init();
	delay_init(mSEC);
	if (sp_init())
                while (1)
                        led_on(BLUE);

	while (1) {
                sp_encode(&f, FLOAT, 1);
                f += 0.1;

                delay(1000);
                sp_decode();
	}
}

void USART1_IRQHandler(void)
{
        if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
                uint8_t data = USART_ReceiveData(USART1);
                sp_frx_put(data);
                led_toggle(BLUE);
                USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        }
}

int sp_tx_send(uint8_t *ptr, uint8_t len)
{
        for (int i = 0; i < len; ++i)
                usart1_put(ptr[i]);

        sp_reg &= ~sp_tx_lock;
}
void sp_handler(void *vptr, uint16_t addr)
{
        f = *((float *) vptr);

        sp_reg &= ~sp_rx_lock;
}
void sp_error(uint8_t n)
{
	sp_encode(&n, UINT8_T, 0);
}
