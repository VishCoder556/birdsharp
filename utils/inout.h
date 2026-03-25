/*inout.h - Input and output system

Use of inout:
    #include "inout.h"


What's defined:
    Basic STDOUT, STDIN, STDERR variables that are given a constant value   
    A couple functions that operate with any char* you have, String_Len, index_of, instances, numberToString
    Basic output with char* and strings
    A function for inputting text from the user
    Opening, writing, reading, and closing files
    
    Seperate approach to STDIN, STDOUT, STDERR and their functions if on Unix


    String_Len function:
        If you want to use the String_Len function, you can simply specify the string and it's length will be given. It's use case is like this:
            String_Len("Hello World")
        It is similar to the strlen function that is defined in string.h but in this implementation
    print_chars function:
        This is a function that is meant to be similar to a built-in function. I will not reccomend using it at all times.
        A better approach to it is the print function that can also print multiple strings, allowing a better ease of use.
    index_of function:
        This takes in a specific character and a string and finds the index of the character within the string. Similar to the C strchr function.
        You can use it like index_of (char*, char)
    instances function:
        This function finds the amount of instances of a character that are in a string.
        You call it with a char* and the char and it will return the integer
    char_to_string function:
        This function converts a single character into a string.
        You call it with just the char that will be converted into a char*

    int_to_char function:
        This converts a one-digit number into a char.
        You use it by taking in the integer and it will return char
    int_to_string function:
        This converts a 10-base digit into a char*.
        You use it by taking in an int and it returns a char*.
    print_delims function:
        Prints in a series of strings. Similar to the print function but with a delimiter
        You use it by calling a char* delimeter, char* firstString, and  you can add how many more char*s you want as long as it ends with 0
    print function:
        Similar to the print_delims function but this time there is no delimeter and all elements are placed directly next to each other.
        You use it by calling a char* firstString, and as many char*s as you want as long as it ends with a 0.
    input function:
        Inputs a string from the user.
        You call it by specifying merely the amount of characters you want to take in and it returns what the user entered.        int* input(int amountOfData)
    rchar function:
        Reads a character from console.
        Similar to file_rchar but to the console
    wchar function:
        Writes a single character to the console.
        Similar to file_wchar but to the console
    cwrite function:
        Write data out into the terminal.
        Similar to file_write but to the console
    cread function:
        Read data from the terminal.
        Similar to the file_read function but to the console

    file_open function:
        Opens a file. Seperately implemented in MacOS
        You call it by giving the file and the mode for it.
    
    file_write function:
        Writes text into a file.
        Needs the file from the file_open function and the text to write.
    
    file_wchar function:
        Writes a single character into a file.
        Needs the character and the file

    file_writebinary function:
        Writes binary data into a file.
        Needs the file from the file_open function, the binary data to write into it, and the size of the binary data.
     
    file_read function:
        Reads any data from a file.
        Takes in the file that is sent from the file_open function, the data to store the read text in, and the size of it
    
    
    file_rchar function:
        Reads a single character from a file and returns it.
        Needs the file

    file_get_length function:
        Gets the length of an entire file.
        Requires just the file
    

    -------------------------------------------------------

    Current INOUT_VERSION: 1.01
    
    Advantages:
        PORTABLE (Just uses two files)
        DOCUMENTED (Lots of documentation, including this right now)
        WRITTEN IN C (FAST)
    

    Preprocessed statements:
        _INOUT_PLATFORM_WIN32 : specifies if the platform is Windows
        _INOUT_PLATFORM_UNIX : specicfies if the platform is Linux, Unix, or MacOS
        STDOUT : location for the stdout (1 if _INOUT_PLATFORM_UNIX, and or else attributed to a specific value)
        STDIN : location for the stdout (0 if _INOUT_PLATFORM_UNIX, and or else attributed to a specific value)
        STDERR : location for the stdout (2 if _INOUT_PLATFORM_UNIX, and or else attributed to a specific value)
        INOUT_VERSION : the version of the inout library
    

    

*/ 


#define INOUT_VERSION 1.01 // The specific version of the inout system (First version  😁😁😁)


#ifndef _IO_H
#define _IO_H


/*
Updated versions:

1.01 -- Add FILE_STDOUT, FILE_STDIN, FILE_STDERR





Usages:

Print "Hello World" using the print function:
    print("Hello World", 0);
Expected response:
    Hello World

Print "Hello World" using print_delims function:
    print_delims(" ", "Hello", "World", 0);
Expected response:
    Hello World

Write "Hello World" into a file:
    struct file file = file_open("result.txt", "w");
    file_write(fd, "Hello World");
    file_close(fd);
Expected response:
    result.txt:
        Hello World

Append "Hello World" to a file:
    struct file file fd = file_open("result.txt", "a");
    file_write(fd, "Hello World");
    file_close(fd);
Expected response:
    result.txt:
        Hello World
    (Preceded with a bunch of text before it)

Read a file:
    struct file fd = file_open("result.txt", "r");
    char result[1024];
    file_read(fd, result, sizeof(result));
    file_close(fd);
Expected result (whatever is in the file)

Read a single character from a file:
    struct file file = file_open("result.txt", "r");
    char c = file_rchar(file);
    file_close(file);
Expected result (one character from the file)

Write a single character into a file:
    struct file file = file_open("result.txt", "w");
    file_wchar(file, 'h');
    file_close(file);
Expected result:
    result.txt:
        h



Write data into a file and then access it(Problem is that the file permissions will not work meaning that you will have to run once, "chmod 644 output.txt", and then run again):
    // Writing data to a file
    struct file file1 = file_open("output.txt", "w");
    file_write(file1, "Hello, ");
    file_wchar(file1, 'W');
    file_wchar(file1, 'o');
    file_wchar(file1, 'r');
    file_wchar(file1, 'l');
    file_wchar(file1, 'd');
    file_write(file1, "!");
    file_close(file1);

    // Reading data from the file
    struct file file2 = file_open("output.txt", "r");
    char contents[128];
    file_read(file2, contents, sizeof(contents));
    file_close(file2);

    // String manipulation and output
    print("Contents of the file: ");
    print(contents);
    print("\n");
Expected result:
    Contents of the file: Hello World

Converting a number to a string:
    int num = 42;
    char* numStr = int_to_string(num);
    print("The answer to life, the universe, and everything is: ");
    print(numStr);
    print("\n");
Expected result:
    The answer to life, the universe, and everything is: 42

Finding the number of instances of a character in a string:
    char* text = "This is a text with multiple instances of 'i'.";
    int count = instances(text, 'i');
    print("Number of 'i' instances in the text: ");
    print_chars(int_to_string(count));
    print("\n");
Expected result:
    Number of 'i' instances in the text: 6


All of these features work in their Unix syscalls and their high-level stdio.h versions.


Different pragma regions:
    Platform_finder:
              Platform name      Platform meaning        Starting value
              _INOUT_PLATFORM_WIN32       Windows system                 0
              _INOUT_PLATFORM_UNIX         MacOS/Apple/Unix              0

            ----------------------------------------------------------

            Buffer    Unix Value    Otherwise value
            STDOUT      1             1966773792
            STDIN       0             1966773760
            STDERR      2             1966773824
        
    handling_chars region:
        String_Len, index_of, instances, char_to_string (functions for working with char* values)
    number region:
        int_to_char, int_to_string (functions for converting numbers)
    inout region:
        print_chars, print_delims, print, input, exit, rchar, wchar, cwrite, cread (writing, reading, and exiting)
    files region:
        file_open, file_write, file_wchar, file_writebinary, file_read, file_rchar, file_close
*/


#pragma region platform_finder


    #define _INOUT_PLATFORM_WIN32 0
    #define _INOUT_PLATFORM_UNIX 0

    // Defines both modes off

    #ifdef _WIN32

        #undef _INOUT_PLATFORM_WIN32
        #define _INOUT_PLATFORM_WIN32 1

    #endif
    #if defined(TARGET_OS_MAC) || defined(__APPLE__) || defined(__unix__)

        #undef _INOUT_PLATFORM_UNIX
        #define _INOUT_PLATFORM_UNIX 1
    
    #endif

    // Sets one of the values to 1 if it is being used

    #ifdef _INOUT_PLATFORM_UNIX

        #define STDOUT 1
        #define STDIN 0
        #define STDERR 2
    
    #else

        #define STDOUT 1966773792
        #define STDIN 1966773760
        #define STDERR 1966773824
    
    #endif

    // Defines STDOUT, STDIN, STDERR based on the system itself


    #define FILE_STDOUT (struct file){STDOUT, STDOUT, "stdout", 0, 1 | 512 | 8}
    #define FILE_STDIN (struct file){STDIN, STDIN, "stdin", 0, 0}
    #define FILE_STDERR (struct file){STDERR, STDERR, "stderr", 0, 1 | 512 | 8}

    // Define FILE_STDOUT, FILE_STDIN, FILE_STDERR as files that represent their streams

#pragma endregion

#ifndef _STDIO_H_
    #ifndef NO_SSIZE_T // NO_SSIZE_T tells whether ssize_t type is already defined or not to avoid multiple definitions
        typedef long long ssize_t;  // Creates the ssize_t type
    #endif
#endif


#ifndef _NO_INOUT_FUNC // _NO_INOUT_FUNC tells whether you want input-output functions or not
    #pragma region inout

        int print_chars(char*); // Print characters to the console

        int print_delims(char*, char*, ...);  // Print char*s with delimeter

        int print(char*, ...);  // Print char*s
        
        int* input(int); // Get input from the user

        char rchar(); // Read a single character from the console

        int wchar(char); // Write a single character to the console

        
        ssize_t cwrite(char*); // Write a string into the console

        ssize_t cread(void*, int); // Read data from the console

        void exit(int status);

    #pragma endregion
#endif

#ifndef _NO_FILE_FUNC // _NO_FILE_FUNC tells whether you want file functions or not
    #pragma region files


        struct file{
            int fd;
            int fd_backup;
            char* file;
            int _pos_;
            int flags;
        };

        // Open a file from the file and mode
        struct file file_open(char*, char*);

        // Write char* data into a file
        ssize_t file_write(struct file, char*);

        // Write just a single character into a file
        int file_wchar(struct file, char);

        // Write binary data into a file
        ssize_t file_writebinary(struct file, void*, int);

        // Read content from a file
        ssize_t file_read(struct file, void*, int);

        //Read a single character from file
        char file_rchar(struct file);

        // Close the file
        ssize_t file_close(struct file);

        // Get the position of the file
        int file_get_position(struct file);

        // Get the length of the file
        int file_get_length(struct file);

    #pragma endregion
#endif

#endif
