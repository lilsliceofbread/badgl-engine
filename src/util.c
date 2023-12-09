#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* get_file_data(const char* filepath)
{
    FILE* file;
    file = fopen(filepath, "rb"); // windows will add carriage returns to \n in ftell count unless using binary read
    if(file == NULL || fseek(file, 0, SEEK_END)) // file not open or fseek failed
    {
        return NULL;
    }

    // since we seeked the end of file, ftell will return the position of the EOF or the file size
    long file_size = ftell(file);
    if(file_size == -1)
    {
        return NULL;
    }
    char* file_data = (char*)malloc((size_t)file_size + 1); // +1 for the null terminator
    
    fseek(file, 0, SEEK_SET); // i think since already checked fseek before should be fine
    if(file_data == NULL || fread(file_data, 1, (size_t)file_size, file) != (size_t)file_size)
    {
        free(file_data);
        return NULL;
    }
    file_data[file_size] = '\0';

    fclose(file);
    return file_data;
}

int str_find_last_of(const char* str, char c)
{
    int i = 0;
    int latest_occurrence = -1;
    const char* curr = str;
    while(curr != NULL && *curr != '\0')
    {
        if(*curr == c) {
            latest_occurrence = i;
        }
        ++curr;
        ++i;
    }

    return latest_occurrence;
}
