#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "lexer.h"
#include "parser.h"
#include "vm.h"

// Function to print usage information
void print_usage() {
    printf("Usage: iberypp <command> [arguments]\n");
    printf("Commands:\n");
    printf("  run <file>           - Run an ibery++ file\n");
    printf("  compile <file> <dir> - Compile an ibery++ file to web files\n");
    printf("  host <file>          - Host an ibery++ file\n");
    printf("  terminal             - Start interactive terminal\n");
}

// Function to read file contents
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    size_t read_size = fread(buffer, 1, size, file);
    buffer[read_size] = '\0';
    fclose(file);

    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [arguments]\n", argv[0]);
        fprintf(stderr, "Commands:\n");
        fprintf(stderr, "  compile <input> <output>  Compile ibery++ source to bytecode\n");
        fprintf(stderr, "  run <input>              Run ibery++ source directly\n");
        fprintf(stderr, "  disassemble <input>      Show bytecode for ibery++ source\n");
        return 1;
    }

    const char* command = argv[1];
    VM vm;
    initVM(&vm);

    if (strcmp(command, "compile") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s compile <input> <output>\n", argv[0]);
            return 1;
        }

        char* source = read_file(argv[2]);
        if (!source) return 1;

        // Compile to bytecode
        compile(&vm, source);
        if (vm.parser->hadError) {
            free(source);
            return 1;
        }

        // Write bytecode to file
        FILE* out = fopen(argv[3], "wb");
        if (!out) {
            fprintf(stderr, "Could not open output file.\n");
            free(source);
            return 1;
        }

        write_chunk(out, vm.chunk);
        fclose(out);
        free(source);
        printf("Compiled successfully to %s\n", argv[3]);
        return 0;
    }
    else if (strcmp(command, "run") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s run <input>\n", argv[0]);
            return 1;
        }

        char* source = read_file(argv[2]);
        if (!source) return 1;

        InterpretResult result = interpret(&vm, source);
        free(source);

        if (result == INTERPRET_COMPILE_ERROR) return 65;
        if (result == INTERPRET_RUNTIME_ERROR) return 70;
        return 0;
    }
    else if (strcmp(command, "disassemble") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s disassemble <input>\n", argv[0]);
            return 1;
        }

        char* source = read_file(argv[2]);
        if (!source) return 1;

        compile(&vm, source);
        if (!vm.parser->hadError) {
            disassemble_chunk(vm.chunk, "code");
        }
        free(source);
        return 0;
    }
    else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    freeVM(&vm);
    return 0;
} 