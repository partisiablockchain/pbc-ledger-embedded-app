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
#include "io.h"
#include "buffer.h"
#include "crypto_helpers.h"

#include "get_address.h"
#include "../globals.h"
#include "../types.h"
#include "../status_words.h"
#include "../ui/display.h"
#include "../helper/send_response.h"

WARN_UNUSED_RESULT
int handler_get_address(buffer_t *cdata, bool display) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADDRESS;
    G_context.state = STATE_NONE;

    // Read BIP32 path
    if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
        !buffer_read_bip32_path(cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    // Derive public key from path
    uint8_t raw_pubkey[65];
    cx_err_t error = bip32_derive_get_pubkey_256(CX_CURVE_256K1,
                                                 G_context.bip32_path,
                                                 G_context.bip32_path_len,
                                                 raw_pubkey,
                                                 NULL,
                                                 CX_SHA512);  // Doesn't matter

    if (error != CX_OK) {
        return io_send_sw(error);
    }

    // Derive blockchain address from path
    if (!blockchain_address_from_pubkey(raw_pubkey, &G_context.pk_info.address)) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    // Display or send
    if (display) {
        return ui_display_address();
    }

    return helper_send_response_address();
}
