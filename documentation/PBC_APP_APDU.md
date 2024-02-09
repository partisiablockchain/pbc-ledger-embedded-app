# Technical Specification of Partisia Blockchain Ledger App

## About

This documentation describes the APDU messages interface to communicate with Partisia Blockchain Ledger App.

The application covers the following functionalities :

  - Get a blockchain address given a [BIP 32 path](https://bips.dev/32/)
  - Sign a basic PBC transaction given a BIP 32 path and raw transaction
  - Retrieve the PBC app version
  - Retrieve the PBC app name

The application interface can be accessed over HID or BLE.

## APDUs

### GET PBC PUBLIC ADDRESS

#### Description

This command returns the blockchain address for the given BIP 32 path.

The address can be optionally checked on the device before being returned.

#### Coding

##### `Command`

| CLA  | INS   | P1                                                 | P2    | Lc       | Le       |
| ---  | ---   | ---                                                | ---   | ---      | ---      |
| `E0` | `05`  | `00`: return address                               |`00`   | variable | variable |
|      |       | `01`: display address and confirm before returning |       |          |          |

##### `Input data`

| Description                                                      | Length |
| ---                                                              | ---    |
| Number of BIP 32 derivations to perform (max 10)                 | 1      |
| First derivation index (big endian)                              | 4      |
| ...                                                              | 4      |
| Last derivation index (big endian)                               | 4      |

##### `Output data`

| Description                                                      | Length |
| ---                                                              | ---    |
| Blockchain address                                               | 21     |


### SIGN PBC TRANSACTION

#### Description

This command signs a PBC transaction after having the user validate the transactions parameters.

The input data is the transaction streamed to the device in 255 bytes maximum
data chunks. The ID of the chain to sign for must be sent in the first block.

#### Coding

##### `Command`

| CLA | INS  | P1                 | P2               | Lc       | Le       |
| --- | ---  | ---                | ---              | ---      | ---      |
|`E0` |`06`  | `00` : first chunk | `00`: last chunk | variable | variable |
|     |      | `01` : not first   | `01`: not last   |          |          |

Chunk are expected to be ordered as:

| `P1=0 P2=1` (First) | `P1=1 P2=1` (Middle) | ... | `P1=1 P2=0` (Last) |

##### `Input data (first transaction data block)`

| Description                                          | Length   |
| ---                                                  | ---      |
| Number of BIP 32 derivations to perform (max 10)     | 1        |
| First derivation index (big endian)                  | 4        |
| ...                                                  | 4        |
| Last derivation index (big endian)                   | 4        |
| Chain ID Length (`N`)                                | 4        |
| Chain ID                                             | `N`      |

##### `Input data (other transaction data block)`

| Description                                          | Length   |
| ---                                                  | ---      |
| Transaction chunk                                    | variable |


##### `Output data`

| Description                                          | Length   |
| ---                                                  | ---      |
| Signature recovery id                                | 1  |
| Signature R                                          | 32 |
| Signature S                                          | 32 |

### GET APP VERSION

#### Description

This command returns PBC application version

#### Coding

##### `Command`

| CLA | INS | P1  | P2  | Lc   | Le |
| --- | --- | --- | --- | ---  | ---|
| `E0`  | `03`  | `00`  | `00`  | `00`   | `04` |

##### `Input data`

None

##### `Output data`

| Description                       | Length |
| ---                               | ---    |
| Application major version         | 1 |
| Application minor version         | 1 |
| Application patch version         | 1 |


### GET APP NAME

#### Description

This command returns the name of the running application.

#### Coding

##### `Command`

| CLA | INS | P1  | P2  | Lc   | Le |
| --- | --- | --- | --- | ---  | ---|
| `E0`  | `04`  | `00`  | `00`  | `00`   | `04` |

##### `Input data`

None

##### `Output data`

| Description           | Length   |
| ---                   | ---      |
| Application name      | variable |


## Status Words

The following standard Status Words are returned for all APDUs.

##### `Status Words`


| Status Word | Status Word Name           | Description                                           |
| ---      | ---                           | ---                                                   |
|  `9000`  | #SW_OK                       | Success                                               |
|  `6985`  | #SW_DENY	                   | Rejected by user                                      |
|  `6A86`  | #SW_WRONG_P1P2               | Either P1 or P2 is incorrect                          |
|  `6A87`  | #SW_WRONG_DATA_LENGTH        | Lc or minimum APDU length is incorrect                |
|  `6D00`  | #SW_INS_NOT_SUPPORTED	       | No command exists with INS                            |
|  `6E00`  | #SW_CLA_NOT_SUPPORTED        | Bad CLA used for this application                     |
|  `B000`  | #SW_WRONG_RESPONSE_LENGTH    | Wrong response length (buffer size problem)           |
|  `B001`  | #SW_DISPLAY_BIP32_PATH_FAIL  | BIP32 path conversion to string failed                |
|  `B002`  | #SW_DISPLAY_ADDRESS_FAIL     | Address conversion to string failed                   |
|  `B003`  | #SW_DISPLAY_AMOUNT_FAIL      | Amount conversion to string failed                    |
|  `B004`  | #SW_WRONG_TX_LENGTH          | Wrong raw transaction length                          |
|  `B005`  | #SW_INVALID_CHAIN_ID         | Parsing of chain id failed.                           |
|  `B006`  | #SW_TX_HASH_FAIL             | Failure in creating transaction hash.                 |
|  `B007`  | #SW_BAD_STATE                | Application ended in a bad state.                     |
|  `B008`  | #SW_SIGNATURE_FAIL           | Unable to sign transaction.                           |
|  `B009`  | #SW_TX_PARSING_FAIL_EXPECTED_MORE_DATA           | Parsing of transaction failed, due to missing data. |
|  `B1XX`  | #SW_TX_PARSING_FAIL `XX`                          | Parsing of transaction failed. Variants listed below. |
|  `B101`  | #SW_TX_PARSING_FAIL #PARSING_FAILED_NONCE         | Failed to parse nonce. |
|  `B102`  | #SW_TX_PARSING_FAIL #PARSING_FAILED_VALID_TO_TIME | Failed to parse valid-to-time. |
|  `B103`  | #SW_TX_PARSING_FAIL #PARSING_FAILED_GAS_COST      | Failed to parse gas cost. |
|  `B104`  | #SW_TX_PARSING_FAIL #PARSING_FAILED_CONTRACT_ADDRESS | Failed to parse contract address. |
|  `B105`  | #SW_TX_PARSING_FAIL #PARSING_FAILED_RPC_LENGTH    | Failed to parse RPC length. |
|  `B106`  | #SW_TX_PARSING_FAIL #PARSING_FAILED_RPC_DATA      | Failed to parse RPC data. |
