//#include <msp430g2553.h>
#include <msp430.h>
#include <signal.h>
#include <stdint.h>

enum Mode {
    INITIAL_WARM,
    MAINTAIN,
};

#define FIRST_TEMP 170
#define SECOND_TEMP 110
#define MAINTAIN_SECS (6 * 60 * 60)
#define TEMP_HYSTERESIS 1

static enum Mode mode;
static int target_temp;
static int current_temp;
static int heater_on;
static int maintain_time;
static int jiffies;

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
void timer(void)
{
    jiffies++;
    ADC10CTL0 |= ENC | ADC10SC;
}

static void set_heater (int on)
{
    heater_on = on;
    if (on)
        P1OUT |= (1 << 0);
    else
        P1OUT &= ~(1 << 0);
}

static int adc_to_temp(uint16_t val)
{
    return 260 - (val / 6);
}

static void set_led (void)
{
    if (heater_on)
        P1OUT |= (1 << 6);
    else {
        if (mode == MAINTAIN) {
            if (jiffies % 2 == 0)
                P1OUT ^= (1 << 6);
        }  else {
            P1OUT ^= (1 << 6);
        }
    }
}

void control (void)
{
    switch (mode) {
        case INITIAL_WARM:
            if (current_temp > target_temp) {
                target_temp = SECOND_TEMP;
                mode = MAINTAIN;
            }
            break;

        case MAINTAIN:
            if (maintain_time > MAINTAIN_SECS)
                target_temp = 0;
            else
                maintain_time++;
            break;
    }

    if (current_temp > target_temp + TEMP_HYSTERESIS)
        set_heater(0);
    if (current_temp < target_temp - TEMP_HYSTERESIS)
        set_heater(1);

    set_led();
}

__attribute__((interrupt(ADC10_VECTOR)))
void adc_irq(void)
{
    uint16_t adc_val = ADC10MEM;
    current_temp = adc_to_temp(adc_val);
    control();
}

static void setup_adc(void)
{
    ADC10CTL1 = INCH_5 | ADC10DIV_7;
    ADC10CTL0 = SREF_1 | REF2_5V | REFON | ADC10SHT_3 | ADC10ON | ADC10IE;
    ADC10AE0 = BIT5;
}

int main(void)
{
    int i;

    mode = INITIAL_WARM;
    target_temp = FIRST_TEMP;
    maintain_time = 0;
    jiffies = 0;
    set_heater(1);

    WDTCTL = WDTPW | WDTHOLD;

    P1DIR = (1 << 0) | (1 << 6);
    P1OUT = 0x00;

    clock_setup();
    start_timer();
    setup_adc();

    _enable_interrupts();

    for (;;) {
        LPM3;
    }
}
