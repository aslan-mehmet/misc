#ifndef __USART1_H
#define __USART1_H

#include "stm32f0xx.h"
#include <stdio.h>

void usart1_init(void);
void usart1_put(uint8_t d);

#endif
