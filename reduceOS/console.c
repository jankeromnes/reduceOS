// Console.c - Handles console functions
#include "console.h"
#include "string.h"
#include "types.h"
#include "vga.h"
#include "keyboard.h"

static uint16 *g_vga_buffer;
static uint32 g_vga_index;
static uint8 cursorx = 0, cursory = 0;
uint8 g_fore_color = COLOR_WHITE, g_back_color = COLOR_BLACK;


void clearConsole(VGA_COLOR_TYPE color1, VGA_COLOR_TYPE color2) {
    
    for (uint32 i = 0; i < VGA_TOTAL_ITEMS; i++) {
        g_vga_buffer[i] = vga_item_entry(NULL, color1, color2);
    }

    g_vga_index = 0;
    cursorx = 0, cursory = 0;
    vga_set_cursor_pos(cursorx, cursory);
}


void initConsole(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color) {
    g_vga_buffer = (uint16 *)VGA_ADDRESS;
    g_fore_color = fore_color, g_back_color = back_color;
    cursorx = 0, cursory = 0;
    clearConsole(fore_color, back_color);
}

static void consoleNewline() {
    if (cursory >= VGA_HEIGHT) {
        cursorx = 0, cursory = 0;
        clearConsole(g_fore_color, g_back_color);
    } else {
        g_vga_index += VGA_WIDTH - (g_vga_index % VGA_WIDTH);
        cursorx = 0;
        ++cursory;
        vga_set_cursor_pos(cursorx, cursory);
    }
}

void consolePutchar(char ch) {
    // Newline handling
    if (ch == '\n') {
        consoleNewline();
    } else if (ch == '\t')  { // Tab handling 
        for (int i=0; i<4;i++) {
            g_vga_buffer[g_vga_index++] = vga_item_entry(' ', g_fore_color, g_back_color);
            vga_set_cursor_pos(cursorx++, cursory);
        }
    } else if (ch == ' ') { // Space handling
        g_vga_buffer[g_vga_index++] = vga_item_entry(' ', g_fore_color, g_back_color);
        vga_set_cursor_pos(cursorx++, cursory);
    } else {
        if (ch > 0) {
            g_vga_buffer[g_vga_index++] = vga_item_entry(ch, g_fore_color, g_back_color);
            vga_set_cursor_pos(++cursorx, cursory);
        }
    }
}

void consoleUngetchar() {
    if(g_vga_index > 0) {
        g_vga_buffer[g_vga_index--] = vga_item_entry(0, g_fore_color, g_back_color);
        if(cursorx > 0) {
            vga_set_cursor_pos(cursorx--, cursory);
        } else {
            cursorx = VGA_WIDTH;
            if (cursory > 0)
                vga_set_cursor_pos(cursorx--, --cursory);
            else
                cursory = 0;
        }
    }

    g_vga_buffer[g_vga_index] = vga_item_entry(0, g_fore_color, g_back_color);
}

void consoleUngetcharBound(uint8 n) {
    if(((g_vga_index % VGA_WIDTH) > n) && (n > 0)) {
        g_vga_buffer[g_vga_index--] = vga_item_entry(0, g_fore_color, g_back_color);
        if(cursorx >= n) {
            vga_set_cursor_pos(cursorx--, cursory);
        } else {
            cursorx = VGA_WIDTH;
            if (cursory > 0)
                vga_set_cursor_pos(cursorx--, --cursory);
            else
                cursory = 0;
        }
    }

    g_vga_buffer[g_vga_index] = vga_item_entry(0, g_fore_color, g_back_color);
}

void consoleGoXY(uint16 x, uint16 y) {
    g_vga_index = (80*y)+x;
    cursorx = x, cursory = y;
    vga_set_cursor_pos(cursorx, cursory);
}

void consolePrintString(const char *str) {
    uint32 index = 0;
    while (str[index]) {
        if (str[index] == '\n')
            consoleNewline();
        else
            consolePutchar(str[index]);
        index++;
    }
}

void printf(const char *format, ...) {
    char **arg = (char **)&format;
    int c;
    char buf[32];

    arg++;

    memset(buf, 0, sizeof(buf));
    while ((c = *format++) != 0) {
        if (c != '%')
            consolePutchar(c);
        else {
            char *p, *p2;
            int pad0 = 0, pad = 0;

            c = *format++;
            if (c == '0') {
                pad0 = 1;
                c = *format++;
            }

            if (c >= '0' && c <= '9') {
                pad = c - '0';
                c = *format++;
            }

            switch (c) {
                case 'd':
                case 'u':
                case 'x':
                    itoa(buf, c, *((int *)arg++));
                    p = buf;
                    goto string;
                    break;

                case 's':
                    p = *arg++;
                    if (!p)
                        p = "(null)";

                string:
                    for (p2 = p; *p2; p2++)
                        ;
                    for (; p2 < p + pad; p2++)
                        consolePutchar(pad0 ? '0' : ' ');
                    while (*p)
                        consolePutchar(*p++);
                    break;

                default:
                    consolePutchar(*((int *)arg++));
                    break;
            }
        }
    }
}

void getString(char *buffer) {
    if (!buffer) return; // If not a buffer, exit.
    while (1) {
        char ch = kb_getchar();
        if (ch == '\n') {
            printf("\n");;
            return ;
        } else {
            *buffer++ = ch;
            printf("%c", ch);
        }
    }
}

void getStringBound(char *buffer, uint8 bound) {
    /* Basically same code as last time, but with some extra steps */
    if (!bound) return;
    while(1) {
        char ch = kb_getchar();
        if (ch == '\n') {
            printf("\n");
            return;
        } else if (ch == '\b') {
            consoleUngetcharBound(bound);
            buffer--;
            *buffer = '\0';
        } else {
            *buffer++ = ch;
            printf("%c", ch);
        }
    }
}