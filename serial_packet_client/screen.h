/*
 * author: Mehmet ASLAN
 * date: February 16, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */

#ifndef __SCREEN_H
#define __SCREEN_H

int init_screen(int number_of_pins);
void destroy_screen(void);
void print_to_screen(char *line, int pin_number);
void get_keys(void);

#endif // __SCREEN_H
