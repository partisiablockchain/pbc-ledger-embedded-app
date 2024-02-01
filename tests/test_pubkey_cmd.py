import pytest

from application_client.command_sender import PbcCommandSender, Errors
from application_client.response_unpacker import unpack_get_public_key_response
from ragger.bip import calculate_public_key_and_chaincode, CurveChoice
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns
from utils import ROOT_SCREENSHOT_PATH, KEY_PATH


# In this test we check that the GET_PUBLIC_KEY works in non-confirmation mode
def test_get_public_key_no_confirm(backend):
    for path in [KEY_PATH, "m/3757'/0'/0/0/0", "m/3757'/0'/910'/0/0", "m/3757'/0'/255/255/255", "m/3757'/0'/2147483647/0/0/0/0/0/0/0"]:
        client = PbcCommandSender(backend)
        response = client.get_public_key(path=path).data
        _, public_key, _, chain_code = unpack_get_public_key_response(response)
        print(public_key.hex(), chain_code.hex())

        ref_public_key, ref_chain_code = calculate_public_key_and_chaincode(CurveChoice.Secp256k1, path=path)
        assert public_key.hex() == ref_public_key
        assert chain_code.hex() == ref_chain_code


# In this test we check that the GET_PUBLIC_KEY works in confirmation mode
def test_get_public_key_confirm_accepted(firmware, backend, navigator, test_name):
    client = PbcCommandSender(backend)
    path = KEY_PATH
    with client.get_public_key_with_confirmation(path=path):
        if firmware.device.startswith("nano"):
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Approve",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        else:
            instructions = [
                NavInsID.USE_CASE_REVIEW_TAP,
                NavIns(NavInsID.TOUCH, (200, 335)),
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS
            ]
            navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                           test_name,
                                           instructions)
    response = client.get_async_response().data
    _, public_key, _, chain_code = unpack_get_public_key_response(response)

    ref_public_key, ref_chain_code = calculate_public_key_and_chaincode(CurveChoice.Secp256k1, path=path)
    assert public_key.hex() == ref_public_key
    assert chain_code.hex() == ref_chain_code


# In this test we check that the GET_PUBLIC_KEY in confirmation mode replies an error if the user refuses
def test_get_public_key_confirm_refused(firmware, backend, navigator, test_name):
    client = PbcCommandSender(backend)
    path = KEY_PATH

    if firmware.device.startswith("nano"):
        with pytest.raises(ExceptionRAPDU) as e:
            with client.get_public_key_with_confirmation(path=path):
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                          [NavInsID.BOTH_CLICK],
                                                          "Reject",
                                                          ROOT_SCREENSHOT_PATH,
                                                          test_name)
        # Assert that we have received a refusal
        assert e.value.status == Errors.SW_DENY
        assert len(e.value.data) == 0
    else:
        instructions_set = [
            [
                NavInsID.USE_CASE_REVIEW_REJECT,
                NavInsID.USE_CASE_STATUS_DISMISS
            ],
            [
                NavInsID.USE_CASE_REVIEW_TAP,
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CANCEL,
                NavInsID.USE_CASE_STATUS_DISMISS
            ]
        ]
        for i, instructions in enumerate(instructions_set):
            with pytest.raises(ExceptionRAPDU) as e:
                with client.get_public_key_with_confirmation(path=path):
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
        test_get_public_key_no_confirm(backend)
