#ifndef PARSER_H
#define PARSER_H

#include "input_buffer.h" 


typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;


typedef struct {
    StatementType type;
} Statement;


void parser(InputBuffer* input_buffer, Statement* statement);

void execute_statement(Statement* statement);

#endif
