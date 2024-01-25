#pragma once

/**
 * These types match the specification given for the Partisia Blockchain.
 *
 * @see https://partisiablockchain.gitlab.io/documentation/smart-contracts/transaction-binary-format.html
 */

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#define MAX_TX_LEN   510  // TODO
#define ADDRESS_LEN  21   // TODO
#define MAX_MEMO_LEN 465  // TODO

/**
 * Parsing error messages.
 */
typedef enum {
    PARSING_OK = 1,
    NONCE_PARSING_ERROR = -1,
    TO_PARSING_ERROR = -2,
    VALUE_PARSING_ERROR = -3,
    MEMO_LENGTH_ERROR = -4,
    MEMO_PARSING_ERROR = -5,
    MEMO_ENCODING_ERROR = -6,
    WRONG_LENGTH_ERROR = -7
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
  uint8_t[ADDRESS_LEN] contract_address;  /// contract address to interact with (ADDRESS_LEN bytes)
  chain_id_t chain_id;                    /// which chain the transaction is targeting (1 byte)
  /// Does not include RPC. Is handled separately
} transaction_basics_t;

