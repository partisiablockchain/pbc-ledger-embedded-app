from io import BytesIO
from typing import Union
from pathlib import Path
from hashlib import sha256

from ecdsa.curves import SECP256k1
from ecdsa.keys import VerifyingKey
from ecdsa.util import sigdecode_der
import dataclasses
from .boilerplate_utils import read, read_uint, UINT64_MAX


class TransactionError(Exception):
    pass


ADDRESS_LENGTH = 21

def from_hex(hex_addr: str) -> bytes:
    assert hex_addr.startswith('0x')
    return bytes.fromhex(hex_addr[2:])

@dataclasses.dataclass(frozen=True, slots=True)
class Transaction:
    '''
    Individual unsigned Transaction on the Partisia Blockchain.
    '''

    nonce: int
    valid_to_time: int
    gas_cost: int
    contract_address: bytes
    rpc: bytes
    chain_id: bytes

    def __post_init__(self):
        if not 0 <= self.nonce <= UINT64_MAX:
            raise TransactionError(f"Bad nonce: '{self.nonce}'!")

        if not 0 <= self.valid_to_time <= UINT64_MAX:
            raise TransactionError(f"Bad valid_to_time: '{self.valid_to_time}'!")

        if not 0 <= self.gas_cost <= UINT64_MAX:
            raise TransactionError(f"Bad gas_cost: '{self.gas_cost}'!")

        if len(self.contract_address) != ADDRESS_LENGTH:
            raise TransactionError(f"Bad address: '{self.contract_address.hex()}'!")

        if self.chain_id not in { b'TESTNET', b'MAINNET' }:
            raise TransactionError(f"Unknown chain id: '{self.chain_id}'!")

    def serialize(self) -> bytes:
        return b"".join([
            self.nonce.to_bytes(8, byteorder="big"),
            self.valid_to_time.to_bytes(8, byteorder="big"),
            self.gas_cost.to_bytes(8, byteorder="big"),
            self.contract_address,
            len(self.rpc).to_bytes(4, byteorder="big"),
            self.rpc,
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

    ''' TODO
    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hexa) if isinstance(hexa, bytes) else hexa

        nonce: int = read_uint(buf, 64, byteorder="big")
        to: bytes = read(buf, 20)
        value: int = read_uint(buf, 64, byteorder="big")
        memo: str = read(buf, memo_len).decode("ascii")

        return cls(nonce=nonce, to=to, value=value, memo=memo)
    '''
