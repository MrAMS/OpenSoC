#ifndef LCD_H_
#define LCD_H_
#include "sys/_stdint.h"
#include <stdbool.h>

#define LCD_HORIZONTAL 2 // 设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#if LCD_HORIZONTAL==0||LCD_HORIZONTAL==1
#define LCD_W 80
#define LCD_H 160

#else
#define LCD_W 160
#define LCD_H 80
#endif

//画笔颜色
enum LCD_COLOR {
     LCD_COLOR_WHITE         	 = 0xFFFF
    ,LCD_COLOR_BLACK         	 = 0x0000	  
    ,LCD_COLOR_BLUE            = 0x001F  
    ,LCD_COLOR_BRED            = 0XF81F
    ,LCD_COLOR_GRED 			 = 0XFFE0
    ,LCD_COLOR_GBLUE			 = 0X07FF
    ,LCD_COLOR_RED           	 = 0xF800
    ,LCD_COLOR_MAGENTA       	 = 0xF81F
    ,LCD_COLOR_GREEN         	 = 0x07E0
    ,LCD_COLOR_CYAN          	 = 0x7FFF
    ,LCD_COLOR_YELLOW        	 = 0xFFE0
    ,LCD_COLOR_BROWN 			 = 0XBC40 //棕色
    ,LCD_COLOR_BRRED 			 = 0XFC07 //棕红色
    ,LCD_COLOR_GRAY  			 = 0X8430 //灰色
    ,LCD_COLOR_DARKBLUE      	 = 0X01CF	//深蓝色
    ,LCD_COLOR_LIGHTBLUE       = 0X7D7C	//浅蓝色  
    ,LCD_COLOR_GRAYBLUE        = 0X5458 //灰蓝色
    ,LCD_COLOR_LIGHTGREEN      = 0X841F //浅绿色
    ,LCD_COLOR_LGRAY 			 = 0XC618 //浅灰色(PANNEL),窗体背景色
    ,LCD_COLOR_LGRAYBLUE       = 0XA651 //浅灰蓝色(中间层颜色)
    ,LCD_COLOR_LBBLUE          = 0X2B12 //浅棕蓝色(选择条目的反色)
};

void lcd_init(void);
void lcd_data_8bit(const uint8_t data);
void lcd_data_16bit(const uint16_t data);
void lcd_cmd(const uint8_t cmd);
void lcd_set_addr(const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2);
void lcd_draw_point(const uint16_t x, const uint16_t y, const uint16_t color);
void lcd_fill(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint16_t color);
void lcd_draw_circle(const uint16_t x, const uint16_t y, const uint8_t r, const uint16_t color);

#endif // LCD_H_