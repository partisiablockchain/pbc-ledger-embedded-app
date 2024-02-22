import pytest
import time

from application_client.transaction import Transaction, MpcTokenTransfer
import application_client.command_sender
from application_client.command_sender import PbcCommandSender, Errors
from application_client.response_unpacker import unpack_get_address_response, unpack_sign_tx_response
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns
from test_sign_cmd import name_for_sign_test
from utils import ROOT_SCREENSHOT_PATH, KEY_PATH, CHAIN_IDS
import transaction_examples


@pytest.mark.parametrize("transaction_name,transaction",
                         transaction_examples.VALID_TRANSACTIONS)
def test_sign_tx_fail_when_parsing_done_but_packet_stream_continues(
        firmware, backend, navigator, test_name, transaction_name, transaction):
    '''Test that interaction fails if we attempt to send a packet stream that includes a fully parsed transaction and then garbage afterwards.'''

    # Test setup
    chain_id = CHAIN_IDS[0]
    test_name = name_for_sign_test(test_name, transaction_name, chain_id)
    client = PbcCommandSender(backend)
    rapdu = client.get_address(path=KEY_PATH)
    address = unpack_get_address_response(rapdu.data)

    # Create packet stream
    #
    # Modify valid packet stream to indicate that there is more data after the
    # last chunk.
    packets = application_client.command_sender.sign_tx_packets(
        path=KEY_PATH, transaction=transaction.serialize(), chain_id=chain_id)
    packets[-1] = packets[-1].replace(
        p2=application_client.command_sender.P2.P2_NOT_LAST_CHUNK)

    try:
        with client.send_packets(packets):
            pass
        assert False  # With should fail in exit
    except ExceptionRAPDU as e:
        assert e.status == Errors.SW_TX_PARSING_FAIL_EXPECTED_LESS_DATA


@pytest.mark.parametrize("transaction_name,transaction",
                         transaction_examples.VALID_TRANSACTIONS)
def test_sign_tx_fail_when_packet_stream_is_cut_short(firmware, backend,
                                                      navigator, test_name,
                                                      transaction_name,
                                                      transaction):

    # Test setup
    chain_id = CHAIN_IDS[0]
    test_name = name_for_sign_test(test_name, transaction_name, chain_id)
    client = PbcCommandSender(backend)
    rapdu = client.get_address(path=KEY_PATH)
    address = unpack_get_address_response(rapdu.data)

    # Create packet stream.
    #
    # Modify valid packet stream by removing the last packet, and patching
    # second-to-last packet to be the new last packet.
    packets = application_client.command_sender.sign_tx_packets(
        path=KEY_PATH, transaction=transaction.serialize(), chain_id=chain_id)
    del packets[-1]
    packets[-1] = packets[-1].replace(
        p2=application_client.command_sender.P2.P2_LAST_CHUNK)

    try:
        with client.send_packets(packets):
            pass
        assert False  # With should fail in exit
    except ExceptionRAPDU as e:
        error = Errors.from_code(e.status)
        assert error in {
            Errors.SW_WRONG_P1P2, Errors.SW_TX_PARSING_FAIL_EXPECTED_MORE_DATA
        }
