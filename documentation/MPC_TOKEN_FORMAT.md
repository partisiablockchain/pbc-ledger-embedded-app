
# MPC Token Format

The MPC Token Contract (Contract Address [`01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881`](https://browser.partisiablockchain.com/contracts/01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881)) manages the goverance token of Partisia Blockchain, the
[MPC tokens](https://partisiablockchain.gitlab.io/documentation/pbc-fundamentals/dictionary.html#mpc-token) themselves.

This page will cover the few interactions that are explicitly supported by the
Partisia Blockchain Ledger App. Read [The MPC Token Contract Javadoc](https://partisiablockchain.gitlab.io/governance/mpc-token/com/partisiablockchain/governance/mpctoken/MpcTokenContract.html) for a complete overview.

## Interactions

### Transfer (no memo; shortname: `0x03`)

Transfers some amount of MPC from the sender to the recipient as specified in
the RPC:

| Field | Size (bytes) | Description |
| --- | :---: | --- |
| `shortname` | 1 | Always `0x03`. |
| `recipient` | 21 | Blockchain address of the recipient. |
| `amount` | 8 | Amount of MPC tokens to send in 10000ths of MPC. |

### Transfer (short memo; shortname: `0x0D`)

Transfers some amount of MPC from the sender to the recipient as specified in
the RPC. Requires a 8-byte memo. This memo is discarded by the MPC Token
contract, but can be read by third-parties directly from the transaction.

| Field | Size (bytes) | Description |
| --- | :---: | --- |
| `shortname` | 1 | Always `0x0D`. |
| `recipient` | 21 | Blockchain address of the recipient. |
| `amount` | 8 | Amount of MPC tokens to send in 10000ths of MPC. |
| `memo` | 8 | Memo as a `u64`. |

### Transfer (long memo; shortname: `0x17`)

Transfers some amount of MPC from the sender to the recipient as specified in
the RPC. Requires a N-byte memo. This memo is discarded by the MPC Token
contract, but can be read by third-parties directly from the transaction.

| Field | Size (bytes) | Description |
| --- | :---: | --- |
| `shortname` | 1 | Always `0x17`. |
| `recipient` | 21 | Blockchain address of the recipient. |
| `amount` | 8 | Amount of MPC tokens to send in 10000ths of MPC. |
| `memo_len` | 4 | Length of string memo. |
| `memo` | `memo_len` | Memo data itself. |

