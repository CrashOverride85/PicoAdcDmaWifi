#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/dma.h"

#include "CAnalogueCapture.h"

CAnalogueCapture analogueCapture;


void enable_disable(uint8_t delay)
{
    uint8_t a[6] = {0}; // Doesn't seem to crash without this
    
    sleep_ms(delay);
    cyw43_arch_init();
    sleep_ms(5);
    cyw43_arch_poll();
    sleep_ms(5);
    cyw43_arch_enable_sta_mode();
    a[0] = cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 200, 1, 1, 10));
    int retval = cyw43_arch_wifi_connect_async("xxx", "xxx", CYW43_AUTH_WPA2_MIXED_PSK);
    if (retval)
    {
        printf("error: cyw43_arch_wifi_connect_async returned %d\n", retval);
    }
    sleep_ms(5);
    cyw43_arch_poll();
    sleep_ms(5);
    cyw43_arch_deinit();
}

int main()
{
    printf("reserve extra DMA chan %d\n", dma_claim_unused_channel(true));

    stdio_init_all();
    analogueCapture.start();
     
    puts("Hello, world!");

    int x = 0;
    while(1)
    {
        printf("loop = %d\n", x);
        enable_disable(x++);
    }
}
