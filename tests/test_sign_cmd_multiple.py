import pytest

from ragger.navigator import NavInsID, NavIns
from application_client.command_sender import PbcCommandSender
from application_client.response_unpacker import unpack_get_address_response, unpack_sign_tx_response
from utils import ROOT_SCREENSHOT_PATH, KEY_PATH, CHAIN_IDS
import transaction_examples
import test_sign_cmd


@pytest.mark.parametrize("chain_id", CHAIN_IDS)
def test_nano_sign_mpc_transfer_multiple(firmware, backend, navigator,
                                         test_name, chain_id):
    '''
    This test checks that no display state is left over between signing of
    transactions.

    Leaving any display state, for example a memo, between transactions would
    mean that the second transaction could be misrepresented.
    '''

    # Only run test for STAX devices
    if firmware.device == 'stax':
        return

    # Determine expected instructions.
    dest_addr_insns = [NavInsID.RIGHT_CLICK]
    chain_id_insns = [NavInsID.RIGHT_CLICK]
    if firmware.device == 'nanos':
        dest_addr_insns = [
            NavInsID.RIGHT_CLICK, NavInsID.RIGHT_CLICK, NavInsID.RIGHT_CLICK
        ]
        if chain_id.endswith(b'Testnet'):
            chain_id_insns = [NavInsID.RIGHT_CLICK, NavInsID.RIGHT_CLICK]

    instructions_tx_1 = [
        NavInsID.RIGHT_CLICK,  # Review
    ] + chain_id_insns + [  # Chain id
    ] + dest_addr_insns + [  # Dest addr
        NavInsID.RIGHT_CLICK,  # amount
        NavInsID.RIGHT_CLICK,  # memo
        NavInsID.RIGHT_CLICK,  # gas cost
        NavInsID.BOTH_CLICK,  # approve
    ]

    instructions_tx_2 = [
        NavInsID.RIGHT_CLICK,  # Review
    ] + chain_id_insns + [  # Chain id
    ] + dest_addr_insns + [  # Dest addr
        NavInsID.RIGHT_CLICK,  # amount
        # no memo!
        NavInsID.RIGHT_CLICK,  # gas cost
        NavInsID.BOTH_CLICK,  # approve
    ]

    # Run test
    # Blind signing disabled

    test_name = test_sign_cmd.name_for_sign_test(test_name, 'test', chain_id)
    client = PbcCommandSender(backend)
    rapdu = client.get_address(path=KEY_PATH)
    address = unpack_get_address_response(rapdu.data)

    with client.sign_tx(path=KEY_PATH,
                        transaction=transaction_examples.
                        TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE.serialize(),
                        chain_id=chain_id):
        # Wait for first screen of the application
        test_sign_cmd.wait_for_first_screen_of_review_flow(navigator)

        # Approve
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH, test_name,
                                       instructions_tx_1)

    rs_signature_1 = unpack_sign_tx_response(client.get_async_response().data)

    with client.sign_tx(path=KEY_PATH,
                        transaction=transaction_examples.
                        TRANSACTION_MPC_TRANSFER.serialize(),
                        chain_id=chain_id):
        # Wait for first screen of the application
        test_sign_cmd.wait_for_first_screen_of_review_flow(navigator)

        # Approve
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                       test_name,
                                       instructions_tx_2,
                                       snap_start_idx=len(instructions_tx_1))

    rs_signature_2 = unpack_sign_tx_response(client.get_async_response().data)

    # Check correct signatures
    assert transaction_examples.TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE.verify_signature_with_address(
        address, rs_signature_1, chain_id)
    assert transaction_examples.TRANSACTION_MPC_TRANSFER.verify_signature_with_address(
        address, rs_signature_2, chain_id)
