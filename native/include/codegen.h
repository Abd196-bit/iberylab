#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include <stdio.h>

// Web code generation context
typedef struct {
    FILE* html_file;
    FILE* css_file;
    FILE* js_file;
    char* output_dir;
    int indent_level;
} CodeGenContext;

// Function declarations
void generate_web_code(ASTNode* ast, const char* output_dir);
void generate_html(ASTNode* node, CodeGenContext* ctx);
void generate_css(ASTNode* node, CodeGenContext* ctx);
void generate_js(ASTNode* node, CodeGenContext* ctx);

// Helper functions
void write_indent(CodeGenContext* ctx);
void write_html_tag(CodeGenContext* ctx, const char* tag, const char* content);
void write_css_rule(CodeGenContext* ctx, const char* selector, const char* properties);
void write_js_function(CodeGenContext* ctx, const char* name, const char* body);

#endif // CODEGEN_H 