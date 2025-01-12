#ifndef GPIO_H_
#define GPIO_H_

#include "sys/_stdint.h"
#include <stdbool.h>
void gpio_init(const uint32_t io, const bool output);
void gpio_out(const uint32_t io, const bool high);

#endif // GPIO_H_