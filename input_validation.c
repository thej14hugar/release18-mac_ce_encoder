#include <stdio.h>
#include <string.h>
#include "encoder.h"
#include "input_validation.h"

//----------------------------------
// FILE VALIDATION
//----------------------------------
int validate_input_file(const char *filename)
{
    const char *ext = strrchr(filename, '.');

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

    if (!ext || strcasecmp(ext, ".txt") != 0)
    {
        printf("ERROR: Invalid file type. Only .txt allowed\n");
        return FAILURE;
    }

 
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("ERROR: file not found\n");
        return FAILURE;
    }

    fclose(fp);
    return SUCCESS;
}