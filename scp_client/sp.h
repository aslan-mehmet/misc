/*
 * author: Mehmet ASLAN
 * date: December 10, 2016
 * rev: 0.1
 *
 * no warranty, no licence agreement
 * use it at your own risk
 */

#ifndef __SP_H
#define __SP_H

#include <stdint.h>
#include "bits.h"

#define PCL 12			/* packet content len for size + 8 byte + addr + checksum */
#define MPL 28          	/* max packet len (4 + PCL*2) */
#define FRXS 50			/* fifo rx size at least one packet must fit */

/* serial packet errors that could happen
 * sp lib does not contain what to do when error happen
 * it just calls sp_error with an error code
 * sp_error declared as a weak function
 */
#define SPE_SUCCESS 0
#define SPE_UNDEF_SEQ 1
#define SPE_PAR 2
#define SPE_LEN 3
#define SPE_UNDEF_ADDR 4
#define SPE_CS 5
#define SPE_RX_FULL 6
#define SPE_FRX_FULL 7
#define SPE_TX_FULL 8 		/* doesnt fit to MPL */

extern uint8_t sp_reg;
#define sp_tx_lock BIT8_0
#define sp_rx_lock BIT8_1
#define sp_start_rcvd BIT8_2

struct _spacket{
	uint8_t vsize; 		/* sizeof packet variable */
	uint16_t addr; 		/* packet address */
};

typedef struct _spacket spacket;

/* check it must return 0
 * otherwise it could not allocate memory space for internal buffer
 */
int8_t sp_init(void);
/* handle bytes in fifo rx non-blocking */
void sp_decode(void);
/* any two packets can't have same packet addr */
/* data type can any of below. it is basically their size */
#define UINT8_T 1
#define UINT16_T 2
#define UINT32_T 4
#define UINT64_T 8
#define INT8_T 1
#define INT16_T 2
#define INT32_T 4
#define INT64_T 8
#define FLOAT 4
#define DOUBLE 8
void spacket_init(uint8_t data_type, uint16_t packet_addr, spacket *p);

/*
 * vptr supports all data types but only one of them for a single packet
 * implicitly locks sp_tx_lock you have to unlock at sp_tx_send
 */
int sp_encode(void *vptr, spacket packet);
/* write to serial packet fifo rx
 * put uint8_t values from usart
 */
void sp_frx_put(uint8_t v);


/* weak functions they must be declared before using lib */
/* sp_tx_lock implicity locked, dont forget to unlock */
int sp_tx_send(uint8_t *ptr, uint8_t len) __attribute__ ((weak));
/* handle received packet and unlock sp_rx_lock */
void sp_handler(void *vptr, uint16_t addr) __attribute__ ((weak));
void sp_error(uint8_t n) __attribute__ ((weak));

#endif
