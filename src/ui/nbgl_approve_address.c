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

#if !defined(HAVE_SHA256)
#error "NBGL requires SHA256"
#endif

static void confirm_address_rejection(void) {
    // display a status page and go back to main
    validate_address(false);
    nbgl_useCaseStatus("Address verification\ncancelled", false, ui_menu_main);
}

static void confirm_address_approval(void) {
    // display a success status page and go back to main
    validate_address(true);
    nbgl_useCaseStatus("ADDRESS\nVERIFIED", true, ui_menu_main);
}

static void review_choice(bool confirm) {
    if (confirm) {
        confirm_address_approval();
    } else {
        confirm_address_rejection();
    }
}

static void continue_review(void) {
    nbgl_useCaseAddressConfirmation(g_address, review_choice);
}

int ui_display_address() {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    if (!set_g_address(&G_context.pk_info.address)) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    nbgl_useCaseReviewStart(&C_app_pbc_64px,
                            "Verify PBC address",
                            NULL,
                            "Cancel",
                            continue_review,
                            confirm_address_rejection);
    return 0;
}

#endif
