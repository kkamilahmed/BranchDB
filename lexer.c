#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct {
    const char* key;
    TokenType value;
} Keyword;

#define KEYWORD_TABLE_SIZE 32
static Keyword keyword_table[KEYWORD_TABLE_SIZE];  

static unsigned int hash(const char* str) {
    unsigned int h = 5381;
    while (*str) {
        h = ((h << 5) + h) + tolower(*str);
        str++;
    }
    return h % KEYWORD_TABLE_SIZE;
}

static void keyword_insert(const char* key, TokenType value) {
    unsigned int index = hash(key);
    while (keyword_table[index].key != NULL) {
        index = (index + 1) % KEYWORD_TABLE_SIZE;
    }
    keyword_table[index].key = key;
    keyword_table[index].value = value;
}


static TokenType keyword_lookup(const char* key) {
    unsigned int index = hash(key);
    while (keyword_table[index].key != NULL) {
        if (strcasecmp(keyword_table[index].key, key) == 0) {
            return keyword_table[index].value;
        }
        index = (index + 1) % KEYWORD_TABLE_SIZE;
    }
    return TOKEN_IDENTIFIER;
}


void init_keywords() {
    keyword_insert("select", TOKEN_SELECT);
    keyword_insert("create",TOKEN_CREATE);
    keyword_insert("table",TOKEN_TABLE);
    keyword_insert("from", TOKEN_FROM);
    keyword_insert("use", TOKEN_USE);
    keyword_insert("show", TOKEN_SHOW);
    keyword_insert("databases", TOKEN_DATABASES);
    keyword_insert("tables", TOKEN_TABLES);
    keyword_insert("insert", TOKEN_INSERT);
    keyword_insert("where", TOKEN_WHERE);
    keyword_insert("and", TOKEN_AND);
    keyword_insert("or", TOKEN_OR);
    keyword_insert("database",TOKEN_DATABASE);
    keyword_insert("varchar",TOKEN_VARCHAR);
    keyword_insert("int",TOKEN_INT);
    keyword_insert("double",TOKEN_DOUBLE);
    keyword_insert("date",TOKEN_DATE);
    
}


char peek(Lexer* lexer) {
    return lexer->input[lexer->pos];
}

char advance(Lexer* lexer) {
    return lexer->input[lexer->pos++];
}


static Token read_word(Lexer* lexer) {
    Token token;
    int start = lexer->pos;

    while (isalnum(peek(lexer)) || peek(lexer) == '_') advance(lexer);

    int length = lexer->pos - start;
    strncpy(token.lexeme, lexer->input + start, length);
    token.lexeme[length] = '\0';

    token.type = keyword_lookup(token.lexeme);
    return token;
}

static Token read_number(Lexer* lexer) {
    Token token;
    int start = lexer->pos;

    while (isdigit(peek(lexer))) advance(lexer);

    int length = lexer->pos - start;
    strncpy(token.lexeme, lexer->input + start, length);
    token.lexeme[length] = '\0';
    token.type = TOKEN_NUMBER;
    return token;
}

static Token read_string(Lexer* lexer) {
    Token token;
    char quote = advance(lexer);
    int start = lexer->pos;

    while (peek(lexer) != quote && peek(lexer) != '\0') advance(lexer);

    int length = lexer->pos - start;
    strncpy(token.lexeme, lexer->input + start, length);
    token.lexeme[length] = '\0';
    token.type = TOKEN_STRING;

    if (peek(lexer) == quote) advance(lexer);
    return token;
}


Token next_token(Lexer* lexer) {
    Token token;

    while (isspace(peek(lexer))) advance(lexer);

    char c = peek(lexer);

    if (c == '\0') {
        token.type = TOKEN_EOF;
        token.lexeme[0] = '\0';
        return token;
    }

    if (isalpha(c)) return read_word(lexer);
    if (isdigit(c)) return read_number(lexer);
    if (c == '"' || c == '\'') return read_string(lexer);

    advance(lexer);

    switch (c) {
        case '*': token.type = TOKEN_STAR; strcpy(token.lexeme, "*"); break;
        case ',': token.type = TOKEN_COMMA; strcpy(token.lexeme, ","); break;
        case ';': token.type = TOKEN_SEMICOLON; strcpy(token.lexeme, ";"); break;
        case '=': token.type = TOKEN_EQUAL; strcpy(token.lexeme, "="); break;
        case '(': token.type = TOKEN_LEFT_PAREN; strcpy(token.lexeme, "("); break;
        case ')': token.type = TOKEN_RIGHT_PAREN; strcpy(token.lexeme, ")"); break;

        case '>':
            if (peek(lexer) == '=') {
                advance(lexer);
                token.type = TOKEN_GREATER_EQUAL;
                strcpy(token.lexeme, ">=");
            } else {
                token.type = TOKEN_GREATER;
                strcpy(token.lexeme, ">");
            }
            break;
        case '<':
            if (peek(lexer) == '=') {
                advance(lexer);
                token.type = TOKEN_LESS_EQUAL;
                strcpy(token.lexeme, "<=");
            } else {
                token.type = TOKEN_LESS;
                strcpy(token.lexeme, "<");
            }
            break;
        default:
            token.type = TOKEN_EOF;
            token.lexeme[0] = '\0';
    }

    return token;
}
