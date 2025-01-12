// ===================================================================================
// Project:   Conway's Game of Life for CH32V003 and SSD1306 128x64 Pixels I2C OLED
// Version:   v1.0
// Year:      2023
// Author:    Stefan Wagner
// Github:    https://github.com/wagiminator
// EasyEDA:   https://easyeda.com/wagiminator
// License:   http://creativecommons.org/licenses/by-sa/3.0/
// ===================================================================================
//
// Description:
// ------------
// The Game of Life is a cellular automaton, and was invented by Cambridge 
// mathematician John Conway.
// This game became widely known when it was mentioned in an article published by 
// Scientific American in 1970. It consists of a grid of cells which, based on a 
// few mathematical rules, can live, die or multiply. Depending on the initial 
// conditions, the cells form various patterns throughout the course of the game.
//
// Rules:
// - For a space that is populated:
//   - Each cell with one or no neighbors dies, as if by solitude.
//   - Each cell with four or more neighbors dies, as if by overpopulation.
//   - Each cell with two or three neighbors survives.
// - For a space that is empty or unpopulated:
//   - Each cell with three neighbors becomes populated.
//
// Connect an SSD1306 128x64 Pixels I2C OLED to PC1 (SDA) and PC2 (SCL). 
// The implementation utilizes DMA for data transfer to the OLED while simultaneously 
// computing the next game step. Press the ACT key connected to PA2 to restart the
// game.
//
// References:
// -----------
// - CNLohr ch32v003fun: https://github.com/cnlohr/ch32v003fun
// - WCH Nanjing Qinheng Microelectronics: http://wch.cn
//
// Compilation Instructions:
// -------------------------
// - Make sure GCC toolchain (gcc-riscv64-unknown-elf, newlib) and Python3 with rvprog
//   are installed. In addition, Linux requires access rights to WCH-LinkE programmer.
// - Connect the WCH-LinkE programmer to the MCU board.
// - Run 'make flash'.


// ===================================================================================
// Libraries, Definitions and Macros
// ===================================================================================
#include "tinys_driver.h"

#define GAME_START    0xBEEFAFFE  // define 32-bit game start code
#define PIN_ACT       PA2         // pin connected to ACT butoon, active low

uint8_t page1[1024], page2[896];  // double screen buffer

const uint8_t GAME_TEXT[] = {
  0x00, 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E,
  0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F,
  0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00, 0x07, 0x08, 0x70, 0x08, 0x07,
  0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x46, 0x49, 0x49, 0x49, 0x31,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A,
  0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F,
  0x00, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, 0x7F, 0x09, 0x09, 0x09, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x40, 0x40, 0x40, 0x40,
  0x00, 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 0x7F, 0x09, 0x09, 0x09, 0x01,
  0x00, 0x7F, 0x49, 0x49, 0x49, 0x41
};

// ===================================================================================
// Pseudo myrandom Number Generator
// ===================================================================================
uint32_t myrandom(uint32_t max) {
  static uint32_t rnval = GAME_START;
  rnval = rnval << 16 | (rnval << 1 ^ rnval << 2) >> 16;
  return(rnval % max);
}

// ===================================================================================
// Conway's Game of Life
// ===================================================================================

// Get pixel from source screen buffer
uint8_t getpixel(uint8_t xpos, uint8_t ypos) {
  xpos &= 127;
  if(ypos == 56)  ypos = 0;
  if(ypos == 255) ypos = 55;
  return((page1[((uint16_t)ypos >> 3) * 128 + 128 + xpos] >> (ypos & 7)) & 1);
}

// Set pixel on destination screen buffer
void setpixel(uint8_t xpos, uint8_t ypos) {
  page2[((uint16_t)ypos >> 3) * 128 + xpos] |= ((uint8_t)1 << (ypos & 7));
}

// Calculate next game step
void calculate(void) {
  for(uint16_t i=0; i<896; i++) page2[i] = 0;
  for(uint8_t y=0; y<56; y++) {
    for(uint8_t x=0; x<128; x++) {
      uint8_t neighbors = getpixel(x-1, y-1) \
                        + getpixel(x  , y-1) \
                        + getpixel(x+1, y-1) \
                        + getpixel(x-1, y  ) \
                        + getpixel(x+1, y  ) \
                        + getpixel(x-1, y+1) \
                        + getpixel(x  , y+1) \
                        + getpixel(x+1, y+1);
      if((getpixel(x,y) == 1) && (neighbors >= 2) && (neighbors <= 3)) setpixel(x,y);
      if((getpixel(x,y) == 0) && (neighbors == 3)) setpixel(x,y);
    }
  }
  for(uint16_t i=0; i<896; i++) page1[i + 128] = page2[i];
}

// Setupt start screen
void GAME_init(void) {
  for(uint16_t i=0; i<896; i++) page2[i] = 0;
  for(uint16_t i=768; i; i--) setpixel(myrandom(128), myrandom(56));
  for(uint16_t i=0; i<896; i++) page1[i + 128] = page2[i];
  for(uint16_t i=0; i<sizeof(GAME_TEXT); i++) page1[i] = GAME_TEXT[i];
}

// ===================================================================================
// Main Function
// ===================================================================================
int main(void) {
  // Setup game
  GAME_init();                            // setup start screen

  JOY_init();

  // Loop
  while(1) {
    if(!JOY_act_pressed()) GAME_init();   // re-setup start screen if ACT button pressed
    else calculate();                     // else calculate next game step
    // I2C_start(OLED_ADDR);                 // start transmission to OLED
    // I2C_write(OLED_DAT_MODE);             // set data mode
    // I2C_writeBuffer(page1, 1024);         // send screen buffer using DMA
    
    JOY_OLED_data_start(0);
    for(int i=0;i<1024;++i) JOY_OLED_send(page1[i]);

  }
}
