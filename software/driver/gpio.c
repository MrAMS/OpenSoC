#include <generated/csr.h>
#include "gpio.h"

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
