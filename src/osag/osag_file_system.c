#include "../zenlib_core.h"

char* zen_read_file_contents(const char* file_path, size_t* file_size) {

    FILE *file = fopen(file_path, "rb"); 
    if (file == NULL) {
        printf(ERRORF "Failed to open file : %s\n", file_path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        printf(ERRORF "Failed to find end of file.\n");
        return NULL;
    }

    size_t r_file_size = ftell(file);
    rewind(file);

    char *buffer = malloc(sizeof(char) * r_file_size);
    if (buffer == NULL) {
        fclose(file);
        printf(ERRORF "Failed to allocate space of file contents.\n");
        return NULL;
    }

    *file_size = fread(buffer, 1, r_file_size, file);
    fclose(file);

    return buffer;


}