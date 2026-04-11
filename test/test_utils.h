#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        printf("FAIL: %s:%d Expected %d, got %d\n", \
            __FILE__, __LINE__, (expected), (actual)); \
        exit(1); \
    }

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        printf("FAIL: %s:%d Condition failed\n", \
            __FILE__, __LINE__); \
        exit(1); \
    }

#define TEST_PASS() \
    printf("PASS: %s\n", __func__);

#endif