#include "stm32f0xx.h"
#include "usart1.h"
#include "sp.h"
#include "delay.h"
#include "board.h"

float f = 0;
double d = 0;

void main()
{
        usart1_init();
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

        NVIC_InitTypeDef n;
        n.NVIC_IRQChannel = USART1_IRQn;
        n.NVIC_IRQChannelPriority = 0;
        n.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&n);

        sp_init();
        delay_init(mSEC);
        board_led_init();

        uint8_t u8 = 0;
        spacket su8;
        spacket_init(UINT8_T, 1, &su8);

        uint16_t u16 = 0;
        spacket su16;
        spacket_init(UINT16_T, 2, &su16);

        uint32_t u32 = 0;
        spacket su32;
        spacket_init(UINT32_T, 3, &su32);

        uint64_t u64 = 0;
        spacket su64;
        spacket_init(UINT64_T, 4, &su64);

        int8_t i8 = 0;
        spacket si8;
        spacket_init(INT8_T, 5, &si8);

        int16_t i16 = 0;
        spacket si16;
        spacket_init(INT16_T, 6, &si16);

        int32_t i32 = 0;
        spacket si32;
        spacket_init(INT32_T, 7, &si32);

        int64_t i64 = 0;
        spacket si64;
        spacket_init(INT64_T, 8, &si64);

        spacket sf;
        spacket_init(FLOAT, 9, &sf);

        spacket sd;
        spacket_init(DOUBLE, 10, &sd);

        while (1) {
                u8 += UINT8_T;
                u16 += UINT16_T;
                u32 += UINT32_T;
                u64 += UINT64_T;

                i8 -= INT8_T;
                i16 -= INT16_T;
                i32 -= INT32_T;
                i64 -= INT64_T;

                f += 0.1;
                d += 0.01;

                sp_encode(&u8, su8);
                sp_encode(&u16, su16);
                sp_encode(&u32, su32);
                sp_encode(&u64, su64);

                sp_encode(&i8, si8);
                sp_encode(&i16, si16);
                sp_encode(&i32, si32);
                sp_encode(&i64, si64);

                sp_encode(&f, sf);
                sp_encode(&d, sd);

                sp_decode();
                delay(2000);
        }
}

int sp_tx_send(uint8_t *ptr, uint8_t len)
{
        for (uint8_t i = 0; i < len; ++i)
                usart1_put(ptr[i]);

        sp_reg &= ~sp_tx_lock;
}

/* this function is not working core stops working it goes Reset_Handler*/
void sp_handler(void *vptr, uint16_t addr)
{
        led_toggle(BLUE);

        sp_reg &= ~sp_rx_lock;
}

void sp_error(uint8_t n)
{

}

void USART1_IRQHandler(void)
{
        if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
                sp_frx_put((uint8_t) USART_ReceiveData(USART1));
                USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        }
}
