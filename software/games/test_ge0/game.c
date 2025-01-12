#include <gpio.h>
#include <lcd.h>
#include <screen.h>
#include <display.h>
#include <stdio.h>

void game(void);

void game(void){
    lcd_init();
    setScreenResolution(100, 100);
    const uint16_t colors [] = {LCD_COLOR_RED, LCD_COLOR_GREEN, LCD_COLOR_BLUE}; 
    changePalette(1, colors[0]);
    changePalette(2, colors[1]);
    changePalette(3, colors[2]);
    for (int v = 0; v < 50; v++) {
        setPix(v, v, v%3+1);
    }
    printf("setPix done\n");

    for(int i=0;i<0xff;++i){
        redrawScreen();
        changePalette(1, colors[i%3]);
        changePalette(2, colors[(i+1)%3]);
        changePalette(3, colors[(i+2)%3]);
    }
}