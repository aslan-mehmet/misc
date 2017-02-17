/*
 * author: Mehmet ASLAN
 * date: December 10, 2016
 *
 * no warranty, no licence agreement
 * use it at your own risk
 */

#include "fifo.h"

int8_t is_fifo_full(fifo *s);

int8_t init_fifo(fifo *s, uint8_t len)
{
	s->tail = 0;
	s->head = 0;
	s->len = len + 1;
	s->buf = (uint8_t *) malloc(s->len);

	if (s->buf == NULL)
		return -1;

	return 0;
}

void deinit_fifo(fifo *s)
{
	free(s->buf);
}

int8_t write_fifo(fifo *s, uint8_t d)
{
	if (is_fifo_full(s))
		return -1;

        s->buf[s->head++] = d;

        if (s->head == s->len)
        	s->head = 0;

	return 0;
}

int16_t read_fifo(fifo *s)
{
	if (s->tail != s->head) {
		uint8_t tmp = s->buf[s->tail++];

		if (s->tail == s->len)
			s->tail = 0;

		return tmp;
	}

	return -1;
}

void flush_fifo(fifo *s)
{
	s->head = s->tail;
}

int8_t is_fifo_full(fifo *s)
{
        int16_t t = s->tail;
        --t;

        if (t < 0) {
        	t = s->len - 1;

        	if (s->head == t)
        		return 1;
        } else if (s->head == t) {
        	return 1;
        }

        return 0;
}

