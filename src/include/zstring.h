/* zstring.h - v1.3.1 - MIT License - https://github.com/zb1ndev/zstring.h 

    MIT License
    Copyright (c) 2025 Joel Zbinden

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    
    Version 1.3.0 Change-Log :
    - Re-wrote base functionality
    - Re-wrote most functions 
    - Verfied Memory Safety

    Version 1.3.1 Change-Log :
    - Added string_to_bytes

*/

#if !defined(Z_STRING_H)
#define  Z_STRING_H

    #include <stdio.h>      // printf @ string_from_format | error checking

    #include <stdlib.h>     // realloc
    #include <string.h>     // memcpy
    #include <stdarg.h>     // va_args
    #include <sys/types.h>  // ssize_t

    typedef struct String {
        
        char* content;
        size_t length;
        size_t capacity;

    } String;

    #define Z_STRING_NULL (String) { NULL, 0, 0 }
    #define Z_STRING_FLOATING_ACCURACY 2

    /** A function that frees the contents of a string.
     * @param ptr The String you want to free.
     */
    void string_drop(String* ptr);

    /** A function that appends a value to a string.
     * @param ptr The String you to append to.
     * @param src The value you want to append.
     * @returns Whether the function has succeded ```0 = success```.
     */
    int string_append(String* ptr, char* src);

    /** A function that appends a character to a string.
     * @param ptr The String you to append to.
     * @param src The character you want to append.
     * @returns Whether the function has succeded ```0 = success```.
     */
    int string_append_c(String* ptr, char src);

    /** A function that initializes a String structure with a value.
     * @param src The value you want the result to be initialized with.
     * @returns A String with the provided value as it's content.
     */
    String string_from(char* src);

    /** A function that creates a String from ```ptr``` with a length of ```index```.
     * @param ptr The String you want to pull from.
     * @param size The number of characters you want to copy.
     * @returns A String with the length of ```size``` containing ```ptr```.
     */
    String string_from_until(char* ptr, size_t size);

    /** A function that initializes a String structure from a format.
     * @param format The format in which you want your String to be formed.
     * @param __VA_ARGS__ The items specifified by the specifiers in ```format```.
     * @returns A String with the provided format as it's content.
     * @note Implemented specifiers : ```%%```, ```%s```, ```%d```, ```%u```, ```%f```.
     */
    String string_from_format(const char* format, ...);

    /** A function that appends a format to a String.
     * @param ptr The String you want to append the formated string to.
     * @param format The format in which you want your String to be formed.
     * @param __VA_ARGS__ The items specifified by the specifiers in ```format```.
     * @note Implemented specifiers : ```%%```, ```%s```, ```%d```, ```%u```, ```%f```.
     */
    #define string_append_format(ptr, format, ...) string_append(ptr, string_from_format(format, __VA_ARGS__).content)

     /** A function that splits the String in two at ```index```. Puts left half in ```return``` and right half in ```ptr```.
     * @param ptr The String you want to split.
     * @param index The index you want to split at.
     * @returns The left half of the string.
     */
    String string_split(String* ptr, size_t index);

    /** A function inserts ```src``` into ```ptr``` at ```index```.
     * @param ptr The String you want to insert into.
     * @param src The String you want to insert.
     * @param index The index you want to insert to.
     * @returns Whether the function has succeded ```0 = success```.
     */
    void string_insert(String* ptr, char* src, size_t index);

    /** A function that removes the spaces from a string.
     * @param ptr The String you want to trim.
     */
    void string_trim(String* ptr);

    /** A function that flips the String around.
     * @param ptr The String you want to flip.
     */
    void string_flip(String* ptr);

    /** A function that turns an integer into a string.
     * @param number The number you want to turn into a string.
     * @returns ```number``` as a string.
     */
    String int_to_string(ssize_t number);

    /** A function that turns an unsigned integer into a string.
     * @param number The number you want to turn into a string.
     * @returns ```number``` as a string.
     */
    String uint_to_string(size_t number);

    /** A function that turns a float into a string.
     * @param number The number you want to turn into a string.
     * @returns ```number``` as a string.
     */
    String float_to_string(long double number);

    /** A function that turns a string into an array of bytes.
     * @param ptr The string you want to turn into bytes.
     * @returns The string as bytes.
     */
    unsigned char* string_to_bytes(String* ptr);

#if defined(Z_STRING_IMPLEMENTATION)

    void string_drop(String* ptr) {

        if (ptr->content)
            free(ptr->content);
        *ptr = Z_STRING_NULL;
    
    }

    int string_append(String* ptr, char* src) {

        // Given an invalid string
        if (src == NULL || ptr == NULL) return -1;
        
        // Given an empty string
        size_t src_length = strlen(src);
        if (src_length <= 0) return -1;

        // Allocate Memory if needed
        size_t new_length = (ptr->length + src_length);
        if (new_length >= ptr->capacity) {

            ptr->capacity += (src_length * 2);
            char* temp = (char*)realloc(ptr->content, ptr->capacity); 
            if (temp == NULL) return -1;
            ptr->content = temp;
       
        }

        // Append Content to String
        memcpy(ptr->content + ptr->length, src, src_length);
        
        // Update Length and Null-Terminate the String
        ptr->length = new_length;
        ptr->content[ptr->length] = '\0';

        return 0;

    }

    int string_append_c(String* ptr, char src) {

        // Given an invalid string
        if (ptr == NULL) return -1;

        // Allocate Memory if needed
        size_t new_length = (ptr->length + 1);
        if (new_length >= ptr->capacity) {

            ptr->capacity += (ptr->length * 2);
            char* temp = (char*)realloc(ptr->content, ptr->capacity); 
            if (temp == NULL) return -1;
            ptr->content = temp;
       
        }

        // Append the Character, Update the Length, and Null-Terminate the String
        ptr->content[ptr->length] = src;
        ptr->content[++ptr->length] = '\0';

        return 0;

    }

    String string_from(char* src) {

        String return_value = Z_STRING_NULL;
        if (string_append(&return_value, src) == 0)
            return return_value;

        // "string_append" failure
        return Z_STRING_NULL;

    }

    String string_from_until(char* ptr, size_t size) {

        // Given invalid source or size
        if (ptr == NULL || size <= 0) { 
            return Z_STRING_NULL;
        }

        // Given size greater than or equal to length of source
        if (strlen(ptr) <= size) {
            return string_from(ptr);
        }

        char* buffer = (char*)malloc(size + 1);
        if (buffer == NULL) return Z_STRING_NULL;

        memcpy(buffer, ptr, size);
        buffer[size] = '\0';

        String result = string_from(buffer);
        free(buffer);
        return result;

    }

    // TODO : Implement all "printf" specifiers plus one that converts numbers to binary
    String string_from_format(const char* format, ...) {

        va_list args; 
        va_start(args, format); 
        String return_value = Z_STRING_NULL;

        for (size_t c = 0; format[c] != '\0'; c++) {

            if (format[c] == '%') {
                if (format[c+1] == '%') {
                    string_append_c(&return_value, '%');
                    c += 1;
                } else {

                    switch (format[++c]) {

                        case 's':
                            string_append(&return_value, va_arg(args, char*));
                            break;
                        case 'd':
                            string_append(&return_value, int_to_string(va_arg(args, int)).content);
                            break;
                        case 'u':
                            string_append(&return_value, uint_to_string(va_arg(args, unsigned int)).content);
                            break;
                        case 'f':
                            string_append(&return_value, float_to_string(va_arg(args, double)).content);
                            break;
                        default:
                            printf("[ERROR] \"%c\" is not a valid specifier.\n", format[c]);
                            break;
                    }
                }
            } else {
                string_append_c(&return_value, format[c]);
            }    
        }

        va_end(args);
        return return_value;

    }

    String string_split(String* ptr, size_t index) {

        // Given invalid index or input string
        if (index <= 0 || ptr == NULL) return Z_STRING_NULL;

        // Given valid index
        if (index <= ptr->length) {

            String ptr_value = string_from(ptr->content + index + 1);
            String return_value = string_from_until(ptr->content, index + 1); 
            
            *ptr = ptr_value;
            return return_value;

        }

        // Given a index greater that the string's length
        return Z_STRING_NULL;

    }

    void string_insert(String* ptr, char* src, size_t index) {

        // Given index, source, or target is invalid
        if (index > ptr->length || ptr == NULL || src == NULL) return;

        String left = string_split(ptr, index);
        String result = string_from_format("%s%s%s", left.content, src, ptr->content);
        *ptr = result;

    }

    void string_trim(String* ptr) {

        char* buffer = (char*)malloc(ptr->length);
        if (buffer == NULL) return; // Make sure allocation has succeeded

        size_t l = 0;
        for (size_t c = 0; c < ptr->length; c++)
            if (ptr->content[c] != ' ') 
                buffer[l++] = ptr->content[c];
        
        buffer[l] = '\0';
        *ptr = string_from(buffer);
        free(buffer); 

    }

    void string_flip(String* ptr) {

        for (int j = 0, k = ptr->length - 1; j < k; j++, k--) {
            char temp = ptr->content[j];
            ptr->content[j] = ptr->content[k];
            ptr->content[k] = temp;
        }

    }

    String int_to_string(ssize_t number) {

        String return_value = Z_STRING_NULL;
        ssize_t sign = number;

        if (number < 0 ) number = -number;
        while (number > 0) {
            string_append_c(&return_value, number%10+'0');
            number/=10;
        }

        if (sign < 0)
            string_append_c(&return_value, '-');
        string_flip(&return_value);
        
        return return_value;

    }

    String uint_to_string(size_t number) {

        String return_value = Z_STRING_NULL;
        while (number > 0) {
            string_append_c(&return_value, number%10+'0');
            number/=10;
        }
        string_flip(&return_value);
        
        return return_value;

    }

    String float_to_string(long double number) {

        String result = Z_STRING_NULL;
        if (number< 0) {
            number = -number;
            string_append_c(&result, '-');  
        }

        int integer_part = (int)number;
        float fractional_part = number - integer_part;
        string_append(&result, int_to_string(integer_part).content);
        string_append_c(&result, '.');

        int count = 0;
        while (count < Z_STRING_FLOATING_ACCURACY) {
            fractional_part *= 10;
            int digit = (int)fractional_part;
            string_append_c(&result, digit + '0');
            fractional_part -= digit;
            count++;
        }

        return result;
                    
    }

    unsigned char* string_to_bytes(String* ptr) {

        unsigned char* result = (unsigned char*)malloc(ptr->length * sizeof(unsigned char));
        if (result == NULL)
            return NULL;
        memcpy(result, ptr->content, ptr->length);
        return result;

    }

#endif // Z_STRING_IMPLEMENTATION
#endif // Z_STRING_H