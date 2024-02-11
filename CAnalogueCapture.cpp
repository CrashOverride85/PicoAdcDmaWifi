
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

#include "CAnalogueCapture.h"

volatile int CAnalogueCapture::_s_irq_counter1;
volatile int CAnalogueCapture::_s_irq_counter2;
uint CAnalogueCapture::_s_dma_chan1;
uint CAnalogueCapture::_s_dma_chan2;

volatile bool CAnalogueCapture::_s_buf1_ready;
volatile bool CAnalogueCapture::_s_buf2_ready;
volatile uint64_t CAnalogueCapture::_capture_buf1_end_time_us;
volatile uint64_t CAnalogueCapture::_capture_buf2_end_time_us;
 
CAnalogueCapture::CAnalogueCapture()
{
    _capture_duration_us = ((double)CAPTURE_DEPTH * ((double)1/(double)SAMPLES_PER_SECOND)) * 1000 * 1000;
}

// Called when capture_buf1 is full.
void CAnalogueCapture::s_dma_handler1()
{
    adc_select_input(ADC_CHANNEL_0);
    _s_irq_counter1++;
    _s_buf1_ready = true;
    _s_buf2_ready = false;
    _capture_buf1_end_time_us = time_us_64();

    // Clear the interrupt request.
    dma_hw->ints0 = 1u << _s_dma_chan1;
}

// Called when capture_buf2 is full.
void CAnalogueCapture::s_dma_handler2()
{
    adc_select_input(ADC_CHANNEL_0);
    _s_irq_counter2++;
    _s_buf1_ready = false;
    _s_buf2_ready = true;
    _capture_buf2_end_time_us = time_us_64();

    // Clear the interrupt request
    dma_hw->ints0 = 1u << _s_dma_chan2;
}

void CAnalogueCapture::start()
{
    if (_running)
    {
        printf("CAnalogueCapture::start(): Error - already running\n");
        return;
    }

    printf("CAnalogueCapture::start(): Capture time = %luus\n", _capture_duration_us);

    _s_irq_counter1 = 0;
    _s_irq_counter2 = 0;
    adc_gpio_init(26 + ADC_CHANNEL_0);
    adc_gpio_init(26 + ADC_CHANNEL_1);
    adc_gpio_init(26 + ADC_CHANNEL_2);
    adc_init();
    adc_select_input(ADC_CHANNEL_0);
    adc_set_round_robin(1 << ADC_CHANNEL_0 | 1 << ADC_CHANNEL_1 | 1 << ADC_CHANNEL_2);

    adc_fifo_setup (true,   // Write each completed conversion to the sample FIFO
                    true,   // Enable DMA data request (DREQ)
                    1,      // DREQ (and IRQ) asserted when at least 1 sample present
                    false,  // Don't collect error bit
                    true    // Reduce samples to 8 bits
    );

    // Determines the ADC sampling rate as a divisor of the basic
    // 48Mhz clock. 
    uint32_t divider = 48000000 / SAMPLES_PER_SECOND;
    adc_set_clkdiv(divider - 1);

    _s_dma_chan1 = dma_claim_unused_channel(true);
    _s_dma_chan2 = dma_claim_unused_channel(true);
    printf("CAnalogueCapture: using DMA channels: %d & %d\n", _s_dma_chan1, _s_dma_chan2);

    // Chan 1
    dma_channel_config dma_config1 = dma_channel_get_default_config(_s_dma_chan1);
    channel_config_set_transfer_data_size(&dma_config1, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_config1, false);  // ADC fifo
    channel_config_set_write_increment(&dma_config1, true);  // RAM buffer.
    // channel_config_set_write_increment(&dma_config1, true);
    // Wrap to beginning of buffer. Assuming buffer is well aligned.
    channel_config_set_ring(&dma_config1, true, CAPTURE_RING_BITS);
    // Paced by ADC generated requests.
    channel_config_set_dreq(&dma_config1, DREQ_ADC);
    // When done, start the other channel.
    channel_config_set_chain_to(&dma_config1, _s_dma_chan2);
    // Using interrupt channel 0
    dma_channel_set_irq0_enabled(_s_dma_chan1, true);
    // Set IRQ handler.
    irq_set_exclusive_handler(DMA_IRQ_0, s_dma_handler1);
    irq_set_enabled(DMA_IRQ_0, true);
    dma_channel_configure(_s_dma_chan1, &dma_config1,
                            capture_buf1,   // dst
                            &adc_hw->fifo,  // src
                            CAPTURE_DEPTH,  // transfer count
                            true            // start immediately
    );


    // Chan 2
    dma_channel_config dma_config2 = dma_channel_get_default_config(_s_dma_chan2);
    channel_config_set_transfer_data_size(&dma_config2, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_config2, false);
    channel_config_set_write_increment(&dma_config2, true);
    channel_config_set_ring(&dma_config2, true, CAPTURE_RING_BITS);
    channel_config_set_dreq(&dma_config2, DREQ_ADC);
    // When done, start the other channel.
    channel_config_set_chain_to(&dma_config2, _s_dma_chan1);
    dma_channel_set_irq1_enabled(_s_dma_chan2, true);
    irq_set_exclusive_handler(DMA_IRQ_1, s_dma_handler2);
    irq_set_enabled(DMA_IRQ_1, true);
    dma_channel_configure(_s_dma_chan2, &dma_config2,
                            capture_buf2,   // dst
                            &adc_hw->fifo,  // src
                            CAPTURE_DEPTH,  // transfer count
                            false           // Do not start immediately
    );

    // Start the ADC free run sampling.
    adc_run(true);

    _running = true;
}

void CAnalogueCapture::stop()
{
    if (_running)
    {
        printf("CAnalogueCapture::stop()\n");

        dma_channel_abort(_s_dma_chan1);
        dma_channel_abort(_s_dma_chan2);

        adc_run(false);
        irq_set_enabled(DMA_IRQ_0, false);
        irq_set_enabled(DMA_IRQ_1, false);
        dma_channel_set_irq0_enabled(_s_dma_chan1, false);
        dma_channel_set_irq1_enabled(_s_dma_chan2, false);

        dma_channel_cleanup(_s_dma_chan1);
        dma_channel_unclaim(_s_dma_chan1);

        dma_channel_cleanup(_s_dma_chan2);
        dma_channel_unclaim(_s_dma_chan2);

        _running = false;
        printf("CAnalogueCapture::stop() done\n");
    }
}
