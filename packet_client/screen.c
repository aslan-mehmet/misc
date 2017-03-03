#include "screen.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "io.h"

WINDOW *g_cmd_win;
WINDOW *g_norm_win;
WINDOW *g_pin_win;

// contains keyboard inputs until return received
char key_buffer[120];
int key_buffer_iter = 0;
// string array
char **scroll_buffer;
int scroll_buffer_iter = 0;
// used for how many string I can fit into norm window
int g_norm_win_height;
// have limited pins
int g_number_of_pins = 0;
// total number of lines in frame
int lines;

// how many pins init file wants
int init_screen(int number_of_pins)
{
        initscr();
        refresh();
        cbreak(); // disables line buffering
        nodelay(stdscr, TRUE); // non-blocking mode
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);

        // at startup have this many lines, so I can check later if size changed
        lines = LINES;

        g_norm_win_height = (lines - number_of_pins - 3);
        g_number_of_pins = number_of_pins;

        g_pin_win = newwin(number_of_pins, 80, 0, 0);
        g_norm_win = newwin(g_norm_win_height, 80, number_of_pins + 1, 0);
        g_cmd_win = newwin(1, 80, lines - 1, 0);

        scroll_buffer = (char **) malloc(sizeof(char *) * g_norm_win_height);

        for (int i = 0; i < g_norm_win_height; ++i) {
                if ((scroll_buffer[i] = (char *) malloc(sizeof(char) * 81)) == NULL) {
                        fprintf(stderr, "cant allocate lines to scroll buffer\n");
                        exit(0);
                }
        }

        return 0;
}
void destroy_screen(void)
{
        endwin();
}

void print_to_scroll_buffer(char *line)
{
        char *remember_line = scroll_buffer[0];
        // shift lines up
        for (int i = 0; i < g_norm_win_height - 1; ++i)
                scroll_buffer[i] = scroll_buffer[i+1];
        // can not longer, dont want to mess with dynamic length
        if (strlen(line) > 80)
                line[80] = '\0';
        // save line, old scroll buffers first
        strcpy(remember_line, line);
        // put new string to end of scroll buffer
        scroll_buffer[g_norm_win_height - 1] = remember_line;
}

void print_to_screen(char *line, int pin_number)
{
        if (pin_number < 0) { // i am not pinned
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

void is_screen_changed(void)
{
        int size_changed = 0;
        // is screen size changed
        if (LINES != lines) {
                lines = LINES;
                size_changed = 1;
        }
        // do i add more pins
        if (g_number_of_pins != get_pin_number()) {
                g_number_of_pins = get_pin_number();
                size_changed = 1;
        }

        if (size_changed) {
                // delete everything, dont care old data
                delwin(g_cmd_win);
                delwin(g_norm_win);
                delwin(g_pin_win);

                for (int i = 0; i < g_norm_win_height; ++i)
                        free(scroll_buffer[i]);

                if (scroll_buffer != NULL)
                        free(scroll_buffer);
                // redefine their locations and sizes
                g_norm_win_height = (lines - g_number_of_pins - 3);

                g_pin_win = newwin(g_number_of_pins, 80, 0, 0);
                g_norm_win = newwin(g_norm_win_height, 80, g_number_of_pins + 1, 0);
                g_cmd_win = newwin(1, 80, lines - 1, 0);

                wclear(g_pin_win);
                wclear(g_norm_win);
                wclear(g_cmd_win);

                wrefresh(g_pin_win);
                wrefresh(g_norm_win);
                wrefresh(g_cmd_win);

                scroll_buffer = (char **) malloc(sizeof(char *) * g_norm_win_height);

                for (int i = 0; i < g_norm_win_height; ++i) {
                        if ((scroll_buffer[i] = (char *) malloc(sizeof(char) * 81)) == NULL) {
                                fprintf(stderr, "cant allocate lines to scroll buffer in size change\n");
                                exit(0);
                        }

                        (scroll_buffer[i])[0] = '\0';
                }
                // key buffer len 120
                for (int i = 0; i < 120; ++i)
                        key_buffer[i] = '\0';

        }
}

void get_keys(void)
{
        int key;

        is_screen_changed();

        while ((key = getch()) != ERR) {
                if (key == KEY_BACKSPACE) {
                        if (!key_buffer_iter)
                                continue;

                        wclear(g_cmd_win);

                        key_buffer[--key_buffer_iter] = '\0';
                        // normally I use wdelch, it doesnt work
                        mvwprintw(g_cmd_win, 0, 0, key_buffer);

                } else if (key == 10) { // return key
                        // have all line
                        handle_keyboard_input(key_buffer);
                        // wanna see what i typed
                        print_to_screen(key_buffer, -1);
                        // done with this command
                        for (int i = 0; i < 120; ++i)
                                key_buffer[i] = '\0';

                        key_buffer_iter = 0;
                        wclear(g_cmd_win);
                } else if (key > 31 && key < 129){
                        // dont have space to display
                        if (key_buffer_iter == 80)
                                continue;

                        waddch(g_cmd_win, key);
                        key_buffer[key_buffer_iter++] = key;
                }
                wrefresh(g_cmd_win);
        }
}
