/*
 * author: Mehmet ASLAN
 * date: February 16, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */

#include "screen.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "io.h"

WINDOW *g_cmd_win;
WINDOW *g_norm_win;
WINDOW *g_pin_win;

char key_buffer[120];
int key_buffer_iter = 0;
char **scroll_buffer;
int scroll_buffer_iter = 0;
int g_norm_win_height;
int g_number_of_pins = 0;
int screen_initiliazed = 0;

int is_screen_initiliazed(void)
{
        return screen_initiliazed;
}

int init_screen(int number_of_pins)
{
        initscr();
        refresh();
        cbreak(); // disables line buffering
        nodelay(stdscr, TRUE); // non-blocking mode
        noecho();
        keypad(stdscr, TRUE);

        g_norm_win_height = (LINES - number_of_pins - 3);
        g_number_of_pins = number_of_pins;

        g_pin_win = newwin(number_of_pins, 80, 0, 0);
        g_norm_win = newwin(g_norm_win_height, 80, number_of_pins + 1, 0);
        g_cmd_win = newwin(1, 80, LINES - 1, 0);

        scroll_buffer = (char **) malloc(sizeof(char *) * g_norm_win_height);

        for (int i = 0; i < g_norm_win_height; ++i)
                if ((scroll_buffer[i] = (char *) malloc(sizeof(char) * 81)) == NULL)
                        return -1;

        screen_initiliazed = 1;

        return 0;
}
void destroy_screen(void)
{
        endwin();
}

void print_to_scroll_buffer(char *line)
{
        char *first_line = scroll_buffer[0];

        for (int i = 0; i < g_norm_win_height - 1; ++i)
                scroll_buffer[i] = scroll_buffer[i+1];

        if (strlen(line) > 80)
                line[80] = '\0';

        strcpy(first_line, line);

        scroll_buffer[g_norm_win_height - 1] = first_line;
}

void print_to_screen(char *line, int pin_number)
{
        if (pin_number < 0) {
                print_to_scroll_buffer(line);
                wclear(g_norm_win);

                for (int i = 0; i < g_norm_win_height; ++i)
                        mvwprintw(g_norm_win, i, 0, scroll_buffer[i]);

                wrefresh(g_norm_win);

        } else if (pin_number < g_number_of_pins){

                wmove(g_pin_win, pin_number, 0);
                wclrtoeol(g_pin_win);
                mvwprintw(g_pin_win, pin_number, 0, line);
                wrefresh(g_pin_win);
        }
}

void get_keys(void)
{
        int key;
        while ((key = getch()) != ERR) {
                if (key == KEY_BACKSPACE) {
                        wclear(g_cmd_win);
                        key_buffer[key_buffer_iter] = '\0';

                        mvwprintw(g_cmd_win, 0, 0, key_buffer);

                        if (key_buffer_iter)
                                --key_buffer_iter;
                } else if (key == 10) {

                        handle_keyboard_input(key_buffer);
                        print_to_screen(key_buffer, -1);

                        for (int i = 0; i < 120; ++i)
                                key_buffer[i] = '\0';

                        key_buffer_iter = 0;
                        wclear(g_cmd_win);
                } else {
                        if (key_buffer_iter == 80)
                                continue;

                        waddch(g_cmd_win, key);
                        key_buffer[key_buffer_iter++] = key;
                }
        }

        wrefresh(g_cmd_win);
}
