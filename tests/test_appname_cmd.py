from application_client.command_sender import PbcCommandSender
from application_client.response_unpacker import unpack_get_app_name_response

def test_app_name(backend):
    '''
    In this test we check that the GET_APP_NAME replies the application name
    '''
    # Use the app interface instead of raw interface
    client = PbcCommandSender(backend)
    # Send the GET_APP_NAME instruction to the app
    response = client.get_app_name()
    # Assert that we have received the correct appname
    app_name_response = unpack_get_app_name_response(response.data)
    assert app_name_response == "Partisia Blockchain"

if __name__ == '__main__':
    from ragger.backend.ledgerwallet import LedgerWalletBackend
    from ragger.firmware import Firmware
    with LedgerWalletBackend(Firmware.NANOS) as backend:
        test_app_name(backend)
