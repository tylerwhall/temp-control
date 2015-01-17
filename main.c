//#include <msp430g2553.h>
#include <msp430.h>
#include <signal.h>
#include <stdint.h>

static void clock_setup(void)
{
    BCSCTL3 |= LFXT1S_2;
}

static void start_timer(void)
{
    /* Use ACLK (12KHz) and count up mode */
    TACTL = TASSEL_1 | MC_1;
    /* Count 1 second */
    TACCR0 = 12000;
    TACCTL0 = CCIE;
}

__attribute__((interrupt(TIMER0_A0_VECTOR)))
void nop(void)
{
    P1OUT ^= 0x01;
}

static uint16_t adc_val = 0;;
static int adc_ran = 0;

__attribute__((interrupt(ADC10_VECTOR)))
void adc_irq(void)
{
    adc_val = ADC10MEM;
    adc_ran = 1;
}

static void setup_adc(void)
{
    ADC10CTL1 = INCH_5 | ADC10DIV_3;
    ADC10CTL0 = SREF_0 | ADC10SHT_3 | ADC10ON | ADC10IE;
    ADC10AE0 = BIT5;
    //P1SEL |= BIT5;
}

int main(void)
{
    int i;

    WDTCTL = WDTPW | WDTHOLD;

    P1DIR = (1 << 0) | (1 << 6);
    P1OUT = 0x00;

    clock_setup();
    start_timer();
    setup_adc();

    _enable_interrupts();

    ADC10CTL0 |= ENC | ADC10SC;
    for (;;) {
        _disable_interrupts();
        if (TAR < (TACCR0 >> 1)) {
            P1OUT |= (1 << 6);
        } else {
            P1OUT &= ~(1 << 6);
        }
        _enable_interrupts();
        //LPM0;
    }
}
