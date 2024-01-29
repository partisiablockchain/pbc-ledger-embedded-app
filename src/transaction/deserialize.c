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
    explicit_bzero(state, sizeof(*state));
}

/**
 * Determines the minimum value of two unsigned integers.
 */
static uint32_t min(uint32_t a, uint32_t b) {
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

/**
 * Reads a contract_address_s from the given buffer.
 */
static bool buffer_read_contract_address(buffer_t *buffer, blockchain_address_s *out) {
    return buffer_read_bytes(buffer, out->raw_bytes, ADDRESS_LEN);
}

static bool parse_rpc_mpc_token(buffer_t* chunk, transaction_t* tx) {

  // Read shortname
  uint8_t shortname;
  if (!buffer_read_u8(chunk, &shortname)) {
    return false;
  }

  if (shortname == MPC_TOKEN_SHORTNAME_TRANSFER) {
    tx->type = MPC_TRANSFER;
    tx->mpc_transfer.memo_length = 0;

    // TODO: Check remaining chunk length
    if (buffer_read_contract_address(chunk, &tx->mpc_transfer.recipient_address)) {
      return false;
    }

    return buffer_read_u64(chunk, &tx->mpc_transfer.token_amount, BE);

  } else if (shortname == MPC_TOKEN_SHORTNAME_TRANSFER_MEMO_SMALL) {
    tx->type = MPC_TRANSFER;
    tx->mpc_transfer.memo_length = 8;

    // TODO: Check remaining chunk length
    if (buffer_read_contract_address(chunk, &tx->mpc_transfer.recipient_address)) {
      return false;
    }

    // TODO: Needs to be interpreted as a long.
    return buffer_read_bytes(chunk, tx->mpc_transfer.memo, 8);
  }

  // TODO: MPC_TOKEN_SHORTNAME_TRANSFER_MEMO_LARGE

  // Unknown shortname
  return false;
}

/**
 * Invariants: Must either consume entire chunk (and return true), or consume any amount (and return false).
 */
static bool parse_rpc(buffer_t* chunk, transaction_t* tx) {
  if (blockchain_address_is_equal(&tx->basic.contract_address, &MPC_TOKEN_ADDRESS)) {
    return parse_rpc_mpc_token(chunk, tx);
  }
  return false;
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

        tx->basic.chain_id = TESTNET;  // TODO

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
        if (parse_rpc(chunk, tx)) {
          // If RPC could be parsed: No skipping required!
          state->rpc_bytes_parsed = state->rpc_bytes_total;

          // TODO: Check that entire chunk has been consumed.
        } else {
          // If RPC couldn't be parsed. Mark as GENERIC. Reset and continue.
          chunk->offset = current_chunk_offset;
          tx->type = GENERIC_TRANSACTION;
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
