/*
 * author: Mehmet ASLAN
 * date: December 10, 2016
 * rev: 0.1
 * no warranty, no licence agreement
 * use it at your own risk
 */

#include "sp.h"
#include "fifo.h"

/* escape sequences */
#define ESC_SP 0x96		/* sp escape byte */
#define ESC_START 1
#define ESC_STOP 2
#define ESC_PAR 3		/* parity */
#define ESC_SELF 4		/* itself */

/* private functions */
/* global variables */
uint8_t sp_reg = 0;

/* private variables */
static uint8_t txi = 0;
static uint8_t tx[MPL];

static uint8_t rx[PCL];
static uint8_t rxi = 0;

static fifo *pfrx;

static uint64_t dword_bfr;
static uint8_t *dword_ptr;

int8_t sp_init(void)
{
	tx[0] = ESC_SP;
	tx[1] = ESC_START;

	static fifo frx;
	pfrx = &frx;
	if (init_fifo(pfrx, FRXS))
		return -1;

        dword_ptr = (uint8_t *) &dword_bfr;

        /* dword has to be aligned, otherwise huge problem */
        if (((uint32_t) dword_ptr) & ((uint32_t) 0x3))
                return -2;

	return 0;
}

/*
 * ENCODING PART
 */

/* marks escape sequence in packet content with ESC_SELF
 * can fill all buffer
 * check at the end is there enough space to put stop seq
 */
static void put_tx(uint8_t b)
{
        if (txi == MPL)
                return;

	tx[txi++] = b;

	if (b == ESC_SP && txi != MPL)
              tx[txi++] = ESC_SELF;
}

int sp_encode(void *vptr, uint8_t vsize, uint16_t addr)
{
	if (sp_reg & sp_tx_lock)
		return -1;

	uint8_t cs = 0;		/* checksum */
	uint8_t *ptr = (uint8_t *) vptr;

	txi = 2;

	put_tx(vsize);

	for (int i = 0; i < vsize; ++i) {
		cs ^= ptr[i];

		put_tx(ptr[i]);
	}

	uint8_t *tp = (uint8_t *) &addr;

	for (int i = 0; i < 2; ++i) {
		cs ^= tp[i];

		put_tx(tp[i]);
	}

	put_tx(cs);

	if (txi > (MPL -2)) {
		/* dont have space to put stop seq */
		sp_error(SPE_TX_FULL);
		return -2;
	}

	tx[txi++] = ESC_SP;
	tx[txi++] = ESC_STOP;

	sp_reg |= sp_tx_lock;

        sp_tx_send(tx, txi);

	return 0;
}

/*
 * DECODING PART
 */

#define var_size rx[0]
/* their locations do not change relative to the end of packet */
#define rcvd_cs rx[rxi-1]
#define addrh rx[rxi-2]
#define addrl rx[rxi-3]

static void process_rx(void)
{
	uint16_t addr;
	uint8_t *paddr = (uint8_t *) (&addr);
	uint8_t cs = 0;

	/* check if packet right size */
	if (rxi != var_size + 4) {
		sp_error(SPE_LEN);
		return;
	}

	for (int i = 0; i < var_size; ++i) {
                dword_ptr[i] = rx[i+1];
		cs ^= rx[i+1];
        }


	cs ^= addrl;
	cs ^= addrh;

	if (cs != rcvd_cs) {
		sp_error(SPE_CS);
                return;
        }

	paddr[0] = addrl;
	paddr[1] = addrh;

        sp_error(SPE_SUCCESS);

	sp_reg |= sp_rx_lock;
	sp_handler(dword_ptr, addr);
}

static void put_rx(uint8_t v)
{
	if (sp_reg & sp_start_rcvd) {
		if (rxi == PCL)
			sp_error(SPE_RX_FULL);
		else
			rx[rxi++] = v;
	}
}

uint8_t esc_flag = 0;

void sp_decode(void)
{
	if (sp_reg & sp_rx_lock)
		return;

	int r;

	while ((r = read_fifo(pfrx)) >= 0) {
		if (r == ESC_SP) {
			esc_flag = 1;
			continue;
		}

		if (esc_flag) {
			switch (r) {
			case ESC_START:
				sp_reg |= sp_start_rcvd;
				rxi = 0;
				break;
			case ESC_STOP:
				if (sp_reg & sp_start_rcvd)
					process_rx();

				sp_reg &= ~sp_start_rcvd;
				break;
			case ESC_PAR:
				sp_error(SPE_PAR);
				break;
			case ESC_SELF:
				put_rx((uint8_t) ESC_SP);
				break;
			default:
				sp_error(SPE_UNDEF_SEQ);
			}
		} else {
			put_rx((uint8_t) r);
		}

		esc_flag = 0;
	}
}

void sp_frx_put(uint8_t v)
{
        if (write_fifo(pfrx,  v))
		sp_error(SPE_FRX_FULL);
}
