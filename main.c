#include <lcd8or4bitsece230winter23.h>
#include "msp.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "keypadscan_subroutines.h"


enum Status {NO, YES};
extern char NewKeyPressed;
extern char FoundKey;

/**
 * main.c
 */
void main(void)
{
    /* Stop Watchdog timer */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    configHFXT();
    ConfigureUART_A0();
    LCD_Initializtion();
    lcd_clear();
    lcd_SetLineNumber(FirstLine);

    lcd_putch('d');
    lcd_putch('e');
    lcd_putch('f');
    lcd_putch('g');
    lcd_putch('h');
    lcd_putch('i');

    printf("keyscan started: press a key on your 4x4 keypad ....\r\n");
    kepadconfiguration();

// Enable global interrupt
    __enable_irq();
//clear keypad output pins to be 0 to be ready for input interrupt
//Do not change input pin values
    KeypadPort->OUT = (KeypadPort->OUT & ~KeypadOutputPins)
            | (0b11110000 & KeypadOutputPins);
    while (1)
    {
        if (NewKeyPressed == YES)
        {
            NewKeyPressed = NO;
            lcd_clear();
            lcd_putch(0x30-1 + FoundKey);
        }
    }
}
