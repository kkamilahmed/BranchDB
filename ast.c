#include "ast.h"

ASTNode* ast_new(ASTNodeType type, const char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    if (value)
        strncpy(node->value, value, 63);
    else
        node->value[0] = '\0';
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* Print AST recursively */
void print_ast(ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  ");

    switch (node->type) {
        case AST_SELECT: printf("SELECT\n"); break;
        case AST_INSERT: printf("INSERT\n"); break;
        case AST_CREATE: printf("CREATE\n"); break;
        case AST_SHOW: printf("SHOW\n"); break;
        case AST_WHERE: printf("WHERE\n"); break;
        case AST_CONDITION: printf("CONDITION\n"); break;
        case AST_STAR: printf("STAR\n"); break;
        case AST_IDENTIFIER: printf("IDENTIFIER(%s)\n", node->value); break;
        case AST_LITERAL_NUMBER: printf("NUMBER(%s)\n", node->value); break;
        case AST_LITERAL_STRING: printf("STRING(%s)\n", node->value); break;
        case AST_DATATYPE: printf("DATATYPE(%s)\n", node->value); break;
    }

    if (node->left) print_ast(node->left, indent + 1);
    if (node->right) print_ast(node->right, indent + 1);
}

/* Free AST memory recursively */
void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}
