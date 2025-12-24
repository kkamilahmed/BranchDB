#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "input_buffer.h"
#include "parser.h"


void toLower(char*input){
        for (int i = 0; input[i] != '\0'; i++) {
            input[i] = tolower((unsigned char)input[i]);
        }
}

void print_prompt() { printf("branchdb > "); }



int main() {
    InputBuffer* input_buffer = new_input_buffer();
    Statement * statement;
    while (1) {
        print_prompt();
        read_input(input_buffer);
        toLower(input_buffer->buffer);

        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            return 0;
        } 

        parser(input_buffer,statement);

        
    }
    return 0;
}


