#ifndef CLASS_H
#define CLASS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
struct FieldDef;
struct MethodDef;

typedef struct {
    char* name;
    struct FieldDef** fields;
    int field_count;
    struct MethodDef** methods;
    int method_count;
} Class;

// Function prototypes
Class* create_class(const char* name);
void add_field(Class* class, struct FieldDef* field);
void add_method(Class* class, struct MethodDef* method);
void set_superclass(Class* class, Class* superclass);
void free_class(Class* class);

#endif // CLASS_H 