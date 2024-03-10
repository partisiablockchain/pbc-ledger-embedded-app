
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

#include "os.h"
#include "glyphs.h"
#include "nbgl_use_case.h"

#include "../globals.h"
#include "menu.h"

//  -----------------------------------------------------------
//  ----------------------- HOME PAGE -------------------------
//  -----------------------------------------------------------

void app_quit(void) {
    // exit app here
    os_sched_exit(-1);
}

static void ui_menu_settings_return_to_main_menu(void) {
    ui_menu_settings(ui_menu_main);
}

// home page definition
void ui_menu_main(void) {
// This parameter shall be set to false if the settings page contains only information
// about the application (version , developer name, ...). It shall be set to
// true if the settings page also contains user configurable parameters related to the
// operation of the application.
#define SETTINGS_BUTTON_ENABLED (true)

    nbgl_useCaseHome(APPNAME,
                     &C_app_pbc_64px,
                     NULL,
                     SETTINGS_BUTTON_ENABLED,
                     ui_menu_settings_return_to_main_menu,
                     app_quit);
}

//  -----------------------------------------------------------
//  --------------------- SETTINGS MENU -----------------------
//  -----------------------------------------------------------

static const char* const INFO_TYPES[] = {"Version", "Developer"};
static const char* const INFO_CONTENTS[] = {APPVERSION, "Partisia Blockchain"};

// settings switches definitions
enum { BLIND_TRANSACTION_SWITCH_TOKEN = FIRST_USER_TOKEN };
enum { BLIND_TRANSACTION_SWITCH_ID = 0, SETTINGS_SWITCHES_NB };

static nbgl_layoutSwitch_t switches[SETTINGS_SWITCHES_NB] = {0};

static bool nav_callback(uint8_t page, nbgl_pageContent_t* content) {
    UNUSED(page);

    // the first settings page contains only the version and the developer name
    // of the app (shall be always on the first setting page)
    if (page == 0) {
        content->type = INFOS_LIST;
        content->infosList.nbInfos = 2;
        content->infosList.infoTypes = INFO_TYPES;
        content->infosList.infoContents = INFO_CONTENTS;
    }
    // the second settings page contains 2 toggle setting switches
    else if (page == 1) {
        switches[BLIND_TRANSACTION_SWITCH_ID].initState =
            (nbgl_state_t) N_storage.allow_blind_signing;
        switches[BLIND_TRANSACTION_SWITCH_ID].text = "Blind Signing";
        switches[BLIND_TRANSACTION_SWITCH_ID].subText = "Enable blind signing";
        switches[BLIND_TRANSACTION_SWITCH_ID].token = BLIND_TRANSACTION_SWITCH_TOKEN;
        switches[BLIND_TRANSACTION_SWITCH_ID].tuneId = TUNE_TAP_CASUAL;

        content->type = SWITCHES_LIST;
        content->switchesList.nbSwitches = SETTINGS_SWITCHES_NB;
        content->switchesList.switches = (nbgl_layoutSwitch_t*) switches;
    } else {
        return false;
    }
    // valid page so return true
    return true;
}

/** Utility for setting whether blind signing is allowed. */
static void set_blind_signing(uint8_t allow) {
    // Store the new value
    nvm_write((void*) &N_storage.allow_blind_signing, &allow, 1);
}

/** Invoked when pressing buttons in the #ui_menu_settings. */
static void settings_controls_callback(int token, uint8_t index) {
    UNUSED(index);
    if (token == BLIND_TRANSACTION_SWITCH_TOKEN) {
        // Blind Signing switch touched

        set_blind_signing(!N_storage.allow_blind_signing);
    }
}

#define TOTAL_SETTINGS_PAGE  (2)
#define INIT_SETTINGS_PAGE   (0)
#define DISABLE_SUB_SETTINGS (false)

// settings menu definition
void ui_menu_settings(void (*exit_callback)(void)) {
    nbgl_useCaseSettings(APPNAME,
                         INIT_SETTINGS_PAGE,
                         TOTAL_SETTINGS_PAGE,
                         DISABLE_SUB_SETTINGS,
                         exit_callback,
                         nav_callback,
                         settings_controls_callback);
}

#endif
