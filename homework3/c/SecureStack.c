#include "SecureStack.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct Stack {
    size_t capacity;
    size_t count;
    char **items;
};

static StackResult make_stack_result(int success, StackStatusCode code, const char *message) {
    StackResult result;

    result.success = success;
    result.code = code;
    result.message = message;
    return result;
}

static StackCreateResult make_stack_create_result(int success,
                                                  StackStatusCode code,
                                                  const char *message,
                                                  Stack *stack) {
    StackCreateResult result;

    result.success = success;
    result.code = code;
    result.message = message;
    result.stack = stack;
    return result;
}

static StackBoolResult make_stack_bool_result(int success,
                                              StackStatusCode code,
                                              const char *message,
                                              int value) {
    StackBoolResult result;

    result.success = success;
    result.code = code;
    result.message = message;
    result.value = value;
    return result;
}

static StackValueResult make_stack_value_result(int success,
                                                StackStatusCode code,
                                                const char *message,
                                                char *value) {
    StackValueResult result;

    result.success = success;
    result.code = code;
    result.message = message;
    result.value = value;
    return result;
}

static int stack_has_valid_state(const Stack *s) {
    return s != NULL && s->items != NULL && s->capacity > 0U && s->count <= s->capacity;
}

static void secure_zero(void *data, size_t length) {
    volatile unsigned char *bytes;
    size_t index;

    if (data == NULL) {
        return;
    }

    bytes = (volatile unsigned char *)data;
    for (index = 0U; index < length; index++) {
        bytes[index] = 0U;
    }
}

static void clear_and_free_string(char *value) {
    size_t length;

    if (value == NULL) {
        return;
    }

    length = strlen(value);
    if (length < SIZE_MAX) {
        secure_zero(value, length + 1U);
    }

    free(value);
}

static char *stack_strdup(const char *value) {
    char *copy;
    size_t value_length;
    size_t length;

    if (value == NULL) {
        return NULL;
    }

    value_length = strlen(value);
    if (value_length == SIZE_MAX) {
        return NULL;
    }

    length = value_length + 1U;
    copy = malloc(length);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, value, length);
    return copy;
}

StackCreateResult stack_create(size_t capacity) {
    Stack *stack;

    if (capacity == 0U) {
        return make_stack_create_result(0, STACK_STATUS_INVALID_ARGUMENT,
                                        "Stack capacity must be greater than zero.", NULL);
    }

    if (capacity > (SIZE_MAX / sizeof(char *))) {
        return make_stack_create_result(0, STACK_STATUS_INVALID_ARGUMENT,
                                        "Stack capacity is too large to allocate safely.", NULL);
    }

    stack = malloc(sizeof(*stack));
    if (stack == NULL) {
        return make_stack_create_result(0, STACK_STATUS_ALLOCATION_FAILURE,
                                        "Failed to allocate stack object.", NULL);
    }

    stack->items = calloc(capacity, sizeof(*stack->items));
    if (stack->items == NULL) {
        free(stack);
        return make_stack_create_result(0, STACK_STATUS_ALLOCATION_FAILURE,
                                        "Failed to allocate stack storage.", NULL);
    }

    stack->capacity = capacity;
    stack->count = 0U;

    return make_stack_create_result(1, STACK_STATUS_OK, "Stack created successfully.", stack);
}

StackResult stack_destroy(Stack *s) {
    size_t index;
    size_t limit;

    if (s == NULL) {
        return make_stack_result(0, STACK_STATUS_INVALID_ARGUMENT, "Stack pointer is NULL.");
    }

    if (s->items != NULL) {
        limit = s->count;
        if (limit > s->capacity) {
            limit = s->capacity;
        }

        for (index = 0U; index < limit; index++) {
            clear_and_free_string(s->items[index]);
            s->items[index] = NULL;
        }

        free(s->items);
    }

    s->items = NULL;
    s->capacity = 0U;
    s->count = 0U;
    free(s);

    return make_stack_result(1, STACK_STATUS_OK, "Stack destroyed successfully.");
}

StackResult stack_push(Stack *s, const char *value) {
    char *copy;

    if (s == NULL) {
        return make_stack_result(0, STACK_STATUS_INVALID_ARGUMENT, "Stack pointer is NULL.");
    }

    if (!stack_has_valid_state(s)) {
        return make_stack_result(0, STACK_STATUS_INVALID_STATE, "Stack is in an invalid state.");
    }

    if (value == NULL) {
        return make_stack_result(0, STACK_STATUS_INVALID_ARGUMENT, "Input value is NULL.");
    }

    if (s->count >= s->capacity) {
        return make_stack_result(0, STACK_STATUS_FULL, "Stack is full.");
    }

    copy = stack_strdup(value);
    if (copy == NULL) {
        return make_stack_result(0, STACK_STATUS_ALLOCATION_FAILURE,
                                 "Failed to allocate a defensive copy of the input value.");
    }

    s->items[s->count] = copy;
    s->count++;
    return make_stack_result(1, STACK_STATUS_OK, "Value pushed successfully.");
}

StackValueResult stack_pop(Stack *s) {
    char *stored_value;
    char *returned_value;

    if (s == NULL) {
        return make_stack_value_result(0, STACK_STATUS_INVALID_ARGUMENT, "Stack pointer is NULL.", NULL);
    }

    if (!stack_has_valid_state(s)) {
        return make_stack_value_result(0, STACK_STATUS_INVALID_STATE, "Stack is in an invalid state.", NULL);
    }

    if (s->count == 0U) {
        return make_stack_value_result(0, STACK_STATUS_EMPTY, "Stack is empty.", NULL);
    }

    s->count--;
    stored_value = s->items[s->count];
    s->items[s->count] = NULL;

    returned_value = stack_strdup(stored_value);
    if (returned_value == NULL) {
        clear_and_free_string(stored_value);
        return make_stack_value_result(0, STACK_STATUS_ALLOCATION_FAILURE,
                                       "Failed to allocate a defensive copy of the popped value.", NULL);
    }

    clear_and_free_string(stored_value);
    return make_stack_value_result(1, STACK_STATUS_OK, "Value popped successfully.", returned_value);
}

StackBoolResult stack_is_empty(const Stack *s) {
    if (s == NULL) {
        return make_stack_bool_result(0, STACK_STATUS_INVALID_ARGUMENT, "Stack pointer is NULL.", 0);
    }

    if (!stack_has_valid_state(s)) {
        return make_stack_bool_result(0, STACK_STATUS_INVALID_STATE, "Stack is in an invalid state.", 0);
    }

    return make_stack_bool_result(1, STACK_STATUS_OK, "Stack emptiness evaluated successfully.",
                                  s->count == 0U);
}

StackBoolResult stack_is_full(const Stack *s) {
    if (s == NULL) {
        return make_stack_bool_result(0, STACK_STATUS_INVALID_ARGUMENT, "Stack pointer is NULL.", 0);
    }

    if (!stack_has_valid_state(s)) {
        return make_stack_bool_result(0, STACK_STATUS_INVALID_STATE, "Stack is in an invalid state.", 0);
    }

    return make_stack_bool_result(1, STACK_STATUS_OK, "Stack fullness evaluated successfully.",
                                  s->count == s->capacity);
}
