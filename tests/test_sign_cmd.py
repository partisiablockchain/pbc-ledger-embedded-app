import pytest
import time

from application_client.transaction import Transaction, MpcTokenTransfer, from_hex
from application_client.command_sender import BoilerplateCommandSender, Errors
from application_client.response_unpacker import unpack_get_public_key_response, unpack_sign_tx_response
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID
from utils import ROOT_SCREENSHOT_PATH

TRANSACTION_GENERIC_CONTRACT = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01de0b295669a9fd93d5f28d9ec85e40f4cb697bae"),
    rpc=from_hex('0xdeadbeef'),
    chain_id=b'TESTNET',  # TODO?
)

TRANSACTION_MPC_TRANSFER_FORGOT_SHORTNAME = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=from_hex(
        '0x000000000000000000000000000000000000012345_0000000000000333'),
    chain_id=b'TESTNET',  # TODO
)

TRANSACTION_MPC_TRANSFER = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444),
    chain_id=b'TESTNET',  # TODO
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_SMALL = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444, 1337),
    chain_id=b'TESTNET',  # TODO
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444,
        b"Hello World"),
    chain_id=b'TESTNET',  # TODO
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_AND_EMPTY = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(from_hex('0x000000000000000000000000000000000000012345'), 0x444, b""),
    chain_id=b'TESTNET',  # TODO
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_AND_SMALL = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(from_hex('0x000000000000000000000000000000000000012345'), 0x444, b"Hello"),
    chain_id=b'TESTNET',  # TODO
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_VERY = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444,
        b"This is a very long memo. "
        b"It will force the app client to send the serialized transaction to be sent in chunk. "
        b"As the maximum chunk size is 255 bytes we will make this memo greater than 255 characters. "
        b"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus. Suspendisse lectus tortor, dignissim sit amet, adipiscing nec, ultricies sed, dolor. Cras elementum ultrices diam."
    ),
    chain_id=b'TESTNET',  # TODO
)


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


KEY_PATH: str = "m/3757'/1'/0'/0/0"

VALID_TRANSACTIONS = [
    ('generic', TRANSACTION_GENERIC_CONTRACT, True),
    ('mpc_transfer', TRANSACTION_MPC_TRANSFER, False),
    ('mpc_memo_small', TRANSACTION_MPC_TRANSFER_WITH_MEMO_SMALL, False),
    ('mpc_memo_large', TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE, False),
    ('mpc_memo_large_empty', TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_AND_EMPTY, False),
    ('mpc_memo_large_small', TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_AND_SMALL, False),
    ('mpc_memo_large_very', TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_VERY, False),
    ('almost_an_mpc_transfer', TRANSACTION_MPC_TRANSFER_FORGOT_SHORTNAME, True),
]


@pytest.mark.parametrize("transaction_name,transaction, is_blind_transaction",
                         VALID_TRANSACTIONS)
def test_sign_valid_transaction(firmware, backend, navigator, test_name,
                                transaction_name, transaction,
                                is_blind_transaction):
    client = BoilerplateCommandSender(backend)

    rapdu = client.get_public_key(path=KEY_PATH)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    transaction_bytes = transaction.serialize()

    with client.sign_tx(path=KEY_PATH, transaction=transaction_bytes):
        # Hacky check for blind transactions
        time.sleep(1.0)
        print(navigator._backend.get_current_screen_content())
        assert navigator._backend.compare_screen_with_text('Review'), 'First screen must be Review'
        reported_as_blind_transaction = navigator._backend.compare_screen_with_text('.*[Bb]lind.*')
        assert reported_as_blind_transaction == is_blind_transaction, 'Transaction should only be reported as blind when expected'

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
    client = BoilerplateCommandSender(backend)

    rapdu = client.get_public_key(path=KEY_PATH)
    _, pub_key, _, _ = unpack_get_public_key_response(rapdu.data)

    transaction_bytes = TRANSACTION_GENERIC_CONTRACT.serialize()

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
