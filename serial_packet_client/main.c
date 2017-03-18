/*
 * author: Mehmet ASLAN
 * date: March 3, 2017
 *
 * no warranty, no licence agreement
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
#include <getopt.h>
#include "rs232.h"

void update_status(void);
int system_check(void);

int port_number = 16;
int is_port_open = 1;
int rplus = 0; // received packet successful
int rminus = 0; // received packet faulty
int splus = 0; // send packet
int sminus = 0;
int should_dump = 0;
FILE *dump_file = NULL;

// exit program properly
void sig_handler(int signum)
{
        if (signum == SIGINT) {
                if (dump_file != NULL) {
                        dump_data();
                        fclose(dump_file);
                }

                if (is_port_open) {
                        RS232_CloseComport(port_number);
                }
                destroy_screen();
                exit(0);
        }
}

int main(int argc, char **argv)
{
        fclose(stderr);

        stderr = fopen("errors", "w+");

        if (stderr == NULL)
                return -1;

        // this program heavily depends on little endian and ieee 754 standart
        // so check are we okay
        int c = system_check();

        if (c) {
                fprintf(stderr, "sys check failed %d\n", c);
                exit(-1);
        }

        // read initilization file
        FILE *f = fopen("spinit", "r");
        char *line;
        size_t line_len = 0;

        // serial port properties
        // to do make getopt defineable
        char mode[] = {'8', 'N', '1', 0}; // bits parity stop_bits unimportant
        int baudrate = 38400;
        uint8_t port_buffer[100];
        int received;

        static struct option long_options[] =
        {
                {"baud",       required_argument, NULL, 'b'},
                {"dump",       required_argument, NULL, 'd'},
                {"port",       required_argument, NULL, 'p'}
        };

        while ((c = getopt_long(argc, argv, "b:s:p:", long_options, NULL)) != EOF) {
                int tmp = -1;
                switch (c) {
                case 'b':
                        // dont pass non standart baudrate i dont check
                        sscanf(optarg, "%d", &baudrate);
                        break;
                case 'd':
                        // dump all received file at the end
                        dump_file = fopen(optarg, "wb+");

                        if (dump_file == NULL) {
                                fprintf(stderr, "%s could not opened\n", optarg);
                                exit(-1);
                        }
                        break;
                case 'p':
                        // ttyUSB0 + optarg
                        // all ttyUSB
                        sscanf(optarg, "%d", &tmp);

                        if (tmp < 6 && tmp >= 0) {
                                port_number += tmp;
                        }

                        break;
                }
        }

        signal(SIGINT, sig_handler);

        if (f == NULL) {
                fprintf(stderr, "no init file\n");
                return -2;
        }

        // default initilatizion lines
        // used for sent packet's response, success or not
        handle_keyboard_input("; status 0 uint8_t r");
        handle_keyboard_input("$ status");

        // read initilization file
        while (getline(&line, &line_len, f) != -1) {
                int tmp = strlen(line);

                if (line[tmp-1] == '\n')
                        line[tmp-1] = '\0';

                handle_keyboard_input(line);
        }

        fclose(f);

        if (RS232_OpenComport(port_number, baudrate, mode)) {
                fprintf(stderr, "can not open port\n");
                // to use every function that depens on serial port
                is_port_open = 0;
        }

        // can i open display
        if (init_screen(get_pin_number())) {
                fprintf(stderr, "cant allocate display buffers why\n");
                // exit if display not opened
                sig_handler(SIGINT);
        }

        sp_init();

        while (1) {
                // show number of received and sent packets
                update_status();
                // get keyboard inputs
                get_keys();
                usleep(30);

                if (!is_port_open) {
                        continue;
                }

                received = RS232_PollComport(port_number, port_buffer, 100);

                if (received > 0) {
                        for (int i = 0; i < received; ++i) {
                                sp_frx_put(port_buffer[i]);
                        }

                        if (dump_file != NULL) {
                                fwrite(port_buffer, sizeof(uint8_t), received, dump_file);
                        }
                }

                sp_decode();
        }
        return 0;
}

// show number of received and sent packets
void update_status(void)
{
        char buf[81];
        snprintf(buf, 81, "r+: %d r-: %d s+: %d s-: %d",
                        rplus, rminus, splus, sminus);

        print_to_screen(buf, 0);
}

// sent to server
int sp_tx_send(uint8_t *ptr, uint8_t len)
{
        int r = -1;

        if (is_port_open)
                r = RS232_SendBuf(port_number, ptr, len);

        sp_reg &= ~sp_tx_lock;
        // i dont check this return,
        // it is here because what if can not sent all buffer
        return r;
}

// client(this program) sent packet to server
// server sent response i got that right or not
void examine_response_packet(uint8_t *p)
{
        uint8_t v = *p;

        if (v == 0) // hurra got it right
                splus++;
        else if (v < 9)
                sminus++;
        else
                v = 10;

        char sp_error_strings[80][80] = {"SPE_SUCCESS", "SPE_UNDEF_SEQ", "SPE_PAR",
                                                "SPE_LEN", "SPE_UNDEF_ADDR", "SPE_CS",
                                                "SPE_RX_FULL", "SPE_FRX_FULL", "SPE_TX_FULL",
                                                "RESP_HORRIBLY_WRONG"};
        // prints what went wrong
        print_to_screen(sp_error_strings[v], -1);
}

// handles received packets
void sp_handler(void *vptr, uint16_t addr)
{
        if (should_dump == 1) {
                saved_packet_handler(vptr, addr);
        } else if (addr) {// for other addr just display
                display_received_packet(vptr, addr);
        } else {// response packet addr is always zero
                examine_response_packet(vptr);
        }

        sp_reg &= ~sp_rx_lock;
}

// handles received packets errors
void sp_error(uint8_t n)
{
        if (n == 0) // received packet correctly
                rplus++;
        else if (n < 9)
                rminus++;
}

int system_check(void)
{
	/* check */
	/* size */
	uint32_t u32;
	float f;
	double d;

        if (sizeof(u32) != 4) /* something really wrong */
		return -1;

	if (sizeof(f) != 4)
		return -2;

	if (sizeof(d) != 8)
		return -3;

	/* little endian */
	u32 = 0x12345678;
	uint8_t *p = (uint8_t *) &u32;
	uint8_t little[4] = {0x78, 0x56, 0x34, 0x12};

	for (int i = 0; i < 4; ++i)
		if (little[i] != p[i])
			return -4;

	/* decimal number representations */
	f = 0.123456;
	u32 = 0x3dfcd680;	/* ieee 754 std */
	uint8_t *fp = (uint8_t *) &f;

	d = 0.123456789123;
	uint8_t *dp = (uint8_t *) &d;
	uint64_t u64 = 0x3fbf9add37c0a0cb;

	for (int i = 0; i < 4; ++i)
		if (fp[i] != p[i])
			return -5;

        p = (uint8_t *) &u64;
	for (int i = 0; i < 8; ++i)
		if (dp[i] != p[i])
			return -6;

	return 0;
}
