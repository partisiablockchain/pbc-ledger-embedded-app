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

#ifdef HAVE_NBGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "glyphs.h"
#include "os_io_seproxyhal.h"
#include "nbgl_use_case.h"
#include "io.h"
#include "bip32.h"
#include "format.h"

#include "common.h"
#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../status_words.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../menu.h"

static nbgl_layoutTagValue_t pairs[4];
static nbgl_layoutTagValueList_t pairList;
static nbgl_pageInfoLongPress_t infoLongPress;

static void confirm_transaction_rejection(void) {
    // display a status page and go back to main
    validate_transaction(false);
    nbgl_useCaseStatus("Transaction rejected", false, ui_menu_main);
}

static void ask_transaction_rejection_confirmation(void) {
    // display a choice to confirm/cancel rejection
    nbgl_useCaseConfirm("Reject transaction?",
                        NULL,
                        "Yes, Reject",
                        "Go back to transaction",
                        confirm_transaction_rejection);
}

// called when long press button on 3rd page is long-touched or when reject footer is touched
static void review_choice(bool confirm) {
    if (confirm) {
        // display a status page and go back to main
        validate_transaction(true);
        nbgl_useCaseStatus("TRANSACTION\nSIGNED", true, ui_menu_main);
    } else {
        ask_transaction_rejection_confirmation();
    }
}

static void review_blind_interaction(void) {
    // Setup data to display
    pairs[0].item = "Fees";
    pairs[0].value = g_gas_cost;
    pairs[1].item = "Contract";
    pairs[1].value = g_address;

    // Setup list
    pairList.nbMaxLinesForValue = 0;
    pairList.nbPairs = 2;
    pairList.pairs = pairs;

    // Info long press
    infoLongPress.icon = &C_app_boilerplate_64px;
    infoLongPress.text = "Sign blind transaction to interact with PBC contract?";
    infoLongPress.longPressText = "Hold to sign";

    nbgl_useCaseStaticReview(&pairList, &infoLongPress, "Reject transaction", review_choice);
}

static void review_mpc_transfer(void) {
    // Setup data to display
    pairs[0].item = "Amount";
    pairs[0].value = g_transfer_amount;
    pairs[1].item = "Fees";
    pairs[1].value = g_gas_cost;
    pairs[2].item = "To";
    pairs[2].value = g_address;
    pairs[3].item = "Memo";
    pairs[3].value = g_memo;

    // Setup list
    pairList.nbMaxLinesForValue = 0;
    pairList.nbPairs = 4;
    pairList.pairs = pairs;

    // Info long press
    infoLongPress.icon = &C_app_boilerplate_64px;
    infoLongPress.text = "Sign transaction\nto send MPC";
    infoLongPress.longPressText = "Hold to sign";

    nbgl_useCaseStaticReview(&pairList, &infoLongPress, "Reject transaction", review_choice);
}

// Public function to start the transaction review
// - Check if the app is in the right state for transaction review
// - Format the amount and address strings in g_amount and g_address buffers
// - Display the first screen of the transaction review
int ui_display_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Update gas cost
    set_g_token_amount(g_gas_cost,
                     sizeof(g_gas_cost),
                     "Gas",
                     G_context.tx_info.transaction.basic.gas_cost);

    // TODO: bool prevent_approval_due_to_blind_signing = false;

    // Either setup clear-sign flows or blind-sign flows.
    if (G_context.tx_info.transaction.type == MPC_TRANSFER) {
        // MPC Transfer

        set_g_fields_for_mpc_transfer(&G_context.tx_info.transaction.mpc_transfer);

        nbgl_useCaseReviewStart(&C_app_boilerplate_64px,
                                "Review transaction to send MPC",
                                NULL,
                                "Reject transaction",
                                review_mpc_transfer,
                                ask_transaction_rejection_confirmation);
    } else {
        // Blind sign

        // Display contract address
        if (!set_g_address(&G_context.tx_info.transaction.basic.contract_address)) {
            return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
        }

        // TODO: Blind signing warning
        nbgl_useCaseReviewStart(&C_app_boilerplate_64px,
                                "Review blind PBC transaction",
                                NULL,
                                "Reject transaction",
                                review_blind_interaction,
                                ask_transaction_rejection_confirmation);
    }

    // Start review
    return 0;
}

#endif
