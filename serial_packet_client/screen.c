/*
 * author: Mehmet ASLAN
 * date: March 3, 2017
 *
 * no warranty, no licence agreement
 * use it at your own risk
 */

#include "screen.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "io.h"

// all lines have 80 chars + 1 null
// which makes line's null obligated to 80 index
#define LINE_NULL_INDEX 80
#define LINE_SIZE 81
#define LINE_LENGTH 80

WINDOW *g_cmd_win;
WINDOW *g_norm_win;
WINDOW *g_pin_win;

// contains keyboard inputs until return received
char key_buffer[120];
int key_buffer_iter = 0;
// line array
char **scroll_buffer;
int scroll_buffer_size = 0;
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

        scroll_buffer_size = (lines - number_of_pins - 3);
        g_number_of_pins = number_of_pins;

        g_pin_win = newwin(number_of_pins, LINE_LENGTH, 0, 0);
        g_norm_win = newwin(scroll_buffer_size, LINE_LENGTH, number_of_pins + 1, 0);
        g_cmd_win = newwin(1, LINE_LENGTH, lines - 1, 0);

        scroll_buffer = (char **) malloc(sizeof(char *) * scroll_buffer_size);

        for (int i = 0; i < scroll_buffer_size; ++i) {
                if ((scroll_buffer[i] = (char *) malloc(sizeof(char) * LINE_SIZE)) == NULL) {
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
        for (int i = 0; i < scroll_buffer_size - 1; ++i)
                scroll_buffer[i] = scroll_buffer[i+1];
        // can not longer, dont want to mess with dynamic length
        if (strlen(line) > LINE_LENGTH)
                line[LINE_NULL_INDEX] = '\0';
        // save line, old scroll buffers first
        strcpy(remember_line, line);
        // put new string to end of scroll buffer
        scroll_buffer[scroll_buffer_size - 1] = remember_line;
}

void print_to_screen(char *line, int pin_number)
{
        if (pin_number < 0) { // i am not pinned
                print_to_scroll_buffer(line);
                wclear(g_norm_win);

                for (int i = 0; i < scroll_buffer_size; ++i)
                        mvwprintw(g_norm_win, i, 0, scroll_buffer[i]);

                wrefresh(g_norm_win);

        } else if (pin_number < g_number_of_pins){

                wmove(g_pin_win, pin_number, 0);
                wclrtoeol(g_pin_win);
                mvwprintw(g_pin_win, pin_number, 0, line);
                wrefresh(g_pin_win);
        }
}

int is_screen_changed(void)
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

        return size_changed;
}

void reinit_screen(void)
{
        int old_scroll_buffer_size = scroll_buffer_size;

        char **new_scroll_buffer = NULL;

        scroll_buffer_size = lines - g_number_of_pins - 3;

        delwin(g_cmd_win);
        delwin(g_norm_win);
        delwin(g_pin_win);
        wclear(stdscr);

        g_pin_win = newwin(g_number_of_pins, LINE_LENGTH, 0, 0);
        g_norm_win = newwin(scroll_buffer_size, LINE_LENGTH, g_number_of_pins + 1, 0);
        g_cmd_win = newwin(1, LINE_LENGTH, lines - 1, 0);

        wclear(g_pin_win);
        wclear(g_norm_win);
        wclear(g_cmd_win);

        new_scroll_buffer = (char **) malloc(sizeof(char *) * scroll_buffer_size);

        if (new_scroll_buffer == NULL) {
                fprintf(stderr, "new scroll buf pp allocation err\n");
                exit(-1);
        }

        for (int i = 0; i < scroll_buffer_size; ++i) {
                new_scroll_buffer[i] = (char *) malloc(sizeof(char) * LINE_SIZE);

                if (new_scroll_buffer[i] == NULL) {
                        fprintf(stderr, "new scroll buf allocation err\n");
                        exit(-1);
                }

                memset(new_scroll_buffer[i], 0, sizeof(char) * LINE_SIZE);
        }

        char **remember_buffer = scroll_buffer;
        scroll_buffer = new_scroll_buffer;

        if (scroll_buffer_size >= old_scroll_buffer_size) {
                for (int i = 0; i < old_scroll_buffer_size; ++i) {
                        print_to_screen(remember_buffer[i], -1);
                }

        } else {
                for (int i = old_scroll_buffer_size - scroll_buffer_size; i < old_scroll_buffer_size; ++i) {
                        print_to_screen(remember_buffer[i], -1);
                }
        }

        for (int i = 0; i < old_scroll_buffer_size; ++i) {
                free(remember_buffer[i]);
        }

        if (remember_buffer != NULL) {
                free(remember_buffer);
        }

        mvwprintw(g_cmd_win, 0, 0, key_buffer);
        wrefresh(g_cmd_win);
}

long int last_read_loc = 0;
char stderr_line_buffer[81];
int stderr_line_iter = 0;

void get_keys(void)
{
        int key;

        if (is_screen_changed()) {
                reinit_screen();
        }

        // display stderr file in our norm_win
        fseek(stderr, last_read_loc, SEEK_SET);

        char c;
        while (fread(&c, 1, 1, stderr)) {
                // accepts line based input
                if (c != '\n') {
                        if (stderr_line_iter < LINE_LENGTH)
                                stderr_line_buffer[stderr_line_iter++] = c;
                } else {
                        stderr_line_buffer[stderr_line_iter] = '\0';
                        stderr_line_buffer[LINE_NULL_INDEX] = '\0';
                        print_to_screen(stderr_line_buffer, -1);

                        stderr_line_iter = 0;
                }
        }

        last_read_loc = ftell(stderr);
        fseek(stderr, 0, SEEK_END);

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
                        if (key_buffer_iter == LINE_LENGTH)
                                continue;

                        waddch(g_cmd_win, key);
                        key_buffer[key_buffer_iter++] = key;
                }
                wrefresh(g_cmd_win);
        }
}
