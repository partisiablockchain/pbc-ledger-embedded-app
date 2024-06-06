import pytest
import time

from application_client.transaction import Transaction, MpcTokenTransfer
from application_client.command_sender import PbcCommandSender, Errors
from application_client.response_unpacker import unpack_get_address_response, unpack_sign_tx_response
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns
from utils import ROOT_SCREENSHOT_PATH, KEY_PATH, CHAIN_IDS
import transaction_examples
import test_sign_cmd

@pytest.mark.parametrize("chain_id", CHAIN_IDS)
def test_sign_mpc_transfer_multiple(firmware, backend, navigator, test_name, chain_id):
    test_name = test_sign_cmd.name_for_sign_test(test_name, 'test', chain_id)

    client = PbcCommandSender(backend)

    rapdu = client.get_address(path=KEY_PATH)
    address = unpack_get_address_response(rapdu.data)

    # Blind signing disabled
    with client.sign_tx(path=KEY_PATH,
                        transaction=transaction_examples.TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE.serialize(),
                        chain_id=chain_id):
        # Wait for first screen of the application
        test_sign_cmd.wait_for_first_screen_of_review_flow(navigator)

        # Approve
        test_sign_cmd.move_to_end_and_approve(firmware, navigator, test_name)

    rs_signature = unpack_sign_tx_response(client.get_async_response().data)
    assert transaction_examples.TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE.verify_signature_with_address(address, rs_signature, chain_id)

    with client.sign_tx(path=KEY_PATH,
                        transaction=transaction_examples.TRANSACTION_MPC_TRANSFER.serialize(),
                        chain_id=chain_id):
        # Wait for first screen of the application
        test_sign_cmd.wait_for_first_screen_of_review_flow(navigator)

        # Approve
        test_sign_cmd.move_to_end_and_approve(firmware, navigator, test_name)

    rs_signature = unpack_sign_tx_response(client.get_async_response().data)
    assert transaction_examples.TRANSACTION_MPC_TRANSFER.verify_signature_with_address(address, rs_signature, chain_id)


