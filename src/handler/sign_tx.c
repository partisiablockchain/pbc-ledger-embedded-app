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

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"
#include "buffer.h"

#include "sign_tx.h"
#include "../status_words.h"
#include "../globals.h"
#include "../ui/display.h"
#include "../transaction/types.h"
#include "../transaction/deserialize.h"

int handler_sign_tx(buffer_t *chunk_data, uint8_t chunk_idx, bool anymore_blocks_after_this_one) {
  // 1. Read initial block requesting signing
  // 2. While reading blocks containing transaction contents
  // 2.1. Read block
  // 2.2. Parse block
  // 2.3. Digest block
  // 3. Display parsed information (possibly very little if parser didn't
  //    recognize the transaction.)
  // 4. When accepted: Sign and return

    // first chunk, parse BIP32 path
    if (chunk_idx == 0) {
        explicit_bzero(&G_context, sizeof(G_context));
        G_context.req_type = CONFIRM_TRANSACTION;
        G_context.state = STATE_NONE;

        // Read length of BIP-32 path
        if (!buffer_read_u8(chunk_data, &G_context.bip32_path_len)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }

        // Read BIP-32 path
        if (!buffer_read_bip32_path(chunk_data,
                                      G_context.bip32_path,
                                      (size_t) G_context.bip32_path_len)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }

        // Initial hashing context in preparation
        if (cx_hash_init_((cx_hash_t *) &G_context.tx_info.digest_state, CX_SHA256) != CX_OK) {
          return false;
        }

        return io_send_sw(SW_OK);

    // parse transaction chunk
    } else {

      // Check that state is consistent
        if (G_context.req_type != CONFIRM_TRANSACTION) {
            return io_send_sw(SW_BAD_STATE);
        }

        // Update parsing state
        parser_status_t status_parsing = transaction_deserialize(chunk_data, &C_context.tx_info.transaction);
        PRINTF("Parsing status: %d.\n", status_parsing );

        if (status_parsing  != PARSING_OK) {
            return io_send_sw(SW_TX_PARSING_FAIL);
        }

        // Update hash digest
        bool initialize_sha256 = chunk_idx == 1;
        cx_err_t status_hashing = cx_hash_update((cx_hash_t *) &G_context.tx_info.digest_state,
                              chunk_data->ptr,
                              chunk_data->size);

        PRINTF("Digest status: %d.\n", status_hashing);
        if (status_hashing  != CX_OK) {
            return io_send_sw(SW_TX_HASH_FAIL);
        }

        if (anymore_blocks_after_this_one) {
            // anymore_blocks_after_this_one APDUs with transaction part are expected.
            // Send a SW_OK to signal that we have received the chunk
            return io_send_sw(SW_OK);
        }

        G_context.state = STATE_PARSED;

        // Finalize hash
        status_hashing = cx_hash_final((cx_hash_t *) &G_context.tx_info.digest_state,
                              &G_context.tx_info.m_hash);

        PRINTF("Hash: %.*H\n", sizeof(G_context.tx_info.m_hash), G_context.tx_info.m_hash);

        // We finally have enough information to display UI.
        return ui_display_transaction();
    }

    return 0;
}
