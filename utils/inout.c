/*Definitions for all of the functions defined in inout.h ----- inout.c:

Only 5 functions are used for implementing all of these functions (fread, fwrite, fopen, fclose, exit)
If you are running your application on Apple device or any Unix operating system, it will instead include (read, write, open, close, exit)



*/


#ifndef _IO_C
#define _IO_C


#include "inout.h"


#ifdef _INOUT_PLATFORM_UNIX

#ifndef INCLUDE_UNISTD

ssize_t  write(int __fd, const void * __buf, int __nbyte);
ssize_t  read(int, void *, int);
int open(const char *pathname, int flags, unsigned int mode);
int close(int fd);
int lseek(int fildes, int offset, int whence);

#endif


#else

int fwrite (const void *, int, int, int);
int fread (void *, int, int, int);
int fopen(const char *file_name, const char *mode_of_operation);
int fclose(int stream);

#endif

#ifdef __NCURSES_H

int printw (const char *,...);

#endif

void _exit(int status);


int _builtin_string_len(char* str){
    int i;
    for (i=0; str[i] != '\0'; i++){};
    return i;
};



int print_chars(char* buffer){
    
    #ifdef __NCURSES_H
        printw(buffer);
    #else
        if(buffer == 0){
            #ifdef _INOUT_PLATFORM_UNIX
                return write(STDOUT, "(none)", _builtin_string_len("(none)"));
            #else
                return fwrite("(none)", 1, _builtin_string_len("(none)"), STDOUT);
            #endif
        };
        #ifdef _INOUT_PLATFORM_UNIX
            return write(STDOUT, buffer, _builtin_string_len(buffer));
        #else
            return fwrite(buffer, 1, _builtin_string_len(buffer), STDOUT);
        #endif
    #endif
}



int print_delims(char* delim, char* string, ...){
    __builtin_va_list ap;
    __builtin_va_start(ap, string);
    __builtin_va_list secList;
    __builtin_va_start(secList, string);
    char* v = string;
    int count = 0;
    while (1){
        if (count != 0){
            print_chars(delim);
        };

        print_chars(v);
        v = __builtin_va_arg(ap, char*);
        count++;
        if (v == 0){
            break;
        };
    };
    __builtin_va_end(ap);
    __builtin_va_end(secList);
    return 0;
}


int _print(char* string, ...){
    __builtin_va_list ap;
    __builtin_va_start(ap, string);
    #ifdef __NCURSES_H
        // vwprintw(0, string, ap);
    #else
        __builtin_va_list secList;
        __builtin_va_start(secList, string);
        while (1){
                print_chars(string);
            string = __builtin_va_arg(ap, char*);
            if (string==0){
                break;
            };
        };
        __builtin_va_end(ap);
        __builtin_va_end(secList);
        return 0;
    #endif
}

#define print(...) _print(__VA_ARGS__, 0)
#define println(...) _print(__VA_ARGS__, "\n", 0)

void exit(int status) {
    _exit(status);
}

int* input(int amountOfData){
    int data[amountOfData];
    #ifdef _INOUT_PLATFORM_UNIX
    read(STDIN, data, amountOfData);
    #else
    fread(data, 1, amountOfData, STDIN);
    #endif
    int *res = (int*)data;
    return res;
}




#define FILE_MODE_WRITE 0x05432

#define FILE_MODE_READ 0x042860

#define FILE_MODE_WRITEBINARY 0x05275

#define FILE_MODE_READBINARY 0x04275

#define FILE_MODE_EOF 0x02834

#define FILE_MODE_GOOD 0x02396

#define FILE_POINTER 0x0099





struct file file_open(char* file, char* type){
    struct file fl;
    fl.file = file;
    fl._pos_ = 0;
    int option = 2;
    if (type[0] == 'w'){
       option = 1 | 512 | 1024;
    }else if (type[0] == 'r'){
        option = 0;
    }else if(type[0] == 'a'){
        option = 1 | 512 | 8;
    };
    fl.flags = option; // Whether _INOUT_PLATFORM_UNIX is used or not, the flags will remain in the unix-used method for ease of understanding
    #ifdef _INOUT_PLATFORM_UNIX
        fl.fd = open(file, option, 436);
    #else
        fl.fd = fopen(file, type);
    #endif
    fl.fd_backup = fl.fd;
    return fl;
};

ssize_t file_write(struct file file, char* buffer){
    file._pos_ += _builtin_string_len(buffer);
    #ifdef _INOUT_PLATFORM_UNIX
        return write(file.fd, buffer, _builtin_string_len(buffer));
    #else
        return fwrite(buffer, 1, _builtin_string_len(buffer), file.fd);
    #endif
};

ssize_t file_writebinary(struct file file, void* buffer, int size){
    file._pos_ += sizeof(buffer);
    #ifdef _INOUT_PLATFORM_UNIX
    return write(file.fd, buffer, size);
    #else
    return fwrite(file.fd, &buffer, size);
    #endif
};


ssize_t file_read(struct file file, void* buffer, int size){
    file._pos_ += size;
    #ifdef _INOUT_PLATFORM_UNIX
        return read(file.fd, buffer, size);
    #else
        ssize_t size = fread(buffer, 1, size, file.fd);
        buffer[size-1] = '\0';
        return size;
    #endif
};

ssize_t cwrite(char* buffer){
    return file_write(FILE_STDOUT, buffer);
};

ssize_t cread(void* buffer, int size){
    return file_read(FILE_STDIN, buffer, size);
};


int file_wchar(struct file file, char c){
    file._pos_++;
    char res[2];
    res[0] = c;
    res[1] = '\0';
    #ifdef _INOUT_PLATFORM_UNIX
        return write(file.fd, res, 1);
    #else
        return fwrite(res, 1, 1, file.fd);
    #endif
    return -1; // Didn't work
};

int wchar(char ch){
    return file_wchar(FILE_STDOUT, ch);
}

int file_get_position(struct file file){
    return file._pos_;
};

int file_get_length(struct file file){
    if (file.fd_backup != file.fd){
        /*Write an error saying that the backup is not correct*/
        struct file file = {STDERR, STDERR, "stderr", 0, 1 | 512 | 1024};
        file_write(file, "\nfatal error: file backup is not the same as what the actual file\n");
        file_close(file);
        exit(1);
    };
    char data[2048]; // Max size
    int count = 0;
    int cur = 0;
    while ((cur = file_read(file, data, 1)) > 0){
        count += cur;
    };
    file._pos_ -= count;
    #ifdef _INOUT_PLATFORM_UNIX
    lseek(file.fd, 0, 0); // 0 = SEEK_SET
    #endif
    return count;
};


char file_rchar(struct file file){
    file._pos_++;
    char res[2];
    #ifdef _INOUT_PLATFORM_UNIX
        read(file.fd, res, sizeof(char));
    #else
        fread(res, sizeof(char), 1, file.fd);
    #endif
    return res[0];
};

char rchar(){
    return file_rchar(FILE_STDIN);
}

ssize_t file_eof(struct file file){
    return file.fd == -1;
};

ssize_t file_close(struct file file){
    #ifdef _INOUT_PLATFORM_UNIX
        close(file.fd);
    #else
        fclose(file.fd)
    #endif
    return 0;
};






#endif
