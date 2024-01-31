import pytest
import time

from application_client.transaction import Transaction, MpcTokenTransfer, from_hex
from application_client.command_sender import PbcCommandSender, Errors
from application_client.response_unpacker import unpack_get_public_key_response, unpack_sign_tx_response
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID
from utils import ROOT_SCREENSHOT_PATH, KEY_PATH
import transaction_examples

def enable_blind_sign(navigator):
    instructions = [
        NavInsID.RIGHT_CLICK, # From "ready" to "version"
        NavInsID.RIGHT_CLICK, # From "version" to "settings"
        NavInsID.BOTH_CLICK, # From "settings" to "blind sign"
        NavInsID.BOTH_CLICK, # Enable blind sign
        NavInsID.RIGHT_CLICK, # From "blind sign" to "back"
        NavInsID.BOTH_CLICK, # From "back" to "ready"
    ]
    navigator.navigate(instructions, screen_change_before_first_instruction=False)

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
                NavInsID.USE_CASE_STATUS_DISMISS
            ], "Hold to sign", ROOT_SCREENSHOT_PATH, test_name)


@pytest.mark.parametrize("transaction_name,transaction", transaction_examples.BLIND_TRANSACTIONS)
def test_sign_blind_transaction(firmware, backend, navigator, test_name,
                                transaction_name, transaction):
    client = PbcCommandSender(backend)

    rapdu = client.get_public_key(path=KEY_PATH)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    transaction_bytes = transaction.serialize()

    enable_blind_sign(navigator)

    with client.sign_tx(path=KEY_PATH, transaction=transaction_bytes):
        # Hacky check for blind transactions
        time.sleep(0.1)
        assert navigator._backend.compare_screen_with_text('Review'), 'First screen must be Review'
        assert navigator._backend.compare_screen_with_text('.*[Bb]lind.*')

        # Approve
        move_to_end_and_approve(firmware, navigator,
                                '{}-{}'.format(test_name, transaction_name))

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    assert transaction.verify_signature(public_key, der_sig)

@pytest.mark.parametrize("transaction_name,transaction", transaction_examples.MPC_TRANSFER_TRANSACTIONS)
def test_sign_mpc_transfer(firmware, backend, navigator, test_name,
                                transaction_name, transaction):
    client = PbcCommandSender(backend)

    rapdu = client.get_public_key(path=KEY_PATH)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    transaction_bytes = transaction.serialize()

    with client.sign_tx(path=KEY_PATH, transaction=transaction_bytes):
        # Hacky check for blind transactions
        time.sleep(1.0)
        assert navigator._backend.compare_screen_with_text('Review'), 'First screen must be Review'
        assert not navigator._backend.compare_screen_with_text('.*[Bb]lind.*')

        # Approve
        move_to_end_and_approve(firmware, navigator,
                                '{}-{}'.format(test_name, transaction_name))

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    assert transaction.verify_signature(public_key, der_sig)


# Transaction signature refused test
# The test will ask for a transaction signature that will be refused on screen
def test_sign_tx_refused(firmware, backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = PbcCommandSender(backend)

    rapdu = client.get_public_key(path=KEY_PATH)
    _, pub_key, _, _ = unpack_get_public_key_response(rapdu.data)

    transaction_bytes = transaction_examples.TRANSACTION_GENERIC_CONTRACT.serialize()

    if firmware.device.startswith("nano"):
        with pytest.raises(ExceptionRAPDU) as e:
            with client.sign_tx(path=KEY_PATH, transaction=transaction_bytes):
                navigator.navigate_until_text_and_compare(
                    NavInsID.RIGHT_CLICK, [NavInsID.BOTH_CLICK], "Reject",
                    ROOT_SCREENSHOT_PATH, test_name)

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
                                    transaction=transaction_bytes):
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
                                    TRANSACTION_MPC_TRANSFER_WITH_MEMO_SMALL , True)
