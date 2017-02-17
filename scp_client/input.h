#ifndef __INPUT_H
#define __INPUT_H

#include "main.h"

// data types
#define TYPE_UINT8_T 1
#define TYPE_UINT16_T 2
#define TYPE_UINT32_T 3
#define TYPE_UINT64_T 4
#define TYPE_INT8_T 5
#define TYPE_INT16_T 6
#define TYPE_INT32_T 7
#define TYPE_INT64_T 8
#define TYPE_FLOAT 9
#define TYPE_DOUBLE 10

// avaliable buffers
#define PINNED_BUFFER 1
#define NORM_BUFFER 2

struct _var{
        uint16_t addr;
        char *str;
        int data_type;
        int is_static;
        int which_buffer;
        int loc;
};

typedef struct _var var;

void read_initvar(void);
void handle_keyboard_input(char *str);
var *get_var_with_addr(uint16_t addr);


#endif // __INPUT_H
