#pragma once

/**
 * These types match the specification given for the Partisia Blockchain.
 *
 * @see https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html
 */

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#define ADDRESS_LEN  21   // TODO
#define MAX_MEMO_LEN 465  // TODO

/**
 * Stores the state of the parser.
 *
 * The transaction parser is capable of parsing a streaming manner.
 */
typedef struct {
  uint32_t rpc_bytes_total;     // Number of RPC bytes declared for the RPC.
  uint32_t rpc_bytes_parsed;    // Number of RPC bytes read.
  bool first_block_parsed;
} transaction_parsing_state_t;

/**
 * Parsing error messages.
 */
typedef enum {
    PARSING_DONE = 1,     // Parsing finished.
    PARSING_CONTINUE = 2, // Parsing is not done.
    PARSING_FAILED = -1   // Some error occured while parsing.
} parser_status_e;


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
 * @see https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html
 */
typedef struct {
  uint64_t nonce;                         /// nonce (8 bytes)
  uint64_t valid_to_time;                 /// last block height that transaction is valid for (8 bytes)
  uint64_t gas_cost;                      /// amount of gas to be used for this transaction (8 bytes)
  uint8_t contract_address[ADDRESS_LEN];  /// contract address to interact with (ADDRESS_LEN bytes)
  chain_id_t chain_id;                    /// which chain the transaction is targeting (1 byte)
  /// Does not include RPC. Is handled separately
} transaction_t; // TODO: transaction_t is a bit disengenious

