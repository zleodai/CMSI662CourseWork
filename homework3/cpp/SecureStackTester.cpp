#include "SecureStackCpp.h"

#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

namespace {

int tests_run = 0;
int tests_failed = 0;

void record_result(const bool condition, const char *test_name) {
    ++tests_run;
    if (condition) {
        std::cout << "[PASS] " << test_name << '\n';
        return;
    }

    ++tests_failed;
    std::cout << "[FAIL] " << test_name << '\n';
}

void test_create_and_destroy() {
    const StackCreateResultCpp create_result = stack_create_cpp(3U);
    StackBoolResultCpp empty_result{false, StackStatusCodeCpp::InvalidState, "", false};
    StackBoolResultCpp full_result{false, StackStatusCodeCpp::InvalidState, "", false};
    StackResultCpp destroy_result{false, StackStatusCodeCpp::InvalidState, ""};

    record_result(create_result.success && create_result.stack != nullptr,
                  "stack_create_cpp returns a success result and non-null stack");
    if (!create_result.success || create_result.stack == nullptr) {
        return;
    }

    empty_result = stack_is_empty_cpp(create_result.stack);
    full_result = stack_is_full_cpp(create_result.stack);

    record_result(empty_result.success && empty_result.value,
                  "stack_is_empty_cpp succeeds for a new stack");
    record_result(full_result.success && !full_result.value,
                  "stack_is_full_cpp succeeds for a new stack");

    destroy_result = stack_destroy_cpp(create_result.stack);
    record_result(destroy_result.success, "stack_destroy_cpp succeeds for a valid stack");

    destroy_result = stack_destroy_cpp(nullptr);
    record_result(!destroy_result.success && destroy_result.code == StackStatusCodeCpp::InvalidArgument,
                  "stack_destroy_cpp fails fast on null input");
}

void test_invalid_create_requests() {
    const StackCreateResultCpp zero_result = stack_create_cpp(0U);
    const StackCreateResultCpp huge_result = stack_create_cpp(std::numeric_limits<std::size_t>::max());
    const StackCreateResultCpp near_overflow_result =
        stack_create_cpp((std::numeric_limits<std::size_t>::max() / sizeof(std::unique_ptr<std::string>)) + 1U);

    record_result(!zero_result.success && zero_result.code == StackStatusCodeCpp::InvalidArgument,
                  "stack_create_cpp rejects zero capacity");
    record_result(!zero_result.message.empty(),
                  "stack_create_cpp provides an error message for zero capacity");
    record_result(!huge_result.success && huge_result.code == StackStatusCodeCpp::InvalidArgument,
                  "stack_create_cpp rejects absurdly large capacity");
    record_result(!near_overflow_result.success && near_overflow_result.code == StackStatusCodeCpp::InvalidArgument,
                  "stack_create_cpp rejects capacity that would overflow item allocation");
}

void test_push_and_pop_lifo() {
    const StackCreateResultCpp create_result = stack_create_cpp(3U);
    StackResultCpp push_result{false, StackStatusCodeCpp::InvalidState, ""};
    StackBoolResultCpp full_result{false, StackStatusCodeCpp::InvalidState, "", false};
    StackBoolResultCpp empty_result{false, StackStatusCodeCpp::InvalidState, "", false};
    StackValueResultCpp top_result{false, StackStatusCodeCpp::InvalidState, "", ""};
    StackValueResultCpp middle_result{false, StackStatusCodeCpp::InvalidState, "", ""};
    StackValueResultCpp bottom_result{false, StackStatusCodeCpp::InvalidState, "", ""};
    StackValueResultCpp underflow_result{false, StackStatusCodeCpp::InvalidState, "", ""};

    record_result(create_result.success && create_result.stack != nullptr,
                  "stack_create_cpp succeeds for LIFO test");
    if (!create_result.success || create_result.stack == nullptr) {
        return;
    }

    push_result = stack_push_cpp(create_result.stack, "first");
    record_result(push_result.success, "stack_push_cpp accepts first element");

    push_result = stack_push_cpp(create_result.stack, "second");
    record_result(push_result.success, "stack_push_cpp accepts second element");

    push_result = stack_push_cpp(create_result.stack, "third");
    record_result(push_result.success, "stack_push_cpp accepts third element");

    full_result = stack_is_full_cpp(create_result.stack);
    record_result(full_result.success && full_result.value,
                  "stack reports full when capacity reached");

    push_result = stack_push_cpp(create_result.stack, "fourth");
    record_result(!push_result.success && push_result.code == StackStatusCodeCpp::Full,
                  "stack_push_cpp rejects overflow");

    top_result = stack_pop_cpp(create_result.stack);
    middle_result = stack_pop_cpp(create_result.stack);
    bottom_result = stack_pop_cpp(create_result.stack);

    record_result(top_result.success && top_result.value == "third",
                  "stack_pop_cpp returns most recent item first");
    record_result(middle_result.success && middle_result.value == "second",
                  "stack_pop_cpp preserves LIFO order");
    record_result(bottom_result.success && bottom_result.value == "first",
                  "stack_pop_cpp returns earliest item last");

    empty_result = stack_is_empty_cpp(create_result.stack);
    record_result(empty_result.success && empty_result.value,
                  "stack is empty after all elements popped");

    underflow_result = stack_pop_cpp(create_result.stack);
    record_result(!underflow_result.success && underflow_result.code == StackStatusCodeCpp::Empty,
                  "stack_pop_cpp returns an empty-stack response on underflow");

    stack_destroy_cpp(create_result.stack);
}

void test_null_argument_handling() {
    StackResultCpp push_result = stack_push_cpp(nullptr, "value");
    StackValueResultCpp pop_result = stack_pop_cpp(nullptr);
    StackBoolResultCpp empty_result = stack_is_empty_cpp(nullptr);
    StackBoolResultCpp full_result = stack_is_full_cpp(nullptr);
    const StackCreateResultCpp create_result = stack_create_cpp(1U);

    record_result(!push_result.success && push_result.code == StackStatusCodeCpp::InvalidArgument,
                  "stack_push_cpp rejects null stack");
    record_result(!push_result.message.empty(),
                  "stack_push_cpp provides an error message for null stack");
    push_result = stack_push_cpp(nullptr, nullptr);
    record_result(!push_result.success && push_result.code == StackStatusCodeCpp::InvalidArgument,
                  "stack_push_cpp rejects null stack and null value");
    record_result(!pop_result.success && pop_result.code == StackStatusCodeCpp::InvalidArgument,
                  "stack_pop_cpp rejects null stack");
    record_result(!empty_result.success && empty_result.code == StackStatusCodeCpp::InvalidArgument,
                  "stack_is_empty_cpp rejects null stack");
    record_result(!full_result.success && full_result.code == StackStatusCodeCpp::InvalidArgument,
                  "stack_is_full_cpp rejects null stack");

    record_result(create_result.success && create_result.stack != nullptr,
                  "stack_create_cpp succeeds for null value test");
    if (create_result.success && create_result.stack != nullptr) {
        push_result = stack_push_cpp(create_result.stack, nullptr);
        record_result(!push_result.success && push_result.code == StackStatusCodeCpp::InvalidArgument,
                      "stack_push_cpp rejects null value on a valid stack");
        record_result(!push_result.message.empty(),
                      "stack_push_cpp provides an error message for null value");
        stack_destroy_cpp(create_result.stack);
    }
}

void test_deep_copy_behavior() {
    const StackCreateResultCpp create_result = stack_create_cpp(2U);
    StackResultCpp push_result{false, StackStatusCodeCpp::InvalidState, ""};
    StackValueResultCpp pop_result{false, StackStatusCodeCpp::InvalidState, "", ""};
    char input[] = "alpha";

    record_result(create_result.success && create_result.stack != nullptr,
                  "stack_create_cpp succeeds for deep-copy test");
    if (!create_result.success || create_result.stack == nullptr) {
        return;
    }

    push_result = stack_push_cpp(create_result.stack, input);
    record_result(push_result.success, "stack_push_cpp accepts mutable caller buffer");
    input[0] = 'X';

    pop_result = stack_pop_cpp(create_result.stack);
    record_result(pop_result.success && !pop_result.value.empty(),
                  "stack_pop_cpp returns stored string after source buffer mutation");
    record_result(pop_result.success && pop_result.value == "alpha",
                  "stack stores a deep copy rather than caller-owned memory");
    record_result(pop_result.success && pop_result.value.c_str() != input,
                  "stack_pop_cpp returns a defensive copy distinct from the caller buffer");

    stack_destroy_cpp(create_result.stack);
}

void test_returned_value_is_defensive_copy() {
    const StackCreateResultCpp create_result = stack_create_cpp(2U);
    StackResultCpp push_result{false, StackStatusCodeCpp::InvalidState, ""};
    StackValueResultCpp first_pop{false, StackStatusCodeCpp::InvalidState, "", ""};
    StackValueResultCpp second_pop{false, StackStatusCodeCpp::InvalidState, "", ""};

    record_result(create_result.success && create_result.stack != nullptr,
                  "stack_create_cpp succeeds for defensive-copy pop test");
    if (!create_result.success || create_result.stack == nullptr) {
        return;
    }

    push_result = stack_push_cpp(create_result.stack, "copy-check");
    record_result(push_result.success, "stack_push_cpp succeeds for defensive-copy pop test");
    if (!push_result.success) {
        stack_destroy_cpp(create_result.stack);
        return;
    }

    first_pop = stack_pop_cpp(create_result.stack);
    record_result(first_pop.success, "first stack_pop_cpp succeeds for defensive-copy pop test");
    if (!first_pop.success) {
        stack_destroy_cpp(create_result.stack);
        return;
    }

    first_pop.value[0] = 'X';
    push_result = stack_push_cpp(create_result.stack, "copy-check");
    record_result(push_result.success, "stack_push_cpp succeeds after caller mutates returned value");

    second_pop = stack_pop_cpp(create_result.stack);
    record_result(second_pop.success && second_pop.value == "copy-check",
                  "caller mutation of returned value does not affect future stack values");

    stack_destroy_cpp(create_result.stack);
}

void test_string_edge_cases() {
    const StackCreateResultCpp create_result = stack_create_cpp(2U);
    StackResultCpp push_result{false, StackStatusCodeCpp::InvalidState, ""};
    StackValueResultCpp long_result{false, StackStatusCodeCpp::InvalidState, "", ""};
    StackValueResultCpp empty_result{false, StackStatusCodeCpp::InvalidState, "", ""};
    char long_input[1024];

    std::memset(long_input, 'A', sizeof(long_input) - 1U);
    long_input[sizeof(long_input) - 1U] = '\0';

    record_result(create_result.success && create_result.stack != nullptr,
                  "stack_create_cpp succeeds for string edge-case test");
    if (!create_result.success || create_result.stack == nullptr) {
        return;
    }

    push_result = stack_push_cpp(create_result.stack, "");
    record_result(push_result.success, "stack_push_cpp accepts empty string");

    push_result = stack_push_cpp(create_result.stack, long_input);
    record_result(push_result.success, "stack_push_cpp accepts long string");

    long_result = stack_pop_cpp(create_result.stack);
    empty_result = stack_pop_cpp(create_result.stack);

    record_result(long_result.success && long_result.value == long_input,
                  "stack_pop_cpp preserves long string contents");
    record_result(empty_result.success && empty_result.value.empty(),
                  "stack_pop_cpp preserves empty string contents");

    stack_destroy_cpp(create_result.stack);
}

void test_destroy_cleans_remaining_items() {
    const StackCreateResultCpp create_result = stack_create_cpp(2U);
    StackResultCpp push_result{false, StackStatusCodeCpp::InvalidState, ""};
    StackResultCpp destroy_result{false, StackStatusCodeCpp::InvalidState, ""};

    record_result(create_result.success && create_result.stack != nullptr,
                  "stack_create_cpp succeeds for destroy cleanup test");
    if (!create_result.success || create_result.stack == nullptr) {
        return;
    }

    push_result = stack_push_cpp(create_result.stack, "secret-1");
    record_result(push_result.success, "stack_push_cpp accepts first cleanup value");

    push_result = stack_push_cpp(create_result.stack, "secret-2");
    record_result(push_result.success, "stack_push_cpp accepts second cleanup value");

    destroy_result = stack_destroy_cpp(create_result.stack);
    record_result(destroy_result.success, "stack_destroy_cpp frees stack with unpopped items");
}

}  // namespace

int main() {
    test_create_and_destroy();
    test_invalid_create_requests();
    test_push_and_pop_lifo();
    test_null_argument_handling();
    test_deep_copy_behavior();
    test_returned_value_is_defensive_copy();
    test_string_edge_cases();
    test_destroy_cleans_remaining_items();

    std::cout << "\nTests run: " << tests_run << '\n';
    std::cout << "Tests failed: " << tests_failed << '\n';

    return tests_failed == 0 ? 0 : 1;
}
