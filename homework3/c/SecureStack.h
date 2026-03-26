#ifndef SECURE_STACK_H
#define SECURE_STACK_H

#include <stddef.h>

typedef struct Stack Stack;

typedef enum {
    STACK_STATUS_OK = 0,
    STACK_STATUS_INVALID_ARGUMENT,
    STACK_STATUS_INVALID_STATE,
    STACK_STATUS_ALLOCATION_FAILURE,
    STACK_STATUS_FULL,
    STACK_STATUS_EMPTY
} StackStatusCode;

typedef struct {
    int success;
    StackStatusCode code;
    const char *message;
} StackResult;

typedef struct {
    int success;
    StackStatusCode code;
    const char *message;
    Stack *stack;
} StackCreateResult;

typedef struct {
    int success;
    StackStatusCode code;
    const char *message;
    int value;
} StackBoolResult;

typedef struct {
    int success;
    StackStatusCode code;
    const char *message;
    char *value;
} StackValueResult;

StackCreateResult stack_create(size_t capacity);
StackResult stack_destroy(Stack *s);
StackResult stack_push(Stack *s, const char *value);
StackValueResult stack_pop(Stack *s);
StackBoolResult stack_is_empty(const Stack *s);
StackBoolResult stack_is_full(const Stack *s);

#endif
