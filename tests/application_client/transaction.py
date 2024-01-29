from hashlib import sha256
import dataclasses
from abc import ABC, abstractmethod
from typing import Union

from ecdsa.curves import SECP256k1
from ecdsa.keys import VerifyingKey
from ecdsa.util import sigdecode_der

UINT64_MAX: int = 2**64 - 1

ADDRESS_LENGTH = 21


class TransactionError(Exception):
    pass


def from_hex(hex_addr: str) -> bytes:
    assert hex_addr.startswith('0x')
    return bytes.fromhex(hex_addr[2:].replace("_", ''))


class Serializable(ABC):

    @abstractmethod
    def serialize(self) -> bytes:
        pass


@dataclasses.dataclass(frozen=True)
class Transaction(Serializable):
    '''
    Individual unsigned Transaction on the Partisia Blockchain.
    '''

    nonce: int
    valid_to_time: int
    gas_cost: int
    contract_address: bytes
    rpc: Union[bytes, Serializable]
    chain_id: bytes

    def __post_init__(self):
        if not 0 <= self.nonce <= UINT64_MAX:
            raise TransactionError(f"Bad nonce: '{self.nonce}'!")

        if not 0 <= self.valid_to_time <= UINT64_MAX:
            raise TransactionError(
                f"Bad valid_to_time: '{self.valid_to_time}'!")

        if not 0 <= self.gas_cost <= UINT64_MAX:
            raise TransactionError(f"Bad gas_cost: '{self.gas_cost}'!")

        if len(self.contract_address) != ADDRESS_LENGTH:
            raise TransactionError(
                f"Bad address: '{self.contract_address.hex()}'!")

        if self.chain_id not in {b'TESTNET', b'MAINNET'}:
            raise TransactionError(f"Unknown chain id: '{self.chain_id}'!")

    def serialize(self) -> bytes:
        rpc = self.rpc if isinstance(self.rpc, bytes) else self.rpc.serialize()
        return b"".join([
            self.nonce.to_bytes(8, byteorder="big"),
            self.valid_to_time.to_bytes(8, byteorder="big"),
            self.gas_cost.to_bytes(8, byteorder="big"),
            self.contract_address,
            len(rpc).to_bytes(4, byteorder="big"),
            rpc,
        ])

    def serialize_for_signing(self) -> bytes:
        return b''.join([
            self.serialize(),
            len(self.chain_id).to_bytes(4, byteorder="big"),
            self.chain_id,
        ])

    def verify_signature(self, public_key: bytes, signature: bytes):
        pk: VerifyingKey = VerifyingKey.from_string(public_key,
                                                    curve=SECP256k1,
                                                    hashfunc=sha256)
        return pk.verify(signature=signature,
                         data=self.serialize_for_signing(),
                         hashfunc=sha256,
                         sigdecode=sigdecode_der)


@dataclasses.dataclass(frozen=True)
class MpcTokenTransfer(Serializable):
    recipient_address: bytes
    token_amount: int
    memo: Union[None, int, bytes] = None

    def __post_init__(self):
        if not isinstance(self.recipient_address, bytes):
            raise TransactionError(f"Bad address: '{self.recipient_address}'!")
        if len(self.recipient_address) != ADDRESS_LENGTH:
            raise TransactionError(
                f"Bad address: '{self.recipient_address.hex()}'!")

    def serialize(self) -> bytes:
        if self.memo is None:
            shortname = (3).to_bytes(1)
            memo = b''
        elif isinstance(self.memo, int):
            shortname = (13).to_bytes(1)
            memo = self.memo.to_bytes(8, byteorder='big')
        elif isinstance(self.memo, bytes):
            shortname = (23).to_bytes(1)
            memo = b''.join([
                len(self.memo).to_bytes(4, byteorder='big'),
                self.memo,
            ])

        return b''.join([
            shortname,
            self.recipient_address,
            self.token_amount.to_bytes(8, byteorder='big'),
            memo,
        ])
