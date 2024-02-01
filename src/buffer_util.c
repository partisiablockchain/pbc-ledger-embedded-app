
#include <string.h>  // memmove
#include "buffer_util.h"

/**
 * Determines the minimum value of two unsigned integers.
 */
static uint32_t min(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

/**
 * Reads as many bytes as possible from the buffer, but at most out_len.
 * Returns the number of bytes read.
 */
size_t buffer_read_bytes(buffer_t *buffer, uint8_t *out, size_t out_len) {
    size_t amount_read = min(out_len, buffer->size - buffer->offset);
    memmove(out, buffer->ptr + buffer->offset, amount_read);
    buffer_seek_cur(buffer, amount_read);
    return amount_read;
}

/**
 * Reads out_len bytes to the out buffer
 */
bool buffer_read_bytes_precisely(buffer_t *buffer, uint8_t *out, size_t out_len) {
    if (!buffer_can_read(buffer, out_len)) {
        return false;
    }

    memmove(out, buffer->ptr + buffer->offset, out_len);
    return buffer_seek_cur(buffer, out_len);
}

/**
 * Reads a contract_address_s from the given buffer.
 */
bool buffer_read_contract_address(buffer_t *buffer, blockchain_address_s *out) {
    return buffer_read_bytes_precisely(buffer, out->raw_bytes, sizeof(out->raw_bytes));
}

bool buffer_read_chain_id(buffer_t *buffer, chain_id_t *out) {
    uint32_t chain_id_len;
    if (!buffer_read_u32(buffer, &chain_id_len, BE)) {
        return false;
    }

    if (chain_id_len != CHAIN_ID_LENGTH) {
        return false;
    }

    return buffer_read_bytes_precisely(buffer, out->raw_bytes, CHAIN_ID_LENGTH);
}
