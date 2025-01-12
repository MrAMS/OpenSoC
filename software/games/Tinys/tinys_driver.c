#include "tinys_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <irq.h>
#include <libbase/uart.h>

#include <lcd.h>

void JOY_init(void){
#ifdef CONFIG_CPU_HAS_INTERRUPT
	irq_setmask(0);
	irq_setie(1);
#endif
	uart_init();
    lcd_init();
    printf("Ready to go\n");
}

void JOY_OLED_end(void){
    return;
}

static uint16_t draw_x, draw_y;

void JOY_OLED_send(uint8_t data){
    draw_x++;
    if(draw_x==128){
        draw_x = 0;
        draw_y += 8;
    }
    for(int i=0;i<8;++i){
        lcd_draw_point(draw_x, draw_y+i, ((data>>i)&1)?LCD_COLOR_WHITE:LCD_COLOR_BLACK);
    }
}

void JOY_OLED_data_start(uint8_t page){
    draw_x = 0;
    draw_y = page*8;
}

uint8_t JOY_act_pressed(void){
    return 1; // to start game
}

uint8_t JOY_act_released(void){
    return 0;
}

uint8_t JOY_pad_pressed(void){
    return 0;
}

uint8_t JOY_pad_released(void){
    return 0;
}

uint8_t JOY_all_released(void){
    return 0;
}

uint8_t JOY_up_pressed(void) {
    return 0;
    // TODO
    // uint16_t val = ADC_read();
    // return(   ((val > JOY_N  - JOY_DEV) && (val < JOY_N  + JOY_DEV))
    //         | ((val > JOY_NE - JOY_DEV) && (val < JOY_NE + JOY_DEV))
    //         | ((val > JOY_NW - JOY_DEV) && (val < JOY_NW + JOY_DEV)) );
}

uint8_t JOY_down_pressed(void) {
    return 0;
    // TODO
    // uint16_t val = ADC_read();
    // return(   ((val > JOY_S  - JOY_DEV) && (val < JOY_S  + JOY_DEV))
    //         | ((val > JOY_SE - JOY_DEV) && (val < JOY_SE + JOY_DEV))
    //         | ((val > JOY_SW - JOY_DEV) && (val < JOY_SW + JOY_DEV)) );
}

uint8_t JOY_left_pressed(void) {
    return 0;
    // TODO
    // uint16_t val = ADC_read();
    // return(   ((val > JOY_W  - JOY_DEV) && (val < JOY_W  + JOY_DEV))
    //         | ((val > JOY_NW - JOY_DEV) && (val < JOY_NW + JOY_DEV))
    //         | ((val > JOY_SW - JOY_DEV) && (val < JOY_SW + JOY_DEV)) );
}

uint8_t JOY_right_pressed(void) {
    return 0;
    // TODO
    // uint16_t val = ADC_read();
    // return(   ((val > JOY_E  - JOY_DEV) && (val < JOY_E  + JOY_DEV))
    //         | ((val > JOY_NE - JOY_DEV) && (val < JOY_NE + JOY_DEV))
    //         | ((val > JOY_SE - JOY_DEV) && (val < JOY_SE + JOY_DEV)) );
}

// Buzzer
void JOY_sound(uint8_t freq, uint8_t dur) {
    // TODO
    //   while(dur--) {
    //     #if JOY_SOUND == 1
    //     if(freq) PIN_low(PIN_BEEP);
    //     #endif
    //     DLY_us(255 - freq);
    //     PIN_high(PIN_BEEP);
    //     DLY_us(255 - freq);
    //   }
}

// Pseudo random number generator
static uint16_t rnval = 0xACE1;
uint16_t JOY_random(void) {
    rnval = (rnval >> 0x01) ^ (-(rnval & 0x01) & 0xB400);
    return rnval;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}