#ifndef LEXER_H
#define LEXER_H

#include <ctype.h>


typedef enum {
    /* Keywords */
    TOKEN_USE,
    TOKEN_DATABASE,
    TOKEN_CREATE,
    TOKEN_SHOW,
    TOKEN_DATABASES,
    TOKEN_TABLE,
    TOKEN_TABLES,
    TOKEN_SELECT,
    TOKEN_FROM,
    TOKEN_INSERT,
    TOKEN_WHERE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_VARCHAR,
    TOKEN_INT,
    TOKEN_DOUBLE,
    TOKEN_DATE,
    TOKEN_INTO,
    TOKEN_VALUES,

    /* Symbols */
    TOKEN_STAR,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_EQUAL,
    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS_EQUAL,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,


    /* Literals */
    TOKEN_NUMBER,
    TOKEN_STRING,

    /* Identifiers */
    TOKEN_IDENTIFIER,

    /* End */
    TOKEN_EOF
} TokenType;



typedef struct {
    TokenType type;
    char lexeme[64];
} Token;



typedef struct {
    const char* input;
    int pos;
} Lexer;


void init_keywords(void);


void toLower(char* input) ;

Token next_token(Lexer* lexer);


char peek(Lexer* lexer);
char advance(Lexer* lexer);

#endif 
