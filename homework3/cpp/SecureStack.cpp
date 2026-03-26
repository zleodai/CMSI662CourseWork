#include "SecureStackCpp.h"

#include <limits>
#include <memory>
#include <new>
#include <utility>

class StackCpp {
public:
    std::size_t capacity;
    std::size_t count;
    std::unique_ptr<std::string> *items;
};

namespace {

StackResultCpp make_stack_result_cpp(const bool success,
                                     const StackStatusCodeCpp code,
                                     const std::string &message) {
    return StackResultCpp{success, code, message};
}

StackCreateResultCpp make_stack_create_result_cpp(const bool success,
                                                  const StackStatusCodeCpp code,
                                                  const std::string &message,
                                                  StackCpp *stack) {
    return StackCreateResultCpp{success, code, message, stack};
}

StackBoolResultCpp make_stack_bool_result_cpp(const bool success,
                                              const StackStatusCodeCpp code,
                                              const std::string &message,
                                              const bool value) {
    return StackBoolResultCpp{success, code, message, value};
}

StackValueResultCpp make_stack_value_result_cpp(const bool success,
                                                const StackStatusCodeCpp code,
                                                const std::string &message,
                                                const std::string &value) {
    return StackValueResultCpp{success, code, message, value};
}

bool stack_has_valid_state(const StackCpp *stack) {
    return stack != nullptr && stack->items != nullptr && stack->capacity > 0U &&
           stack->count <= stack->capacity;
}

void secure_clear_string(std::string &value) {
    for (char &ch : value) {
        ch = '\0';
    }
}

}  // namespace

StackCreateResultCpp stack_create_cpp(const std::size_t capacity) {
    StackCpp *stack = nullptr;

    if (capacity == 0U) {
        return make_stack_create_result_cpp(false, StackStatusCodeCpp::InvalidArgument,
                                            "Stack capacity must be greater than zero.", nullptr);
    }

    if (capacity > (std::numeric_limits<std::size_t>::max() / sizeof(std::unique_ptr<std::string>))) {
        return make_stack_create_result_cpp(false, StackStatusCodeCpp::InvalidArgument,
                                            "Stack capacity is too large to allocate safely.", nullptr);
    }

    stack = new (std::nothrow) StackCpp{};
    if (stack == nullptr) {
        return make_stack_create_result_cpp(false, StackStatusCodeCpp::AllocationFailure,
                                            "Failed to allocate stack object.", nullptr);
    }

    stack->items = new (std::nothrow) std::unique_ptr<std::string>[capacity];
    if (stack->items == nullptr) {
        delete stack;
        return make_stack_create_result_cpp(false, StackStatusCodeCpp::AllocationFailure,
                                            "Failed to allocate stack storage.", nullptr);
    }

    stack->capacity = capacity;
    stack->count = 0U;
    return make_stack_create_result_cpp(true, StackStatusCodeCpp::Ok, "Stack created successfully.", stack);
}

StackResultCpp stack_destroy_cpp(StackCpp *stack) {
    std::size_t index;
    std::size_t limit;

    if (stack == nullptr) {
        return make_stack_result_cpp(false, StackStatusCodeCpp::InvalidArgument, "Stack pointer is null.");
    }

    if (stack->items != nullptr) {
        limit = stack->count;
        if (limit > stack->capacity) {
            limit = stack->capacity;
        }

        for (index = 0U; index < limit; ++index) {
            if (stack->items[index] != nullptr) {
                secure_clear_string(*stack->items[index]);
                stack->items[index].reset();
            }
        }

        delete[] stack->items;
    }

    stack->items = nullptr;
    stack->capacity = 0U;
    stack->count = 0U;
    delete stack;

    return make_stack_result_cpp(true, StackStatusCodeCpp::Ok, "Stack destroyed successfully.");
}

StackResultCpp stack_push_cpp(StackCpp *stack, const char *value) {
    if (stack == nullptr) {
        return make_stack_result_cpp(false, StackStatusCodeCpp::InvalidArgument, "Stack pointer is null.");
    }

    if (!stack_has_valid_state(stack)) {
        return make_stack_result_cpp(false, StackStatusCodeCpp::InvalidState, "Stack is in an invalid state.");
    }

    if (value == nullptr) {
        return make_stack_result_cpp(false, StackStatusCodeCpp::InvalidArgument, "Input value is null.");
    }

    if (stack->count >= stack->capacity) {
        return make_stack_result_cpp(false, StackStatusCodeCpp::Full, "Stack is full.");
    }

    try {
        stack->items[stack->count] = std::make_unique<std::string>(value);
    } catch (...) {
        return make_stack_result_cpp(false, StackStatusCodeCpp::AllocationFailure,
                                     "Failed to allocate a defensive copy of the input value.");
    }

    if (stack->items[stack->count] == nullptr) {
        return make_stack_result_cpp(false, StackStatusCodeCpp::AllocationFailure,
                                     "Failed to allocate a defensive copy of the input value.");
    }

    ++stack->count;
    return make_stack_result_cpp(true, StackStatusCodeCpp::Ok, "Value pushed successfully.");
}

StackValueResultCpp stack_pop_cpp(StackCpp *stack) {
    std::string defensive_copy;

    if (stack == nullptr) {
        return make_stack_value_result_cpp(false, StackStatusCodeCpp::InvalidArgument, "Stack pointer is null.", "");
    }

    if (!stack_has_valid_state(stack)) {
        return make_stack_value_result_cpp(false, StackStatusCodeCpp::InvalidState, "Stack is in an invalid state.", "");
    }

    if (stack->count == 0U) {
        return make_stack_value_result_cpp(false, StackStatusCodeCpp::Empty, "Stack is empty.", "");
    }

    --stack->count;

    try {
        defensive_copy = *(stack->items[stack->count]);
    } catch (...) {
        if (stack->items[stack->count] != nullptr) {
            secure_clear_string(*stack->items[stack->count]);
            stack->items[stack->count].reset();
        }

        return make_stack_value_result_cpp(false, StackStatusCodeCpp::AllocationFailure,
                                           "Failed to allocate a defensive copy of the popped value.", "");
    }

    if (stack->items[stack->count] != nullptr) {
        secure_clear_string(*stack->items[stack->count]);
        stack->items[stack->count].reset();
    }

    return make_stack_value_result_cpp(true, StackStatusCodeCpp::Ok, "Value popped successfully.", defensive_copy);
}

StackBoolResultCpp stack_is_empty_cpp(const StackCpp *stack) {
    if (stack == nullptr) {
        return make_stack_bool_result_cpp(false, StackStatusCodeCpp::InvalidArgument, "Stack pointer is null.", false);
    }

    if (!stack_has_valid_state(stack)) {
        return make_stack_bool_result_cpp(false, StackStatusCodeCpp::InvalidState, "Stack is in an invalid state.", false);
    }

    return make_stack_bool_result_cpp(true, StackStatusCodeCpp::Ok,
                                      "Stack emptiness evaluated successfully.", stack->count == 0U);
}

StackBoolResultCpp stack_is_full_cpp(const StackCpp *stack) {
    if (stack == nullptr) {
        return make_stack_bool_result_cpp(false, StackStatusCodeCpp::InvalidArgument, "Stack pointer is null.", false);
    }

    if (!stack_has_valid_state(stack)) {
        return make_stack_bool_result_cpp(false, StackStatusCodeCpp::InvalidState, "Stack is in an invalid state.", false);
    }

    return make_stack_bool_result_cpp(true, StackStatusCodeCpp::Ok,
                                      "Stack fullness evaluated successfully.", stack->count == stack->capacity);
}
