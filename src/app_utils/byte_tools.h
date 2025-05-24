#ifndef BYTE_TOOLS_H
#define BYTE_TOOLS_H

#include <stdint.h>
#include <stddef.h> // For size_t

/**
 * @brief Converts a byte array to a hexadecimal string.
 *
 * @param byte_array Pointer to the input byte array.
 * @param array_len Length of the byte array.
 * @param hex_string Pointer to the output buffer for the hex string.
 *                   The buffer must be large enough (at least 2 * array_len + 1 bytes).
 * @param string_buf_len Length of the output buffer.
 * @return 0 on success, -1 on error (e.g., output buffer too small).
 */
int bytes_to_hex_string(const uint8_t* byte_array, size_t array_len, char* hex_string, size_t string_buf_len);

#endif // BYTE_TOOLS_H
