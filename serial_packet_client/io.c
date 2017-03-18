/*
 * author: Mehmet ASLAN
 * date: March 3, 2017
 *
 * no warranty, no licence agreement
 * use it at your own risk
 */

#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sp.h"
#include "screen.h"

// send packet via usart
int send_packet(void);
int get_data_size(uint8_t type_number);
int add_new_packet(void);
int get_data_type_number(char *type_name);
void separate_words(char *line);

char g_words[10][200];
int g_words_iter = 0;
// different packets server can send
packet packets[80];
// initiliza packet in order, in same it gives number of packets
int packets_iter = 0;
// rather than scrolling all packets, display frequent packets in pinned buffer
// it also gives number of pinned packets
int g_packet_pin_iter = 0;

extern int should_dump;
extern FILE *dump_file;
extern int is_port_open;
extern int dump_mode;
FILE *packet_file = NULL;
int dumping_packet_number = -1;

// all packets received from usart displayed
int display_received_packet(void *vptr, uint16_t addr)
{
        char display_str[200];
        // intentionally invalid packet number
        int packet_number = -1;
        // pointer to packet name for ease of use
        char *name;

        // does such a packet valid with given addr
        for (int i = 0; i < packets_iter; ++i) {
                // do i recognize packet addr
                if (packets[i].addr == addr) {
                        // index of packet
                        // so i can access all property of packet struct
                        packet_number = i;
                        break;
                }
        }

        if (packet_number == -1) {
                fprintf(stderr, "invalid packet received\n");
                return -1;
        }
        // found valid packet and its name
        name = packets[packet_number].name;

        // getting received data type, I can receive all data types
        // case numbers and their received data types clear
        switch (packets[packet_number].type_number) {
        case 0:
                sprintf(display_str, "%s %" PRIu8, name, *((uint8_t *) vptr));
                break;
        case 1:
                sprintf(display_str, "%s %" PRIu16, name, *((uint16_t *) vptr));
                break;
        case 2:
                sprintf(display_str, "%s %" PRIu32, name, *((uint32_t *) vptr));
                break;
        case 3:
                sprintf(display_str, "%s %" PRIu64, name, *((uint64_t *) vptr));
                break;
        case 4:
                sprintf(display_str, "%s %" PRId8, name, *((int8_t *) vptr));
                break;
        case 5:
                sprintf(display_str, "%s %" PRId16, name, *((int16_t *) vptr));
                break;
        case 6:
                sprintf(display_str, "%s %" PRId32, name, *((int32_t *) vptr));
                break;
        case 7:
                sprintf(display_str, "%s %" PRId64, name, *((int64_t *) vptr));
                break;
        case 8:
                sprintf(display_str, "%s %f", name, *((float *) vptr));
                break;
        case 9:
                sprintf(display_str, "%s %f", name, ((float) *((double *) vptr)));
                break;
        }

        // with data type, str can be longer than 80 chars
        // lower array size may result error due not enough memory
        // make sure I always display 80 char in a line
        display_str[80] = '\0';

        // say how you display with pin state
        // negative pin_state means pinned buffer, otherwise represents where
        // to put in pinned buffer
        print_to_screen(display_str, packets[packet_number].pin_state);

        return 0;
}

// how many pins do i have
int get_pin_number(void)
{
        return g_packet_pin_iter;
}

// handle commands line by line
// lines in init file and keyboard input lines treated as same
int handle_keyboard_input(char *line)
{
        // separate chars to different str arrays, so i can compare
        // delimeter is space
        separate_words(line);
        // is there defined command, otherwise ridicules
        if (g_words_iter == 0)
                return -1;
        // first word's first char defines what kind of input it is
        switch (g_words[0][0]) {
        case ';':
                if (add_new_packet())
                        return -2;

                break;
        case '#': // for commenting do nothing
                break;
        case '$': // pin packet
                for (int i = 0; i < packets_iter; ++i) {
                        // packet must exist for pinning
                        if (strcmp(packets[i].name, g_words[1]) == 0) {
                                packets[i].pin_state = g_packet_pin_iter;
                                ++g_packet_pin_iter;
                        }

                }
                break;
        default: // no special char send to server
                if (send_packet())
                        return -3;
        }

        return 0;
}

int send_packet(void)
{
        if (g_words_iter != 2)
                return -2;

        int packet_number = -1;
        // get packet number
        for (int i = 0; i < packets_iter; ++i) {
                if (strcmp(g_words[0], packets[i].name) == 0) {
                        packet_number = i;
                        break;
                }
        }

        if (packet_number < 0)
                return -3;
        // am I allowed to write to server
        if (!packets[packet_number].mode)
                return -4;

        uint8_t *binary;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        float f;
        double d;
        char *data = g_words[1];

        switch (packets[packet_number].type_number) { // this number what data type I have
        case 0:
                if (sscanf(data, "%" SCNu8, &u8) < 0) // could I get data
                        return -5;

                binary = (uint8_t *) &u8;
                break;
        case 1:
                if (sscanf(data, "%" SCNu16, &u16) < 0)
                        return -5;

                binary = (uint8_t *) &u16;
                break;
        case 2:
                if (sscanf(data, "%" SCNu32, &u32) < 0)
                        return -5;

                binary = (uint8_t *) &u32;
        case 3:
                if (sscanf(data, "%" SCNu64, &u64) < 0)
                        return -5;

                binary = (uint8_t *) &u64;
                break;
        case 4:
                if (sscanf(data, "%" SCNd8, &i8) < 0)
                        return -5;

                binary = (uint8_t *) &i8;
                break;
        case 5:
                if (sscanf(data, "%" SCNd16, &i16) < 0)
                        return -5;

                binary = (uint8_t *) &i16;
                break;
        case 6:
                if (sscanf(data, "%" SCNd32, &i32) < 0)
                        return -5;

                binary = (uint8_t *) &i32;
                break;
        case 7:
                if (sscanf(data, "%" SCNd64, &i64) < 0)
                        return -5;

                binary = (uint8_t *) &i64;
                break;
        case 8:
                if (sscanf(data, "%f", &f) < 0)
                        return -5;

                binary = (uint8_t *) &f;
                break;
        case 9:
                if (sscanf(data, "%f", &f) < 0)
                        return -5;

                d = (double) f;
                binary = (uint8_t *) &d;
                break;
        default:
                fprintf(stderr, "dont support type_number(%d)\n", packets[packet_number].type_number);
                return -6;
        }

        if (!is_port_open) {
                fprintf(stderr, "port is not open, where do i send packet :)\n");
                return -1;
        }

        // send server via usart
        sp_encode(binary, get_data_size(packets[packet_number].type_number),
                packets[packet_number].addr);

        return 0;
}

// specific type how many bytes
int get_data_size(uint8_t type_number)
{
        uint8_t sizes[] = {1, 2, 4, 8, 1, 2, 4, 8, 4, 8};

        if (type_number > 9)
                return -1;

        return sizes[type_number];
}

// gimme all packet info
int add_new_packet(void)
{
        int tmp;
        // does it fit my standarts
        if (g_words_iter != 5) {
                fprintf(stderr, "invalid word count(%d) in input line\n", g_words_iter);
                return -1;
        }
        // check if packet with same name exist
        for (int i = 0; i < packets_iter; ++i) {
                if (strcmp(g_words[1], packets[i].name) == 0) {
                        fprintf(stderr, "packet name(%s) alread defined\n", g_words[1]);
                        return -2;
                }
        }

        packets[packets_iter].name = (char *) malloc(strlen(g_words[1]) + 1);
        strcpy(packets[packets_iter].name, g_words[1]);

        if (sscanf(g_words[2], "%d", &tmp) < 0)
                return -3;

        // packet addr can not be negative
        if (tmp < 0)
                return -4;
        // check if packet with same addr
        for (int i = 0; i < packets_iter; ++i) {
                if (tmp == packets[i].addr) {
                        fprintf(stderr, "already existing addr(%d)\n", tmp);
                        return -5;
                }
        }

        packets[packets_iter].addr = (uint16_t) tmp;

        tmp = get_data_type_number(g_words[3]);
        // is type valid
        if (tmp < 0)
                return -6;

        packets[packets_iter].type_number = tmp;

        if (g_words[4][0] == 'r')
                packets[packets_iter].mode = 0;
        else if (g_words[4][0] == 'w') // can write received packet data in server
                packets[packets_iter].mode = 1;
        else
                return -7;
        // default all packets in scroll buffer
        packets[packets_iter].pin_state = -1;

        ++packets_iter;
        return 0;
}

int get_data_type_number(char *type_name)
{
        char names[10][10] = {"uint8_t", "uint16_t", "uint32_t", "uint64_t",
                                "int8_t", "int16_t", "int32_t", "int64_t", "float", "double"};

        for (int i = 0; i < 10; ++i)
                if (strcmp(type_name, names[i]) == 0)
                        return i;

        return -1;
}
// separates char in single line, delimeter is space
void separate_words(char *line)
{
        int line_iter = 0, word_iter = 0;
        g_words_iter = 0;

        while (line[line_iter] != '\0') {
                // get rid of empty spaces
                while (line[line_iter] == ' ')
                        ++line_iter;

                while (line[line_iter] != ' ' && line[line_iter] != '\0')
                        g_words[g_words_iter][word_iter++] = line[line_iter++];

                g_words[g_words_iter][word_iter] = '\0';

                if (word_iter)
                        ++g_words_iter;

                word_iter = 0;
        }
}

void dump_data(void)
{
        should_dump = 1;
        uint8_t buf[20];
        size_t read = 0;
        char packet_file_mode[3] = {'w', '\0', '\0'};

        if (dump_mode == DUMP_MODE_HEX) {
                packet_file_mode[1] = 'b';
        }

        for (int i = 0; i < packets_iter; ++i) {
                dumping_packet_number = i;

                packet_file = fopen(packets[i].name, packet_file_mode);

                if (packet_file == NULL) {
                        fprintf(stderr, "%s packet dump failed\n", packets[i].name);
                        continue;
                }

                fseek(dump_file, 0, SEEK_SET);

                while ((read = fread(buf, sizeof(uint8_t), 20, dump_file)) > 0) {
                        for (int i = 0; i < read; ++i) {
                                sp_frx_put(buf[i]);
                        }

                        sp_decode();
                }

                fclose(packet_file);
        }
}

void saved_packet_handler(void *vptr, uint16_t addr)
{
        if (packet_file == NULL || dumping_packet_number == -1) {
                fprintf(stderr, "invalid packet to be dumped\n");
                return;
        }

        if (addr != packets[dumping_packet_number].addr) {
                return;
        }

        if (dump_mode == DUMP_MODE_TEXT) {
                char display_str[200];

                switch (packets[dumping_packet_number].type_number) {
                case 0:
                        sprintf(display_str, "%" PRIu8, *((uint8_t *) vptr));
                        break;
                case 1:
                        sprintf(display_str, "%" PRIu16, *((uint16_t *) vptr));
                        break;
                case 2:
                        sprintf(display_str, "%" PRIu32, *((uint32_t *) vptr));
                        break;
                case 3:
                        sprintf(display_str, "%" PRIu64, *((uint64_t *) vptr));
                        break;
                case 4:
                        sprintf(display_str, "%" PRId8, *((int8_t *) vptr));
                        break;
                case 5:
                        sprintf(display_str, "%" PRId16, *((int16_t *) vptr));
                        break;
                case 6:
                        sprintf(display_str, "%" PRId32, *((int32_t *) vptr));
                        break;
                case 7:
                        sprintf(display_str, "%" PRId64, *((int64_t *) vptr));
                        break;
                case 8:
                        sprintf(display_str, "%f", *((float *) vptr));
                        break;
                case 9:
                        sprintf(display_str, "%f", ((float) *((double *) vptr)));
                        break;
                }

                fprintf(packet_file, "%s\n", display_str);
        } else if (dump_mode == DUMP_MODE_HEX) {
                fwrite((uint8_t *) vptr, sizeof(uint8_t), get_data_size(packets[dumping_packet_number].type_number),packet_file);
        }
}
