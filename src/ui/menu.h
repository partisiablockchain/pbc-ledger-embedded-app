#pragma once

/**
 * Show main menu (ready screen, version, about, quit).
 */
void ui_menu_main(void);

/**
 * Show settings menu (App version, developer name and settings).
 *
 * @param[in] exit_callback
 *   Callback for when user exits the settings menu. Only supported for NBGL.
 *   Allows for writing inline settings flows.
 */
void ui_menu_settings(void (*exit_callback)(void));

/**
 * Show about submenu (copyright, date).
 */
void ui_menu_about(void);
