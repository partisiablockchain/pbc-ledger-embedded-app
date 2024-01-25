from io import BytesIO
from typing import Union

import dataclasses

from .boilerplate_utils import read, read_uint, read_varint, write_varint, UINT64_MAX


class TransactionError(Exception):
    pass


ADDRESS_LENGTH = 21


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

    def __postinit__(self):
        if not 0 <= self.nonce <= UINT64_MAX:
            raise TransactionError(f"Bad nonce: '{self.nonce}'!")

        if not 0 <= self.value <= UINT64_MAX:
            raise TransactionError(f"Bad value: '{self.value}'!")

        if len(self.to) != ADDRESS_LENGTH:
            raise TransactionError(f"Bad address: '{self.to.hex()}'!")

    def serialize(self) -> bytes:
        return b"".join([
            self.nonce.to_bytes(8, byteorder="big"),
            self.valid_to_time.to_bytes(8, byteorder="big"),
            self.gas_cost.to_bytes(8, byteorder="big"),
            self.contract_address,
            len(self.rpc).to_bytes(5, byteorder="big"),
            self.rpc,
        ])

    ''' TODO
    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hexa) if isinstance(hexa, bytes) else hexa

        nonce: int = read_uint(buf, 64, byteorder="big")
        to: bytes = read(buf, 20)
        value: int = read_uint(buf, 64, byteorder="big")
        memo_len: int = read_varint(buf)
        memo: str = read(buf, memo_len).decode("ascii")

        return cls(nonce=nonce, to=to, value=value, memo=memo)
    '''
