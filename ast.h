#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =======================
   AST NODE TYPES
   ======================= */

typedef enum {
    /* Statements */
    AST_SELECT,
    AST_INSERT,
    AST_CREATE,
    AST_SHOW,
    AST_USE,

    /* Clauses */
    AST_WHERE,
    AST_CONDITION,      // AND/OR conditions

    /* Expressions / Columns / Tables */
    AST_STAR,
    AST_IDENTIFIER,
    AST_LITERAL_NUMBER,
    AST_LITERAL_STRING,
    AST_DATATYPE         // INT, VARCHAR, DOUBLE, DATE
} ASTNodeType;

/* AST node structure */
typedef struct ASTNode {
    ASTNodeType type;
    char value[64];           // For identifiers or literals
    struct ASTNode* left;     // Child node (columns, table, condition)
    struct ASTNode* right;    // Right child or next node
} ASTNode;

/* =======================
   AST FUNCTIONS
   ======================= */

/* Create a new AST node */
ASTNode* ast_new(ASTNodeType type, const char* value);

/* Print AST recursively */
void print_ast(ASTNode* node, int indent);

/* Free AST memory recursively */
void free_ast(ASTNode* node);

#endif
