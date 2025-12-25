#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"


typedef struct {
    Lexer* lexer;
    Token current;  
} Parser;


void parser_init(Parser* parser, Lexer* lexer);


ASTNode* parse_statement(Parser* parser);


ASTNode* parse_select(Parser* parser);
ASTNode* parse_insert(Parser* parser);
ASTNode* parse_createdatabase(Parser* parser);
ASTNode* parse_createtable(Parser* parser);
ASTNode* parse_use(Parser* parser);
ASTNode* parse_show(Parser* parser);


ASTNode* parse_column(Parser* parser);
ASTNode* parse_column_list(Parser* parser);
ASTNode* parse_condition(Parser* parser);
ASTNode* parse_where(Parser* parser);

void advance_token(Parser* parser);
void expect(Parser* parser, TokenType type);

#endif
