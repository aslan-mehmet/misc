#ifndef __INTERFACE_H
#define __INTERFACE_H

void init_interface(void);
void destroy_interface(void);
void set_title(char *str);
void write_input_buffer(char *format, ...);
void write_pinned_buffer(int line_number, char *format, ...);
void got_key_input(int c);

#endif // __INTERFACE_H
