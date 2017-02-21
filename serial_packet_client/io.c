/*
 * author: Mehmet ASLAN
 * date: February 16, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */

#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sp.h"
#include "screen.h"

int send_packet(void);
int get_data_size(uint8_t type_number);
int add_new_packet(void);
int get_data_type_number(char *type_name);
void separate_words(char *line);

char g_words[10][200];
int g_words_iter = 0;
packet packets[80];
int packets_iter = 0;
int g_packet_pin_iter = 0;

int display_received_packet(void *vptr, uint16_t addr)
{
        char output_string[200];
        int packet_number = -1;
        char *name;

        for (int i = 0; i < packets_iter; ++i) {
                if (packets[i].addr == addr) {
                        packet_number = i;
                        break;
                }
        }

        if (packet_number == -1)
                return -1;

        name = packets[packet_number].name;

        switch (packets[packet_number].type_number) {
        case 0:
                sprintf(output_string, "%s %" PRIu8, name, *((uint8_t *) vptr));
                break;
        case 1:
                sprintf(output_string, "%s %" PRIu16, name, *((uint16_t *) vptr));
                break;
        case 2:
                sprintf(output_string, "%s %" PRIu32, name, *((uint32_t *) vptr));
                break;
        case 3:
                sprintf(output_string, "%s %" PRIu64, name, *((uint64_t *) vptr));
                break;
        case 4:
                sprintf(output_string, "%s %" PRId8, name, *((int8_t *) vptr));
                break;
        case 5:
                sprintf(output_string, "%s %" PRId16, name, *((int16_t *) vptr));
                break;
        case 6:
                sprintf(output_string, "%s %" PRId32, name, *((int32_t *) vptr));
                break;
        case 7:
                sprintf(output_string, "%s %" PRId64, name, *((int64_t *) vptr));
                break;
        case 8:
                sprintf(output_string, "%s %f", name, *((float *) vptr));
                break;
        case 9:
                sprintf(output_string, "%s %f", name, ((float) *((double *) vptr)));
                break;
        }

        output_string[80] = '\0';

        print_to_screen(output_string, packets[packet_number].pin_state);

        return 0;
}

int get_pin_number(void)
{
        return g_packet_pin_iter;
}

int handle_keyboard_input(char *line)
{
        separate_words(line);

        if (g_words_iter == 0)
                return -1;

        switch (g_words[0][0]) {
        case ';':
                if (add_new_packet())
                        return -2;

                break;
        case '#':
                break;
        case '$':
                if (is_screen_initiliazed())
                        break;

                for (int i = 0; i < packets_iter; ++i)
                        if (packets[i].pin_state == g_packet_pin_iter)
                                break;

                for (int i = 0; i < packets_iter; ++i) {
                        if (strcmp(packets[i].name, g_words[1]) == 0) {
                                packets[i].pin_state = g_packet_pin_iter;
                                ++g_packet_pin_iter;
                        }

                }
                break;
        default:
                if (send_packet())
                        return -3;
        }

        return 0;
}

int send_packet(void)
{
        if (g_words_iter != 2)
                return -1;

        int packet_number = -1;

        for (int i = 0; i < packets_iter; ++i) {
                if (strcmp(g_words[0], packets[i].name) == 0) {
                        packet_number = i;
                        break;
                }
        }

        if (packet_number < 0)
                return -2;

        if (!packets[packet_number].mode)
                return -3;

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

        switch (packets[packet_number].type_number) {
        case 0:
                if (sscanf(data, "%" SCNu8, &u8) < 0)
                        return -4;

                binary = (uint8_t *) &u8;
                break;
        case 1:
                if (sscanf(data, "%" SCNu16, &u16) < 0)
                        return -4;

                binary = (uint8_t *) &u16;
                break;
        case 2:
                if (sscanf(data, "%" SCNu32, &u32) < 0)
                        return -4;

                binary = (uint8_t *) &u32;
        case 3:
                if (sscanf(data, "%" SCNu64, &u64) < 0)
                        return -4;

                binary = (uint8_t *) &u64;
                break;
        case 4:
                if (sscanf(data, "%" SCNd8, &i8) < 0)
                        return -4;

                binary = (uint8_t *) &i8;
                break;
        case 5:
                if (sscanf(data, "%" SCNd16, &i16) < 0)
                        return -4;

                binary = (uint8_t *) &i16;
                break;
        case 6:
                if (sscanf(data, "%" SCNd32, &i32) < 0)
                        return -4;

                binary = (uint8_t *) &i32;
                break;
        case 7:
                if (sscanf(data, "%" SCNd64, &i64) < 0)
                        return -4;

                binary = (uint8_t *) &i64;
                break;
        case 8:
                if (sscanf(data, "%f", &f) < 0)
                        return -4;

                binary = (uint8_t *) &f;
                break;
        case 9:
                if (sscanf(data, "%f", &f) < 0)
                        return -4;

                d = (double) f;
                binary = (uint8_t *) &d;
                break;
        default:
                return -5;
        }

        sp_encode(binary, get_data_size(packets[packet_number].type_number),
                packets[packet_number].addr);

        return 0;
}

int get_data_size(uint8_t type_number)
{
        uint8_t sizes[] = {1, 2, 4, 8, 1, 2, 4, 8, 4, 8};

        if (type_number > 9)
                return -1;

        return sizes[type_number];
}

int add_new_packet(void)
{
        int tmp;

        if (g_words_iter != 5)
                return -1;

        for (int i = 0; i < packets_iter; ++i)
                if (strcmp(g_words[1], packets[i].name) == 0)
                        return -2;

        packets[packets_iter].name = (char *) malloc(strlen(g_words[1]) + 1);
        strcpy(packets[packets_iter].name, g_words[1]);

        if (sscanf(g_words[2], "%d", &tmp) < 0)
                return -3;

        if (tmp < 0)
                return -4;

        for (int i = 0; i < packets_iter; ++i)
                if (tmp == packets[i].addr)
                        return -5;

        packets[packets_iter].addr = (uint16_t) tmp;

        tmp = get_data_type_number(g_words[3]);

        if (tmp < 0)
                return -6;

        packets[packets_iter].type_number = tmp;

        if (g_words[4][0] == 'r')
                packets[packets_iter].mode = 0;
        else if (g_words[4][0] == 'w')
                packets[packets_iter].mode = 1;
        else
                return -7;

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
