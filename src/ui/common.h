#pragma once

#include <stdbool.h>  // bool

#include "../address.h"
#include "../types.h"

#define PRIu64_MAX_LENGTH 20
#define TOKEN_SUFFIX_LEN  3

/*** Common UI fields ***/

// Text buffer for transaction gas cost
extern char g_gas_cost[PRIu64_MAX_LENGTH + 1];
// Text buffer for MPC transfer amounts
extern char g_transfer_amount[TOKEN_SUFFIX_LEN + 1 + PRIu64_MAX_LENGTH + 1];
// Text buffer for MPC transfer recipient or contract address
extern char g_address[2 * ADDRESS_LEN + 1];
// Text buffer for MPC transfer memo
extern char g_memo[MEMO_MAX_LENGTH + 1];
// Whether the #g_memo field contains any useful data.
extern bool g_memo_contains_text;
// Text buffer for Chain Id
extern char g_chain_id[CHAIN_ID_MAX_LENGTH + 1];

/*** Common UI methods ***/

/**
 * Replaces the displayed address with the given address.
 *
 * @return false when field failed to be displayed.
 */
WARN_UNUSED_RESULT
bool set_g_address(blockchain_address_s* address);

/**
 * Replaces the displayed token amount with the given amount with a suffix.
 *
 * @return false when field failed to be displayed.
 */
WARN_UNUSED_RESULT
bool set_g_token_amount(char* out,
                        size_t out_size,
                        const char suffix[const TOKEN_SUFFIX_LEN + 1],
                        uint64_t amount,
                        uint8_t decimals);

/**
 * Replaces the fields for displaying an MPC transfer with the values from the
 * given MPC transfer.
 *
 * @return false when any field failed to be displayed.
 */
WARN_UNUSED_RESULT
bool set_g_fields_for_mpc_transfer(mpc_transfer_transaction_type_s* mpc_transfer);

/**
 * Replaces the fields for displaying the chain id, with the given chain id.
 *
 * @return false when any field failed to be displayed.
 */
WARN_UNUSED_RESULT
bool set_g_chain_id(chain_id_t* chain_id);
