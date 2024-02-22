import dataclasses
from enum import IntEnum
from typing import Generator, List, Optional
from contextlib import contextmanager

from ragger.backend.interface import BackendInterface, RAPDU
from ragger.bip import pack_derivation_path

MAX_APDU_LEN: int = 255

CLA: int = 0xE0


class P1(IntEnum):
    # SIGN_TX: Parameter 1 indicating first chunk
    P1_FIRST_CHUNK = 0x00
    # SIGN_TX: Parameter 1 indicating non-first chunk.
    P1_NOT_FIRST_CHUNK = 0x01
    # GET_ADDRESS: Parameter 1 to skip screen confirmation
    P1_SILENT = 0x00
    # GET_ADDRESS: Parameter 1 for screen confirmation
    P1_CONFIRM = 0x01


class P2(IntEnum):
    # Parameter 2 for last chunk to receive.
    P2_LAST_CHUNK = 0x00
    # Parameter 2 for more chunk to receive.
    P2_NOT_LAST_CHUNK = 0x80


class InsType(IntEnum):
    GET_VERSION = 0x03
    GET_APP_NAME = 0x04
    SIGN_TX = 0x06
    GET_ADDRESS = 0x07


class Errors(IntEnum):
    SW_DENY = 0x6985
    SW_WRONG_P1P2 = 0x6A86
    SW_WRONG_DATA_LENGTH = 0x6A87
    SW_INS_NOT_SUPPORTED = 0x6D00
    SW_CLA_NOT_SUPPORTED = 0x6E00
    SW_WRONG_RESPONSE_LENGTH = 0xB000
    SW_DISPLAY_BIP32_PATH_FAIL = 0xB001
    SW_DISPLAY_ADDRESS_FAIL = 0xB002
    SW_DISPLAY_AMOUNT_FAIL = 0xB003
    SW_WRONG_TX_LENGTH = 0xB004
    SW_TX_PARSING_FAIL = 0xB005
    SW_TX_HASH_FAIL = 0xB006
    SW_BAD_STATE = 0xB007
    SW_SIGNATURE_FAIL = 0xB008
    SW_TX_PARSING_FAIL_EXPECTED_LESS_DATA = 0xB00B


@dataclasses.dataclass(frozen=True)
class ApduPacket:
    ins: InsType
    p1: P1
    p2: P2
    data: bytes
    cla: int = CLA

    def replace(self, **kwargs):
        return dataclasses.replace(self, **kwargs)


def split_message(message: bytes, max_size: int) -> List[bytes]:
    return [message[x:x + max_size] for x in range(0, len(message), max_size)]


def create_apdu_packets_from_contents(
        ins: InsType, packet_contents: list[bytes]) -> list[ApduPacket]:
    packets = []
    for packet_idx, packet_content in enumerate(packet_contents):
        p1 = P1.P1_NOT_FIRST_CHUNK if packet_idx != 0 else P1.P1_FIRST_CHUNK
        p2 = P2.P2_NOT_LAST_CHUNK if packet_idx != len(
            packet_contents) - 1 else P2.P2_LAST_CHUNK

        packets.append(ApduPacket(ins, p1, p2, packet_content))
    return packets


def sign_tx_packets(path: str, transaction: bytes,
                    chain_id: bytes) -> list[ApduPacket]:

    # Initial packet includes key path and chain id
    initial_packet_contents = b''.join([
        pack_derivation_path(path),
        len(chain_id).to_bytes(4, byteorder="big"),
        chain_id,
    ])

    packet_contents = [initial_packet_contents] + split_message(
        transaction, MAX_APDU_LEN)
    return create_apdu_packets_from_contents(InsType.SIGN_TX, packet_contents)


class PbcCommandSender:

    def __init__(self, backend: BackendInterface) -> None:
        self.backend = backend

    def get_app_and_version(self) -> RAPDU:
        return self.backend.exchange(
            cla=0xB0,  # specific CLA for BOLOS
            ins=0x01,  # specific INS for get_app_and_version
            p1=P1.P1_FIRST_CHUNK,
            p2=P2.P2_LAST_CHUNK,
            data=b"")

    def get_version(self) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_VERSION,
                                     p1=P1.P1_FIRST_CHUNK,
                                     p2=P2.P2_LAST_CHUNK,
                                     data=b"")

    def get_app_name(self) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_APP_NAME,
                                     p1=P1.P1_FIRST_CHUNK,
                                     p2=P2.P2_LAST_CHUNK,
                                     data=b"")

    def get_address(self, path: str) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_ADDRESS,
                                     p1=P1.P1_SILENT,
                                     p2=P2.P2_LAST_CHUNK,
                                     data=pack_derivation_path(path))

    @contextmanager
    def send_packets(self,
                     packets: list[ApduPacket]) -> Generator[None, None, None]:
        '''Capable of sending raw packets and returning a response'''
        for packet in packets[:-1]:
            self.backend.exchange(**dataclasses.asdict(packet))

        with self.backend.exchange_async(
                **dataclasses.asdict(packets[-1])) as response:
            yield response

    @contextmanager
    def get_address_with_confirmation(self,
                                      path: str) -> Generator[None, None, None]:
        with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.GET_ADDRESS,
                p1=P1.P1_CONFIRM,
                p2=P2.P2_LAST_CHUNK,
                data=pack_derivation_path(path)) as response:
            yield response

    @contextmanager
    def sign_tx(self, path: str, transaction: bytes,
                chain_id: bytes) -> Generator[None, None, None]:
        with self.send_packets(sign_tx_packets(path, transaction,
                                               chain_id)) as response:
            yield response

    def get_async_response(self) -> Optional[RAPDU]:
        return self.backend.last_async_response
