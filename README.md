# CNC-Router-Handwheel
This repo contains the firmware for an ESP32 based CNC router handwheel to be used with Candle2

# Important Note
Candle2 communicates with the pendant at 230400 baud. There is an issue in the ESP32 core in file packages\esp32\hardware\esp32\3.3.8\cores\esp32\esp32-hal-uart.c (as of 3.3.8). UART uses REF_TCIK (1MHz) as clock for baud rates under a certain limit and the APB clock (80MHz) ```UART_SCLK_APB``` over that limit. This is controlled via a define ```REF_TICK_BAUDRATE_LIMIT``` with default value ```250000```. However, I noticed corruption and garbled characters at 230400 baud using the REF_TCIK clock, which go away when switching to the APB clock by 
changing the ```REF_TICK_BAUDRATE_LIMIT``` to 200000: 

```
#define REF_TICK_BAUDRATE_LIMIT 200000  // this is maximum UART badrate using REF_TICK as clock
```

This is likely because at 1MHz, REF_TICK cannot generate baud 230400 with low error: 

```
1,000,000 / 230,400 ≈ 4.340
```

The UART divider must be integer or fractional with limited resolution.
The actual achievable rate ends up being off by ~3–4%, which is way outside the safe UART tolerance (typically <2%).

Using APB:

```
80,000,000 / 230,400 ≈ 347.22
```

The UART fractional divider can represent this with very low error (typically <0.1%).

Unfortunately, this is a hard define and cannot be overriden via a compiler directive, so we must sadly edit the core file and unfortunately, this patch needs to be applied every time the core is upgraded :(

The long term fix is to implement this code: 

```
#include "driver/uart.h"
#include "hal/uart_ll.h"
#include "soc/uart_reg.h"
#include "soc/uart_struct.h"

void forceUart0ApbClock(uint32_t speed)
{
    uart_ll_set_sclk(UART_LL_GET_HW(0), SOC_MOD_CLK_APB);
    uart_set_baudrate(UART_NUM_0, speed);
}
```

and call after ```Serial``` has started with ```Serial.Begin()```.