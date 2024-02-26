
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
 *
 * Cannot overflow the input buffer, and will not ready anything from an
 * already overflown buffer.
 *
 * Returns the number of bytes read.
 */
size_t buffer_read_bytes(buffer_t *buffer, uint8_t *out, size_t out_len) {
    // Choose an amount to read that cannot overflow unless it is already
    // overflown
    size_t amount_read = min(out_len, buffer->size - buffer->offset);

    bool read_success = buffer_read_bytes_precisely(buffer, out, amount_read);
    return read_success ? amount_read : 0;
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

    if (chain_id_len > CHAIN_ID_MAX_LENGTH) {
        return false;
    }
    out->length = (uint8_t) chain_id_len;
    return buffer_read_bytes_precisely(buffer, out->raw_bytes, chain_id_len);
}
