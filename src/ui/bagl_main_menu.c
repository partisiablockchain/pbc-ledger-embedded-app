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

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "../globals.h"
#include "menu.h"

/***** Main Menu *****/

UX_STEP_NOCB(ux_menu_ready_step, pnn, {&C_app_pbc_16px, "PBC App", "is ready"});
UX_STEP_NOCB(ux_menu_version_step, bn, {"Version", APPVERSION});
UX_STEP_CB(ux_menu_settings_step, pb, ui_menu_settings(), {&C_icon_coggle, "Settings"});
UX_STEP_NOCB(ux_menu_info_step, bn, {"(c) 2024", "Partisia Blockchain"});
UX_STEP_VALID(ux_menu_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

// FLOW for the main menu:
// #1 screen: ready
// #2 screen: version of the app
// #3 screen: about submenu
// #4 screen: quit
UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_version_step,
        &ux_menu_settings_step,
        &ux_menu_info_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

void ui_menu_main() {
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);
}

/***** Settings *****/

void ui_menu_toggle_blind_sign(void) {
    // Flip blind signing setting
    internal_storage_t storage = N_storage;
    storage.allow_blind_signing = !storage.allow_blind_signing;
    nvm_write((void *) &N_storage, &storage, sizeof(internal_storage_t));

    // Redraw menu
    ui_menu_settings();
}

static char g_enabled_text[12];

UX_STEP_CB(ux_menu_blind_sign_toggle_step,
           bn,
           ui_menu_toggle_blind_sign(),
           {"Blind Signing", g_enabled_text});
UX_STEP_CB(ux_menu_back_step, pb, ui_menu_main(), {&C_icon_back, "Back"});

// FLOW for the settings submenu:
// #1 screen: blind signing toggle
// #2 screen: back button to main menu
// #3 loop
UX_FLOW(ux_menu_settings_flow, &ux_menu_blind_sign_toggle_step, &ux_menu_back_step, FLOW_LOOP);

void ui_menu_settings(void) {
    bool blind_signing_enabled = N_storage.allow_blind_signing;

    snprintf(g_enabled_text,
             sizeof(g_enabled_text),
             blind_signing_enabled ? "Enabled" : "NOT Enabled");

    ux_flow_init(0, ux_menu_settings_flow, NULL);
}
#endif
