#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define JOY_SOUND           1     // 0: no sound, 1: with sound

// in system.h
void busy_wait(unsigned int ms);
void busy_wait_us(unsigned int us);

// Game slow-down delay
#define JOY_SLOWDOWN()      busy_wait(20)

// Delays
#define JOY_DLY_ms          busy_wait
#define JOY_DLY_us          busy_wait_us

void JOY_init(void);

void JOY_OLED_end(void);
void JOY_OLED_send(uint8_t data);
void JOY_OLED_data_start(uint8_t y);

uint8_t JOY_act_pressed(void);
uint8_t JOY_act_released(void);
uint8_t JOY_pad_pressed(void);
uint8_t JOY_pad_released(void);
uint8_t JOY_all_released(void);

uint8_t JOY_up_pressed(void);
uint8_t JOY_down_pressed(void);
uint8_t JOY_left_pressed(void);
uint8_t JOY_right_pressed(void);

void JOY_sound(uint8_t freq, uint8_t dur);

uint16_t JOY_random(void);

long map(long x, long in_min, long in_max, long out_min, long out_max);
