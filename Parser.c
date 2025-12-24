#include "parser.h"
#include <string.h>

void parser(InputBuffer* input_buffer, Statement* statement) {
    char* keyword = strtok(input_buffer->buffer, " ");

    if (strcmp(keyword, "insert") == 0) {
        statement->type = STATEMENT_INSERT;
    } else if (strcmp(keyword, "select") == 0) {
        statement->type = STATEMENT_SELECT;
    }

}


void execute_statement(Statement* statement) {
    switch (statement->type) {
        case STATEMENT_INSERT:
            printf("Executing INSERT statement\n");
            break;

        case STATEMENT_SELECT:
            printf("Executing SELECT statement\n");
            break;

        default:
            printf("Unrecognized statement\n");
            break;
    }
}
