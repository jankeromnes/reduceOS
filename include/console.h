#ifndef CONSOLE_H
#define CONSOLE_H

#include "vga.h"

void clearConsole(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color);

//initialize console
void initConsole(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color);

void consolePutchar(char ch);
// revert back the printed character and add 0 to it
void consoleUngetchar();
// revert back the printed character until n characters
void consoleUngetcharBound(uint8 n);

void consoleGoXY(uint16 x, uint16 y);

void consolePrintString(const char *str);
void printf(const char *format, ...);


// Get string from console

void getString(char *buffer);
// Get string from console and erase/go back until bound occcurs
void getStringBound(char *buffer, uint8 bound);
#endif