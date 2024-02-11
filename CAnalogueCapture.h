#ifndef _CANALOGUECAPTURE_H
#define _CANALOGUECAPTURE_H

#include <inttypes.h>
#include <stdio.h>

// DMA/ADC code copied/adapted from https://forums.raspberrypi.com/viewtopic.php?p=1861895#p1861895

#define ADC_CHANNEL_0 0
#define ADC_CHANNEL_1 1
#define ADC_CHANNEL_2 2

#define CAPTURE_DEPTH 1024
#define CAPTURE_RING_BITS 10
#define SAMPLES_PER_SECOND 62500 

class CAnalogueCapture
{
    public:
        CAnalogueCapture();
        void start();
        void stop();

    private:
        static void s_dma_handler1();
        static void s_dma_handler2();

        uint8_t capture_buf1[CAPTURE_DEPTH] __attribute__((aligned(2048)));
        uint8_t capture_buf2[CAPTURE_DEPTH] __attribute__((aligned(2048)));

        uint8_t _audio_buffer_l[CAPTURE_DEPTH/3] = {0};
        uint8_t _audio_buffer_r[CAPTURE_DEPTH/3] = {0};
        
        static uint _s_dma_chan1;
        static uint _s_dma_chan2;
        static volatile int _s_irq_counter1;
        static volatile int _s_irq_counter2;
        static volatile bool _s_buf1_ready;
        static volatile bool _s_buf2_ready;
        static volatile uint64_t _capture_buf1_end_time_us;
        static volatile uint64_t _capture_buf2_end_time_us;

        uint64_t time_last_print=0;
        bool _new_battery_readings = false;
        uint32_t _capture_duration_us;
        uint64_t _last_buffer_update_time_us;
        uint64_t _capture_end_time_time_us;
        bool _running = false;
};

#endif
