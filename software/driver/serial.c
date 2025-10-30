#include "serial.h"
#include <generated/csr.h>
#include <stdio.h>
#include <libbase/uart.h>

void serial_init(void){
    uart_init();
}

char* serial_readstr(void){
    char c[2];
	static char s[64];
	static int ptr = 0;

	if(uart_read_nonblock()) {
		c[0] = getchar();
		c[1] = 0;
		switch(c[0]) {
			case 0x7f:
			case 0x08:
				if(ptr > 0) {
					ptr--;
					fputs("\x08 \x08", stdout);
				}
				break;
			case 0x07:
				break;
			case '\r':
			case '\n':
				s[ptr] = 0x00;
				fputs("\n", stdout);
				ptr = 0;
				return s;
			default:
				if(ptr >= (sizeof(s) - 1))
					break;
				fputs(c, stdout);
				s[ptr] = c[0];
				ptr++;
				break;
		}
	}

	return NULL;
}

char serial_readchar(void){
	if(!uart_read_nonblock()) return 0;
    return getchar();
}