#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <errno.h>
#include <sys/stat.h>
#include <windows.h>
#include "executor.h"
#include "ast.h"

// Global to track current database
char current_database[128] = "";

/* ============================================
   HELPER FUNCTIONS (Must be defined first)
   ============================================ */

/* Calculate row size based on schema */
int calculate_row_size(TableSchema* schema) {
    int size = 0;
    for (int i = 0; i < schema->column_count; i++) {
        if (strcmp(schema->columns[i].data_type, "int") == 0) {
            size += sizeof(int);
        } else if (strcmp(schema->columns[i].data_type, "varchar") == 0) {
            size += 64;  // Fixed size for varchar
        } else if (strcmp(schema->columns[i].data_type, "double") == 0) {
            size += sizeof(double);
        } else if (strcmp(schema->columns[i].data_type, "date") == 0) {
            size += sizeof(int);  // Store as Unix timestamp
        }
    }
    return size;
}

/* Write a value to the table file based on data type */
void write_value(FILE* file, const char* value, const char* data_type) {
    if (strcmp(data_type, "int") == 0) {
        int int_val = atoi(value);
        fwrite(&int_val, sizeof(int), 1, file);
    } else if (strcmp(data_type, "varchar") == 0) {
        char buffer[64] = {0};  // Initialize with zeros
        strncpy(buffer, value, 63);  // Copy up to 63 chars (leave room for null terminator)
        fwrite(buffer, sizeof(char), 64, file);
    } else if (strcmp(data_type, "double") == 0) {
        double double_val = atof(value);
        fwrite(&double_val, sizeof(double), 1, file);
    } else if (strcmp(data_type, "date") == 0) {
        // Simple date parsing (you can improve this)
        int date_val = atoi(value);  // For now, treat as timestamp
        fwrite(&date_val, sizeof(int), 1, file);
    }
}

/* Read a value from table file based on data type */
void read_value(FILE* file, const char* data_type, char* buffer, size_t buffer_size) {
    if (strcmp(data_type, "int") == 0) {
        int val;
        fread(&val, sizeof(int), 1, file);
        snprintf(buffer, buffer_size, "%d", val);
    } else if (strcmp(data_type, "varchar") == 0) {
        char varchar_buffer[64];
        fread(varchar_buffer, sizeof(char), 64, file);
        varchar_buffer[63] = '\0';  // Ensure null termination
        snprintf(buffer, buffer_size, "%s", varchar_buffer);
    } else if (strcmp(data_type, "double") == 0) {
        double val;
        fread(&val, sizeof(double), 1, file);
        snprintf(buffer, buffer_size, "%.2f", val);
    } else if (strcmp(data_type, "date") == 0) {
        int val;
        fread(&val, sizeof(int), 1, file);
        snprintf(buffer, buffer_size, "%d", val);
    }
}

/* Check if a column should be selected (handles * and specific columns) */
int should_select_column(ASTNode* column_list, const char* column_name) {
    // If it's *, select all columns
    if (column_list->type == AST_STAR) {
        return 1;
    }
    
    // Check if column is in the list
    ASTNode* current = column_list;
    while (current) {
        if (current->type == AST_IDENTIFIER && strcmp(current->value, column_name) == 0) {
            return 1;
        }
        current = current->right;
    }
    
    return 0;
}

/* Evaluate WHERE condition for a row */
int evaluate_condition(ASTNode* condition, TableSchema* schema, FILE* table_file, long row_offset) {
    if (!condition) return 1;  // No condition means select all
    
    if (condition->type == AST_WHERE) {
        return evaluate_condition(condition->left, schema, table_file, row_offset);
    }
    
    if (condition->type == AST_CONDITION) {
        // Get column name from left child
        if (!condition->left || condition->left->type != AST_IDENTIFIER) {
            return 0;
        }
        
        const char* column_name = condition->left->value;
        const char* operator = condition->value;
        
        // Get expected value from right child
        if (!condition->right) return 0;
        const char* expected_value = condition->right->value;
        
        // Find column index in schema
        int col_index = -1;
        for (int i = 0; i < schema->column_count; i++) {
            if (strcmp(schema->columns[i].column_name, column_name) == 0) {
                col_index = i;
                break;
            }
        }
        
        if (col_index == -1) {
            printf("Column '%s' not found\n", column_name);
            return 0;
        }
        
        // Seek to the column in the row
        long offset = row_offset;
        for (int i = 0; i < col_index; i++) {
            if (strcmp(schema->columns[i].data_type, "int") == 0) {
                offset += sizeof(int);
            } else if (strcmp(schema->columns[i].data_type, "varchar") == 0) {
                offset += 64;
            } else if (strcmp(schema->columns[i].data_type, "double") == 0) {
                offset += sizeof(double);
            } else if (strcmp(schema->columns[i].data_type, "date") == 0) {
                offset += sizeof(int);
            }
        }
        
        fseek(table_file, offset, SEEK_SET);
        
        // Read the actual value
        char actual_value[256];
        read_value(table_file, schema->columns[col_index].data_type, actual_value, sizeof(actual_value));
        
        // Compare based on operator
        if (strcmp(operator, "=") == 0) {
            return strcmp(actual_value, expected_value) == 0;
        } else if (strcmp(operator, ">") == 0) {
            return atoi(actual_value) > atoi(expected_value);
        } else if (strcmp(operator, "<") == 0) {
            return atoi(actual_value) < atoi(expected_value);
        } else if (strcmp(operator, ">=") == 0) {
            return atoi(actual_value) >= atoi(expected_value);
        } else if (strcmp(operator, "<=") == 0) {
            return atoi(actual_value) <= atoi(expected_value);
        }
    }
    
    return 0;
}

/* ============================================
   SCHEMA OPERATIONS
   ============================================ */

/* Write schema to .schema file */
void write_schema(const char* db_name, const char* table_name, ASTNode* columns) {
    char schema_path[256];
    snprintf(schema_path, sizeof(schema_path), "databases\\%s\\%s.schema", db_name, table_name);
    
    FILE* schema_file = fopen(schema_path, "w");
    if (!schema_file) {
        perror("Failed to create schema file");
        return;
    }
    
    // Write table name
    fprintf(schema_file, "TABLE:%s\n", table_name);
    fprintf(schema_file, "COLUMNS:\n");
    
    // Traverse column list
    ASTNode* current = columns;
    while (current) {
        if (current->type == AST_IDENTIFIER) {
            // Column name
            fprintf(schema_file, "%s,", current->value);
            
            // Data type (stored in right child)
            if (current->right && current->right->type == AST_DATATYPE) {
                fprintf(schema_file, "%s\n", current->right->value);
                
                // Move to next column (skip the datatype node)
                current = current->right->right;
            } else {
                current = current->right;
            }
        } else {
            current = current->right;
        }
    }
    
    fclose(schema_file);
    printf("Schema file created: %s\n", schema_path);
}

/* Read schema from .schema file */
TableSchema* read_schema(const char* db_name, const char* table_name) {
    char schema_path[256];
    snprintf(schema_path, sizeof(schema_path), "databases\\%s\\%s.schema", db_name, table_name);
    
    FILE* schema_file = fopen(schema_path, "r");
    if (!schema_file) {
        return NULL;
    }
    
    TableSchema* schema = malloc(sizeof(TableSchema));
    char line[256];
    
    // Read table name
    fgets(line, sizeof(line), schema_file);
    sscanf(line, "TABLE:%s", schema->table_name);
    
    // Skip "COLUMNS:" line
    fgets(line, sizeof(line), schema_file);
    
    // Count columns
    schema->column_count = 0;
    long pos = ftell(schema_file);
    while (fgets(line, sizeof(line), schema_file)) {
        schema->column_count++;
    }
    
    // Allocate column array
    schema->columns = malloc(sizeof(ColumnSchema) * schema->column_count);
    
    // Read columns
    fseek(schema_file, pos, SEEK_SET);
    int i = 0;
    while (fgets(line, sizeof(line), schema_file)) {
        char* comma = strchr(line, ',');
        if (comma) {
            *comma = '\0';
            strcpy(schema->columns[i].column_name, line);
            
            // Remove newline from data type
            char* newline = strchr(comma + 1, '\n');
            if (newline) *newline = '\0';
            strcpy(schema->columns[i].data_type, comma + 1);
            i++;
        }
    }
    
    fclose(schema_file);
    return schema;
}

/* Free schema */
void free_schema(TableSchema* schema) {
    if (schema) {
        free(schema->columns);
        free(schema);
    }
}

/* ============================================
   FILE OPERATIONS
   ============================================ */

/* Create empty .table file for data storage */
void create_table_file(const char* db_name, const char* table_name) {
    char table_path[256];
    snprintf(table_path, sizeof(table_path), "databases\\%s\\%s.table", db_name, table_name);
    
    FILE* table_file = fopen(table_path, "wb");
    if (!table_file) {
        perror("Failed to create table file");
        return;
    }
    
    // Write header: number of rows (initially 0)
    int row_count = 0;
    fwrite(&row_count, sizeof(int), 1, table_file);
    
    fclose(table_file);
    printf("Table file created: %s\n", table_path);
}

/* Create empty .idx file for B-tree index */
void create_index_file(const char* db_name, const char* table_name) {
    char index_path[256];
    snprintf(index_path, sizeof(index_path), "databases\\%s\\%s.idx", db_name, table_name);
    
    FILE* index_file = fopen(index_path, "wb");
    if (!index_file) {
        perror("Failed to create index file");
        return;
    }
    
    // Write B-tree metadata
    int degree = 3;  // Minimum degree (order)
    int node_count = 0;  // Initially no nodes
    
    fwrite(&degree, sizeof(int), 1, index_file);
    fwrite(&node_count, sizeof(int), 1, index_file);
    
    fclose(index_file);
    printf("Index file created: %s\n", index_path);
}

/* Load B-tree index into memory */
BTreeNode* load_index(const char* db_name, const char* table_name) {
    char index_path[256];
    snprintf(index_path, sizeof(index_path), "databases\\%s\\%s.idx", db_name, table_name);
    
    FILE* index_file = fopen(index_path, "rb");
    if (!index_file) {
        return NULL;
    }
    
    int degree, node_count;
    fread(&degree, sizeof(int), 1, index_file);
    fread(&node_count, sizeof(int), 1, index_file);
    
    // TODO: Implement B-tree deserialization
    // For now, return NULL (empty index)
    
    fclose(index_file);
    return NULL;
}

/* ============================================
   DATABASE OPERATIONS
   ============================================ */

void createdatabase(ASTNode *database) {
    char path[128];

    if (snprintf(path, sizeof(path), "databases\\%s", database->right->value) >= sizeof(path)) {
        fprintf(stderr, "Database name too long\n");
        return;
    }

    if (_mkdir(path) == -1) {
        if (errno == EEXIST) {
            printf("Database already exists\n");
        } else {
            perror("mkdir failed");
        }
    } else {
        printf("Database created: %s\n", path);
    }
}

void deletedatabase(const char* db_name) {
    char path[256];
    snprintf(path, sizeof(path), "databases\\%s", db_name);
    
    // Check if database exists
    DWORD attributes = GetFileAttributes(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        printf("Database '%s' does not exist\n", db_name);
        return;
    }
    
    // Delete all files in the database directory
    char search_path[256];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(search_path, &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(findFileData.cFileName, ".") != 0 && 
                strcmp(findFileData.cFileName, "..") != 0) {
                char file_path[512];
                snprintf(file_path, sizeof(file_path), "%s\\%s", path, findFileData.cFileName);
                DeleteFile(file_path);
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
    
    // Remove the directory
    if (_rmdir(path) == 0) {
        printf("Database '%s' deleted successfully\n", db_name);
        
        // Clear current database if it was deleted
        if (strcmp(current_database, db_name) == 0) {
            current_database[0] = '\0';
        }
    } else {
        perror("Failed to delete database");
    }
}

void use_database(ASTNode* use_node) {
    if (!use_node->right || use_node->right->type != AST_IDENTIFIER) {
        printf("Invalid database name\n");
        return;
    }
    
    const char* db_name = use_node->right->value;
    char path[256];
    snprintf(path, sizeof(path), "databases\\%s", db_name);
    
    // Check if database exists using Windows API
    DWORD attributes = GetFileAttributes(path);
    
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        printf("Error: Database '%s' does not exist\n", db_name);
        return;
    }
    
    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Error: '%s' is not a database directory\n", db_name);
        return;
    }
    
    // Successfully switch database
    strcpy(current_database, db_name);
    printf("Database changed to '%s'\n", current_database);
}

void show_databases() {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    hFind = FindFirstFile("databases\\*", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error: 'databases' directory not found\n");
        return;
    }

    int found = 0;
    printf("Databases:\n");
    printf("+------------------+\n");

    do {
        // Skip "." and ".."
        if (strcmp(findFileData.cFileName, ".") == 0 || 
            strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }

        // Check if it's a directory
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            printf("| %-16s |\n", findFileData.cFileName);
            found = 1;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    printf("+------------------+\n");

    if (!found) {
        printf("No databases found.\n");
    }

    FindClose(hFind);
}

/* ============================================
   TABLE OPERATIONS
   ============================================ */

void createtable(ASTNode* table) {
    if (current_database[0] == '\0') {
        printf("No database selected. Use 'USE <database>;' first.\n");
        return;
    }
    
    if (!table->right || table->right->type != AST_IDENTIFIER) {
        printf("Invalid table structure\n");
        return;
    }
    
    const char* table_name = table->right->value;
    ASTNode* columns = table->right->left;
    
    if (!columns) {
        printf("No columns defined for table\n");
        return;
    }
    
    // Check if table already exists
    TableSchema* existing = read_schema(current_database, table_name);
    if (existing) {
        printf("Table '%s' already exists\n", table_name);
        free_schema(existing);
        return;
    }
    
    // Create schema file
    write_schema(current_database, table_name, columns);
    
    // Create table data file
    create_table_file(current_database, table_name);
    
    // Create index file
    create_index_file(current_database, table_name);
    
    printf("Table '%s' created successfully in database '%s'\n", table_name, current_database);
}

void deletetable(const char* table_name) {
    if (current_database[0] == '\0') {
        printf("No database selected. Use 'USE <database>;' first.\n");
        return;
    }
    
    // Check if table exists
    TableSchema* schema = read_schema(current_database, table_name);
    if (!schema) {
        printf("Table '%s' does not exist\n", table_name);
        return;
    }
    free_schema(schema);
    
    // Delete .schema, .table, and .idx files
    char file_path[256];
    
    // Delete schema file
    snprintf(file_path, sizeof(file_path), "databases\\%s\\%s.schema", current_database, table_name);
    DeleteFile(file_path);
    
    // Delete table file
    snprintf(file_path, sizeof(file_path), "databases\\%s\\%s.table", current_database, table_name);
    DeleteFile(file_path);
    
    // Delete index file
    snprintf(file_path, sizeof(file_path), "databases\\%s\\%s.idx", current_database, table_name);
    DeleteFile(file_path);
    
    printf("Table '%s' deleted successfully\n", table_name);
}

void show_tables() {
    if (current_database[0] == '\0') {
        printf("No database selected. Use 'USE <database>;' first.\n");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "databases\\%s\\*", current_database);

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(path, &findFileData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error: Could not open database '%s'\n", current_database);
        return;
    }

    int found = 0;
    printf("Tables in database '%s':\n", current_database);
    printf("+--------------------------+\n");

    do {
        // Skip "." and ".."
        if (strcmp(findFileData.cFileName, ".") == 0 || 
            strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }

        // Check for .schema files (representing tables)
        char* ext = strrchr(findFileData.cFileName, '.');
        if (ext && strcmp(ext, ".schema") == 0) {
            // Remove .schema extension for display
            *ext = '\0';
            printf("| %-24s |\n", findFileData.cFileName);
            found = 1;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    printf("+--------------------------+\n");

    if (!found) {
        printf("No tables found.\n");
    }

    FindClose(hFind);
}

/* ============================================
   DATA OPERATIONS
   ============================================ */

void execute_select(ASTNode* select) {
    if (current_database[0] == '\0') {
        printf("No database selected. Use 'USE <database>;' first.\n");
        return;
    }
    
    // Get column list (left child) and table name (right child)
    ASTNode* column_list = select->left;
    ASTNode* table_node = select->right;
    
    if (!table_node || table_node->type != AST_IDENTIFIER) {
        printf("Invalid SELECT statement\n");
        return;
    }
    
    const char* table_name = table_node->value;
    
    // Get WHERE clause if it exists (right child of table node)
    ASTNode* where_clause = table_node->right;
    
    // Read schema
    TableSchema* schema = read_schema(current_database, table_name);
    if (!schema) {
        printf("Table '%s' does not exist\n", table_name);
        return;
    }
    
    // Open table file
    char table_path[256];
    snprintf(table_path, sizeof(table_path), "databases\\%s\\%s.table", current_database, table_name);
    
    FILE* table_file = fopen(table_path, "rb");
    if (!table_file) {
        perror("Failed to open table file");
        free_schema(schema);
        return;
    }
    
    // Read row count
    int row_count;
    fread(&row_count, sizeof(int), 1, table_file);
    
    // Calculate row size
    int row_size = calculate_row_size(schema);
    
    // Determine which columns to display
    int* display_columns = malloc(sizeof(int) * schema->column_count);
    int display_count = 0;
    
    for (int i = 0; i < schema->column_count; i++) {
        if (should_select_column(column_list, schema->columns[i].column_name)) {
            display_columns[display_count++] = i;
        }
    }
    
    // Print column headers
    printf("+");
    for (int i = 0; i < display_count; i++) {
        printf("----------------+");
    }
    printf("\n|");
    
    for (int i = 0; i < display_count; i++) {
        printf(" %-14s |", schema->columns[display_columns[i]].column_name);
    }
    printf("\n+");
    
    for (int i = 0; i < display_count; i++) {
        printf("----------------+");
    }
    printf("\n");
    
    // Read and print rows
    int rows_selected = 0;
    for (int row = 0; row < row_count; row++) {
        long row_offset = sizeof(int) + (row * row_size);
        
        // Evaluate WHERE clause if exists
        if (where_clause && !evaluate_condition(where_clause, schema, table_file, row_offset)) {
            continue;  // Skip this row
        }
        
        // Seek to the beginning of this row
        fseek(table_file, row_offset, SEEK_SET);
        
        // Read all column values into an array first
        char** row_values = malloc(sizeof(char*) * schema->column_count);
        for (int col = 0; col < schema->column_count; col++) {
            row_values[col] = malloc(256);
            read_value(table_file, schema->columns[col].data_type, row_values[col], 256);
        }
        
        // Now print only the columns that should be displayed
        printf("|");
        for (int i = 0; i < display_count; i++) {
            int col_index = display_columns[i];
            printf(" %-14s |", row_values[col_index]);
        }
        printf("\n");
        
        // Free row values
        for (int col = 0; col < schema->column_count; col++) {
            free(row_values[col]);
        }
        free(row_values);
        
        rows_selected++;
    }
    
    printf("+");
    for (int i = 0; i < display_count; i++) {
        printf("----------------+");
    }
    printf("\n");
    
    printf("%d row(s) selected\n", rows_selected);
    
    free(display_columns);
    fclose(table_file);
    free_schema(schema);
}

void execute_insert(ASTNode* insert) {
    if (current_database[0] == '\0') {
        printf("No database selected. Use 'USE <database>;' first.\n");
        return;
    }
    
    // Get table name from AST
    if (!insert->right || insert->right->type != AST_IDENTIFIER) {
        printf("Invalid INSERT statement\n");
        return;
    }
    
    const char* table_name = insert->right->value;
    
    // Read schema
    TableSchema* schema = read_schema(current_database, table_name);
    if (!schema) {
        printf("Table '%s' does not exist\n", table_name);
        return;
    }
    
    // Get values from AST (stored in table_node->right)
    ASTNode* values_head = insert->right->right;
    if (!values_head) {
        printf("No values provided for INSERT\n");
        free_schema(schema);
        return;
    }
    
    // Count values
    int value_count = 0;
    ASTNode* temp = values_head;
    while (temp) {
        value_count++;
        temp = temp->right;
    }
    
    // Validate value count matches column count
    if (value_count != schema->column_count) {
        printf("Error: Table '%s' expects %d values (", table_name, schema->column_count);
        for (int i = 0; i < schema->column_count; i++) {
            printf("%s", schema->columns[i].column_name);
            if (i < schema->column_count - 1) printf(", ");
        }
        printf("), but got %d\n", value_count);
        free_schema(schema);
        return;
    }
    
    // Open table file for reading and writing
    char table_path[256];
    snprintf(table_path, sizeof(table_path), "databases\\%s\\%s.table", current_database, table_name);
    
    FILE* table_file = fopen(table_path, "r+b");
    if (!table_file) {
        perror("Failed to open table file");
        free_schema(schema);
        return;
    }
    
    // Read current row count
    int row_count;
    fread(&row_count, sizeof(int), 1, table_file);
    
    // Seek to end of file to append new row
    fseek(table_file, 0, SEEK_END);
    
    // Write values in order
    ASTNode* current_value = values_head;
    for (int i = 0; i < schema->column_count; i++) {
        if (!current_value) break;
        
        const char* value_str = current_value->value;
        const char* data_type = schema->columns[i].data_type;
        
        write_value(table_file, value_str, data_type);
        
        current_value = current_value->right;
    }
    
    // Update row count
    row_count++;
    fseek(table_file, 0, SEEK_SET);
    fwrite(&row_count, sizeof(int), 1, table_file);
    
    fclose(table_file);
    free_schema(schema);
    
    printf("1 row inserted into '%s'\n", table_name);
}

/* ============================================
   MAIN EXECUTOR
   ============================================ */

void execute_statement(ASTNode* root) {
    if (!root) return;

    switch (root->type) {
        case AST_CREATE:
            if (root->right && root->right->type == AST_IDENTIFIER) {
                // Check if it's a database or table by looking at the structure
                if (root->right->left) {
                    // Has columns, so it's a table
                    createtable(root);
                } else {
                    // No columns, so it's a database
                    createdatabase(root);
                }
            }
            break;
        case AST_USE:
            use_database(root);
            break;
        case AST_SHOW:
            if (strcmp(root->value, "DATABASES") == 0) {
                show_databases();
            } else if (strcmp(root->value, "TABLES") == 0) {
                show_tables();
            }
            break;
        case AST_SELECT:
            execute_select(root);
            break;
        case AST_INSERT:
            execute_insert(root);
            break;
        default:
            printf("Unknown statement type\n");
    }
}