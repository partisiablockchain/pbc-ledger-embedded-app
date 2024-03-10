import pytest
import time

from application_client.transaction import Transaction, MpcTokenTransfer
from application_client.command_sender import PbcCommandSender, Errors
from application_client.response_unpacker import unpack_get_address_response, unpack_sign_tx_response
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns
from utils import ROOT_SCREENSHOT_PATH, KEY_PATH, CHAIN_IDS
import transaction_examples


def enable_blind_sign(firmware, navigator):
    if firmware.device.startswith("nano"):
        instructions = [
            NavInsID.RIGHT_CLICK,  # From "ready" to "version"
            NavInsID.RIGHT_CLICK,  # From "version" to "settings"
            NavInsID.BOTH_CLICK,  # From "settings" to "blind sign"
            NavInsID.BOTH_CLICK,  # Enable blind sign
            NavInsID.RIGHT_CLICK,  # From "blind sign" to "back"
            NavInsID.BOTH_CLICK,  # From "back" to "ready"
        ]
    else:
        instructions = [
            # Enter settings
            NavInsID.USE_CASE_HOME_SETTINGS,
            NavInsID.USE_CASE_SETTINGS_NEXT,

            # Enable blind sign
            NavIns(NavInsID.TOUCH, (200, 113)),

            # Exit menu
            NavInsID.USE_CASE_SETTINGS_MULTI_PAGE_EXIT
        ]
    navigator.navigate(instructions,
                       screen_change_before_first_instruction=False)


def move_to_end_and_approve(firmware, navigator, test_name):
    # Validate the on-screen request by performing the navigation appropriate for this device
    if firmware.device.startswith("nano"):
        navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                  [NavInsID.BOTH_CLICK],
                                                  "Approve",
                                                  ROOT_SCREENSHOT_PATH,
                                                  test_name)
    else:
        navigator.navigate_until_text_and_compare(
            NavInsID.USE_CASE_REVIEW_TAP, [
                NavInsID.USE_CASE_REVIEW_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS,
            ], "Hold to sign", ROOT_SCREENSHOT_PATH, test_name)


def move_to_end_and_reject(firmware, navigator, test_name):
    # Validate the on-screen request by performing the navigation appropriate for this device
    if firmware.device.startswith("nano"):
        navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                  [NavInsID.BOTH_CLICK],
                                                  "Reject",
                                                  ROOT_SCREENSHOT_PATH,
                                                  test_name)

    else:
        navigator.navigate_until_text_and_compare(
            NavInsID.USE_CASE_REVIEW_TAP, [
                NavInsID.USE_CASE_CHOICE_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS,
            ], "This message", ROOT_SCREENSHOT_PATH, test_name)


def name_for_sign_test(base_test_name, transaction_name, chain_name):
    return '{}-{}-{}'.format(base_test_name, transaction_name,
                             chain_name.decode('utf-8'))


def wait_for_first_screen_of_review_flow(navigator, timeout=10):
    '''Will wait for the screen to change to one that includes the word
    "Review", or time out after 10 seconds (by default).

    The first screen of review UI flows will always contain the word "Review".
    '''
    time_left = timeout
    while not (navigator._backend.compare_screen_with_text('Review') or
               navigator._backend.compare_screen_with_text('Blind Signing') or
               navigator._backend.compare_screen_with_text('This message')):
        time.sleep(0.1)
        time_left -= 0.1
        assert time_left > 0, 'timeout when waiting for the first screen (First screen must include word "Review")'


@pytest.mark.parametrize("transaction_name,transaction",
                         transaction_examples.BLIND_TRANSACTIONS)
@pytest.mark.parametrize("chain_id", CHAIN_IDS)
def test_sign_blind_transaction(firmware, backend, navigator, test_name,
                                transaction_name, transaction, chain_id):
    test_name = name_for_sign_test(test_name, transaction_name, chain_id)

    client = PbcCommandSender(backend)

    rapdu = client.get_address(path=KEY_PATH)
    address = unpack_get_address_response(rapdu.data)

    transaction_bytes = transaction.serialize()

    # enable blind signing
    enable_blind_sign(firmware, navigator)

    with client.sign_tx(path=KEY_PATH,
                        transaction=transaction_bytes,
                        chain_id=chain_id):
        # Wait for first screen of the application
        wait_for_first_screen_of_review_flow(navigator)

        # Approve
        move_to_end_and_approve(firmware, navigator, test_name)

    # Assert that we have received signature
    response = client.get_async_response().data
    rs_signature = unpack_sign_tx_response(response)
    assert transaction.verify_signature_with_address(address, rs_signature,
                                                     chain_id)


@pytest.mark.parametrize("transaction_name,transaction",
                         transaction_examples.BLIND_TRANSACTIONS)
@pytest.mark.parametrize("chain_id", CHAIN_IDS)
def test_block_blind_sign(firmware, backend, navigator, test_name,
                          transaction_name, transaction, chain_id):
    test_name = name_for_sign_test(test_name, transaction_name, chain_id)

    client = PbcCommandSender(backend)

    rapdu = client.get_address(path=KEY_PATH)
    address = unpack_get_address_response(rapdu.data)

    transaction_bytes = transaction.serialize()

    # Blind signing disabled
    with pytest.raises(ExceptionRAPDU) as e:
        with client.sign_tx(path=KEY_PATH,
                            transaction=transaction_bytes,
                            chain_id=chain_id):
            # Wait for first screen of the application
            wait_for_first_screen_of_review_flow(navigator)

            move_to_end_and_reject(firmware, navigator, test_name)
    # Assert that we have received a refusal
    assert e.value.status == Errors.SW_DENY
    assert len(e.value.data) == 0


@pytest.mark.parametrize("transaction_name,transaction",
                         transaction_examples.BLIND_TRANSACTIONS)
@pytest.mark.parametrize("chain_id", CHAIN_IDS)
def test_enable_blind_sign_after_block(firmware, backend, navigator, test_name,
                          transaction_name, transaction, chain_id):
    # Only run test for STAX devices
    if firmware.device != 'stax':
        return


    test_name = name_for_sign_test(test_name, transaction_name, chain_id)
    client = PbcCommandSender(backend)
    rapdu = client.get_address(path=KEY_PATH)
    address = unpack_get_address_response(rapdu.data)

    transaction_bytes = transaction.serialize()

    # Blind signing disabled
    with client.sign_tx(path=KEY_PATH,
                        transaction=transaction_bytes,
                        chain_id=chain_id):
        # Wait for first screen of the application
        wait_for_first_screen_of_review_flow(navigator)

        # Should show "exit" or "go to settings"
        navigator.navigate_until_text_and_compare(
            NavInsID.USE_CASE_REVIEW_TAP, [
                # Go to settings
                NavInsID.USE_CASE_CHOICE_REJECT,

                # Enable blind sign
                NavInsID.USE_CASE_SETTINGS_NEXT,
                NavIns(NavInsID.TOUCH, (200, 113)),

                # Exit settings
                NavInsID.USE_CASE_SETTINGS_MULTI_PAGE_EXIT,

                # Should be able to sign.
                NavInsID.USE_CASE_REVIEW_TAP,
                NavInsID.USE_CASE_REVIEW_TAP,
                NavInsID.USE_CASE_REVIEW_TAP,
                NavInsID.USE_CASE_REVIEW_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS,

            ], "This message", ROOT_SCREENSHOT_PATH, test_name)

    # Assert that we have received signature
    response = client.get_async_response().data
    rs_signature = unpack_sign_tx_response(response)
    assert transaction.verify_signature_with_address(address, rs_signature,
                                                     chain_id)


@pytest.mark.parametrize("transaction_name,transaction",
                         transaction_examples.MPC_TRANSFER_TRANSACTIONS)
@pytest.mark.parametrize("chain_id", CHAIN_IDS)
def test_sign_mpc_transfer(firmware, backend, navigator, test_name,
                           transaction_name, transaction, chain_id):
    test_name = name_for_sign_test(test_name, transaction_name, chain_id)

    client = PbcCommandSender(backend)

    rapdu = client.get_address(path=KEY_PATH)
    address = unpack_get_address_response(rapdu.data)

    transaction_bytes = transaction.serialize()

    # Blind signing disabled
    with client.sign_tx(path=KEY_PATH,
                        transaction=transaction_bytes,
                        chain_id=chain_id):
        # Wait for first screen of the application
        wait_for_first_screen_of_review_flow(navigator)

        # Approve
        move_to_end_and_approve(firmware, navigator, test_name)

    response = client.get_async_response().data
    rs_signature = unpack_sign_tx_response(response)
    assert transaction.verify_signature_with_address(address, rs_signature,
                                                     chain_id)


# Transaction signature refused test
# The test will ask for a transaction signature that will be refused on screen
def test_sign_tx_refused(firmware, backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = PbcCommandSender(backend)
    chain_id = CHAIN_IDS[0]

    rapdu = client.get_address(path=KEY_PATH)
    address = unpack_get_address_response(rapdu.data)

    transaction_bytes = transaction_examples.TRANSACTION_MPC_TRANSFER.serialize(
    )

    if firmware.device.startswith("nano"):
        with pytest.raises(ExceptionRAPDU) as e:
            with client.sign_tx(path=KEY_PATH,
                                transaction=transaction_bytes,
                                chain_id=chain_id):
                move_to_end_and_reject(firmware, navigator, test_name)

        # Assert that we have received a refusal
        assert e.value.status == Errors.SW_DENY
        assert len(e.value.data) == 0
    else:
        for i in range(3):
            instructions = [NavInsID.USE_CASE_REVIEW_TAP] * i
            instructions += [
                NavInsID.USE_CASE_REVIEW_REJECT,
                NavInsID.USE_CASE_CHOICE_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS
            ]
            with pytest.raises(ExceptionRAPDU) as e:
                with client.sign_tx(path=KEY_PATH,
                                    transaction=transaction_bytes,
                                    chain_id=chain_id):
                    navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                                   test_name + f"/part{i}",
                                                   instructions)
            # Assert that we have received a refusal
            assert e.value.status == Errors.SW_DENY
            assert len(e.value.data) == 0


if __name__ == '__main__':
    from ragger.backend.ledgerwallet import LedgerWalletBackend
    from ragger.firmware import Firmware

    with LedgerWalletBackend(Firmware.NANOS) as backend:
        test_sign_valid_transaction(Firmware.NANOS, backend, None, 'test',
                                    'test',
                                    TRANSACTION_MPC_TRANSFER_WITH_MEMO_SMALL,
                                    True)
