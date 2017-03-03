/*
 * author: Mehmet ASLAN
 * date: December 10, 2016
 *
 * no warranty, no licence agreement
 * use it at your own risk
 */

#ifndef __FIFO
#define __FIFO

#include <stdint.h>
#include <stdlib.h>

struct my_fifo{
	uint8_t *buf;
	uint8_t head, tail;
	uint8_t len;
};

typedef struct my_fifo fifo;

int8_t init_fifo(fifo *s, uint8_t len);
void deinit_fifo(fifo *s);

int8_t write_fifo(fifo *s, uint8_t d);
int16_t read_fifo(fifo *s);
void flush_fifo(fifo *s);

#endif
