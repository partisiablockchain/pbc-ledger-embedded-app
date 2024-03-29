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
#include <stdio.h>
#include "buffer.h"

#include "deserialize.h"
#include "../buffer_util.h"
#include "types.h"
#include "well_known.h"
#include "address.h"

#if defined(TEST) || defined(FUZZ)
#include "assert.h"
#define LEDGER_ASSERT(x, y) assert(x)
#else
#include "ledger_assert.h"
#endif

void transaction_parser_init(transaction_parsing_state_t *state) {
    state->rpc_bytes_total = 0;
    state->rpc_bytes_parsed = 0;
    state->first_block_parsed = false;
}

/**
 * Determines the minimum value of two unsigned integers.
 */
static uint32_t min(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

static parser_status_e parse_rpc_mpc_token(buffer_t *chunk, transaction_t *tx) {
    // Read shortname
    uint8_t shortname;
    if (!buffer_read_u8(chunk, &shortname)) {
        return PARSING_FAILED_SHORTNAME;
    }

    if (shortname == MPC_TOKEN_SHORTNAME_TRANSFER) {
        tx->type = MPC_TRANSFER;
        tx->mpc_transfer.memo_length = 0;

        // Recipient
        if (!buffer_read_contract_address(chunk, &tx->mpc_transfer.recipient_address)) {
            return PARSING_FAILED_MPC_RECIPIENT;
        }

        // MPC Amount
        if (!buffer_read_u64(chunk, &tx->mpc_transfer.token_amount_10000ths, BE)) {
            return PARSING_FAILED_MPC_TOKEN_AMOUNT;
        }

        return PARSING_DONE;

    } else if (shortname == MPC_TOKEN_SHORTNAME_TRANSFER_MEMO_SMALL) {
        tx->type = MPC_TRANSFER;
        tx->mpc_transfer.memo_length = 8;

        // Recipient
        if (!buffer_read_contract_address(chunk, &tx->mpc_transfer.recipient_address)) {
            return PARSING_FAILED_MPC_RECIPIENT;
        }

        // MPC Amount
        if (!buffer_read_u64(chunk, &tx->mpc_transfer.token_amount_10000ths, BE)) {
            return PARSING_FAILED_MPC_TOKEN_AMOUNT;
        }

        // Short memo (u64)
        if (!buffer_read_u64(chunk, &tx->mpc_transfer.memo_u64, BE)) {
            return PARSING_FAILED_MPC_MEMO;
        }
        tx->mpc_transfer.has_u64_memo = true;

        return PARSING_DONE;

    } else if (shortname == MPC_TOKEN_SHORTNAME_TRANSFER_MEMO_LARGE) {
        tx->type = MPC_TRANSFER;

        // Recipient
        if (!buffer_read_contract_address(chunk, &tx->mpc_transfer.recipient_address)) {
            return PARSING_FAILED_MPC_RECIPIENT;
        }

        // MPC Amount
        if (!buffer_read_u64(chunk, &tx->mpc_transfer.token_amount_10000ths, BE)) {
            return PARSING_FAILED_MPC_TOKEN_AMOUNT;
        }

        // Read memo length
        uint32_t memo_length;
        if (!buffer_read_u32(chunk, &memo_length, BE)) {
            return PARSING_FAILED_MPC_MEMO;
        }

        // Check that the memo can be read into the buffer
        if (memo_length > sizeof(tx->mpc_transfer.memo)) {
            return PARSING_FAILED_MPC_MEMO;
        }

        // Read long memo
        size_t read_bytes = buffer_read_bytes(chunk, tx->mpc_transfer.memo, memo_length);

        // Check that the entire memo was read, and not just a part (such that
        // the rest of the memo is on the next chunk.)
        if (read_bytes < memo_length) {
            return PARSING_FAILED_MPC_MEMO;
        }

        tx->mpc_transfer.memo_length = memo_length;
        tx->mpc_transfer.has_u64_memo = false;

        return PARSING_DONE;
    } else {
        // Unknown shortname
        return PARSING_FAILED_SHORTNAME_UNKNOWN;
    }
}

/**
 * Invariants: Must either consume entire chunk (and return true), or consume any amount (and return
 * false).
 */
static parser_status_e parse_rpc(buffer_t *chunk, transaction_t *tx) {
    if (blockchain_address_is_equal(&tx->basic.contract_address, &MPC_TOKEN_ADDRESS)) {
        return parse_rpc_mpc_token(chunk, tx);
    }
    return PARSING_FAILED_ADDRESS_UNKNOWN;
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

        // nonce
        if (!buffer_read_u64(chunk, &tx->basic.nonce, BE)) {
            return PARSING_FAILED_NONCE;
        }

        // valid-to time
        if (!buffer_read_u64(chunk, &tx->basic.valid_to_time, BE)) {
            return PARSING_FAILED_VALID_TO_TIME;
        }

        // gas cost
        if (!buffer_read_u64(chunk, &tx->basic.gas_cost, BE)) {
            return PARSING_FAILED_GAS_COST;
        }

        // Contract address
        if (!buffer_read_contract_address(chunk, &tx->basic.contract_address)) {
            return PARSING_FAILED_CONTRACT_ADDRESS;
        }

        // Parse RPC length
        if (!buffer_read_u32(chunk, &state->rpc_bytes_total, BE)) {
            return PARSING_FAILED_RPC_LENGTH;
        }

        // Try to parse RPC
        size_t current_chunk_offset = chunk->offset;
        parser_status_e rpc_parsing_status = parse_rpc(chunk, tx);
        bool rpc_parsing_consumed_entire_chunk = chunk->offset == chunk->size;
        if (rpc_parsing_status == PARSING_DONE && rpc_parsing_consumed_entire_chunk) {
            // If RPC could be parsed: No skipping required!
            state->rpc_bytes_parsed = state->rpc_bytes_total;
        } else {
            // If RPC couldn't be parsed. Mark as GENERIC. Reset and continue.
            chunk->offset = current_chunk_offset;
            tx->type = GENERIC_TRANSACTION;
            tx->rpc_parsing_error = rpc_parsing_status;
        }
    }

    // Skip over RPC
    uint32_t skip_amount =
        min(chunk->size - chunk->offset, state->rpc_bytes_total - state->rpc_bytes_parsed);
    buffer_seek_cur(chunk, skip_amount);  // Cannot fail
    state->rpc_bytes_parsed += skip_amount;

    return state->rpc_bytes_total == state->rpc_bytes_parsed ? PARSING_DONE : PARSING_CONTINUE;
}
