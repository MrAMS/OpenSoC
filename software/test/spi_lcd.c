#include <stdio.h>

#include "gpio.h"
#include "lcd.h"

void lcd(void);

void lcd(void){
	lcd_init();
	for(int i=-20;i<20;++i){
        lcd_draw_circle(40+i, 40+i, 20, LCD_COLOR_RED);
    }
}