/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/
#include <string.h>
#include "buffer.h"

#include "deserialize.h"
#include "utils.h"
#include "types.h"

#if defined(TEST) || defined(FUZZ)
#include "assert.h"
#define LEDGER_ASSERT(x, y) assert(x)
#else
#include "ledger_assert.h"
#endif

void transaction_parser_init(transaction_parsing_state_t *state) {
    // TODO?
    explicit_bzero(state, sizeof(*state));
}

uint32_t min(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

/**
 * Reads out_len bytes to the out buffer
 */
static bool buffer_read_bytes(buffer_t *buffer, uint8_t *out, size_t out_len) {
    if (!buffer_can_read(buffer, out_len)) {
        return false;
    }

    memmove(out, buffer->ptr + buffer->offset, out_len);
    return buffer_seek_cur(buffer, out_len);
}

parser_status_e transaction_parser_update(transaction_parsing_state_t *state,
                                          buffer_t *chunk,
                                          transaction_t *tx) {
    LEDGER_ASSERT(state != NULL, "NULL state");
    LEDGER_ASSERT(chunk != NULL, "NULL chunk");
    LEDGER_ASSERT(tx != NULL, "NULL tx");

    // First block
    if (!state->first_block_parsed) {
        state->first_block_parsed = true;

        tx->chain_id = TESTNET;  // TODO

        // nonce
        if (!buffer_read_u64(chunk, &tx->nonce, BE)) {
            return PARSING_FAILED_NONCE;
        }

        // valid-to time
        if (!buffer_read_u64(chunk, &tx->valid_to_time, BE)) {
            return PARSING_FAILED_VALID_TO_TIME;
        }

        // gas cost
        if (!buffer_read_u64(chunk, &tx->gas_cost, BE)) {
            return PARSING_FAILED_GAS_COST;
        }

        // Contract address
        if (!buffer_read_bytes(chunk, tx->contract_address, ADDRESS_LEN)) {
            return PARSING_FAILED_CONTRACT_ADDRESS;
        }

        // Parse RPC length
        if (!buffer_read_u32(chunk, &state->rpc_bytes_total, BE)) {
            return PARSING_FAILED_RPC_LENGTH;
        }
    }

    // Skip over RPC
    uint32_t skip_amount =
        min(chunk->size - chunk->offset, state->rpc_bytes_total - state->rpc_bytes_parsed);
    if (!buffer_seek_cur(chunk, skip_amount)) {
        return PARSING_FAILED_RPC_DATA;
    }
    state->rpc_bytes_parsed += skip_amount;

    return state->rpc_bytes_total == state->rpc_bytes_parsed ? PARSING_DONE : PARSING_CONTINUE;
}
