#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "ast.h"
#include <stdio.h>

/* Global current database */
extern char current_database[128];

/* B-Tree Node Structure */
typedef struct BTreeNode {
    int* keys;
    int key_count;
    struct BTreeNode** children;
    int is_leaf;
    int degree;
} BTreeNode;

/* Table Schema Structure */
typedef struct {
    char column_name[64];
    char data_type[32];
} ColumnSchema;

typedef struct {
    char table_name[64];
    int column_count;
    ColumnSchema* columns;
} TableSchema;

/* ============================================
   MAIN EXECUTOR
   ============================================ */

/* Execute AST nodes */
void execute_statement(ASTNode* root);

/* ============================================
   DATABASE OPERATIONS
   ============================================ */

void createdatabase(ASTNode* database);
void deletedatabase(const char* db_name);
void use_database(ASTNode* use_node);
void show_databases();

/* ============================================
   TABLE OPERATIONS
   ============================================ */

void createtable(ASTNode* table);
void deletetable(const char* table_name);
void show_tables();

/* ============================================
   DATA OPERATIONS
   ============================================ */

void execute_select(ASTNode* select);
void execute_insert(ASTNode* insert);

/* ============================================
   SCHEMA OPERATIONS
   ============================================ */

void write_schema(const char* db_name, const char* table_name, ASTNode* columns);
TableSchema* read_schema(const char* db_name, const char* table_name);
void free_schema(TableSchema* schema);

/* ============================================
   FILE OPERATIONS
   ============================================ */

void create_table_file(const char* db_name, const char* table_name);
void create_index_file(const char* db_name, const char* table_name);

/* ============================================
   INDEX OPERATIONS
   ============================================ */

BTreeNode* load_index(const char* db_name, const char* table_name);

/* ============================================
   HELPER FUNCTIONS
   ============================================ */

int calculate_row_size(TableSchema* schema);
void write_value(FILE* file, const char* value, const char* data_type);
void read_value(FILE* file, const char* data_type, char* buffer, size_t buffer_size);
int should_select_column(ASTNode* column_list, const char* column_name);
int evaluate_condition(ASTNode* condition, TableSchema* schema, FILE* table_file, long row_offset);

#endif