# Partisia Blockchain Transaction Serialization

## Overview

This page presents a quick overview of the Partisia Blockchain serialization
format, with focus on the Ledger use cases. For a more complete overview, see
[The Partisia Blockchain Documentation](https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html) and the
[Java reference implementation](https://gitlab.com/partisiablockchain/core/blockchain).

### Address format

PBC addresses are 21-byte long hexadecimal numbers, identifiers derived from the last 20 bytes of the SHA256 hash of the public key, prefixed by an address type, which is one of the following values:

| Id | Address Type | Description |
| --- | --- | --- |
| `00` | #BLOCKCHAIN_ADDRESS_ACCOUNT | External users that have an account on the blockchain. |
| `01` | #BLOCKCHAIN_ADDRESS_CONTRACT_SYSTEM | System contracts are the public part of the operating system on the blockchain. |
| `02` | #BLOCKCHAIN_ADDRESS_CONTRACT_PUBLIC | Public contracts are regular smart contract on the blockchain. |
| `03` | #BLOCKCHAIN_ADDRESS_CONTRACT_ZK | Zero knowledge contracts are the more complicated contracts that are similar to public contracts, but have confidential state as well.  |
| `04` | #BLOCKCHAIN_ADDRESS_CONTRACT_GOVERNANCE | Governance contracts are a special type of system contract that is deemed necessary for the system to run and therefore are (mostly) free to interact with.  |

Examples of governance and system contracts can be found in the [Partisia Blockchain Governance Contracts Overview](https://partisiablockchain.gitlab.io/documentation/pbc-fundamentals/governance-system-smart-contracts-overview.html).

### Transaction

Transactions are laid out as follows:

| Field | Size (bytes) | Description |
| --- | :---: | --- |
| `nonce` | 8 | A sequence number used to prevent message replay. |
| `valid_to_time` | 8 | The maximum block time/height that the transaction can be included in. |
| `gas_cost` | 8 | The amount of gas to be used to run the transaction. |
| `contract_address` | 21 | Contract to invoke. |
| `rpc_length` | 4 | Length of RPC data. |
| `rpc_data` | `rpc_length` | Length of RPC data. |

Big-endianness is used for all integers.

### Chain ID

Each instance of Partisia Blockchain has a unique chain ID, to distinguish
them. The ids are used when creating a signature, to prevent replaying of
transactions across chains. Ids are always prefixed by their size as a u32. The most relevant ids are:

| Id | Blockchain | Browser link |
| --- | --- | --- |
| `0x00000013 .. "Partisia Blockchain"` | Partisia Blockchain Mainnet | [Browser](https://browser.partisiablockchain.com/blocks) |
| `0x0000001B .. "Partisia Blockchain Testnet"` | Partisia Blockchain Testnet | [Browser](https://browser.testnet.partisiablockchain.com/blocks) |

### Signature

Deterministic ECDSA ([RFC 6979](https://tools.ietf.org/html/rfc6979)) is used to sign transaction on the [SECP-256k1](https://www.secg.org/sec2-v2.pdf#subsubsection.2.4.1) curve.

The signed message is the SHA256 hash of the transaction concatted with the
chain identifier: `m = SHA256(transaction || chain_id)`.

The signature is transmitted as a 65-byte array, and we avoid using the [BIP66 DER format](https://bips.dev/66/). Signatures are transmitted as follows:

| Field | Size (bytes) | Description |
| --- | :---: | --- |
| `recovery_id` | 1 | Recovery ID. |
| `r` | 32 | R value. |
| `s` | 32 | S value. |

### Gas Fee

Gas and fees on Partisia Blockchain are separated from the [MPC governance token](documentation/MPC_TOKEN_FORMAT.md), and is implemented as Bring-Your-Own-Coin (BYOC), allowing blockchain users to pay for their activities using several different currencies. Choosing which currency to use is automatically handled by the blockchain.
