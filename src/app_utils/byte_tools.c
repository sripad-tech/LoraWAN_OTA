#include "byte_tools.h"
#include <stdio.h> // For snprintf

int bytes_to_hex_string(const uint8_t* byte_array, size_t array_len, char* hex_string, size_t string_buf_len) {
    if (byte_array == NULL || hex_string == NULL) {
        return -1;
    }
    if (string_buf_len < (array_len * 2 + 1)) {
        return -1; // Output buffer too small
    }

    for (size_t i = 0; i < array_len; ++i) {
        snprintf(&hex_string[i * 2], 3, "%02X", byte_array[i]);
    }
    hex_string[array_len * 2] = '\0'; // Null-terminate

    return 0;
}
