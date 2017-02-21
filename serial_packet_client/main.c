/*
 * author: Mehmet ASLAN
 * date: February 16, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */

#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "sp.h"
#include <inttypes.h>
#include "screen.h"
#include <signal.h>
#include <unistd.h>
#include "rs232.h"

int port_number = 16;

void sig_handler(int signum)
{
        if (signum == SIGINT) {
                RS232_CloseComport(port_number);
                destroy_screen();
                exit(0);
        }
}

int main()
{
        signal(SIGINT, sig_handler);

        FILE *f = fopen("spinit", "r");
        char *line;
        size_t line_len = 0;

        if (f == NULL)
                return -1;

        char mode[] = {'8', 'N', '1', 0};
        int baudrate = 38400;
        uint8_t port_buffer[100];

        while (getline(&line, &line_len, f) != -1) {
                int tmp = strlen(line);

                if (line[tmp-1] == '\n')
                        line[tmp-1] = '\0';

                handle_keyboard_input(line);
        }

        init_screen(get_pin_number());
        sp_init();

        if (RS232_OpenComport(port_number, baudrate, mode))
                return -2;

        while (1) {
                int received = RS232_PollComport(port_number, port_buffer, 100);

                if (received > 0)
                        for (int i = 0; i < received; ++i)
                                sp_frx_put(port_buffer[i]);

                sp_decode();
                get_keys();
                usleep(30);
        }
        return 0;
}

int sp_tx_send(uint8_t *ptr, uint8_t len)
{
        RS232_SendBuf(port_number, ptr, len);
        sp_reg &= ~sp_tx_lock;
        return len;
}
void sp_handler(void *vptr, uint16_t addr)
{
        display_received_packet(vptr, addr);
        sp_reg &= ~sp_rx_lock;
}
void sp_error(uint8_t n)
{
}
