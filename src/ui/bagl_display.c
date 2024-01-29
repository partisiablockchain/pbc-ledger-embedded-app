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

#define PRIu64_MAX_LENGTH 20

static action_validate_cb g_validate_callback;
static char g_gas_cost[PRIu64_MAX_LENGTH + 1];
static char g_transfer_amount[PRIu64_MAX_LENGTH + 1];
static char g_address[2*ADDRESS_LEN + 1];
static char g_review_text[20];

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
UX_STEP_NOCB(ux_display_step_confirm_addr, pn, {&C_icon_eye, "Confirm Transaction"});
// Step with title/text for address
UX_STEP_NOCB(ux_display_step_address,
             bnnn_paging,
             {
                 .title = "Contract",
                 .text = g_address,
             });
// Step with approve button
UX_STEP_CB(ux_display_step_approve,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_step_reject,
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
        &ux_display_step_confirm_addr,
        &ux_display_step_address,
        &ux_display_step_approve,
        &ux_display_step_reject);

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
    memset(g_address, 0, sizeof(g_address));
    blockchain_address_s address;
    if (!blockchain_address_from_pubkey(G_context.pk_info.raw_public_key, &address)) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }
    if (!blockchain_address_format(&address, g_address, sizeof(g_address))) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    // Start flow
    g_validate_callback = &ui_action_validate_pubkey;
    ux_flow_init(0, ux_display_pubkey_flow, NULL);
    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_step_review,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 g_review_text,
             });
// Step with title/text for amount
UX_STEP_NOCB(ux_display_step_gas_cost,
             bnnn_paging,
             {
                 .title = "Gas Cost",
                 .text = g_gas_cost,
             });

UX_STEP_NOCB(ux_display_step_transfer_amount,
             bnnn_paging,
             {
                 .title = "Transfer Amount",
                 .text = g_transfer_amount,
             });

#define MAX_NUM_STEPS 8

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display amount
// #3 screen : display destination address
// #4 screen : approve button
// #5 screen : reject button
const ux_flow_step_t* ux_display_transaction_flow[MAX_NUM_STEPS + 1];

static bool set_address(blockchain_address_s* address) {
    memset(g_address, 0, sizeof(g_address));
    return blockchain_address_format(address, g_address, sizeof(g_address));
}

static void set_gas_cost(uint64_t gas_cost) {
    // TODO: Return error if gas cost is not fully written
    memset(g_gas_cost, 0, sizeof(g_gas_cost));
    format_u64(g_gas_cost, sizeof(g_gas_cost), gas_cost);
    PRINTF("Gas Cost: %s\n", g_gas_cost);
}

static void set_transfer_amount(uint64_t transfer_amount) {
    // TODO: Return error if transfer amount is not fully written
    memset(g_transfer_amount, 0, sizeof(g_transfer_amount));
    format_u64(g_transfer_amount, sizeof(g_transfer_amount), transfer_amount);
    PRINTF("Gas Cost: %s\n", g_transfer_amount);
}

int ui_display_transaction(void) {
    // Check current state
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // TODO: Cleanup

    uint8_t ux_flow_idx = 0;

    // Display initial
    ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_review;

    // Either setup clear-sign flows or blind-sign flows.
    if (G_context.tx_info.transaction.type == MPC_TRANSFER) {
        // MPC Transfer

        snprintf(g_review_text, sizeof(g_review_text), "MPC Transfer");

        // Display recipient
        ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_address;
        set_address(&G_context.tx_info.transaction.basic.contract_address);

        // Display token transfer amount
        ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_transfer_amount;
        set_transfer_amount(G_context.tx_info.transaction.mpc_transfer.token_amount);

        // Display memo
        // TODO

    } else {
        // Blind sign
        snprintf(g_review_text, sizeof(g_review_text), "Blind Interaction");

        // TODO: Change review icon
        // TODO: Blind sign settings toggle

        // Display contract address
        ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_address;
        if (!set_address(&G_context.tx_info.transaction.basic.contract_address))
            return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    // TODO: Blind Warning?

    // Display gas cost
    ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_gas_cost;
    set_gas_cost(G_context.tx_info.transaction.basic.gas_cost);

    // Setup UI flow
    ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_approve;
    ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_reject;
    ux_display_transaction_flow[ux_flow_idx++] = FLOW_END_STEP;

    g_validate_callback = &ui_action_validate_transaction;
    ux_flow_init(0, ux_display_transaction_flow, NULL);
    return 0;
}

#endif
