#include "SecureStack.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_failed = 0;

static void record_result(int condition, const char *test_name) {
    tests_run++;
    if (condition) {
        printf("[PASS] %s\n", test_name);
        return;
    }

    tests_failed++;
    printf("[FAIL] %s\n", test_name);
}

static void test_create_and_destroy(void) {
    StackCreateResult create_result = stack_create(3U);
    StackBoolResult empty_result;
    StackBoolResult full_result;
    StackResult destroy_result;

    record_result(create_result.success && create_result.stack != NULL,
                  "stack_create returns a success result and non-NULL stack");
    if (!create_result.success || create_result.stack == NULL) {
        return;
    }

    empty_result = stack_is_empty(create_result.stack);
    full_result = stack_is_full(create_result.stack);

    record_result(empty_result.success && empty_result.value,
                  "stack_is_empty succeeds for a new stack");
    record_result(full_result.success && !full_result.value,
                  "stack_is_full succeeds for a new stack");

    destroy_result = stack_destroy(create_result.stack);
    record_result(destroy_result.success, "stack_destroy succeeds for a valid stack");

    destroy_result = stack_destroy(NULL);
    record_result(!destroy_result.success && destroy_result.code == STACK_STATUS_INVALID_ARGUMENT,
                  "stack_destroy fails fast on NULL input");
}

static void test_invalid_create_requests(void) {
    StackCreateResult zero_result = stack_create(0U);
    StackCreateResult huge_result = stack_create(SIZE_MAX);
    StackCreateResult near_overflow_result = stack_create((SIZE_MAX / sizeof(char *)) + 1U);

    record_result(!zero_result.success && zero_result.code == STACK_STATUS_INVALID_ARGUMENT,
                  "stack_create rejects zero capacity");
    record_result(zero_result.message != NULL && strlen(zero_result.message) > 0U,
                  "stack_create provides an error message for zero capacity");
    record_result(!huge_result.success && huge_result.code == STACK_STATUS_INVALID_ARGUMENT,
                  "stack_create rejects absurdly large capacity");
    record_result(!near_overflow_result.success && near_overflow_result.code == STACK_STATUS_INVALID_ARGUMENT,
                  "stack_create rejects capacity that would overflow item allocation");
}

static void test_push_and_pop_lifo(void) {
    StackCreateResult create_result = stack_create(3U);
    StackResult push_result;
    StackBoolResult full_result;
    StackBoolResult empty_result;
    StackValueResult top_result;
    StackValueResult middle_result;
    StackValueResult bottom_result;
    StackValueResult underflow_result;

    record_result(create_result.success && create_result.stack != NULL,
                  "stack_create succeeds for LIFO test");
    if (!create_result.success || create_result.stack == NULL) {
        return;
    }

    push_result = stack_push(create_result.stack, "first");
    record_result(push_result.success, "stack_push accepts first element");

    push_result = stack_push(create_result.stack, "second");
    record_result(push_result.success, "stack_push accepts second element");

    push_result = stack_push(create_result.stack, "third");
    record_result(push_result.success, "stack_push accepts third element");

    full_result = stack_is_full(create_result.stack);
    record_result(full_result.success && full_result.value,
                  "stack reports full when capacity reached");

    push_result = stack_push(create_result.stack, "fourth");
    record_result(!push_result.success && push_result.code == STACK_STATUS_FULL,
                  "stack_push rejects overflow");

    top_result = stack_pop(create_result.stack);
    middle_result = stack_pop(create_result.stack);
    bottom_result = stack_pop(create_result.stack);

    record_result(top_result.success && top_result.value != NULL &&
                      strcmp(top_result.value, "third") == 0,
                  "stack_pop returns most recent item first");
    record_result(middle_result.success && middle_result.value != NULL &&
                      strcmp(middle_result.value, "second") == 0,
                  "stack_pop preserves LIFO order");
    record_result(bottom_result.success && bottom_result.value != NULL &&
                      strcmp(bottom_result.value, "first") == 0,
                  "stack_pop returns earliest item last");

    empty_result = stack_is_empty(create_result.stack);
    record_result(empty_result.success && empty_result.value,
                  "stack is empty after all elements popped");

    underflow_result = stack_pop(create_result.stack);
    record_result(!underflow_result.success && underflow_result.code == STACK_STATUS_EMPTY,
                  "stack_pop returns an empty-stack response on underflow");

    free(top_result.value);
    free(middle_result.value);
    free(bottom_result.value);
    stack_destroy(create_result.stack);
}

static void test_null_argument_handling(void) {
    StackResult push_result = stack_push(NULL, "value");
    StackValueResult pop_result = stack_pop(NULL);
    StackBoolResult empty_result = stack_is_empty(NULL);
    StackBoolResult full_result = stack_is_full(NULL);
    StackCreateResult create_result = stack_create(1U);

    record_result(!push_result.success && push_result.code == STACK_STATUS_INVALID_ARGUMENT,
                  "stack_push rejects NULL stack");
    record_result(push_result.message != NULL && strlen(push_result.message) > 0U,
                  "stack_push provides an error message for NULL stack");
    push_result = stack_push(NULL, NULL);
    record_result(!push_result.success && push_result.code == STACK_STATUS_INVALID_ARGUMENT,
                  "stack_push rejects NULL stack and NULL value");
    record_result(!pop_result.success && pop_result.code == STACK_STATUS_INVALID_ARGUMENT,
                  "stack_pop rejects NULL stack");
    record_result(!empty_result.success && empty_result.code == STACK_STATUS_INVALID_ARGUMENT,
                  "stack_is_empty rejects NULL stack");
    record_result(!full_result.success && full_result.code == STACK_STATUS_INVALID_ARGUMENT,
                  "stack_is_full rejects NULL stack");

    record_result(create_result.success && create_result.stack != NULL,
                  "stack_create succeeds for NULL value test");
    if (create_result.success && create_result.stack != NULL) {
        push_result = stack_push(create_result.stack, NULL);
        record_result(!push_result.success && push_result.code == STACK_STATUS_INVALID_ARGUMENT,
                      "stack_push rejects NULL value on a valid stack");
        record_result(push_result.message != NULL && strlen(push_result.message) > 0U,
                      "stack_push provides an error message for NULL value");
        stack_destroy(create_result.stack);
    }
}

static void test_deep_copy_behavior(void) {
    StackCreateResult create_result = stack_create(2U);
    StackResult push_result;
    StackValueResult pop_result;
    char input[] = "alpha";

    record_result(create_result.success && create_result.stack != NULL,
                  "stack_create succeeds for deep-copy test");
    if (!create_result.success || create_result.stack == NULL) {
        return;
    }

    push_result = stack_push(create_result.stack, input);
    record_result(push_result.success, "stack_push accepts mutable caller buffer");
    input[0] = 'X';

    pop_result = stack_pop(create_result.stack);
    record_result(pop_result.success && pop_result.value != NULL,
                  "stack_pop returns stored string after source buffer mutation");
    record_result(pop_result.success && pop_result.value != NULL &&
                      strcmp(pop_result.value, "alpha") == 0,
                  "stack stores a deep copy rather than caller-owned memory");
    record_result(pop_result.success && pop_result.value != NULL &&
                      pop_result.value != input,
                  "stack_pop returns a defensive copy distinct from the caller buffer");

    free(pop_result.value);
    stack_destroy(create_result.stack);
}

static void test_returned_value_is_defensive_copy(void) {
    StackCreateResult create_result = stack_create(2U);
    StackResult push_result;
    StackValueResult first_pop;
    StackValueResult second_pop;

    record_result(create_result.success && create_result.stack != NULL,
                  "stack_create succeeds for defensive-copy pop test");
    if (!create_result.success || create_result.stack == NULL) {
        return;
    }

    push_result = stack_push(create_result.stack, "copy-check");
    record_result(push_result.success, "stack_push succeeds for defensive-copy pop test");
    if (!push_result.success) {
        stack_destroy(create_result.stack);
        return;
    }

    first_pop = stack_pop(create_result.stack);
    record_result(first_pop.success && first_pop.value != NULL,
                  "first stack_pop succeeds for defensive-copy pop test");
    if (!first_pop.success || first_pop.value == NULL) {
        stack_destroy(create_result.stack);
        return;
    }

    first_pop.value[0] = 'X';
    push_result = stack_push(create_result.stack, "copy-check");
    record_result(push_result.success, "stack_push succeeds after caller mutates returned value");

    second_pop = stack_pop(create_result.stack);
    record_result(second_pop.success && second_pop.value != NULL &&
                      strcmp(second_pop.value, "copy-check") == 0,
                  "caller mutation of returned value does not affect future stack values");

    free(first_pop.value);
    free(second_pop.value);
    stack_destroy(create_result.stack);
}

static void test_string_edge_cases(void) {
    StackCreateResult create_result = stack_create(2U);
    StackResult push_result;
    StackValueResult long_result;
    StackValueResult empty_result;
    char long_input[1024];

    memset(long_input, 'A', sizeof(long_input) - 1U);
    long_input[sizeof(long_input) - 1U] = '\0';

    record_result(create_result.success && create_result.stack != NULL,
                  "stack_create succeeds for string edge-case test");
    if (!create_result.success || create_result.stack == NULL) {
        return;
    }

    push_result = stack_push(create_result.stack, "");
    record_result(push_result.success, "stack_push accepts empty string");

    push_result = stack_push(create_result.stack, long_input);
    record_result(push_result.success, "stack_push accepts long string");

    long_result = stack_pop(create_result.stack);
    empty_result = stack_pop(create_result.stack);

    record_result(long_result.success && long_result.value != NULL &&
                      strcmp(long_result.value, long_input) == 0,
                  "stack_pop preserves long string contents");
    record_result(empty_result.success && empty_result.value != NULL &&
                      strcmp(empty_result.value, "") == 0,
                  "stack_pop preserves empty string contents");

    free(long_result.value);
    free(empty_result.value);
    stack_destroy(create_result.stack);
}

static void test_destroy_cleans_remaining_items(void) {
    StackCreateResult create_result = stack_create(2U);
    StackResult push_result;
    StackResult destroy_result;

    record_result(create_result.success && create_result.stack != NULL,
                  "stack_create succeeds for destroy cleanup test");
    if (!create_result.success || create_result.stack == NULL) {
        return;
    }

    push_result = stack_push(create_result.stack, "secret-1");
    record_result(push_result.success, "stack_push accepts first cleanup value");

    push_result = stack_push(create_result.stack, "secret-2");
    record_result(push_result.success, "stack_push accepts second cleanup value");

    destroy_result = stack_destroy(create_result.stack);
    record_result(destroy_result.success, "stack_destroy frees stack with unpopped items");
}

int main(void) {
    test_create_and_destroy();
    test_invalid_create_requests();
    test_push_and_pop_lifo();
    test_null_argument_handling();
    test_deep_copy_behavior();
    test_returned_value_is_defensive_copy();
    test_string_edge_cases();
    test_destroy_cleans_remaining_items();

    printf("\nTests run: %d\n", tests_run);
    printf("Tests failed: %d\n", tests_failed);

    return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
