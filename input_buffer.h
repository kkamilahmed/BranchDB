#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

#include <stddef.h>   

typedef struct {
    char* buffer;
    size_t buffer_length;
    long input_length;
} InputBuffer;

/* Function declarations */
InputBuffer* new_input_buffer(void);
void read_input(InputBuffer* input_buffer);
void close_input_buffer(InputBuffer* input_buffer);

#endif
