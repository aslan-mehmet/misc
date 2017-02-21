/*
 * author: Mehmet ASLAN
 * date: February 16, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */

#ifndef __IO_H
#define __IO_H

#include <stdint.h>

typedef struct{
        char *name;
        uint16_t addr;
        int type_number;
        int mode;
        int pin_state;
}packet;

int handle_keyboard_input(char *line);
int display_received_packet(void *vptr, uint16_t addr);
int get_pin_number(void);

#endif
