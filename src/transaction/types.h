#pragma once

/**
 * \file These types match the specification given for the Partisia Blockchain.
 *
 * @see
 * https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html
 */

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "address.h"

/**
 * The maximum length of memo supported for #MPC_TRANSFER transactions. While
 * the MPC Token contract supports larger memoes, we need a limit for the
 * Ledger device.
 */
#define MEMO_MAX_LENGTH 20

/**
 * The number of decimals used when formatting MPC values. MPC values are
 * stored as unsigned integers of the smallest transferrable amount of MPC
 * tokens.
 */
#define MPC_TOKEN_DECIMALS 4

/**
 * Stores the state of the parser.
 *
 * The transaction parser is capable of parsing a streaming manner.
 */
typedef struct {
    /** Number of RPC bytes declared for the RPC. */
    uint32_t rpc_bytes_total;
    /** Number of RPC bytes read. */
    uint32_t rpc_bytes_parsed;
    /** Whether the first transaction has been parsed? */
    bool first_block_parsed;
} transaction_parsing_state_t;

/**
 * Parsing error messages.
 */
typedef enum {
    /** Parsing succeeded, and does not expect more data. */
    PARSING_DONE = 1,
    /** Parsing succeeded, and expects more data. */
    PARSING_CONTINUE = 2,
    /** Parsing failed while parsing nonce. */
    PARSING_FAILED_NONCE = -1,
    /** Parsing failed while parsing valid to time. */
    PARSING_FAILED_VALID_TO_TIME = -2,
    /** Parsing failed while parsing gas cost. */
    PARSING_FAILED_GAS_COST = -3,
    /** Parsing failed while parsing contract address. */
    PARSING_FAILED_CONTRACT_ADDRESS = -4,
    /** Parsing failed while parsing RPC length. */
    PARSING_FAILED_RPC_LENGTH = -5,
    /** Parsing failed while parsing RPC data. */
    PARSING_FAILED_RPC_DATA = -6,
} parser_status_e;

/**
 * Type of the parsed transaction. Not all transaction kinds are known, and
 * these will be sorted as #GENERIC_TRANSACTION.
 */
typedef enum {
    /** Transaction to an arbitrary contract. Requires blind-signing. */
    GENERIC_TRANSACTION = 0,
    /** MPC transfer involving the MPC Token contract. Can be clear-signed. */
    MPC_TRANSFER = 1
} transaction_type_e;

/**
 * Information about an MPC transfer transaction.
 */
typedef struct {
    /** Recipient of the MPC tokens. */
    blockchain_address_s recipient_address;
    /** Amount of MPC tokens sent. MPC values are
     * stored as unsigned integers of the smallest transferrable amount of MPC
     * tokens, and must be divided by 10000 when represented.
     *
     * @see MPC_TOKEN_DECIMALS
     */
    uint64_t token_amount_10000ths;
    /** Length of associated memo. */
    uint8_t memo_length;
    /** Tag for which memo field is relevant. */
    bool has_u64_memo;
    /** Contents of memo. */
    union {
        /** Contents of memo when memo is an u64. */
        uint64_t memo_u64;
        /** Contents of memo when memo is an string. Possibly cut off. */
        uint8_t memo[MEMO_MAX_LENGTH];
    };
} mpc_transfer_transaction_type_s;

/**
 * Must be large enough to be able to contain both "Partisia Blockchain" and "Partisia Blockchain
 * Testnet".
 */
#define CHAIN_ID_MAX_LENGTH 27

/**
 * Short id representing the chain that a transaction is targeting.
 */
typedef struct {
    uint8_t length;
    uint8_t raw_bytes[CHAIN_ID_MAX_LENGTH];
} chain_id_t;

/**
 * Basic and common transaction information
 *
 * Does not include RPC as these bytes are handled separately.
 *
 * @see
 * https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html
 */
typedef struct {
    uint64_t nonce;          /// nonce (not shown in UI)
    uint64_t valid_to_time;  /// last block height that transaction is valid for (not shown in UI)
    uint64_t gas_cost;       /// amount of gas to be used for this transaction
    blockchain_address_s contract_address;  /// contract address to interact with
} transaction_basic_t;

/**
 * Parsed transaction information, potentially including specific types of
 * well-known formats.
 *
 * @see
 * https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html
 */
typedef struct {
    /** Basic information. */
    transaction_basic_t basic;
    /** The type of the parsed transaction. */
    transaction_type_e type;
    /** Only when transaction_t.type == #MPC_TRANSFER */
    mpc_transfer_transaction_type_s mpc_transfer;
} transaction_t;
