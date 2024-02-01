#pragma once

#include "buffer.h"

#include "types.h"

/**
 * Reads as many bytes as possible from the buffer, but at most out_len.
 * Returns the number of bytes read.
 */
size_t buffer_read_bytes(buffer_t *buffer, uint8_t *out, size_t out_len);

/**
 * Reads out_len bytes to the out buffer
 */
bool buffer_read_bytes_precisely(buffer_t *buffer, uint8_t *out, size_t out_len);

/**
 * Reads a contract_address_s from the given buffer.
 */
bool buffer_read_contract_address(buffer_t *buffer, blockchain_address_s *out);

/**
 * Reads a chain_id_t from the given buffer.
 */
bool buffer_read_chain_id(buffer_t *chunk_data, chain_id_t *out);
