/*
 * author: Mehmet ASLAN
 * date: February 16, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */

#include "main.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "interface.h"

#define INPUT_WIN_LEN 7

struct _scroll_buffer{
        char **bfr;
        int lines, cols;
};

typedef struct _scroll_buffer scroll_buffer;

void init_pinned_buffer(int len);
void create_scroll_buffer(scroll_buffer *scb);

// make global scroll buffer
scroll_buffer g_scb_input;
scroll_buffer g_scb_norm;

WINDOW *g_input_win;
WINDOW *g_pinned_win;
WINDOW *g_norm_win;

char input_str[TERMINAL_LEN + 1];
int input_str_i = 0;

void init_interface()
{
        // common init
        // ncurses stuff
        initscr();
        refresh();
        cbreak(); // disables line buffering
        nodelay(stdscr, TRUE); // non-blocking mode
        noecho();
        keypad(stdscr, TRUE);

        g_input_win = newwin(INPUT_WIN_LEN, TERMINAL_LEN, LINES - INPUT_WIN_LEN, 0);
        box(g_input_win, 0, 0);
        wprintw(g_input_win, "input buffer");
        wrefresh(g_input_win);

        g_scb_input.lines = INPUT_WIN_LEN -3;
        g_scb_input.cols = TERMINAL_LEN + 1;
        create_scroll_buffer(&g_scb_input);
        // for now just variable mode
        switch (sp_mode) {
        case SP_MODE_VARIABLE:
                init_pinned_buffer(LINES - INPUT_WIN_LEN -1);
        break;
        }

        // pinned buffer
}

void init_pinned_buffer(int len)
{
        g_pinned_win = newwin(len, TERMINAL_LEN, 1, 0);
        box(g_pinned_win, 0, 0);
        wprintw(g_pinned_win, "pinned buffer");
        wrefresh(g_pinned_win);
}

void create_scroll_buffer(scroll_buffer *scb)
{
        // allocate memory
        scb->bfr = (char **) malloc(sizeof(char *) * scb->lines);

        for (int i = 0; i < scb->lines; ++i)
                scb->bfr[i] = (char *) malloc(sizeof(char) * scb->cols);
}

void write_line_to_scroll_buffer(scroll_buffer *scb, char *str)
{
        // shift string pointers
        char *tmp_ptr = scb->bfr[0];

        for (int i = 0; i < scb->lines - 1; ++i)
                scb->bfr[i] = scb->bfr[i+1];

        scb->bfr[scb->lines-1] = tmp_ptr;

        // strcpy str to last
        strcpy(scb->bfr[scb->lines-1], str);
}

void got_key_input(int c)
{
        // backspace
        if (c == KEY_BACKSPACE) {
                if (input_str_i)
                        input_str_i--;
                input_str[input_str_i] = '\0';
        } else if (c == 10) {
        // return
                handle_keyboard_input(input_str);
                write_input_buffer(input_str);
                for (int i = 0; i < TERMINAL_LEN + 1; ++i)
                        input_str[i] = '\0';
                input_str_i = 0;
        } else if (1) {
        // alphanumeric
                if (input_str_i != TERMINAL_LEN)
                        input_str[input_str_i++] = c;
        }

        wmove(g_input_win, INPUT_WIN_LEN - 2, 1);
        wclrtoeol(g_input_win);
        mvwprintw(g_input_win, INPUT_WIN_LEN - 2, 1, input_str);
        wrefresh(g_input_win);
}

// like printf with more color
void write_pinned_buffer(int line_number, char *format, ...)
{
        va_list args;
	char str[85];

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

        wmove(g_pinned_win, line_number + 1, 1);
        wclrtoeol(g_pinned_win);
        mvwprintw(g_pinned_win, line_number + 1, 1, str);
        wrefresh(g_pinned_win);
}

// like printf
void write_norm_buffer()
{
}

void write_input_buffer(char *format, ...)
{
        va_list args;
	char str[85];

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

        write_line_to_scroll_buffer(&g_scb_input, str);

        for (int i = 0; i < g_scb_input.lines; ++i) {
                wmove(g_input_win, i + 1, 1);
                wclrtoeol(g_input_win);
                mvwprintw(g_input_win, i + 1, 1, g_scb_input.bfr[i]);
        }

        wrefresh(g_input_win);
}

void set_title(char *str)
{
        printw(str);
        refresh();
}

void destroy_interface(void)
{
        free(g_scb_input.bfr);
        endwin();
}
