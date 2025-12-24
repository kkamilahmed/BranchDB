#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    // Keywords
    TOKEN_USE,
    TOKEN_SHOW,
    TOKEN_DATABASES,
    TOKEN_TABLES,
    TOKEN_SELECT,
    TOKEN_FROM,
    TOKEN_INSERT,
    TOKEN_WHERE,
    TOKEN_AND,
    TOKEN_OR,

    // Symbols
    TOKEN_STAR,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_EQUAL,
    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS_EQUAL,

    // Literals
    TOKEN_NUMBER,
    TOKEN_STRING,

    // Identifiers
    TOKEN_IDENTIFIER,

    // End of input
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

char peek(Lexer* lexer) {
    return lexer->input[lexer->pos];
}

char advance(Lexer* lexer) {
    return lexer->input[lexer->pos++];
}

Token read_word(Lexer* lexer) {
    Token token;
    int start = lexer->pos;

    while (isalnum(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }

    int length = lexer->pos - start;
    strncpy(token.lexeme, lexer->input + start, length);
    token.lexeme[length] = '\0';

    // Keywords
    if (strcasecmp(token.lexeme, "select") == 0) token.type = TOKEN_SELECT;
    else if (strcasecmp(token.lexeme, "from") == 0) token.type = TOKEN_FROM;
    else if (strcasecmp(token.lexeme, "use") == 0) token.type = TOKEN_USE;
    else if (strcasecmp(token.lexeme, "show") == 0) token.type = TOKEN_SHOW;
    else if (strcasecmp(token.lexeme, "databases") == 0) token.type = TOKEN_DATABASES;
    else if (strcasecmp(token.lexeme, "tables") == 0) token.type = TOKEN_TABLES;
    else if (strcasecmp(token.lexeme, "insert") == 0) token.type = TOKEN_INSERT;
    else if (strcasecmp(token.lexeme, "where") == 0) token.type = TOKEN_WHERE;
    else if (strcasecmp(token.lexeme, "and") == 0) token.type = TOKEN_AND;
    else if (strcasecmp(token.lexeme, "or") == 0) token.type = TOKEN_OR;
    else token.type = TOKEN_IDENTIFIER;

    return token;
}

// ---- Read numbers ----
Token read_number(Lexer* lexer) {
    Token token;
    int start = lexer->pos;
    int has_dot = 0;

    while (isdigit(peek(lexer)) || peek(lexer) == '.') {
        if (peek(lexer) == '.') {
            if (has_dot) break; // only one dot allowed
            has_dot = 1;
        }
        advance(lexer);
    }

    int length = lexer->pos - start;
    strncpy(token.lexeme, lexer->input + start, length);
    token.lexeme[length] = '\0';
    token.type = TOKEN_NUMBER;
    return token;
}


Token read_string(Lexer* lexer) {
    Token token;
    char quote = advance(lexer);
    int start = lexer->pos;

    while (peek(lexer) != quote && peek(lexer) != '\0') {
        advance(lexer);
    }

    int length = lexer->pos - start;
    strncpy(token.lexeme, lexer->input + start, length);
    token.lexeme[length] = '\0';
    token.type = TOKEN_STRING;

    if (peek(lexer) == quote) advance(lexer); 
    return token;
}


Token next_token(Lexer* lexer) {
    Token token;

    // Skip whitespace
    while (isspace(peek(lexer))) advance(lexer);

    char c = peek(lexer);

    if (c == '\0') { token.type = TOKEN_EOF; strcpy(token.lexeme, ""); return token; }

    if (isalpha(c)) return read_word(lexer);
    if (isdigit(c)) return read_number(lexer);
    if (c == '"' || c == '\'') return read_string(lexer);


    advance(lexer);
    switch (c) {
        case '*': 
            token.type = TOKEN_STAR; strcpy(token.lexeme, "*"); break;
        case ',': 
            token.type = TOKEN_COMMA; strcpy(token.lexeme, ","); break;
        case ';': 
            token.type = TOKEN_SEMICOLON; strcpy(token.lexeme, ";"); break;
        case '=': 
            token.type = TOKEN_EQUAL; strcpy(token.lexeme, "="); break;
        case '>': 
            if (peek(lexer) == '=') 
                { advance(lexer); token.type = TOKEN_GREATER_EQUAL; strcpy(token.lexeme, ">="); }
            else 
                { token.type = TOKEN_GREATER; strcpy(token.lexeme, ">"); }
            break;
        case '<': 
            if (peek(lexer) == '=') 
                { advance(lexer); token.type = TOKEN_LESS_EQUAL; strcpy(token.lexeme, "<="); }
            else 
                { token.type = TOKEN_LESS; strcpy(token.lexeme, "<"); }
            break;
        default:
            token.type = TOKEN_EOF;
            strcpy(token.lexeme, "");
    }

    return token;
}


int main() {
    const char* input = "select * from users";

    Lexer lexer = { input, 0 };

    while (1) {
        Token token = next_token(&lexer);

        if (token.type == TOKEN_EOF) {
            printf("TOKEN_EOF\n");
            break;
        }

        printf("TOKEN_%d (%s)\n", token.type, token.lexeme);
    }

    return 0;
}
