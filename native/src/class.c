#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ASTNode* parse_class_definition(FILE* source) {
    next_token(source); // consume 'class'
    if (current_token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Expected class name at line %d\n", current_token->line_number);
        exit(1);
    }
    
    ASTNode* node = create_node(NODE_CLASS_DEF);
    node->data.class_def.name = strdup(current_token->value.string_val);
    node->data.class_def.fields = NULL;
    node->data.class_def.methods = NULL;
    node->data.class_def.field_count = 0;
    node->data.class_def.method_count = 0;
    
    next_token(source);
    if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword == KW_EXTENDS) {
        next_token(source);
        if (current_token->type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "Expected superclass name at line %d\n", current_token->line_number);
            exit(1);
        }
        node->data.class_def.superclass = strdup(current_token->value.string_val);
        next_token(source);
    } else {
        node->data.class_def.superclass = NULL;
    }
    
    if (current_token->type != TOKEN_LBRACE) {
        fprintf(stderr, "Expected '{' after class definition at line %d\n", current_token->line_number);
        exit(1);
    }
    
    next_token(source);
    while (current_token->type != TOKEN_RBRACE) {
        if (current_token->type == TOKEN_KEYWORD) {
            switch (current_token->value.keyword) {
                case KW_PUBLIC:
                case KW_PRIVATE:
                case KW_PROTECTED:
                case KW_STATIC:
                case KW_FINAL: {
                    int modifier = current_token->value.keyword;
                    next_token(source);
                    
                    if (current_token->type == TOKEN_IDENTIFIER) {
                        // Field declaration
                        node->data.class_def.field_count++;
                        node->data.class_def.fields = realloc(node->data.class_def.fields,
                            node->data.class_def.field_count * sizeof(ASTNode*));
                        
                        ASTNode* field = create_node(NODE_FIELD_DEF);
                        field->data.field_def.modifier = modifier;
                        field->data.field_def.name = strdup(current_token->value.string_val);
                        field->data.field_def.type = strdup("auto"); // Type inference
                        
                        next_token(source);
                        if (current_token->type == TOKEN_OPERATOR && current_token->value.operator == '=') {
                            next_token(source);
                            field->data.field_def.initializer = parse_expression(source);
                        } else {
                            field->data.field_def.initializer = NULL;
                        }
                        
                        node->data.class_def.fields[node->data.class_def.field_count - 1] = field;
                    } else if (current_token->type == TOKEN_KEYWORD && 
                              current_token->value.keyword == KW_FUNCTION) {
                        // Method declaration
                        node->data.class_def.method_count++;
                        node->data.class_def.methods = realloc(node->data.class_def.methods,
                            node->data.class_def.method_count * sizeof(ASTNode*));
                        
                        ASTNode* method = parse_function_def(source);
                        method->data.function_def.modifier = modifier;
                        node->data.class_def.methods[node->data.class_def.method_count - 1] = method;
                    }
                    break;
                }
                case KW_FUNCTION: {
                    // Public method by default
                    node->data.class_def.method_count++;
                    node->data.class_def.methods = realloc(node->data.class_def.methods,
                        node->data.class_def.method_count * sizeof(ASTNode*));
                    
                    ASTNode* method = parse_function_def(source);
                    method->data.function_def.modifier = KW_PUBLIC;
                    node->data.class_def.methods[node->data.class_def.method_count - 1] = method;
                    break;
                }
                default:
                    fprintf(stderr, "Unexpected keyword in class definition at line %d\n", 
                            current_token->line_number);
                    exit(1);
            }
        } else {
            fprintf(stderr, "Unexpected token in class definition at line %d\n", 
                    current_token->line_number);
            exit(1);
        }
    }
    
    next_token(source);
    return node;
}

static ASTNode* parse_object_creation(FILE* source) {
    next_token(source); // consume 'new'
    if (current_token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Expected class name after 'new' at line %d\n", current_token->line_number);
        exit(1);
    }
    
    ASTNode* node = create_node(NODE_OBJECT_CREATION);
    node->data.object_creation.class_name = strdup(current_token->value.string_val);
    
    next_token(source);
    if (current_token->type == TOKEN_LPAREN) {
        next_token(source);
        node->data.object_creation.args = parse_expression(source);
        if (current_token->type != TOKEN_RPAREN) {
            fprintf(stderr, "Expected ')' after constructor arguments at line %d\n", 
                    current_token->line_number);
            exit(1);
        }
        next_token(source);
    } else {
        node->data.object_creation.args = NULL;
    }
    
    return node;
}

static ASTNode* parse_method_call(FILE* source) {
    if (current_token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Expected object name at line %d\n", current_token->line_number);
        exit(1);
    }
    
    ASTNode* node = create_node(NODE_METHOD_CALL);
    node->data.method_call.object = strdup(current_token->value.string_val);
    
    next_token(source);
    if (current_token->type != TOKEN_DOT) {
        fprintf(stderr, "Expected '.' after object name at line %d\n", current_token->line_number);
        exit(1);
    }
    
    next_token(source);
    if (current_token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Expected method name at line %d\n", current_token->line_number);
        exit(1);
    }
    
    node->data.method_call.method = strdup(current_token->value.string_val);
    
    next_token(source);
    if (current_token->type == TOKEN_LPAREN) {
        next_token(source);
        node->data.method_call.args = parse_expression(source);
        if (current_token->type != TOKEN_RPAREN) {
            fprintf(stderr, "Expected ')' after method arguments at line %d\n", 
                    current_token->line_number);
            exit(1);
        }
        next_token(source);
    } else {
        node->data.method_call.args = NULL;
    }
    
    return node;
} 