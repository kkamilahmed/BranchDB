#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct {
    char* buffer;
    size_t buffer_length;
    long input_length;  
} InputBuffer;

InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

char* toLower(char* input);

void print_prompt() { printf("branchdb > "); }

void read_input(InputBuffer* input_buffer) {
  long bytes_read =getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    return ;
  }

  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}



int main() {
    char  input [100];
    InputBuffer* input_buffer = new_input_buffer();
    while (1) {
        print_prompt();
        read_input(input_buffer);
        toLower(input_buffer->buffer);

        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            return 0;
        } 
        
        else {
        printf("Unrecognized command '%s'.\n", input_buffer->buffer);
        }
        


    }
    return 0;
}


char * toLower(char*input){
        for (int i = 0; input[i] != '\0'; i++) {
            input[i] = tolower((unsigned char)input[i]);
        }
}