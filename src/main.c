#include <math.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "shift_reg.h"
#include "seven_seg.h"
#include "uart.h"

void adc_init(void)
{
    ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    ADCSRA |= (1<<ADIE) | (1<<ADEN);
}

void adc_read(uint8_t channel)
{
    // Kanal waehlen, ohne andere Bits zu beeinflußen
    ADMUX = (ADMUX & ~(0x1F)) | (channel & 0x1F);
    ADCSRA |= (1<<ADSC);
}

static seven_seg sseg;

ISR(TIMER0_OVF_vect)
{
    seven_seg_loop(&sseg);
}

ISR(ADC_vect)
{
    static uint16_t avg = 0;
    static uint8_t avg_n = 0;
    float u,r,t;

    avg += ADCW;

    if(++avg_n>79)
    {
        u = 5.f*(((float)avg/(float)avg_n)/1024.f);
        r = ((5.f*3.9f)/u)-3.9f;
        t = 25.f+2000.f*(sqrt(48425.f*r-9616.f)-197.f)/1937.f;

        uart_buf_putf(t);
        uart_buf_putc('\n');

        seven_seg_set_val(&sseg, (uint8_t)t);

        avg = avg_n = 0;
    }

    adc_read(2);
}

int main(void)
{
    uart_init(UBRR_VALUE);

    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    shift_reg reg;

    DDRC  = 0xff;
    PORTC = 0x0;

    shift_reg_init(&reg, &PORTC, PC2, PC1, PC0);

    sseg.num_seg = 2;
    sseg.port = &PORTC;
    sseg.seg_ano = &reg;
    sseg.seg_cat[0] = PC3;
    sseg.seg_cat[1] = PC4;
    seven_seg_init(&sseg);
    seven_seg_set_val(&sseg, 0);

    TCCR0 = (1<<CS02);
    TIMSK |= (1<<TOIE0);

    adc_init();
    adc_read(2);

    sei();

    while(1)
    {
        sleep_cpu();
    }

    /* wird nie erreicht */
    return 0;
}
