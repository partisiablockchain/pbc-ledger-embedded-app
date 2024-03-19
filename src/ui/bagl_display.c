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

#include "common.h"
#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../status_words.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../menu.h"

// Text buffer review text
static char g_review_text[20];
// Text buffer for title of address.
static char g_address_title[10];

static action_validate_cb g_validate_callback;

// Validate/Invalidate public key and go back to home
static void ui_action_validate_address(bool choice) {
    validate_address(choice);
    ui_menu_main();
}

// Validate/Invalidate transaction and go back to home
static void ui_action_validate_transaction(bool choice) {
    validate_transaction(choice);
    ui_menu_main();
}

// Step with icon and text
UX_STEP_NOCB(ux_display_step_confirm_addr, pn, {&C_icon_eye, "Confirm Address"});
// Step with title/text for address
UX_STEP_NOCB(ux_display_step_address,
             bnnn_paging,
             {
                 .title = g_address_title,
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

UX_STEP_NOCB(ux_display_step_prevent_approve_due_to_blind_signing,
             pnn,
             {
                 &C_icon_warning,
                 "Blind signing",
                 "not enabled",
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

WARN_UNUSED_RESULT
int ui_display_address() {
    // Check current state
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format address
    if (!set_g_address(&G_context.pk_info.address)) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    snprintf(g_address_title, sizeof(g_address_title), "Address");

    // Start flow
    g_validate_callback = &ui_action_validate_address;
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

// Step with icon and text
UX_STEP_NOCB(ux_display_step_blind_sign_warning,
             pbb,
             {
                 &C_icon_warning,
                 "Blind",
                 "Signing",
             });

// Step with title/text for amount
UX_STEP_NOCB(ux_display_step_gas_cost,
             bnnn_paging,
             {
                 .title = "Fee",
                 .text = g_gas_cost,
             });

UX_STEP_NOCB(ux_display_step_transfer_amount,
             bnnn_paging,
             {
                 .title = "Amount",
                 .text = g_transfer_amount,
             });

UX_STEP_NOCB(ux_display_step_memo,
             bnnn_paging,
             {
                 .title = "Memo",
                 .text = g_memo,
             });
UX_STEP_NOCB(ux_display_step_chain_id,
             bnnn_paging,
             {
                 .title = "Chain",
                 .text = g_chain_id,
             });

#define MAX_NUM_STEPS 9

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display amount
// #3 screen : display destination address
// #4 screen : approve button
// #5 screen : reject button
const ux_flow_step_t* ux_display_transaction_flow[MAX_NUM_STEPS + 1];

WARN_UNUSED_RESULT
int ui_display_transaction(void) {
    // Check current state
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Update gas cost
    if (!set_g_token_amount(g_gas_cost,
                            sizeof(g_gas_cost),
                            "Gas",
                            G_context.tx_info.transaction.basic.gas_cost,
                            0)) {
        return io_send_sw(SW_DISPLAY_AMOUNT_FAIL);
    }

    // Update chain id
    if (!set_g_chain_id(&G_context.tx_info.chain_id)) {
        return io_send_sw(SW_DISPLAY_CHAIN_ID_FAIL);
    }

    uint8_t ux_flow_idx = 0;

    bool prevent_approval_due_to_blind_signing =
        G_context.tx_info.transaction.type == GENERIC_TRANSACTION && !N_storage.allow_blind_signing;

    if (prevent_approval_due_to_blind_signing) {
        ux_display_transaction_flow[ux_flow_idx++] =
            &ux_display_step_prevent_approve_due_to_blind_signing;
        ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_reject;
        ux_display_transaction_flow[ux_flow_idx++] = FLOW_END_STEP;
    } else {
        // Display initial
        ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_review;
        ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_chain_id;

        // Either setup clear-sign flows or blind-sign flows.
        if (G_context.tx_info.transaction.type == MPC_TRANSFER) {
            // MPC Transfer

            if (!set_g_fields_for_mpc_transfer(&G_context.tx_info.transaction.mpc_transfer)) {
                return io_send_sw(SW_DISPLAY_AMOUNT_FAIL);
            }

            snprintf(g_review_text, sizeof(g_review_text), "MPC Transfer");
            snprintf(g_address_title, sizeof(g_address_title), "Recipient");

            ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_address;
            ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_transfer_amount;

        } else {
            // Blind sign

            if (!set_g_address(&G_context.tx_info.transaction.basic.contract_address)) {
                return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
            }

            snprintf(g_review_text, sizeof(g_review_text), "Transaction");
            snprintf(g_address_title, sizeof(g_address_title), "Contract");

            // Warning
            ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_blind_sign_warning;
            ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_address;
        }

        // Setup UI flow
        ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_gas_cost;
        ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_approve;
        ux_display_transaction_flow[ux_flow_idx++] = &ux_display_step_reject;
        ux_display_transaction_flow[ux_flow_idx++] = FLOW_END_STEP;
    }

    g_validate_callback = &ui_action_validate_transaction;
    ux_flow_init(0, ux_display_transaction_flow, NULL);
    return 0;
}

#endif
