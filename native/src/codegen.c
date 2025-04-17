#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Forward declarations
static void generate_expression(FILE* output, ASTNode* node);
static void generate_statement(FILE* output, ASTNode* node, int indent);
static void generate_block(FILE* output, ASTNode* node, int indent);

static void generate_indent(FILE* output, int indent) {
    for (int i = 0; i < indent; i++) {
        fprintf(output, "    ");
    }
}

static void generate_expression(FILE* output, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_NUMBER:
            fprintf(output, "%f", node->data.number.value);
            break;
        case NODE_STRING:
            fprintf(output, "\"%s\"", node->data.string.value);
            break;
        case NODE_VARIABLE:
            fprintf(output, "%s", node->data.variable.name);
            break;
        case NODE_BINARY_OP:
            fprintf(output, "(");
            generate_expression(output, node->data.binary_op.left);
            fprintf(output, " %c ", node->data.binary_op.op);
            generate_expression(output, node->data.binary_op.right);
            fprintf(output, ")");
            break;
        case NODE_FUNCTION_CALL:
            fprintf(output, "%s(", node->data.function_call.name);
            for (int i = 0; i < node->data.function_call.arg_count; i++) {
                generate_expression(output, node->data.function_call.args[i]);
                if (i < node->data.function_call.arg_count - 1) {
                    fprintf(output, ", ");
                }
            }
            fprintf(output, ")");
            break;
        case NODE_STRING_CONCAT:
            fprintf(output, "strcat(strcat(");
            generate_expression(output, node->data.string_concat.left);
            fprintf(output, ", ");
            generate_expression(output, node->data.string_concat.right);
            fprintf(output, "), \"\")");
            break;
        default:
            fprintf(stderr, "Unsupported expression type in code generation\n");
            exit(1);
    }
}

static void generate_statement(FILE* output, ASTNode* node, int indent) {
    if (!node) return;
    
    generate_indent(output, indent);
    
    switch (node->type) {
        case NODE_ASSIGNMENT:
            fprintf(output, "%s = ", node->data.assignment.var_name);
            generate_expression(output, node->data.assignment.value);
            fprintf(output, ";\n");
            break;
        case NODE_IF:
            fprintf(output, "if (");
            generate_expression(output, node->data.if_stmt.condition);
            fprintf(output, ") {\n");
            generate_block(output, node->data.if_stmt.then_branch, indent + 1);
            generate_indent(output, indent);
            fprintf(output, "}");
            if (node->data.if_stmt.else_branch) {
                fprintf(output, " else {\n");
                generate_block(output, node->data.if_stmt.else_branch, indent + 1);
                generate_indent(output, indent);
                fprintf(output, "}");
            }
            fprintf(output, "\n");
            break;
        case NODE_LOOP:
            fprintf(output, "while (");
            generate_expression(output, node->data.loop.condition);
            fprintf(output, ") {\n");
            generate_block(output, node->data.loop.body, indent + 1);
            generate_indent(output, indent);
            fprintf(output, "}\n");
            break;
        case NODE_TEXT:
            fprintf(output, "printf(\"%s\\n\");\n", node->data.text.text);
            break;
        case NODE_INPUT:
            fprintf(output, "printf(\"%s\");\n", node->data.input.prompt);
            fprintf(output, "scanf(\"%%s\", %s);\n", node->data.input.var_name);
            break;
        case NODE_AI_CALL:
            fprintf(output, "// AI call: %s(", node->data.ai_call.function);
            generate_expression(output, node->data.ai_call.args);
            fprintf(output, ");\n");
            break;
        case NODE_UI_CALL:
            fprintf(output, "// UI component: %s(", node->data.ui_call.component);
            generate_expression(output, node->data.ui_call.args);
            fprintf(output, ");\n");
            break;
        case NODE_SUMMARY_CALL:
            fprintf(output, "// Summary: %s(", node->data.summary_call.type);
            generate_expression(output, node->data.summary_call.args);
            fprintf(output, ");\n");
            break;
        case NODE_GRAPH_CALL:
            fprintf(output, "// Graph: %s(", node->data.graph_call.type);
            generate_expression(output, node->data.graph_call.args);
            fprintf(output, ");\n");
            break;
        case NODE_FUNCTION_DEF:
            fprintf(output, "void %s(", node->data.function_def.name);
            for (int i = 0; i < node->data.function_def.param_count; i++) {
                fprintf(output, "char* %s", node->data.function_def.params[i]->data.variable.name);
                if (i < node->data.function_def.param_count - 1) {
                    fprintf(output, ", ");
                }
            }
            fprintf(output, ") {\n");
            generate_block(output, node->data.function_def.body, indent + 1);
            generate_indent(output, indent);
            fprintf(output, "}\n\n");
            break;
        default:
            fprintf(stderr, "Unsupported statement type in code generation\n");
            exit(1);
    }
}

static void generate_block(FILE* output, ASTNode* node, int indent) {
    if (!node || node->type != NODE_BLOCK) return;
    
    for (int i = 0; i < node->data.block.statement_count; i++) {
        generate_statement(output, node->data.block.statements[i], indent);
    }
}

void generate_code(FILE* output, ASTNode* program) {
    if (!program || program->type != NODE_PROGRAM) {
        fprintf(stderr, "Invalid program node\n");
        exit(1);
    }
    
    // Generate header
    fprintf(output, "#include <stdio.h>\n");
    fprintf(output, "#include <stdlib.h>\n");
    fprintf(output, "#include <string.h>\n\n");
    
    // Generate forward declarations for functions
    for (int i = 0; i < program->data.program.child_count; i++) {
        ASTNode* child = program->data.program.children[i];
        if (child->type == NODE_FUNCTION_DEF) {
            fprintf(output, "void %s(", child->data.function_def.name);
            for (int j = 0; j < child->data.function_def.param_count; j++) {
                fprintf(output, "char*");
                if (j < child->data.function_def.param_count - 1) {
                    fprintf(output, ", ");
                }
            }
            fprintf(output, ");\n");
        }
    }
    fprintf(output, "\n");
    
    // Generate main function
    fprintf(output, "int main() {\n");
    
    // Declare variables
    fprintf(output, "    char student1[100], student2[100], student3[100];\n");
    fprintf(output, "    float s1_math, s1_sci, s1_eng, s2_math, s2_sci, s2_eng, s3_math, s3_sci, s3_eng;\n");
    fprintf(output, "    float s1_total, s2_total, s3_total, s1_avg, s2_avg, s3_avg;\n");
    fprintf(output, "    char topper[100];\n");
    fprintf(output, "    float top_score;\n\n");
    
    // Generate program statements
    for (int i = 0; i < program->data.program.child_count; i++) {
        ASTNode* child = program->data.program.children[i];
        if (child->type != NODE_FUNCTION_DEF) {
            generate_statement(output, child, 1);
        }
    }
    
    fprintf(output, "    return 0;\n");
    fprintf(output, "}\n\n");
    
    // Generate function definitions
    for (int i = 0; i < program->data.program.child_count; i++) {
        ASTNode* child = program->data.program.children[i];
        if (child->type == NODE_FUNCTION_DEF) {
            generate_statement(output, child, 0);
        }
    }
}

static void create_output_directory(const char* dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0700);
    }
}

static void write_indent(CodeGenContext* ctx) {
    for (int i = 0; i < ctx->indent_level; i++) {
        fprintf(ctx->html_file, "  ");
    }
}

static void write_html_tag(CodeGenContext* ctx, const char* tag, const char* content) {
    write_indent(ctx);
    fprintf(ctx->html_file, "<%s>%s</%s>\n", tag, content, tag);
}

static void write_css_rule(CodeGenContext* ctx, const char* selector, const char* properties) {
    write_indent(ctx);
    fprintf(ctx->css_file, "%s {\n", selector);
    ctx->indent_level++;
    write_indent(ctx);
    fprintf(ctx->css_file, "%s\n", properties);
    ctx->indent_level--;
    write_indent(ctx);
    fprintf(ctx->css_file, "}\n");
}

static void write_js_function(CodeGenContext* ctx, const char* name, const char* body) {
    write_indent(ctx);
    fprintf(ctx->js_file, "function %s() {\n", name);
    ctx->indent_level++;
    write_indent(ctx);
    fprintf(ctx->js_file, "%s\n", body);
    ctx->indent_level--;
    write_indent(ctx);
    fprintf(ctx->js_file, "}\n");
}

void generate_html(ASTNode* node, CodeGenContext* ctx) {
    switch (node->type) {
        case NODE_CLASS_DEF: {
            // Convert class to HTML component
            write_indent(ctx);
            fprintf(ctx->html_file, "<div class=\"%s\">\n", node->data.class_def.name);
            ctx->indent_level++;
            
            // Generate HTML for fields
            ASTNode* field = node->data.class_def.fields;
            while (field) {
                if (field->type == NODE_FIELD_DEF) {
                    write_html_tag(ctx, "span", field->data.field_def.name);
                }
                field = field->next;
            }
            
            ctx->indent_level--;
            write_indent(ctx);
            fprintf(ctx->html_file, "</div>\n");
            break;
        }
        case NODE_FUNCTION_DEF: {
            // Convert function to HTML event handler
            write_indent(ctx);
            fprintf(ctx->html_file, "<button onclick=\"%s()\">%s</button>\n", 
                   node->data.function_def.name, node->data.function_def.name);
            break;
        }
        // Add more cases for other node types
    }
}

void generate_css(ASTNode* node, CodeGenContext* ctx) {
    switch (node->type) {
        case NODE_CLASS_DEF: {
            // Generate CSS for class
            char selector[256];
            sprintf(selector, ".%s", node->data.class_def.name);
            write_css_rule(ctx, selector, "display: block; margin: 10px; padding: 10px;");
            break;
        }
        // Add more cases for other node types
    }
}

void generate_js(ASTNode* node, CodeGenContext* ctx) {
    switch (node->type) {
        case NODE_FUNCTION_DEF: {
            // Convert function to JavaScript
            char body[1024] = "";
            // TODO: Convert function body to JavaScript
            write_js_function(ctx, node->data.function_def.name, body);
            break;
        }
        // Add more cases for other node types
    }
}

void generate_web_code(ASTNode* ast, const char* output_dir) {
    // Create output directory
    create_output_directory(output_dir);
    
    // Initialize context
    CodeGenContext ctx = {0};
    ctx.output_dir = strdup(output_dir);
    ctx.indent_level = 0;
    
    // Open output files
    char html_path[256], css_path[256], js_path[256];
    sprintf(html_path, "%s/index.html", output_dir);
    sprintf(css_path, "%s/styles.css", output_dir);
    sprintf(js_path, "%s/script.js", output_dir);
    
    ctx.html_file = fopen(html_path, "w");
    ctx.css_file = fopen(css_path, "w");
    ctx.js_file = fopen(js_path, "w");
    
    if (!ctx.html_file || !ctx.css_file || !ctx.js_file) {
        fprintf(stderr, "Error opening output files\n");
        exit(1);
    }
    
    // Write HTML boilerplate
    fprintf(ctx.html_file, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(ctx.html_file, "  <title>Ibery++ App</title>\n");
    fprintf(ctx.html_file, "  <link rel=\"stylesheet\" href=\"styles.css\">\n");
    fprintf(ctx.html_file, "  <script src=\"script.js\"></script>\n");
    fprintf(ctx.html_file, "</head>\n<body>\n");
    
    // Generate code for each node
    ASTNode* current = ast;
    while (current) {
        generate_html(current, &ctx);
        generate_css(current, &ctx);
        generate_js(current, &ctx);
        current = current->next;
    }
    
    // Close HTML
    fprintf(ctx.html_file, "</body>\n</html>\n");
    
    // Close files
    fclose(ctx.html_file);
    fclose(ctx.css_file);
    fclose(ctx.js_file);
    free(ctx.output_dir);
} 