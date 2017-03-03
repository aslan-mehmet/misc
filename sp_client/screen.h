#ifndef __SCREEN_H
#define __SCREEN_H

int init_screen(int number_of_pins);
void destroy_screen(void);
void print_to_screen(char *line, int pin_number);
void get_keys(void);

#endif // __SCREEN_H
