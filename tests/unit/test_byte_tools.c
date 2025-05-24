#include "unity.h"
#include "byte_tools.h" // Module to test
#include <string.h>

void setUp(void) {
    // Set up conditions before each test case
}

void tearDown(void) {
    // Clean up after each test case
}

void test_bytes_to_hex_string_valid(void) {
    uint8_t input[] = {0xDE, 0xAD, 0xBE, 0xEF};
    char output_buffer[20]; // Needs 2*4 + 1 = 9 bytes minimum
    char expected_output[] = "DEADBEEF";

    int result = bytes_to_hex_string(input, sizeof(input), output_buffer, sizeof(output_buffer));

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING(expected_output, output_buffer);
}

void test_bytes_to_hex_string_empty_input(void) {
    uint8_t input[] = {};
    char output_buffer[5];
    char expected_output[] = "";

    int result = bytes_to_hex_string(input, 0, output_buffer, sizeof(output_buffer));

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING(expected_output, output_buffer);
}

void test_bytes_to_hex_string_buffer_too_small(void) {
    uint8_t input[] = {0x01, 0x02, 0x03};
    char output_buffer[5]; // Needs 2*3 + 1 = 7 bytes, this is too small

    int result = bytes_to_hex_string(input, sizeof(input), output_buffer, sizeof(output_buffer));

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_bytes_to_hex_string_null_input_array(void) {
    char output_buffer[5];
    int result = bytes_to_hex_string(NULL, 5, output_buffer, sizeof(output_buffer));
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_bytes_to_hex_string_null_output_buffer(void) {
    uint8_t input[] = {0x01, 0x02};
    int result = bytes_to_hex_string(input, sizeof(input), NULL, 10);
    TEST_ASSERT_EQUAL_INT(-1, result);
}
