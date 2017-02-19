#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f0xx.h"

#define GREEN GPIO_ODR_9
#define BLUE GPIO_ODR_8

#define led_on(v) GPIOC->ODR |= (v)
#define led_off(v) GPIOC->ODR &= ~(v)
#define led_toggle(v) GPIOC->ODR ^= (v)

void board_led_init(void);

#endif
