#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "sp.h"
#include <inttypes.h>
#include "screen.h"
#include <signal.h>
#include <unistd.h>
#include "rs232.h"

void update_status(void);

int port_number = 16;
int is_port_open = 1;
int rcvd_successful_packet = 0;
int rcvd_faulty_packet = 0;
int sent_packet_successful = 0;
int sent_packet_faulty = 0;

void sig_handler(int signum)
{
        if (signum == SIGINT) {
                if (is_port_open)
                        RS232_CloseComport(port_number);
                destroy_screen();
                exit(0);
        }
}

int main()
{
        fclose(stderr);

        stderr = fopen("errors", "w");

        if (stderr == NULL)
                return -1;

        signal(SIGINT, sig_handler);

        FILE *f = fopen("spinit", "r");
        char *line;
        size_t line_len = 0;

        if (f == NULL) {
                fprintf(stderr, "no init file\n");
                return -2;
        }

        char mode[] = {'8', 'N', '1', 0};
        int baudrate = 38400;
        uint8_t port_buffer[100];
        int received;

        if (RS232_OpenComport(port_number, baudrate, mode)) {
                fprintf(stderr, "can not open port\n");
                is_port_open = 0;
        }

        handle_keyboard_input("; status 0 uint8_t r");
        handle_keyboard_input("$ status");

        while (getline(&line, &line_len, f) != -1) {
                int tmp = strlen(line);

                if (line[tmp-1] == '\n')
                        line[tmp-1] = '\0';

                handle_keyboard_input(line);
        }

        if (init_screen(get_pin_number())) {
                fprintf(stderr, "cant allocate display buffers why\n");
                sig_handler(SIGINT);
        }

        if (!is_port_open)
                print_to_screen("port not opened", -1);

        sp_init();

        while (1) {
                update_status();
                get_keys();
                usleep(30);

                if (!is_port_open)
                        continue;

                received = RS232_PollComport(port_number, port_buffer, 100);

                if (received > 0)
                        for (int i = 0; i < received; ++i)
                                sp_frx_put(port_buffer[i]);

                sp_decode();
        }
        return 0;
}

void update_status(void)
{
        char buf[81];
        snprintf(buf, 81, "rcvd_suc: %d rcvd_fault: %d sent_suc: %d sent_fault: %d",
                        rcvd_successful_packet, rcvd_faulty_packet, sent_packet_successful, sent_packet_faulty);

        print_to_screen(buf, 0);
}

int sp_tx_send(uint8_t *ptr, uint8_t len)
{
        int r = -1;

        if (is_port_open)
                r = RS232_SendBuf(port_number, ptr, len);

        sp_reg &= ~sp_tx_lock;
        return r;
}

void examine_response_packet(uint8_t *p)
{
        uint8_t v = *p;

        if (v == 0)
                sent_packet_successful++;
        else if (v < 9)
                sent_packet_faulty++;
        else
                v = 10;

        char sp_error_strings[80][80] = {"SPE_SUCCESS", "SPE_UNDEF_SEQ", "SPE_PAR",
                                                "SPE_LEN", "SPE_UNDEF_ADDR", "SPE_CS",
                                                "SPE_RX_FULL", "SPE_FRX_FULL", "SPE_TX_FULL",
                                                "RESP_HORRIBLY_WRONG"};

        print_to_screen(sp_error_strings[v], -1);
}

void sp_handler(void *vptr, uint16_t addr)
{
        if (addr)
                display_received_packet(vptr, addr);
        else
                examine_response_packet(vptr);

        sp_reg &= ~sp_rx_lock;
}

void sp_error(uint8_t n)
{
        if (n == 0)
                rcvd_successful_packet++;
        else if (n < 9)
                rcvd_faulty_packet++;
}
