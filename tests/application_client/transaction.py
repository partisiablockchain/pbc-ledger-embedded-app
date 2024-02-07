from hashlib import sha256
import dataclasses
from abc import ABC, abstractmethod
from typing import Union

from ecdsa.curves import SECP256k1  # type: ignore
from ecdsa.keys import VerifyingKey  # type: ignore
import ecdsa.util  # type: ignore

UINT64_MAX: int = 2**64 - 1

ADDRESS_LENGTH = 21


class TransactionError(Exception):
    pass


def from_hex(hexstr: str) -> bytes:
    print(hexstr)
    return bytes.fromhex(hexstr.replace("_", ''))


# pylint: disable=too-few-public-methods
class Serializable(ABC):
    '''
    A serializable transaction object.
    '''

    @abstractmethod
    def serialize(self) -> bytes:
        pass


@dataclasses.dataclass(frozen=True)
class Address(Serializable):
    raw_bytes: bytes

    TYPE_ACCOUNT = (0).to_bytes(1, byteorder='big')

    def __post_init__(self):
        assert len(self.raw_bytes) == ADDRESS_LENGTH

    @staticmethod
    def deserialize(bts: bytes) -> 'Address':
        return Address(bts)

    def serialize(self) -> bytes:
        return self.raw_bytes

    @staticmethod
    def from_hex(address: str) -> 'Address':
        assert len(address) == 21 * 2
        return Address(from_hex(address))

    @staticmethod
    def from_public_key(public_key: bytes) -> 'Address':
        assert len(public_key) == 65

        digest = sha256()
        digest.update(public_key)
        hashed = digest.digest()
        return Address(b''.join([Address.TYPE_ACCOUNT, hashed[12:32]]))


@dataclasses.dataclass(frozen=True)
class Signature(Serializable):
    recovery_id: int
    r: bytes
    s: bytes

    def __post_init__(self):
        assert 0 <= self.recovery_id < 4
        assert len(self.r) == 32
        assert len(self.s) == 32

    @staticmethod
    def deserialize(bts: bytes) -> 'Signature':
        assert len(bts) == 65
        return Signature(
            bts[0],
            bts[1:1 + 32],
            bts[1 + 32:1 + 32 + 32],
        )

    def serialize(self) -> bytes:
        return b''.join([
            self.recovery_id.to_bytes(1, byteorder='big'),
            self.r,
            self.s,
        ])


@dataclasses.dataclass(frozen=True)
class Transaction(Serializable):
    '''
    Individual unsigned Transaction on the Partisia Blockchain.
    '''

    nonce: int
    valid_to_time: int
    gas_cost: int
    contract_address: Address
    rpc: Union[bytes, Serializable]

    def __post_init__(self):
        if not 0 <= self.nonce <= UINT64_MAX:
            raise TransactionError(f"Bad nonce: '{self.nonce}'!")

        if not 0 <= self.valid_to_time <= UINT64_MAX:
            raise TransactionError(
                f"Bad valid_to_time: '{self.valid_to_time}'!")

        if not 0 <= self.gas_cost <= UINT64_MAX:
            raise TransactionError(f"Bad gas_cost: '{self.gas_cost}'!")

        if not isinstance(self.contract_address, Address):
            raise TransactionError(
                f"Bad address: '{self.contract_address}', expected Address!")

    def serialize(self) -> bytes:
        rpc = self.rpc if isinstance(self.rpc, bytes) else self.rpc.serialize()
        return b"".join([
            self.nonce.to_bytes(8, byteorder="big"),
            self.valid_to_time.to_bytes(8, byteorder="big"),
            self.gas_cost.to_bytes(8, byteorder="big"),
            self.contract_address.serialize(),
            len(rpc).to_bytes(4, byteorder="big"),
            rpc,
        ])

    def verify_signature_with_address(self, address: Address,
                                      signature: Signature, chain_id: bytes):
        assert isinstance(address, Address)

        # Determine the signed transaction
        bytes_to_verify = b''.join([
            self.serialize(),
            len(chain_id).to_bytes(4, byteorder="big"),
            chain_id,
        ])

        # Determine public key from the signature
        rs_signature = signature.r + signature.s
        pks: VerifyingKey = VerifyingKey.from_public_key_recovery(
            rs_signature, bytes_to_verify, curve=SECP256k1, hashfunc=sha256)

        # Verify that derived public key is the desired address

        return pks[0].verify(signature=rs_signature,
                             data=bytes_to_verify,
                             hashfunc=sha256,
                             sigdecode=ecdsa.util.sigdecode_string)


@dataclasses.dataclass(frozen=True)
class MpcTokenTransfer(Serializable):
    '''
    MPC token transfer from the sender of the transaction to the given
    recipient.
    '''
    recipient_address: Address
    token_amount: int
    memo: Union[None, int, bytes] = None

    SHORTNAME_TRANSFER = (3).to_bytes(1, byteorder='big')
    SHORTNAME_TRANSFER_WITH_SMALL_MEMO = (13).to_bytes(1, byteorder='big')
    SHORTNAME_TRANSFER_WITH_LARGE_MEMO = (23).to_bytes(1, byteorder='big')

    def __post_init__(self):
        if not isinstance(self.recipient_address, Address):
            raise TransactionError(f"Bad address: '{self.recipient_address}'!")

    def serialize(self) -> bytes:
        if self.memo is None:
            shortname = MpcTokenTransfer.SHORTNAME_TRANSFER
            memo = b''
        elif isinstance(self.memo, int):
            shortname = MpcTokenTransfer.SHORTNAME_TRANSFER_WITH_SMALL_MEMO
            memo = self.memo.to_bytes(8, byteorder='big')
        elif isinstance(self.memo, bytes):
            shortname = MpcTokenTransfer.SHORTNAME_TRANSFER_WITH_LARGE_MEMO
            memo = b''.join([
                len(self.memo).to_bytes(4, byteorder='big'),
                self.memo,
            ])

        return b''.join([
            shortname,
            self.recipient_address.serialize(),
            self.token_amount.to_bytes(8, byteorder='big'),
            memo,
        ])
