/*
 * author: Mehmet ASLAN
 * date: February 16, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */

#include "input.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "interface.h"
#include "sp.h"

void separate_words(char *str);
void handle_input_mode_var(void);
var *get_var_with_name(char *str);

char g_input_str[80][80];
int g_input_i = 0;
var g_var_arr[120];
int g_var_i = 0;

char g_type_strs[11][80] = {"types","uint8_t", "uint16_t", "uint32_t", "uint64_t",
                                "int8_t", "int16_t", "int32_t", "int64_t", "float", "double"};

void read_initvar(void)
{
        FILE *f = fopen(".initvar", "r");
        char *line;
        size_t line_len = 0;
        int read;

        if (f == NULL)
                return;

        while ((read = getline(&line, &line_len, f)) !=-1) {
                if (line[0] == '#')
                        continue;

                char *tmp;

                tmp = strtok(line, ",");
                sscanf(tmp, "%x", &g_var_arr[g_var_i].addr);

                tmp = strtok(NULL, ",");
                g_var_arr[g_var_i].str = (char *) malloc(50);
                strcpy(g_var_arr[g_var_i].str, tmp);

                tmp = strtok(NULL, ",");
                for (int i = 0; i < 11; ++i) {
                        if (strcmp(g_type_strs[i], tmp) == 0) {
                                g_var_arr[g_var_i].data_type = i;
                                break;
                        }
                }

                tmp = strtok(NULL, ",");
                sscanf(tmp, "%d", &g_var_arr[g_var_i].is_static);

                tmp = strtok(NULL, ",");
                if (strcmp(tmp, "pinned") == 0)
                        g_var_arr[g_var_i].which_buffer = PINNED_BUFFER;
                else if (strcmp(tmp, "norm") == 0)
                        g_var_arr[g_var_i].which_buffer = NORM_BUFFER;

                tmp = strtok(NULL, ",");
                sscanf(tmp, "%d", &g_var_arr[g_var_i].loc);

                ++g_var_i;
        }

        fclose(f);
}

// handle keyboard input
void handle_keyboard_input(char *str)
{
        separate_words(str);

        switch (sp_mode) {
        case SP_MODE_VARIABLE:
                handle_input_mode_var();
                break;
        }
}

// separate words for getopt
void separate_words(char *str)
{
        int i = 0, wordi = 0;
        g_input_i = 0;

        while (str[i] == ' ')
                ++i;

        if (str[i] == '\0')
                return;

        while (str[i] != '\0') {
                if (str[i] == ' ') {
                        g_input_str[g_input_i][wordi] = '\0';
                        g_input_i++;
                        wordi = 0;
                } else {
                        g_input_str[g_input_i][wordi++] = str[i];
                }

                ++i;
        }

        g_input_str[g_input_i][wordi] = '\0';
        g_input_i++;
}

// input in var mode
void handle_input_mode_var(void)
{
        if (g_input_i != 2) {
                write_input_buffer("var mode incorrect input size");
                return;
        }

        // have the var name
        uint8_t *bin;
        spacket pack;
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

        var *p = get_var_with_name(g_input_str[0]);

        if (p == NULL) {
                write_input_buffer("err no var in that name");
                return;
        }

        // if static can set var in server
        if (!p->is_static) {
                write_input_buffer("var not static cant write");
                return;
        }

        pack.addr = p->addr;
        // get type, cast, binary, send serial
        switch (p->data_type) {
        case TYPE_UINT8_T:
                sscanf(g_input_str[1], "%d", &u8);
                bin = &u8;
                pack.vsize = UINT8_T;
                break;
        case TYPE_UINT16_T:
                sscanf(g_input_str[1], "%d", &u16);
                bin = (uint8_t *) &u16;
                pack.vsize = UINT16_T;
                break;
        case TYPE_UINT32_T:
                sscanf(g_input_str[1], "%d", &u32);
                bin = (uint8_t *) &u32;
                pack.vsize = UINT32_T;
                break;
        case TYPE_UINT64_T:
                sscanf(g_input_str[1], "%d", &u64);
                bin = (uint8_t *) &u64;
                pack.vsize = UINT64_T;
                break;
        case TYPE_INT8_T:
                sscanf(g_input_str[1], "%d", &i8);
                bin = (uint8_t *) &i8;
                pack.vsize = INT8_T;
                break;
        case TYPE_INT16_T:
                sscanf(g_input_str[1], "%d", &i16);
                bin = (uint8_t *) &i16;
                pack.vsize = INT16_T;
                break;
        case TYPE_INT32_T:
                sscanf(g_input_str[1], "%d", &i32);
                bin = (uint8_t *) &i32;
                pack.vsize = INT32_T;
                break;
        case TYPE_INT64_T:
                sscanf(g_input_str[1], "%d", &i64);
                bin = (uint8_t *) &i64;
                pack.vsize = INT64_T;
                break;
        case TYPE_FLOAT:
                sscanf(g_input_str[1], "%f", &f);
                bin = (uint8_t *) &f;
                pack.vsize = FLOAT;
                break;
        case TYPE_DOUBLE:
                sscanf(g_input_str[1], "%f", &d);
                bin = (uint8_t *) &d;
                pack.vsize = DOUBLE;
                break;
        }

        sp_encode(bin, pack);
}

// in comm mode fancy getopt
void handle_input_mode_comm()
{

}

// got serial data, but what it is type and string
var *get_var_with_addr(uint16_t addr)
{
        for (int i = 0; i < g_var_i; ++i)
                if (g_var_arr[i].addr == addr)
                        return &g_var_arr[i];

        return NULL;
}

// got string of data, what it is addr
var *get_var_with_name(char *str)
{
        for (int i = 0; i < g_var_i; ++i)
                if (strcmp(str, g_var_arr[i].str) == 0)
                        return &g_var_arr[i];

        return NULL;
}
