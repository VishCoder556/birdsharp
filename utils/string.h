/*string.h - file for using strings


Including string.h is as easy as for any header file, use just the line #include "string.h"


What's defined:
    A string struct that helps manage strings
    A STRING function that creates a string from a char*
    A string_none that is an empty string
    Functions for using the string like a lexer
    Functions for removing whitespace
    Function for getting the index of things
    Function for removing first __ characters


    String struct:
        The current position of the string --- so that you can use the string for purposes like lexers
        The length of the string --- so it is easier to find the length instead of using any strlen or StringLen function
        The current character --- again to help for purposes like lexers
        The string --- Because without it, the String struct cannot represent a string at all!

    STRING NONE
        an empty string
        Used like this : STRING_NONE // Gets an empty char*

    string_trim_left:
        Removes all of the whitespace on the left of the string
        Used like this:
            char* string = "     Hi";
            string = string_trim_left(string);
    
    string_trim_right:
        Removes all of the whitespace on the right of the string
        Used like this:
            char* string = "Hi    ";
            string = string_trim_right(string);
    
    string_chop_start:
        Chops the start of the string until a specific index
        Used like this:
            char* string = "Hho";
            string = string_chop_start(string, 1);
    
    string_index_of:
        Gets the index of a specific character inside of a string.
        Used like this:
            char* string = "Hi";
            int c = string_index_of(string, 'c');
    
    
    string_len function:
        Get the length of the string.
        Returns an integer.
        Used like this:
            char* string = "John";
            int length = string_len(string);
            
    string_instances function:
        Get the amount of string_instances of a char in a char*.
        Used like this:
            int amount = string_instances("John", 'J')

    string_compare:
        Compare two strings for a given amount of characters.
            if (string_compare("Hello", "Hello Guys!", 5) == 0) return;

    string_char_to_string function:
        Convert a character into a string, return char*.
        Used like this:
            char* res = string_char_to_string('d');

    string_int_to_char function:
        Convert an integer into a character.
        Used like this:
            char c = string_int_to_char(5);

    string_int_to_string function:
        Convert an integer into a string.
        Used like this:
            char* res = string_int_to_string(25);

    string_dup function:
        Duplicates a string.
        Used like this:
            char *res = string_dup("Hello")


    string_concat function:
        Concatenates two strings.
        Used like this:
            char *res = string_concat("Hello ", "World");

    string_slice function:
        Gets a slice of a string from start to end index.
        Used like this:
            char *res = string_slice("Hello World", 0, 5);


            ------------------------------------------------------------------
        


        STRING_VERSION: 1.00


        Current INOUT_VERSION: 1.00
    
        Advantages:
            PORTABLE (Just uses two files)
            DOCUMENTED (Lots of documentation, including this right now)
            WRITTEN IN C (FAST)
        


        Credits:
            Tsoding Daily -- for the idea of creating my own string library
*/


#define STRING_VERSION 1.00 //     Current version   :   1.00 (First version 😁😁😁)

#ifndef __STRING
#define __STRING
#endif


#define STRING_NONE "\0"  // Empty string

#ifndef SVDEF
#define SVDEF
#endif



// Trims all the whitespace at the left of the string
char* string_trim_left(char*);

// Trim all whitespace at the end of the string
char* string_trim_right(char*);

// Get the index of a character in a string
int string_index_of(char*, char);

// Remove the characters from a string until a specific point
char* string_chop_start(char*, int);


int string_len(char*); // Get the length of a string
        
int string_instances(char*, char); // Get the string_instances of a char in a char*

int string_compare(char *a, char *b, int len); // Compare two strings for a specific number of characters

char* string_char_to_string(char); // Convert a character into a string

char string_int_to_char(int);  // Convert a digit to a single character

char* string_int_to_string(int);  // Convert a number to a char*

char* string_dup(char* str); // Duplicates a string

char* string_concat(char *str1, char *str2); // Concatenates two strings

char* string_slice(char *str, int start, int end); // Gets a slice of a string from start to end index

// #include "array/array.h"

// ;
