#include "string.h"
#ifndef __STRING
#define __STRING
#endif


char* string_trim_left(char* string){
    if (string == NULL) return NULL;
    for (int i=0; string[i] == ' ' || string[i] == '\n' || string[i] == '\t'; i++){
        string++;
    };
    return string_dup(string);
}



char* string_trim_right(char* string){
    if (string == NULL) return NULL;
    int i = string_len(string)-1;
    for (; string[i] == ' ' || string[i] == '\n' || string[i] == '\t'; i--){
        ;
    };
    string[i] = '\0';
    return string_dup(string);
}

int string_index_of(char* str, char c){
    for (int i=0; i<string_len(str); i++){
        if (str[i] == c){
            return i;
        };
    };
    return -1;
}



int string_len(char* str){
    int i;
    for (i=0; str[i] != '\0'; i++){};
    return i;
};

int string_instances(char* str, char c){
    int count = 0;
    for (int i=0; i<string_len(str); i++){
        if (str[i] == c){
            count++;
        };
    };
    return count;
}

int string_compare(char *a, char *b, int len){
    if (a == NULL) return -1;
    if (b == NULL) return -1;
    for (int i=0; i < len; i++){
        if (a[i] != b[i]){
            return 1;
        };
    };
    return 0;
};


char string_int_to_char(int c){
    return (char)48+c;
};

char* string_int_to_string(int num) {
    static char str[12]; // Max length of 32-bit integer in base 10 is 11 characters + 1 for '\0'
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
    } else {
        // Handle negative numbers
        if (num < 0) {
            isNegative = 1;
            num = -num;
        }

        // Process individual digits
        while (num != 0) {
            str[i++] = num % 10 + '0';
            num /= 10;
        }

        // Append negative sign
        if (isNegative) {
            str[i++] = '-';
        }
    }

    str[i] = '\0'; // 0-terminate the string

    // Reverse the string (optional)
    for (int j = 0, end = i - 1; j < end; j++, end--) {
        char temp = str[j];
        str[j] = str[end];
        str[end] = temp;
    }

    return str;
}



char* string_char_to_string(char w){
    char res[2];
    res[0] = w;
    res[1] = '\0';
    char *res2 = (char*)res;
    return res2;
};



char* string_dup(char* str) {
    char* copy = (char*)malloc((string_len((char*)str) + 1) * sizeof(char));
    if (copy == NULL) {
        return NULL;
    }
    for (int i = 0; str[i] != '\0'; i++) {
        copy[i] = str[i];
    }
    copy[string_len((char*)str)] = '\0';
    return copy;
}



char* string_chop_start(char* string, int mov){
    string += mov;
    return string;
}

char* string_concat(char *str1, char *str2) {
    if (!str1) return string_dup(str2 ? str2 : "");
    if (!str2) return string_dup(str1);
    
    int len1 = string_len(str1);
    int len2 = string_len(str2);
    char* result = malloc(len1 + len2 + 1);
    
    if (result) {
        for (int i = 0; i < len1; i++) result[i] = str1[i];
        for (int i = 0; i < len2; i++) result[len1 + i] = str2[i];
        result[len1 + len2] = '\0';
    }
    return result;
}

char* string_slice(char *str, int start, int end) {
    if (!str || start < 0 || end < start) return string_dup("");
    
    int len = string_len(str);
    if (start > len) return string_dup("");
    if (end > len) end = len;
    
    int slice_len = end - start;
    char *result = malloc(slice_len + 1);
    
    if (result) {
        for (int i = 0; i < slice_len; i++) result[i] = str[start + i];
        result[slice_len] = '\0';
    }
    return result;
}
