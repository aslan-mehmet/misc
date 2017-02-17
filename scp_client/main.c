/*
 * author: Mehmet ASLAN
 * date: February 16, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "main.h"
#include "interface.h"
#include <ncurses.h>
#include "input.h"
#include "rs232.h"
#include "sp.h"

#define BUF_LEN 30

void auto_write_received(void *vptr, uint16_t addr);
void sig_handler(int signum);

int sp_mode = SP_MODE_VARIABLE;
int serial_port = 16; // ttyUSB0

int main(int argc, char **argv)
{
        // 8bit, no parity, 1 stop bit, not used
        char mode[] = {'8', 'N', '1', 0};
        int baud = 38400;
        uint8_t buf[BUF_LEN];
        int rcvd;

        signal(SIGINT, sig_handler);
        // interface depends on mode
        init_interface();

        switch (sp_mode) {
        case SP_MODE_VARIABLE:
                set_title("serial comm protocol (variable mode)");
                // var mode read init script
        break;
        }

        read_initvar();
        sp_init();

        if (RS232_OpenComport(serial_port, baud, mode)) {
                write_input_buffer("can not open port");
                while (1)
                        ;
        }

        while (1) {
                int tmp_key;

                while ((tmp_key = getch()) != ERR)
                        got_key_input(tmp_key);

                rcvd = RS232_PollComport(serial_port, buf, BUF_LEN);
                if (rcvd > 0)
                        for (int i = 0; i < rcvd; ++i)
                                sp_frx_put(buf[i]);

                sp_decode();

                usleep(30);
        }

        return 0;
}

// use sigint to exit
void sig_handler(int signum)
{
        if (signum == SIGINT) {
                RS232_CloseComport(serial_port);
                destroy_interface();
                exit(0);
        }
}


// serial communication weak functions
void sp_handler(void *vptr, uint16_t addr)
{
        // got data with addr, get type cast
        switch (sp_mode) {
        case SP_MODE_VARIABLE:
                auto_write_received(vptr, addr); // directl send to screen
                break;
        }

        sp_reg &= ~sp_rx_lock;
}

void sp_error(uint8_t n)
{
        write_input_buffer("err %d", n);
}

int sp_tx_send(uint8_t *ptr, uint8_t len)
{
        RS232_SendBuf(serial_port, ptr, len);
        sp_reg &= ~sp_tx_lock;
}

void auto_write_received(void *vptr, uint16_t addr)
{
// got addr, get var from initvar
        var *vp = get_var_with_addr(addr);
        if (vp == NULL) {
                write_input_buffer("i dont have that addr %x", addr);
                return;
        }

        uint8_t *u8;
        uint16_t *u16;
        uint32_t *u32;
        uint64_t *u64;
        int8_t *i8;
        int16_t *i16;
        int32_t *i32;
        int64_t *i64;
        float *f;
        double *d;
// get type, cast void pointer
        switch (vp->data_type) {
        case TYPE_UINT8_T:
                u8 = (uint8_t *) vptr;
                write_pinned_buffer(vp->loc, "%s %d", vp->str, *u8);
                break;
        case TYPE_UINT16_T:
                u16 = (uint16_t *) vptr;
                write_pinned_buffer(vp->loc, "%s %d", vp->str, *u16);
                break;
        case TYPE_UINT32_T:
                u32 = (uint32_t *) vptr;
                write_pinned_buffer(vp->loc, "%s %d", vp->str, *u32);
                break;
        case TYPE_UINT64_T:
                u64 = (uint64_t *) vptr;
                write_pinned_buffer(vp->loc, "%s %d", vp->str, *u64);
                break;
        case TYPE_INT8_T:
                i8 = (int8_t *) vptr;
                write_pinned_buffer(vp->loc, "%s %d", vp->str, *i8);
                break;
        case TYPE_INT16_T:
                i16 = (int16_t *) vptr;
                write_pinned_buffer(vp->loc, "%s %d", vp->str, *i16);
                break;
        case TYPE_INT32_T:
                i32 = (int32_t *) vptr;
                write_pinned_buffer(vp->loc, "%s %d", vp->str, *i32);
                break;
        case TYPE_INT64_T:
                i64 = (int64_t *) vptr;
                write_pinned_buffer(vp->loc, "%s %d", vp->str, *i64);
                break;
        case TYPE_FLOAT:
                f = (float *) vptr;
                write_pinned_buffer(vp->loc, "%s %f", vp->str, *f);
                break;
        case TYPE_DOUBLE:
                d = (double *) vptr;
                write_pinned_buffer(vp->loc, "%s %f", vp->str, *d);
                break;
        }
// var.str : value to pinned buffer
}
