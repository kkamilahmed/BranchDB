#include <stdio.h>
#include <stdlib.h>
#include "parser.h"


void advance_token(Parser* parser) {
    parser->current = next_token(parser->lexer);
}

void expect(Parser* parser, TokenType type) {
    if (parser->current.type != type) {
        printf("Parse error: expected %d but got %d (%s)\n",
               type, parser->current.type, parser->current.lexeme);
        exit(1);
    }
    advance_token(parser);
}

void parser_init(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    advance_token(parser);
}



ASTNode* parse_column(Parser* parser) {
    if (parser->current.type == TOKEN_STAR) {
        ASTNode* star_node = ast_new(AST_STAR, NULL);
        advance_token(parser);
        return star_node;
    }

    if (parser->current.type != TOKEN_IDENTIFIER) {
        printf("Expected column name or *\n");
        exit(1);
    }

    ASTNode* col_node = ast_new(AST_IDENTIFIER, parser->current.lexeme);
    advance_token(parser);
    return col_node;
}

ASTNode* parse_column_list(Parser* parser) {
    ASTNode* head = parse_column(parser);
    ASTNode* current = head;

    while (parser->current.type == TOKEN_COMMA) {
        advance_token(parser); 
        ASTNode* next_col = parse_column(parser);
        current->right = next_col; 
        current = next_col;
    }

    return head;
}



ASTNode* parse_condition(Parser* parser) {
    if (parser->current.type != TOKEN_IDENTIFIER) {
        printf("Expected column in WHERE condition\n");
        exit(1);
    }
    ASTNode* column = ast_new(AST_IDENTIFIER, parser->current.lexeme);
    advance_token(parser);

    TokenType op = parser->current.type;
    if (op != TOKEN_EQUAL && op != TOKEN_GREATER && op != TOKEN_LESS &&
        op != TOKEN_GREATER_EQUAL && op != TOKEN_LESS_EQUAL) {
        printf("Expected comparison operator in WHERE\n");
        exit(1);
    }
    ASTNode* condition = ast_new(AST_CONDITION, parser->current.lexeme);
    condition->left = column;
    advance_token(parser);

    if (parser->current.type == TOKEN_NUMBER || parser->current.type == TOKEN_STRING) {
        ASTNode* value_node = ast_new(
            parser->current.type == TOKEN_NUMBER ? AST_LITERAL_NUMBER : AST_LITERAL_STRING,
            parser->current.lexeme
        );
        condition->right = value_node;
        advance_token(parser);
    } else {
        printf("Expected literal value in WHERE\n");
        exit(1);
    }

    return condition;
}

ASTNode* parse_where(Parser* parser) {
    expect(parser, TOKEN_WHERE);

    ASTNode* where_node = ast_new(AST_WHERE, NULL);
    ASTNode* current = parse_condition(parser);

    while (parser->current.type == TOKEN_AND || parser->current.type == TOKEN_OR) {
        ASTNode* logic = ast_new(AST_CONDITION, parser->current.lexeme); 
        advance_token(parser);
        ASTNode* next_cond = parse_condition(parser);
        logic->left = current;
        logic->right = next_cond;
        current = logic; 
    }

    where_node->left = current;
    return where_node;
}



ASTNode* parse_select(Parser* parser) {
    expect(parser, TOKEN_SELECT);

    ASTNode* select_node = ast_new(AST_SELECT, NULL);
    select_node->left = parse_column_list(parser);

    expect(parser, TOKEN_FROM);

    if (parser->current.type != TOKEN_IDENTIFIER) {
        printf("Expected table name\n");
        exit(1);
    }
    ASTNode* table_node = ast_new(AST_IDENTIFIER, parser->current.lexeme);
    select_node->right = table_node;
    advance_token(parser);

    if (parser->current.type == TOKEN_WHERE) {
        ASTNode* where_node = parse_where(parser);
        table_node->right = where_node; 
    }

    expect(parser, TOKEN_SEMICOLON);
    return select_node;
}

ASTNode* parse_insert(Parser* parser) {
    expect(parser, TOKEN_INSERT);
    expect(parser, TOKEN_INTO);

    if (parser->current.type != TOKEN_IDENTIFIER) {
        printf("Expected table name after INSERT INTO\n");
        exit(1);
    }

    ASTNode* insert_node = ast_new(AST_INSERT, NULL);
    ASTNode* table_node = ast_new(AST_IDENTIFIER, parser->current.lexeme);
    insert_node->right = table_node;
    advance_token(parser);

    if (parser->current.type == TOKEN_LEFT_PAREN) {
        advance_token(parser);
        table_node->left = parse_column_list(parser);
        expect(parser, TOKEN_RIGHT_PAREN);
    }

    expect(parser, TOKEN_VALUES);

    expect(parser, TOKEN_LEFT_PAREN);
    ASTNode* value_head = NULL;
    ASTNode* value_current = NULL;

    while (parser->current.type == TOKEN_NUMBER || parser->current.type == TOKEN_STRING) {
        ASTNode* val_node = ast_new(
            parser->current.type == TOKEN_NUMBER ? AST_LITERAL_NUMBER : AST_LITERAL_STRING,
            parser->current.lexeme
        );
        advance_token(parser);

        if (!value_head) {
            value_head = val_node;
            value_current = val_node;
        } else {
            value_current->right = val_node;
            value_current = val_node;
        }

        if (parser->current.type == TOKEN_COMMA) advance_token(parser);
    }

    expect(parser, TOKEN_RIGHT_PAREN);
    expect(parser, TOKEN_SEMICOLON);

    table_node->right = value_head;
    return insert_node;
}

ASTNode* parse_createdatabase(Parser* parser) {
    expect(parser, TOKEN_CREATE);
    expect(parser, TOKEN_DATABASE);

    if (parser->current.type != TOKEN_IDENTIFIER) {
        printf("Expected database name\n");
        exit(1);
    }

    ASTNode* create_node = ast_new(AST_CREATE, NULL);
    ASTNode* database_node = ast_new(AST_IDENTIFIER, parser->current.lexeme);
    create_node->right = database_node;
    advance_token(parser);
    expect(parser, TOKEN_SEMICOLON);

    return create_node;
}

ASTNode* parse_createtable(Parser* parser) {
    expect(parser, TOKEN_CREATE);
    expect(parser, TOKEN_TABLE);

    if (parser->current.type != TOKEN_IDENTIFIER) {
        printf("Expected table name\n");
        exit(1);
    }

    ASTNode* create_node = ast_new(AST_CREATE, NULL);
    ASTNode* table_node = ast_new(AST_IDENTIFIER, parser->current.lexeme);
    create_node->right = table_node;
    advance_token(parser);

    expect(parser, TOKEN_LEFT_PAREN);

    ASTNode* col_head = NULL;
    ASTNode* col_current = NULL;

    while (parser->current.type == TOKEN_IDENTIFIER) {
        ASTNode* col_node = ast_new(AST_IDENTIFIER, parser->current.lexeme);
        advance_token(parser);

        if (parser->current.type != TOKEN_VARCHAR &&
            parser->current.type != TOKEN_INT &&
            parser->current.type != TOKEN_DOUBLE &&
            parser->current.type != TOKEN_DATE) {
            printf("Expected datatype for column\n");
            exit(1);
        }

        ASTNode* type_node = ast_new(AST_DATATYPE, parser->current.lexeme);
        col_node->right = type_node;
        advance_token(parser);

        if (!col_head) {
            col_head = col_node;
            col_current = col_node;
        } else {
            col_current->right = col_node;
            col_current = col_node;
        }

        if (parser->current.type == TOKEN_COMMA) advance_token(parser);
        else break;
    }

    expect(parser, TOKEN_RIGHT_PAREN);
    expect(parser, TOKEN_SEMICOLON);

    table_node->left = col_head;
    return create_node;
}

ASTNode* parse_use(Parser* parser) {
    expect(parser, TOKEN_USE);

    if (parser->current.type != TOKEN_IDENTIFIER) {
        printf("Expected database name\n");
        exit(1);
    }

    ASTNode* use_node = ast_new(AST_USE, NULL);
    ASTNode* db_node = ast_new(AST_IDENTIFIER, parser->current.lexeme);
    use_node->right = db_node;
    advance_token(parser);
    expect(parser, TOKEN_SEMICOLON);

    return use_node;
}

ASTNode* parse_show(Parser* parser) {
    expect(parser, TOKEN_SHOW);

    if (parser->current.type == TOKEN_DATABASES) {
        ASTNode* node = ast_new(AST_SHOW, "DATABASES");
        advance_token(parser);
        expect(parser, TOKEN_SEMICOLON);
        return node;
    }

    if (parser->current.type == TOKEN_TABLES) {
        ASTNode* node = ast_new(AST_SHOW, "TABLES");
        advance_token(parser);
        expect(parser, TOKEN_SEMICOLON);
        return node;
    }

    printf("Expected DATABASES or TABLES after SHOW\n");
    exit(1);
}



ASTNode* parse_statement(Parser* parser) {
    switch (parser->current.type) {
        case TOKEN_SELECT: return parse_select(parser);
        case TOKEN_INSERT: return parse_insert(parser);
        case TOKEN_CREATE:

            {
                Lexer temp_lexer = *(parser->lexer);
                Parser temp_parser = *parser;
                temp_parser.lexer = &temp_lexer;
                advance_token(&temp_parser);
                
                if (temp_parser.current.type == TOKEN_DATABASE)
                    return parse_createdatabase(parser);
                else if (temp_parser.current.type == TOKEN_TABLE)
                    return parse_createtable(parser);
                else {
                    printf("Expected DATABASE or TABLE after CREATE\n");
                    exit(1);
                }
            }
        case TOKEN_USE: return parse_use(parser);
        case TOKEN_SHOW: return parse_show(parser);
        default:
            printf("Unknown statement\n");
            exit(1);
    }
}