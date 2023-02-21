/*
 * utilities.c
 *
 *  Created on: Feb 14, 2023
 *      Author: Simarjit Dhillon
 *
 *
 *      The purpose of this file is to contain miscellaneous helper functions for the Keypad and LCD
 *
 *
 */

#include "utilities.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


/*
 * Convert a key number to it's ASCII character (ex, 4-> 'A')
 */
char convert_key_val (char FoundKey) {
    switch (FoundKey) {
      case 1:
          return '1' ; // return ASCII for '1'
        break;
      case 2:
          return '2' ;// return ASCII for '2'
        break;
      case 3:
          return '3' ; // return ASCII for '3'
        break;
      case 4:
          return 'A' ;// return ASCII for 'A'
        break;
      case 5:
          return '4' ; // return ASCII for '4'
        break;
      case 6:
          return '5' ; // return ASCII for '5'
        break;
      case 7:
          return '6' ; // return ASCII for '6'
        break;
      case 8:
          return 'B' ; // return ASCII for 'B'
        break;
      case 9:
          return '7' ; // return ASCII for '7'
        break;
      case 10:
          return '8' ; // return ASCII for '8'
        break;
      case 11:
          return '9' ; // return ASCII for '9'
        break;
      case 12:
          return 'C' ; // return ASCII for 'C'
        break;
      case 13:
          return '*' ; // return ASCII for '*'
        break;
      case 14:
          return '0' ; // return ASCII for '0'
        break;
      case 15:
          return '#' ; // return ASCII for '#'
        break;
      case 16:
          return 'D' ; // return ASCII for 'D'
        break;
      default:
        // code to execute when none of the above cases match
          return 0x31 ; // return ASCII for '1'
    }

}

/**
 * Return true if 2 character arrays are equal. Used for checking if the password is correct
 */
bool arraysEqual(char arr1[], char arr2[]) {
    int i;
    i = 0;
    // '*' marks the end of the password, so only iterate up to the *
    while((arr1[i] != '*') || (arr2[i] != '*')){

        if(arr1[i] != arr2[i]) {
            return false;
        }

        i++;
    }

    return true;
}

/**
 * Returns the length of an array
 */
int get_array_length(int arr[]) {
  int length = 0;
  while (arr[length] != '\0') {
    length++;
  }
  return length;
}





