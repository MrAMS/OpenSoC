#include "sys/_stdint.h"
#include <stdio.h>

#include <libbase/i2c.h>

#include <irq.h>
#include <generated/csr.h>

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

const uint32_t IO_SPI_LCD_RST = 11;
const uint32_t IO_SPI_LCD_DC = 8;
const uint32_t LCD_HORIZONTAL = 2; // 设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

void gpio_init(const uint32_t io, const bool output){
    if(output){
        gpio0_oe_write(gpio0_oe_read() | (1U<<io));
    }else{
        gpio0_oe_write(gpio0_oe_read() & (~(1U<<io)));
    }
}

void gpio_out(const uint32_t io, const bool high){
    if(high){
        gpio0_out_write(gpio0_out_read() | (1U<<io));
    }else{
        gpio0_out_write(gpio0_out_read() & (~(1U<<io)));
    }
}

void lcd_data_8bit(const uint8_t data){
    spi0_mosi_write(data<<24);
    spi0_control_write(8U<<8|1);
    while((spi0_status_read()&1)!=1){
        // printf("waiting\n");
    }
}

void lcd_data_16bit(const uint16_t data){
    // lcd_data_8bit(data>>8);
    // lcd_data_8bit(data);

    spi0_mosi_write(data<<16);
    spi0_control_write(16U<<8|1);
    while((spi0_status_read()&1)!=1){
        // printf("waiting\n");
    }
}

void lcd_cmd(const uint8_t cmd){
    gpio_out(IO_SPI_LCD_DC, 0);
    lcd_data_8bit(cmd);
    gpio_out(IO_SPI_LCD_DC, 1);
}

void lcd_set_addr(const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2){
    switch (LCD_HORIZONTAL) {
        case 0:{
            lcd_cmd(0x2a);//列地址设置
            lcd_data_16bit(x1+26);
            lcd_data_16bit(x2+26);
            lcd_cmd(0x2b);//行地址设置
            lcd_data_16bit(y1+1);
            lcd_data_16bit(y2+1);
            lcd_cmd(0x2c);//储存器写
        }break;
        case 1:{
            lcd_cmd(0x2a);//列地址设置
            lcd_data_16bit(x1+26);
            lcd_data_16bit(x2+26);
            lcd_cmd(0x2b);//行地址设置
            lcd_data_16bit(y1+1);
            lcd_data_16bit(y2+1);
            lcd_cmd(0x2c);//储存器写
        }break;
        case 2:{
            lcd_cmd(0x2a);//列地址设置
            lcd_data_16bit(x1+1);
            lcd_data_16bit(x2+1);
            lcd_cmd(0x2b);//行地址设置
            lcd_data_16bit(y1+26);
            lcd_data_16bit(y2+26);
            lcd_cmd(0x2c);//储存器写

        }break;
        default:{
            lcd_cmd(0x2a);//列地址设置
            lcd_data_16bit(x1+1);
            lcd_data_16bit(x2+1);
            lcd_cmd(0x2b);//行地址设置
            lcd_data_16bit(y1+26);
            lcd_data_16bit(y2+26);
            lcd_cmd(0x2c);//储存器写
        }
    }
}

void lcd_draw_point(const uint16_t x, const uint16_t y, const uint16_t color){
    lcd_set_addr(x, y, x, y);
    lcd_data_16bit(color);
}

void lcd_fill(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint16_t color){
    lcd_set_addr(x0, y0, x1-1, y1-1);
    for(int i=y0;i<y1;++i)
        for(int j=x0;j<x1;++j)
            lcd_data_16bit(color);
}

void lcd_draw_circle(const uint16_t x, const uint16_t y, const uint8_t r, const uint16_t color){
    int a=0,b=r;
	while(a<=b)
	{
		lcd_draw_point(x-b,y-a,color);             //3           
		lcd_draw_point(x+b,y-a,color);             //0           
		lcd_draw_point(x-a,y+b,color);             //1                
		lcd_draw_point(x-a,y-b,color);             //2             
		lcd_draw_point(x+b,y+a,color);             //4               
		lcd_draw_point(x+a,y-b,color);             //5
		lcd_draw_point(x+a,y+b,color);             //6 
		lcd_draw_point(x-b,y+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r)) //判断要画的点是否过远
		{
			b--;
		}
	}
}

void lcd(void){
    gpio_init(IO_SPI_LCD_RST, true);
    gpio_init(IO_SPI_LCD_DC, true);

    gpio_out(IO_SPI_LCD_RST, 0);
    busy_wait(100);
    gpio_out(IO_SPI_LCD_RST, 1);
    busy_wait(100);

    lcd_cmd(0x11);     //Sleep out
	busy_wait(120);                //Delay 120ms
	lcd_cmd(0xB1);     //Normal mode
	lcd_data_8bit(0x05);   
	lcd_data_8bit(0x3C);   
	lcd_data_8bit(0x3C);   
	lcd_cmd(0xB2);     //Idle mode
	lcd_data_8bit(0x05);   
	lcd_data_8bit(0x3C);   
	lcd_data_8bit(0x3C);   
	lcd_cmd(0xB3);     //Partial mode
	lcd_data_8bit(0x05);   
	lcd_data_8bit(0x3C);   
	lcd_data_8bit(0x3C);   
	lcd_data_8bit(0x05);   
	lcd_data_8bit(0x3C);   
	lcd_data_8bit(0x3C);   
	lcd_cmd(0xB4);     //Dot inversion
	lcd_data_8bit(0x03);   
	lcd_cmd(0xC0);     //AVDD GVDD
	lcd_data_8bit(0xAB);   
	lcd_data_8bit(0x0B);   
	lcd_data_8bit(0x04);   
	lcd_cmd(0xC1);     //VGH VGL
	lcd_data_8bit(0xC5);   //C0
	lcd_cmd(0xC2);     //Normal Mode
	lcd_data_8bit(0x0D);   
	lcd_data_8bit(0x00);   
	lcd_cmd(0xC3);     //Idle
	lcd_data_8bit(0x8D);   
	lcd_data_8bit(0x6A);   
	lcd_cmd(0xC4);     //Partial+Full
	lcd_data_8bit(0x8D);   
	lcd_data_8bit(0xEE);   
	lcd_cmd(0xC5);     //VCOM
	lcd_data_8bit(0x0F);   
	lcd_cmd(0xE0);     //positive gamma
	lcd_data_8bit(0x07);   
	lcd_data_8bit(0x0E);   
	lcd_data_8bit(0x08);   
	lcd_data_8bit(0x07);   
	lcd_data_8bit(0x10);   
	lcd_data_8bit(0x07);   
	lcd_data_8bit(0x02);   
	lcd_data_8bit(0x07);   
	lcd_data_8bit(0x09);   
	lcd_data_8bit(0x0F);   
	lcd_data_8bit(0x25);   
	lcd_data_8bit(0x36);   
	lcd_data_8bit(0x00);   
	lcd_data_8bit(0x08);   
	lcd_data_8bit(0x04);   
	lcd_data_8bit(0x10);   
	lcd_cmd(0xE1);     //negative gamma
	lcd_data_8bit(0x0A);   
	lcd_data_8bit(0x0D);   
	lcd_data_8bit(0x08);   
	lcd_data_8bit(0x07);   
	lcd_data_8bit(0x0F);   
	lcd_data_8bit(0x07);   
	lcd_data_8bit(0x02);   
	lcd_data_8bit(0x07);   
	lcd_data_8bit(0x09);   
	lcd_data_8bit(0x0F);   
	lcd_data_8bit(0x25);   
	lcd_data_8bit(0x35);   
	lcd_data_8bit(0x00);   
	lcd_data_8bit(0x09);   
	lcd_data_8bit(0x04);   
	lcd_data_8bit(0x10);
		 
	lcd_cmd(0xFC);    
	lcd_data_8bit(0x80);  
		
	lcd_cmd(0x3A);     
	lcd_data_8bit(0x05);   
	lcd_cmd(0x36);
	if(LCD_HORIZONTAL==0)lcd_data_8bit(0x08);
	else if(LCD_HORIZONTAL==1)lcd_data_8bit(0xC8);
	else if(LCD_HORIZONTAL==2)lcd_data_8bit(0x78);
	else lcd_data_8bit(0xA8);   
	lcd_cmd(0x21);     //Display inversion
	lcd_cmd(0x29);     //Display on
	lcd_cmd(0x2A);     //Set Column Address
	lcd_data_8bit(0x00);   
	lcd_data_8bit(0x1A);  //26  
	lcd_data_8bit(0x00);   
	lcd_data_8bit(0x69);   //105 
	lcd_cmd(0x2B);     //Set Page Address
	lcd_data_8bit(0x00);   
	lcd_data_8bit(0x01);    //1
	lcd_data_8bit(0x00);   
	lcd_data_8bit(0xA0);    //160
	lcd_cmd(0x2C);

    for(int i=-20;i<20;++i){
        lcd_fill(0, 0, 160, 80, BLUE);
        lcd_draw_circle(40+i, 40+i, 20, RED);
    }
}