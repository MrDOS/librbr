/**
 * \file tests.h
 *
 * \brief Common testing definitions, structures, and functions.
 *
 * TODO: Documentation. End users shouldn't need to consume anything from this
 * file, but library developers will need to know how to write tests.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_TESTS_H
#define LIBRBR_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRInstrument.h"
/* Required for printf. */
#include <stdio.h>

#define TEST_ASSERT(_condition) do { \
        if (!(_condition)) \
        { \
            printf(" assertion failure at %s:%d", \
                   __FILE__, \
                   __LINE__); \
            return false; \
        } \
} while (0)

#define TEST_ASSERT_EQ(_expected, _actual, _type) do { \
        if ((_expected) != (_actual)) \
        { \
            printf(" assertion failure at %s:%d:" \
                   " expected " _type "; actual " _type, \
                   __FILE__, \
                   __LINE__, \
                   _expected, \
                   _actual); \
            return false; \
        } \
} while (0)

#define TEST_ASSERT_ENUM_EQ(_expected, _actual, _enum) do { \
        if ((_expected) != (_actual)) \
        { \
            printf(" assertion failure at %s:%d:" \
                   " expected %s; actual %s", \
                   __FILE__, \
                   __LINE__, \
                   _enum##_name(_expected), \
                   _enum##_name(_actual)); \
            return false; \
        } \
} while (0)

#define TEST_ASSERT_STR_EQ(_expected, _actual) do { \
        if (strcmp((_expected), (_actual)) != 0) \
        { \
            printf(" assertion failure at %s:%d:\n" \
                   "\texpected \"%s\"\n" \
                   "\t  actual \"%s\"", \
                   __FILE__, \
                   __LINE__, \
                   _expected, \
                   _actual); \
            return false; \
        } \
} while (0)

#define TESTIOBUFFERS_WRITE_BUFFER_SIZE 1024

#define COMMAND_TERMINATOR "\r\n"
#define PROMPT "Ready: "

typedef struct TestIOBuffers
{
    const char *readBuffer;
    int32_t readBufferSize;
    int32_t readBufferPos;
    char writeBuffer[TESTIOBUFFERS_WRITE_BUFFER_SIZE];
    int32_t writeBufferPos;
    RBRInstrumentSample streamSample;
} TestIOBuffers;

/**
 * \brief (Re)initialize a set of test buffers.
 *
 * If \a readBufferSize is given as 0, its length will be determined
 * automatically with `strlen()`.
 *
 * \param [in,out] buffers the buffers to initialize
 * \param [in] readBuffer the new contents of the read buffer
 * \param [in] readBufferSize the length of the read buffer
 */
void TestIOBuffers_init(TestIOBuffers *buffers,
                        const char *readBuffer,
                        int32_t readBufferSize);

/** \brief Enables the use of TEST_ASSERT_ENUM_EQ for boolean values. */
const char *bool_name(bool value);

/* Uncrustify thinks that asterisks in macros are multiplication operators and
 * incorrectly adds spacing, so we'll turn *INDENT-OFF* just for this. */
#define _TEST(fn) bool test_##fn(RBRInstrument *instrument, \
                                 TestIOBuffers *buffers)
/* *INDENT-ON* */
#define TEST_LOGGER2(fn) _TEST(fn##_l2)
#define TEST_LOGGER3(fn) _TEST(fn##_l3)

typedef bool (TestFunction)(RBRInstrument *instrument, TestIOBuffers *buffers);
typedef struct TestDeclaration
{
    char *name;
    RBRInstrumentGeneration generation;
    TestFunction *function;
} TestDeclaration;

extern TestDeclaration tests[];

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_TESTS_H */
