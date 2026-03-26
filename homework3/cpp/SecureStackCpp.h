#ifndef SECURE_STACK_CPP_H
#define SECURE_STACK_CPP_H

#include <cstddef>
#include <string>

class StackCpp;

enum class StackStatusCodeCpp {
    Ok = 0,
    InvalidArgument,
    InvalidState,
    AllocationFailure,
    Full,
    Empty
};

struct StackResultCpp {
    bool success;
    StackStatusCodeCpp code;
    std::string message;
};

struct StackCreateResultCpp {
    bool success;
    StackStatusCodeCpp code;
    std::string message;
    StackCpp *stack;
};

struct StackBoolResultCpp {
    bool success;
    StackStatusCodeCpp code;
    std::string message;
    bool value;
};

struct StackValueResultCpp {
    bool success;
    StackStatusCodeCpp code;
    std::string message;
    std::string value;
};

StackCreateResultCpp stack_create_cpp(std::size_t capacity);
StackResultCpp stack_destroy_cpp(StackCpp *stack);
StackResultCpp stack_push_cpp(StackCpp *stack, const char *value);
StackValueResultCpp stack_pop_cpp(StackCpp *stack);
StackBoolResultCpp stack_is_empty_cpp(const StackCpp *stack);
StackBoolResultCpp stack_is_full_cpp(const StackCpp *stack);

#endif
