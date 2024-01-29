#pragma once

/**
 * These types match the specification given for the Partisia Blockchain.
 *
 * @see
 * https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html
 */

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "address.h"

#define MEMO_MAX_LENGTH 20

/**
 * Stores the state of the parser.
 *
 * The transaction parser is capable of parsing a streaming manner.
 */
typedef struct {
    uint32_t rpc_bytes_total;   // Number of RPC bytes declared for the RPC.
    uint32_t rpc_bytes_parsed;  // Number of RPC bytes read.
    bool first_block_parsed;
} transaction_parsing_state_t;

/**
 * Parsing error messages.
 */
typedef enum {
    PARSING_DONE = 1,      // Parsing finished.
    PARSING_CONTINUE = 2,  // Parsing is not done.
    PARSING_FAILED_NONCE = -1,
    PARSING_FAILED_VALID_TO_TIME = -2,
    PARSING_FAILED_GAS_COST = -3,
    PARSING_FAILED_CONTRACT_ADDRESS = -4,
    PARSING_FAILED_RPC_LENGTH = -5,
    PARSING_FAILED_RPC_DATA = -6,
} parser_status_e;

/**
 * Type of the parsed transaction. Not all transaction kinds are known, and
 * these will be sorted as GENERIC.
 */
typedef enum {
    GENERIC_TRANSACTION = 0,  // A transaction to an arbitrary contract. Requires blind-signing
    MPC_TRANSFER = 1
} transaction_type_e;

/**
 *
 */
typedef struct {
    blockchain_address_s recipient_address;  // Recipient of the MPC tokens.
    uint64_t token_amount;                   // Amount of MPC tokens sent
    uint8_t memo_length;                     // Length of associated memo.
                                             // Possibly cut off.
    bool has_u64_memo;
    union {
        uint64_t memo_u64;
        uint8_t memo[MEMO_MAX_LENGTH];  // Contents of associated memo
                                        // Possibly cut off.
    };
} mpc_transfer_transaction_type_s;

/**
 * Short id representing the chain that a transaction is targeting.
 */
typedef enum {
    UNKNOWN = 0,
    MAINNET = 1,
    TESTNET = 2,
} chain_id_t;

/**
 * Basic and common transaction information
 *
 * @see
 * https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html
 */
typedef struct {
    uint64_t nonce;          /// nonce (not shown in UI)
    uint64_t valid_to_time;  /// last block height that transaction is valid for (not shown in UI)
    uint64_t gas_cost;       /// amount of gas to be used for this transaction
    blockchain_address_s contract_address;  /// contract address to interact with
    chain_id_t chain_id;                    /// which chain the transaction is targeting
                                            /// Does not include RPC. Is handled separately
} transaction_basic_t;                      // TODO: transaction_t is a bit disengenious

/**
 * Parsed transaction information, potentially including specific types of
 * well-known formats.
 *
 * @see
 * https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html
 */
typedef struct {
    transaction_basic_t basic;  // Basic information
    transaction_type_e type;    // The type of the parsed transaction
    union {
        mpc_transfer_transaction_type_s mpc_transfer;  // Only when type == MPC_TRANSFER
    };
} transaction_t;
