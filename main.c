#include <lcd8or4bitsece230winter23.h>
#include "msp.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>



#include "utilities.h"
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

    initStepperMotor();
    setStepFrequency(1);

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
            char key = convert_key_val(FoundKey);
//            lcd_putch(key);

            addKey(key);
        }
    }
}

char userCode[4] = {'1', '2', '3', '4'};
char enteredCode[4] = {NULL, NULL, NULL, NULL};
uint8_t numsPressed = 0;
uint8_t numDigits = 4;
void addKey(char k) {
    enteredCode[numsPressed] = k;

    if(numsPressed == numDigits) {
        if(arraysEqual(userCode, enteredCode, numDigits) == 1) {
            unlock();
            numsPressed = 0;
        }
    }

    int i;
    for(i = 0; i < numsPressed; i++) {
        lcd_putch(enteredCode[i]);
    }

    numsPressed++;
}
