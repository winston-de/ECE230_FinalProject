#include <lcd8or4bitsece230winter23.h>
#include "msp.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "utilities.h"
#include "keypadscan_subroutines.h"

#define ACLK 32768 //Hz
#define BEEP_PERIOD       ACLK/8    //1 second

// Note A4 - 440 Hz, B4 - 493.88 Hz, C5 - 523.26 Hz
#define NOTEA4  27273
#define NOTEB4  24297
#define NOTEC5  22933

enum Status {NO, YES};
extern char NewKeyPressed;
extern char FoundKey;

int LIMIT = 3;
int x_count = 0;


/**
 * main.c
 */
void main(void)
{
    /* Stop Watchdog timer */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    configHFXT();
    configLFXT();
    ConfigureUART_A0();
    LCD_Initializtion();

    initStepperMotor();
    setStepFrequency(1);

    initSpeaker();
    initBluetooth();

    __enable_irq();

    lcd_clear();
    lcd_SetLineNumber(FirstLine);

   lcd_puts("Reset...");
   lcd_SetLineNumber(SecondLine);
   lcd_puts("Enter Command");
   lcd_SetLineNumber(FirstLine);

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

            int key_pressed = convert_key_val(FoundKey);
            keyPressed(FoundKey);

            switch (key_pressed) {

            case 'A':
                enterCode();
                break;

            case 'B':
                setCode();
                break;

            case 'D':
                restoreDefault();

            default:
                  // code to execute when none of the above cases match
                lcd_putch('X') ;
            }



        } // key pressed - if


    } // close while
} // close main




int code_length = 16;
char master_code[16] = {'#', '*'};         //{'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '*'};
char user_code[16] = {'1', '2', '3', '4','*' };
char entered_code[16];




void enterCode(void){

    //code_length = get_array_length(user_code);
    int i;

    for(i = 0; i < code_length; i++){
       entered_code[i] = NULL;
    }

//    for( i = 0; i < code_length; i++){
//
//        NewKeyPressed = NO;
//        while(NewKeyPressed != YES) {
//            debounce();
//
//            if(NewKeyPressed == YES){
//
//                int key = convert_key_val(FoundKey);
//                entered_code[i] = key;
//
//            }
//        }
//
//    }

    i = 0;
    while(1){

        NewKeyPressed = NO;
        int key;

        while(NewKeyPressed != YES) {
           debounce();

           if(NewKeyPressed == YES){
               key = convert_key_val(FoundKey);
               keyPressed(key);
               entered_code[i] = key;
               i++;
           }
        }

        if(key == '*'){break;}
    }

    if(arraysEqual(entered_code, user_code)){

//        for(i = 0; i < code_length; i++){
//            lcd_putch(entered_code[i]);
//            debounce();
//        }

        i = 0;
        while(entered_code[i] != '*'){
            lcd_putch(entered_code[i]);
            i++;
            debounce();

        }

        unlock();
        lcd_SetLineNumber(SecondLine);
        lcd_puts("Unlocked");
        NewKeyPressed = NO;
        return;

    } else {

//        for(i = 0; i < code_length; i++){
//            lcd_putch(entered_code[i]);
//            debounce();
//        }

        i = 0;
        while(entered_code[i] != '*'){
            lcd_putch(entered_code[i]);
            i++;
            debounce();

        }

        x_count ++;
        lcd_SetLineNumber(SecondLine);
        lcd_puts("Wrong -> #");
        lcd_putch((char) x_count);



        if(x_count == LIMIT){
            lcd_clear();
            lcd_puts(" --TIMED OUT-- ");


            // ADD ALERT TO BLUETOOTH MODULE HERE
        }

        NewKeyPressed = NO;
        return;

    } // end for if wrong code was entered


} // end of enterCode






void setCode(void){

        int i;
  //  user_code = (char*) malloc(code_length * sizeof(char));
  //  entered_code = (char*) malloc(code_length * sizeof(char));

        lcd_puts("Old code ->");


       for(i = 0; i < code_length; i++){
          entered_code[i] = NULL;
       }


       i = 0;
        while(1){

            NewKeyPressed = NO;
            int key;

            while(NewKeyPressed != YES) {
               debounce();

               if(NewKeyPressed == YES){
                   key = convert_key_val(FoundKey);
                   keyPressed(key);
                   entered_code[i] = key;
                   i++;
               }
            }

            if(key == '*'){break;}
        }

        if(arraysEqual(entered_code, user_code)){

               i = 0;
               while(entered_code[i] != '*'){
                   lcd_putch(entered_code[i]);
                   i++;
                   debounce();

               }

               lcd_SetLineNumber(SecondLine);
               lcd_puts("Authorized");
               NewKeyPressed = NO;
               for(i = 0; i < 5000000; i++); //arbitrary delay
               lcd_clear();


               lcd_SetLineNumber(FirstLine);

               for(i = 0; i < code_length; i++){
                   user_code[i] = NULL;
               }

                 i = 0;
                 while(1){

                     NewKeyPressed = NO;
                     int key;


                     while(NewKeyPressed != YES) {
                        debounce();

                        if(NewKeyPressed == YES){

                            key = convert_key_val(FoundKey);
                            user_code[i] = key;
                            i++;
                        }
                     }

                     if(key == '*'){break;}
                 }

             //        for( i = 0; i < code_length; i++){
             //
             //            NewKeyPressed = NO;
             //            while(NewKeyPressed != YES) {
             //                debounce();
             //
             //                if(NewKeyPressed == YES){
             //
             //                    int key = convert_key_val(FoundKey);
             //                    user_code[i] = key;
             //
             //
             //
             //                }
             //            }
             //
             //        }

             //        for(i = 0; i < code_length; i++){
             //                    lcd_putch(user_code[i]);
             //                    debounce();
             //
             //        }

                 i = 0;
                 while(user_code[i] != '*'){
                     lcd_putch(user_code[i]);
                     i++;
                     debounce();

                 }

                 lcd_SetLineNumber(SecondLine);
                 lcd_puts("Code Set");
                 NewKeyPressed = NO;
                 return;

           } else {


               lcd_clear();

               lcd_SetLineNumber(FirstLine);
               i = 0;
               while(entered_code[i] != '*'){
                   lcd_putch(entered_code[i]);
                   i++;
                   debounce();

               }

               lcd_SetLineNumber(SecondLine);
               lcd_puts("Wrong code");
               for(i = 0; i < 5000000; i++); //arbitrary delay
               return;


           } // end of else in case of failiure to change

} // end of setCode



void restoreDefault(void){
      int i;
      lcd_puts("Master Code ::");


      for(i = 0; i < code_length; i++){
         entered_code[i] = NULL;
      }


       i = 0;
       while(1){

           NewKeyPressed = NO;
           int key;

           while(NewKeyPressed != YES) {
              debounce();

              if(NewKeyPressed == YES){
                  key = convert_key_val(FoundKey);
                  keyPressed(key);
                  entered_code[i] = key;
                  i++;
              }
           }

           if(key == '*'){break;}
        }

           if(arraysEqual(entered_code, master_code)){


              lcd_SetLineNumber(SecondLine);
              lcd_puts("Reseting...");
              NewKeyPressed = NO;
              for(i = 0; i < 5000000; i++); //arbitrary delay
              lcd_clear();


              lcd_SetLineNumber(FirstLine);


              user_code[0] = '1';user_code[1] = '2';user_code[2] = '3';user_code[3] = '4';user_code[4] = '*'; //Better way?

              x_count = 0;

              } else {


                  lcd_clear();

                  lcd_SetLineNumber(FirstLine);
                  lcd_puts("Wrong code");
                  lcd_SetLineNumber(SecondLine);
                  lcd_puts(" --TIMED OUT-- ");
                  for(i = 0; i < 5000000; i++); //arbitrary delay
                  return;


              } // end of else in case of failiure to change

} // end of restoreDefault



// debounce for 5 ms
void debounce(void){
//delay time for debouncing switches
    int delay;
    for(delay = 0; delay < 150; delay++);
} //end debounce()

void keyPressed(char key) {
    //TODO: add different
    playNote(NOTEA4, BEEP_PERIOD);
    printMessage("please help me i am dying inside", 33);
}




//char key = convert_key_val(FoundKey);
////            lcd_putch(key);
//
//         addKey(key);





//char userCode[4] = {'1', '2', '3', '4'};
//char enteredCode[4] = {NULL, NULL, NULL, NULL};
//uint8_t numsPressed = 0;
//uint8_t numDigits = 4;
//void addKey(char k) {
//    enteredCode[numsPressed] = k;
//
//    if(numsPressed == numDigits) {
//        if(arraysEqual(userCode, enteredCode) == 1) {
//            unlock();
//            numsPressed = 0;
//        }
//    }
//
//    int i;
//    for(i = 0; i < numsPressed; i++) {
//        lcd_putch(enteredCode[i]);
//    }
//
//    numsPressed++;
//}






