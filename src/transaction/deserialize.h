#pragma once

#include "buffer.h"

#include "types.h"

/**
 * Initializes the transaction parse.
 */
void transaction_parser_init(transaction_parsing_state_t* state);

/**
 * Deserialize raw transaction in structure.
 *
 * @param[in, out] chunk
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx
 *   Pointer to transaction structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e transaction_parser_update(transaction_parsing_state_t* state, buffer_t *chunk, transaction_t *tx);
