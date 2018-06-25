/**
 * \file tests.h
 *
 * \brief Common testing definitions, structures, and functions.
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
                   " expected " _type " but got " _type, \
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
                   " expected %s but got %s", \
                   __FILE__, \
                   __LINE__, \
                   _enum##_name(_expected), \
                   _enum##_name(_actual)); \
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

#define _TEST(fn) bool test_##fn(RBRInstrument *instrument, \
                                 TestIOBuffers *buffers)
#define TEST_LOGGER2(fn) _TEST(fn)
#define TEST_LOGGER3(fn) _TEST(fn)

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
