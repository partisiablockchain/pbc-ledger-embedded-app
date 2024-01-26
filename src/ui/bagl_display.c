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

#ifdef HAVE_BAGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "ux.h"
#include "glyphs.h"
#include "io.h"
#include "bip32.h"
#include "format.h"

#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../status_words.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../menu.h"

static action_validate_cb g_validate_callback;
static char g_gas_cost[30];
static char g_contract_address[43];

// Validate/Invalidate public key and go back to home
static void ui_action_validate_pubkey(bool choice) {
    validate_pubkey(choice);
    ui_menu_main();
}

// Validate/Invalidate transaction and go back to home
static void ui_action_validate_transaction(bool choice) {
    validate_transaction(choice);
    ui_menu_main();
}

// Step with icon and text
UX_STEP_NOCB(ux_display_confirm_addr_step, pn, {&C_icon_eye, "Confirm Transaction"});
// Step with title/text for address
UX_STEP_NOCB(ux_display_address_step,
             bnnn_paging,
             {
                 .title = "Contract",
                 .text = g_contract_address,
             });
// Step with approve button
UX_STEP_CB(ux_display_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

// FLOW to display address:
// #1 screen: eye icon + "Confirm Transaction"
// #2 screen: display address
// #3 screen: approve button
// #4 screen: reject button
UX_FLOW(ux_display_pubkey_flow,
        &ux_display_confirm_addr_step,
        &ux_display_address_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

/**
 * Formats a blockchain_address_s as a hex string.
 */
static bool blockchain_address_format(blockchain_address_s* address, char* out, size_t out_len) {
    return format_hex(address->raw_bytes, ADDRESS_LEN, out, out_len) != -1;
}

int ui_display_address() {
    // Check current state
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format address
    memset(g_contract_address, 0, sizeof(g_contract_address));
    blockchain_address_s address;
    if (!blockchain_address_from_pubkey(G_context.pk_info.raw_public_key, &address)) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }
    if (!blockchain_address_format(&address, g_contract_address, sizeof(g_contract_address))) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    // Start flow
    g_validate_callback = &ui_action_validate_pubkey;
    ux_flow_init(0, ux_display_pubkey_flow, NULL);
    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_review_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 "Transaction",
             });
// Step with title/text for amount
UX_STEP_NOCB(ux_display_amount_step,
             bnnn_paging,
             {
                 .title = "Gas Cost",
                 .text = g_gas_cost,
             });

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display amount
// #3 screen : display destination address
// #4 screen : approve button
// #5 screen : reject button
UX_FLOW(ux_display_transaction_flow,
        &ux_display_review_step,
        &ux_display_address_step,
        &ux_display_amount_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_transaction() {
    // Check current state
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format gas cost
    memset(g_gas_cost, 0, sizeof(g_gas_cost));
    char amount[30] = {0};

    // TODO: Remove weird unit system or replace with K/M/G etc.
    if (!format_fpu64(amount,
                      sizeof(amount),
                      G_context.tx_info.transaction.basic.gas_cost,
                      EXPONENT_SMALLEST_UNIT)) {
        return io_send_sw(SW_DISPLAY_AMOUNT_FAIL);
    }
    snprintf(g_gas_cost, sizeof(g_gas_cost), "%.*s", sizeof(amount), amount);
    PRINTF("Gas Cost: %s\n", g_gas_cost);

    // Format contract address
    memset(g_contract_address, 0, sizeof(g_contract_address));
    if (!blockchain_address_format(&G_context.tx_info.transaction.basic.contract_address,
                                   g_contract_address,
                                   sizeof(g_contract_address))) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    // Setup UI flow
    g_validate_callback = &ui_action_validate_transaction;
    ux_flow_init(0, ux_display_transaction_flow, NULL);
    return 0;
}

#endif
