#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "buffer.h"

/**
 * Handler for SIGN_TX command. If successfully parse BIP32 path
 * and transaction, sign transaction and send APDU response.
 *
 * @see G_context.bip32_path, G_context.tx_info.raw_transaction,
 * G_context.tx_info.signature and G_context.tx_info.v.
 *
 * @param[in,out] chunk_data
 *   Command data with BIP32 path and raw transaction serialized.
 * @param[in]     chunk_idx
 *   Index number of the APDU chunk_idx.
 * @param[in]       anymore_blocks_after_this_one
 *   Whether anymore_blocks_after_this_one APDU chunk_idx to be received or not.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_sign_tx(buffer_t *chunk_data, uint8_t chunk_idx, bool anymore_blocks_after_this_one);
