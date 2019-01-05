// Distributed under MIT License, see LICENSE file
// (c) 2018 Zbigniew Skowron, zbychs@gmail.com

#pragma once

#include "zpreprocessor.h"

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

/// Prevents compiler warnings about "unreferenced variable".
#define ZUNUSED(x) (void)(x)

namespace terminal_editor {

/// Example:
///
///   ZTHROW() << "File '" << fileName << "' not found.";
///   ZASSERT(index <= 0) << "Index must be positive, but is: " << index;
///
/// Synopsis:
///
///   GenericException - exception class used as a base for custom exceptions.
///   
///   ZTHROW() << args...;
///   ZTHROW(exc) << args...;
///   Throws exc (or GenericException).
///
///   ZASSERT(cond) << args...;
///   ZASSERT(exc, cond) << args...;
///   If cond is not true throws exc (or GenericException). In both Debug and Release.
///   
///   ZHARDASSERT(cond) << args...;
///   If cond is not true writes message to stderr and calls std::abort(). In both Debug and Release.
///   
///   ZIMPOSSIBLE()
///   Prevents compiler warnings about "not all paths return a value".


/// Generic exception class.
/// Useful also as a base class for custom exceptions.
/// ZTHROW() and ZASSERT() macros below assume that passed exception is a GenericException (or it's subclass).
/// Example derived class:
///   class DerivedException : public GenericException {};
class GenericException : public std::exception {
    std::string message;

public:
    GenericException() {}

    explicit GenericException(std::string message)
        : message(std::move(message))
    {}

    const char* what() const noexcept override {
        return message.c_str();
    }

    GenericException& operator<<(const std::string& msg) {
        message += msg;
        return *this;
    }
};

#define ZHARDASSERT(cond) \
    if (cond) {       \
    } else            \
        (terminal_editor::AssertHelper(__FILE__ "(" ZTOKEN_STRINGIZE(__LINE__) "): Assertion failed. Condition is false: " #cond " ").message)

#define ZIMPOSSIBLE() \
    do { \
        ZHARDASSERT(false); \
        std::abort(); \
    } while (false)


#if ZVA_OPT_SUPPORTED
        
#define ZTHROW(...) (terminal_editor::make_throw_helper(__FILE__ "(" ZTOKEN_STRINGIZE(__LINE__) "): Exception: " __VA_OPT__(,) __VA_ARGS__).message << "")

#define ZASSERT(arg0, ...) \
    if ((arg0) __VA_OPT__(,) __VA_ARGS__) {             \
    } else                  \
        (ZTHROW(__VA_OPT__(arg0)) << "Condition is false: " << (#arg0 __VA_OPT__(,) #__VA_ARGS__) << " ")

#elif defined(_MSC_VER)
        
#define ZTHROW(...) (terminal_editor::make_throw_helper(__FILE__ "(" ZTOKEN_STRINGIZE(__LINE__) "): Exception: ", __VA_ARGS__).message << "")

#define ZASSERT(arg0, ...) \
    if ((arg0), __VA_ARGS__) {             \
    } else                  \
        (terminal_editor::make_throw_helper2(__FILE__ "(" ZTOKEN_STRINGIZE(__LINE__) "): Exception: ", (arg0), __VA_ARGS__).message << "Condition is false: " << terminal_editor::select_helper(#arg0, #__VA_ARGS__ +0) << " ")

#else
        
#define ZTHROW(...) (terminal_editor::make_throw_helper(__FILE__ "(" ZTOKEN_STRINGIZE(__LINE__) "): Exception: ", ## __VA_ARGS__).message << "")

#define ZASSERT(arg0, ...) \
    if ((arg0), ## __VA_ARGS__) {             \
    } else                  \
        (terminal_editor::make_throw_helper2(__FILE__ "(" ZTOKEN_STRINGIZE(__LINE__) "): Exception: ", (arg0), ## __VA_ARGS__).message << "Condition is false: " << terminal_editor::select_helper(#arg0, #__VA_ARGS__) << " ")

#endif

class AssertHelper {
public:
    std::stringstream message;

    AssertHelper(const char* messageBase) {
        message << messageBase;
    }

    [[noreturn]] ~AssertHelper() {
        auto messageStr = message.str();
        std::cerr << messageStr << std::endl;
        std::abort();
    }
};

template<typename Exc>
class ThrowHelper {
public:
    Exc exception;
    std::stringstream message;

    ThrowHelper(ThrowHelper&&) = default;

    ThrowHelper(Exc exception, const char* messageBase)
        : exception(std::move(exception))
    {
        message << messageBase;
    }

    [[noreturn]] ~ThrowHelper() noexcept(false) {
        auto messageStr = message.str();
        exception << std::move(messageStr);
        throw exception;
    }
};

inline ThrowHelper<GenericException> make_throw_helper(const char* messageBase) {
    return ThrowHelper<GenericException>(terminal_editor::GenericException(), messageBase);
}

template<typename Exc>
ThrowHelper<Exc> make_throw_helper(const char* messageBase, Exc exc) {
    return ThrowHelper<Exc>(std::move(exc), messageBase);
}

inline ThrowHelper<GenericException> make_throw_helper2(const char* messageBase, bool cond) {
    ZUNUSED(cond);
    return ThrowHelper<GenericException>(terminal_editor::GenericException(), messageBase);
}

template<typename Exc>
ThrowHelper<Exc> make_throw_helper2(const char* messageBase, Exc exc, bool cond) {
    ZUNUSED(cond);
    return ThrowHelper<Exc>(std::move(exc), messageBase);
}

inline const char* select_helper(const char* arg0, int arg1) {
    ZUNUSED(arg1);
    return arg0;
}

inline const char* select_helper(const char* arg0, const char (&arg1)[1]) {
    ZUNUSED(arg1);
    return arg0;
}

inline const char* select_helper(const char* arg0, const std::string& arg1) {
    ZUNUSED(arg0);
    return arg1.c_str();
}

} // namespace terminal_editor
