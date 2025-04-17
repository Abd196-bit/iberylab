#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

// Forward declarations
static ASTNode* parse_primary(Parser* parser);
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_function_definition(Parser* parser);
static ASTNode* parse_text_statement(Parser* parser);
static ASTNode* parse_game_engine(Parser* parser);

// Helper functions
static void parser_advance(Parser* parser) {
    parser->current = get_next_token(parser->lexer);
}

static void expect(Parser* parser, TokenType type) {
    if (parser->current.type != type) {
        fprintf(stderr, "Expected token type %d but got %d\n", type, parser->current.type);
        parser->had_error = 1;
        return;
    }
    parser_advance(parser);
}

// Create a new parser
Parser* create_parser(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) {
        fprintf(stderr, "Failed to allocate memory for parser\n");
        return NULL;
    }
    parser->lexer = lexer;
    parser->had_error = 0;
    parser_advance(parser); // Load first token
    return parser;
}

// Free parser memory
void free_parser(Parser* parser) {
    if (parser) {
        free(parser);
    }
}

// Create a new AST node
static ASTNode* create_node(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Failed to allocate memory for AST node\n");
        exit(1);
    }
    node->type = type;
    memset(&node->data, 0, sizeof(node->data));
    return node;
}

// Parse a program (sequence of statements)
ASTNode* parse_program(Parser* parser) {
    ASTNode* program = create_node(NODE_PROGRAM);
    program->data.program.statements = NULL;
    program->data.program.statement_count = 0;

    while (!parser->had_error && parser->current.type != TOKEN_EOF) {
        ASTNode* statement = parse_statement(parser);
        if (!statement) break;

        // Add statement to program
        program->data.program.statement_count++;
        program->data.program.statements = realloc(program->data.program.statements,
            program->data.program.statement_count * sizeof(ASTNode*));
        program->data.program.statements[program->data.program.statement_count - 1] = statement;
    }

    return program;
}

// Parse a statement
static ASTNode* parse_statement(Parser* parser) {
    switch (parser->current.type) {
        case TOKEN_FUNCTION:
            return parse_function_definition(parser);
        case TOKEN_TEXT:
            return parse_text_statement(parser);
        case TOKEN_EOF:
            return NULL;
        case TOKEN_IDENTIFIER: {
            ASTNode* node = parse_primary(parser);
            if (!node) return NULL;
            
            expect(parser, TOKEN_SEMICOLON);
            if (parser->had_error) {
                free_ast(node);
                return NULL;
            }
            return node;
        }
        case TOKEN_GAME_ENGINE:
            parser_advance(parser);
            return parse_game_engine(parser);
        default:
            fprintf(stderr, "Unexpected token in statement\n");
            parser->had_error = 1;
            return NULL;
    }
}

// Parse a function definition
static ASTNode* parse_function_definition(Parser* parser) {
    parser_advance(parser); // Consume 'function'
    
    if (parser->current.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Expected function name\n");
        parser->had_error = 1;
        return NULL;
    }
    
    ASTNode* node = create_node(NODE_FUNCTION_DEFINITION);
    node->data.function_definition.name = strdup(parser->current.value.string_val);
    parser_advance(parser);
    
    expect(parser, TOKEN_LBRACE);
    if (parser->had_error) {
        free(node->data.function_definition.name);
        free(node);
        return NULL;
    }
    
    // Parse function body
    node->data.function_definition.body = parse_program(parser);
    
    expect(parser, TOKEN_RBRACE);
    if (parser->had_error) {
        free_ast(node->data.function_definition.body);
        free(node->data.function_definition.name);
        free(node);
        return NULL;
    }
    
    return node;
}

// Parse a text statement
static ASTNode* parse_text_statement(Parser* parser) {
    parser_advance(parser); // Consume 'text'
    
    if (parser->current.type != TOKEN_STRING) {
        fprintf(stderr, "Expected string after 'text'\n");
        parser->had_error = 1;
        return NULL;
    }
    
    ASTNode* node = create_node(NODE_TEXT);
    node->data.text.content = strdup(parser->current.value.string_val);
    parser_advance(parser);
    
    expect(parser, TOKEN_SEMICOLON);
    if (parser->had_error) {
        free_ast(node);
        return NULL;
    }
    
    return node;
}

// Parse a primary expression
static ASTNode* parse_primary(Parser* parser) {
    Token token = parser->current;
    parser_advance(parser);
    
    switch (token.type) {
        case TOKEN_STRING: {
            ASTNode* node = create_node(NODE_STRING_LITERAL);
            node->data.string_literal.value = strdup(token.value.string_val);
            return node;
        }
        case TOKEN_NUMBER: {
            ASTNode* node = create_node(NODE_NUMBER);
            node->data.number.value = token.value.number_val;
            return node;
        }
        case TOKEN_INPUT: {
            ASTNode* node = create_node(NODE_INPUT);
            expect(parser, TOKEN_LBRACE);
            if (parser->had_error) {
                free(node);
                return NULL;
            }
            
            if (parser->current.type != TOKEN_STRING) {
                fprintf(stderr, "Expected string in input statement\n");
                parser->had_error = 1;
                free(node);
                return NULL;
            }
            
            node->data.input.prompt = strdup(parser->current.value.string_val);
            parser_advance(parser);
            
            expect(parser, TOKEN_RBRACE);
            if (parser->had_error) {
                free(node->data.input.prompt);
                free(node);
                return NULL;
            }
            
            return node;
        }
        case TOKEN_IDENTIFIER: {
            if (parser->current.type == TOKEN_LBRACE) {
                parser_advance(parser);
                if (parser->current.type == TOKEN_NUM) {
                    parser_advance(parser);
                    expect(parser, TOKEN_RBRACE);
                    if (parser->had_error) return NULL;
                    
                    ASTNode* node = create_node(NODE_NUMBER_CONVERSION);
                    node->data.number_conversion.expr = parse_expression(parser);
                    return node;
                }
            }
            ASTNode* node = create_node(NODE_IDENTIFIER);
            node->data.identifier.name = strdup(token.value.string_val);
            return node;
        }
        default:
            fprintf(stderr, "Unexpected token in primary expression\n");
            parser->had_error = 1;
            return NULL;
    }
}

// Parse a game engine statement
static ASTNode* parse_game_engine(Parser* parser) {
    ASTNode* node = create_node(NODE_GAME_ENGINE);
    
    expect(parser, TOKEN_LBRACE);
    if (parser->had_error) {
        free(node);
        return NULL;
    }
    
    // Parse the expression first
    node->data.game_engine.expr = parse_expression(parser);
    if (!node->data.game_engine.expr) {
        free(node);
        return NULL;
    }
    
    // Initialize animations array
    node->data.game_engine.animations = NULL;
    node->data.game_engine.animation_count = 0;
    
    // Parse animations if they exist
    while (parser->current.type != TOKEN_RBRACE && !parser->had_error) {
        ASTNode* animation = parse_animation(parser);
        if (!animation) {
            // Free any previously allocated animations
            for (int i = 0; i < node->data.game_engine.animation_count; i++) {
                free_ast(node->data.game_engine.animations[i]);
            }
            free(node->data.game_engine.animations);
            free_ast(node->data.game_engine.expr);
            free(node);
            return NULL;
        }
        
        // Add animation to the list
        node->data.game_engine.animation_count++;
        node->data.game_engine.animations = realloc(node->data.game_engine.animations, 
            node->data.game_engine.animation_count * sizeof(ASTNode*));
        node->data.game_engine.animations[node->data.game_engine.animation_count - 1] = animation;
    }
    
    expect(parser, TOKEN_RBRACE);
    if (parser->had_error) {
        // Free all animations and expression
        for (int i = 0; i < node->data.game_engine.animation_count; i++) {
            free_ast(node->data.game_engine.animations[i]);
        }
        free(node->data.game_engine.animations);
        free_ast(node->data.game_engine.expr);
        free(node);
        return NULL;
    }
    
    return node;
}

// Add parse_animation function
static ASTNode* parse_animation(Parser* parser) {
    ASTNode* node = create_node(NODE_ANIMATION);
    
    // Parse emoji
    if (parser->current.type != TOKEN_STRING) {
        fprintf(stderr, "Expected emoji string\n");
        parser->had_error = 1;
        free(node);
        return NULL;
    }
    node->data.animation.emoji = strdup(parser->current.value.string_val);
    parser_advance(parser);
    
    // Parse action
    if (parser->current.type != TOKEN_STRING) {
        fprintf(stderr, "Expected action string\n");
        parser->had_error = 1;
        free(node->data.animation.emoji);
        free(node);
        return NULL;
    }
    node->data.animation.action = strdup(parser->current.value.string_val);
    parser_advance(parser);
    
    // Parse distance
    if (parser->current.type != TOKEN_NUMBER) {
        fprintf(stderr, "Expected distance number\n");
        parser->had_error = 1;
        free(node->data.animation.emoji);
        free(node->data.animation.action);
        free(node);
        return NULL;
    }
    node->data.animation.distance = parser->current.value.number_val;
    parser_advance(parser);
    
    // Parse repeat (optional)
    node->data.animation.repeat = 1; // Default value
    if (parser->current.type == TOKEN_NUMBER) {
        node->data.animation.repeat = parser->current.value.number_val;
        parser_advance(parser);
    }
    
    // Parse speed (optional)
    node->data.animation.speed = 1; // Default value
    if (parser->current.type == TOKEN_NUMBER) {
        node->data.animation.speed = parser->current.value.number_val;
        parser_advance(parser);
    }
    
    return node;
}

// Parse an expression
static ASTNode* parse_expression(Parser* parser) {
    return parse_primary(parser);
}

// Free an AST node and its children
void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.statement_count; i++) {
                free_ast(node->data.program.statements[i]);
            }
            free(node->data.program.statements);
            break;
        case NODE_FUNCTION_DEFINITION:
            free(node->data.function_definition.name);
            free_ast(node->data.function_definition.body);
            break;
        case NODE_TEXT:
            free(node->data.text.content);
            break;
        case NODE_STRING_LITERAL:
            free(node->data.string_literal.value);
            break;
        case NODE_IDENTIFIER:
            free(node->data.identifier.name);
            break;
        case NODE_INPUT:
            free(node->data.input.prompt);
            break;
        case NODE_NUMBER_CONVERSION:
            free_ast(node->data.number_conversion.expr);
            break;
        case NODE_GAME_ENGINE:
            free_ast(node->data.game_engine.expr);
            for (int i = 0; i < node->data.game_engine.animation_count; i++) {
                free_ast(node->data.game_engine.animations[i]);
            }
            free(node->data.game_engine.animations);
            break;
        default:
            break;
    }
    
    free(node);
}

// Print AST for debugging
void print_ast(ASTNode* node, int depth) {
    if (!node) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program (%d statements)\n", node->data.program.statement_count);
            for (int i = 0; i < node->data.program.statement_count; i++) {
                print_ast(node->data.program.statements[i], depth + 1);
            }
            break;
        case NODE_FUNCTION_DEFINITION:
            printf("Function: %s\n", node->data.function_definition.name);
            print_ast(node->data.function_definition.body, depth + 1);
            break;
        case NODE_TEXT:
            printf("Text: %s\n", node->data.text.content);
            break;
        case NODE_STRING_LITERAL:
            printf("String: %s\n", node->data.string_literal.value);
            break;
        case NODE_NUMBER:
            printf("Number: %f\n", node->data.number.value);
            break;
        case NODE_IDENTIFIER:
            printf("Identifier: %s\n", node->data.identifier.name);
            break;
        case NODE_INPUT:
            printf("Input: %s\n", node->data.input.prompt);
            break;
        case NODE_NUMBER_CONVERSION:
            printf("Number conversion:\n");
            print_ast(node->data.number_conversion.expr, depth + 1);
            break;
        case NODE_GAME_ENGINE:
            printf("Game engine:\n");
            print_ast(node->data.game_engine.expr, depth + 1);
            break;
        default:
            printf("Unknown node type\n");
            break;
    }
} 