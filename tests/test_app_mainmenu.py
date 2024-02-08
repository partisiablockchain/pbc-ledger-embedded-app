from ragger.navigator import NavInsID, NavIns

from utils import ROOT_SCREENSHOT_PATH


# In this test we check the behavior of the device main menu
def test_app_mainmenu(firmware, navigator, test_name):
    # Navigate in the main menu
    if firmware.device.startswith("nano"):
        instructions = [
            NavInsID.RIGHT_CLICK, # From "ready" to "version"
            NavInsID.RIGHT_CLICK, # From "version" to "settings"
            NavInsID.BOTH_CLICK, # From "settings" to "blind sign"
            NavInsID.BOTH_CLICK, # Enable blind sign
            NavInsID.RIGHT_CLICK, # From "blind sign" to "back"
            NavInsID.BOTH_CLICK, # From "back" to "ready"
            NavInsID.RIGHT_CLICK, # From "ready" to "version"
            NavInsID.RIGHT_CLICK, # From "version" to "settings"
            NavInsID.RIGHT_CLICK, # From "settings" to "about"
            NavInsID.RIGHT_CLICK, # From "about" to "quit"
            NavInsID.RIGHT_CLICK, # From "quit" to "ready"
        ]
    else:
        instructions = [
            # Enter settings
            NavInsID.USE_CASE_HOME_SETTINGS,
            NavInsID.USE_CASE_SETTINGS_NEXT,

            # Enable blind sign
            NavIns(NavInsID.TOUCH, (200, 113)),
            NavInsID.USE_CASE_CHOICE_CONFIRM,

            # Re-enter settings
            NavInsID.USE_CASE_SETTINGS_NEXT,

            # Disable blind sign
            NavIns(NavInsID.TOUCH, (200, 113)),

            # Attempt to enable blind sign, but refuse
            NavIns(NavInsID.TOUCH, (200, 113)),
            NavInsID.USE_CASE_CHOICE_REJECT,

            # Exit menu
            NavInsID.USE_CASE_SETTINGS_MULTI_PAGE_EXIT
        ]
    navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH, test_name, instructions,
                                   screen_change_before_first_instruction=False)
