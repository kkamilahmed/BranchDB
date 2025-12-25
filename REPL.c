#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "input_buffer.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"

/* Convert string to lowercase */
void toLower(char* input) {
    for (int i = 0; input[i] != '\0'; i++) {
        input[i] = tolower((unsigned char)input[i]);
    }
}

/* Prompt */
void print_prompt() { 
    printf("branchdb > "); 
}


int main() {
    InputBuffer* input_buffer = new_input_buffer();
    init_keywords();

    while (1) {
        print_prompt();
        read_input(input_buffer);

        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            return 0;
        }

        /* Initialize lexer and parser */
        Lexer lexer = { input_buffer->buffer, 0 };
        Parser parser;
        parser_init(&parser, &lexer);

        /* Parse the statement */
        ASTNode* root = NULL;
        // Handle parse errors gracefully
        if (strlen(input_buffer->buffer) > 0) {
            root = parse_statement(&parser);
        }

        /* Print the AST */
        if (root) {
            printf("AST:\n");
            print_ast(root, 0);
        }

        // Free AST nodes here if you implement a destroy function
    }

    close_input_buffer(input_buffer);
    return 0;
}
