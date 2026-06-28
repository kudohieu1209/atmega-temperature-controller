/*
 * Temperature Controller - ATmega16 + LM35 + CodeVisionAVR
 * --------------------------------------------------
 * Doc nhiet do qua ADC (cam bien LM35 tren ADC0, Vref 2.56V -> 1 LSB = 1 do C),
 * hien thi nhiet do thuc te + nhiet do cai dat tren LCD 16x2.
 * Dieu khien QUAT (FAN) va DEN (LIGHT) qua relay theo nguong cai dat.
 * Nhiet do cai dat luu trong EEPROM, chinh bang nut UP/DOWN/RESET.
 *
 * Chan ket noi:
 *   PB0 - nut UP    (tang nhiet do cai dat)
 *   PB1 - nut DOWN  (giam nhiet do cai dat)
 *   PB2 - relay FAN  (quat)
 *   PB3 - relay LIGHT (den)
 *   PB4 - nut RESET (dua cai dat ve 25)
 *   PA0 - dau vao ADC (chan OUT cua LM35)
 *   PORTC/PORTD - LCD 16x2 (theo cau hinh CodeWizard)
 */

#include <io.h>
#include <alcd.h>
#include <delay.h>

// Dinh nghia chan nut bam va relay
#define UP        PINB.0
#define DW        PINB.1
#define RESET_BTN PINB.4   // Nut RESET dau vao PB4
#define FAN       PORTB.2
#define LIGHT     PORTB.3

// VREF noi bo 2.56V + ADLAR=1 (8-bit)
#define ADC_VREF_TYPE  ((1<<REFS1) | (1<<REFS0) | (1<<ADLAR))

// Hang so cau hinh
#define TEMP_DEFAULT   25   // Nhiet do cai dat mac dinh
#define TEMP_MAX       99   // Gioi han tren nhiet do cai dat
#define EEPROM_ADDR_M   0   // Dia chi EEPROM luu nhiet do cai dat
#define DEBOUNCE_MS    20   // Thoi gian chong doi nut bam

unsigned char temp = 0;
unsigned char m = TEMP_DEFAULT;   // Nhiet do cai dat hien tai

// EEPROM
void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
    while (EECR & (1<<EEWE));
    EEAR = uiAddress;
    EEDR = ucData;
    EECR |= (1<<EEMWE);
    EECR |= (1<<EEWE);
    while (EECR & (1<<EEWE));
}

unsigned char EEPROM_read(unsigned int uiAddress)
{
    while (EECR & (1<<EEWE));
    EEAR = uiAddress;
    EECR |= (1<<EERE);
    return EEDR;
}

// ADC
unsigned char read_adc(unsigned char adc_input)
{
    ADMUX = adc_input | ADC_VREF_TYPE;
    delay_us(10);
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADIF)) == 0);
    ADCSRA |= (1<<ADIF);
    return ADCH;
}

void ADC_init(void)
{
    ADMUX = ADC_VREF_TYPE;
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
}

// Nut bam
void button(void)
{
    // Nut UP (tang nhiet do)
    if (UP == 0)
    {
        delay_ms(DEBOUNCE_MS);
        if (UP == 0)
        {
            m++;
            if (m > TEMP_MAX) m = 0;
            EEPROM_write(EEPROM_ADDR_M, m);
            while (UP == 0);
        }
    }

    // Nut DOWN (giam nhiet do)
    if (DW == 0)
    {
        delay_ms(DEBOUNCE_MS);
        if (DW == 0)
        {
            if (m == 0) m = TEMP_MAX;
            else m--;
            EEPROM_write(EEPROM_ADDR_M, m);
            while (DW == 0);
        }
    }

    // Nut RESET (dua nhiet do cai dat ve mac dinh)
    if (RESET_BTN == 0)
    {
        delay_ms(DEBOUNCE_MS);
        if (RESET_BTN == 0)
        {
            m = TEMP_DEFAULT;
            EEPROM_write(EEPROM_ADDR_M, m);
            while (RESET_BTN == 0);
        }
    }
}

// Dieu khien relay
void out_relay(void)
{
    if (temp > m)
    {
        FAN = 1;      // Bat quat
        LIGHT = 0;    // Tat den
    }
    else
    {
        FAN = 0;      // Tat quat
        LIGHT = 1;    // Bat den
    }
}

void main(void)
{
    // Khoi tao LCD 16x2
    lcd_init(16);

    // Khoi tao ADC
    ADC_init();

    // Cau hinh PORT
    DDRA = 0x00;                // PORTA input cho ADC
    DDRB = 0xFC;                // PB2, PB3 output; PB0, PB1, PB4 input
    PORTB = 0x13;               // Pull-up noi cho PB0, PB1, PB4 (0b00010011)
    DDRD = 0xFF;

    // Doc nhiet do cai dat tu EEPROM
    m = EEPROM_read(EEPROM_ADDR_M);
    if (m > TEMP_MAX) m = TEMP_DEFAULT;

    while (1)
    {
        temp = read_adc(0);

        // Hien thi nhiet do thuc te
        lcd_gotoxy(0, 0);
        lcd_puts("NHIET DO: ");
        lcd_gotoxy(10, 0);
        lcd_putchar(0x30 + temp/10);
        lcd_putchar(0x30 + temp%10);
        lcd_gotoxy(13, 0);
        lcd_puts("\xDF" "C ");

        // Hien thi nhiet do cai dat
        lcd_gotoxy(0, 1);
        lcd_puts("CAI DAT : ");
        lcd_gotoxy(10, 1);
        lcd_putchar(0x30 + m/10);
        lcd_putchar(0x30 + m%10);
        lcd_gotoxy(13, 1);
        lcd_puts("\xDF" "C ");

        out_relay();
        button();

        delay_ms(150);
    }
}
