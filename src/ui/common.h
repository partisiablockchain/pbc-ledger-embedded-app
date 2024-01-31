#pragma once

#include "../address.h"
#include "../types.h"

#define PRIu64_MAX_LENGTH 20
#define TOKEN_SUFFIX_LEN  3

// Text buffer for transaction gas cost
static char g_gas_cost[PRIu64_MAX_LENGTH + 1];
// Text buffer for MPC transfer amounts
static char g_transfer_amount[TOKEN_SUFFIX_LEN + 1 + PRIu64_MAX_LENGTH + 1];
// Text buffer for MPC transfer recipient or contract address
static char g_address[2 * ADDRESS_LEN + 1];
// Text buffer for MPC transfer memo
static char g_memo[MEMO_MAX_LENGTH + 1];  // TODO

/**
 * Formats a blockchain_address_s as a hex string.
 */
static bool blockchain_address_format(blockchain_address_s* address, char* out, size_t out_len) {
    memset(g_address, 0, sizeof(g_address));
    return format_hex(address->raw_bytes, ADDRESS_LEN, out, out_len) != -1;
}

static void replace_unreadable(char* str, size_t str_len) {
    for (size_t i = 0; i < str_len; i++) {
        if (str[i] == 0) {
            continue;
        } else if (str[i] < ' ' || '~' < str[i]) {
            str[i] = '?';
        }
    }
}

static void set_memo_text(uint8_t* text, size_t text_len) {
    // TODO: Return error if not fully written
    size_t g_memo_len = sizeof(g_memo);
    size_t copy_amount = text_len < g_memo_len ? text_len : g_memo_len;

    memcpy(g_memo, text, copy_amount);
    replace_unreadable(g_memo, copy_amount);
    g_memo[copy_amount - 1] = 0;
}

static bool set_address(blockchain_address_s* address) {
    memset(g_address, 0, sizeof(g_address));
    return blockchain_address_format(address, g_address, sizeof(g_address));
}

static void set_token_amount(char* out,
                             size_t out_size,
                             const char suffix[const TOKEN_SUFFIX_LEN + 1],
                             uint64_t gas_cost) {
    // TODO: Return error if amount is not fully written
    char number_buffer[PRIu64_MAX_LENGTH + 1];
    memset(number_buffer, 0, sizeof(number_buffer));
    format_u64(number_buffer, sizeof(number_buffer), gas_cost);
    snprintf(out, out_size, "%s %s", number_buffer, suffix);
}


static void set_text_fields_for_mpc_transfer(mpc_transfer_transaction_type_s* mpc_transfer) {
    // Display recipient
    set_address(&mpc_transfer->recipient_address);

    // Display token transfer amount
    set_token_amount(g_transfer_amount,
                     sizeof(g_transfer_amount),
                     "MPC",
                     mpc_transfer->token_amount);

    // Display Memo
    if (mpc_transfer->memo_length > 0) {
        if (mpc_transfer->has_u64_memo) {
            set_token_amount(g_memo,
                             sizeof(g_memo),
                             "",
                             mpc_transfer->memo_u64);
        } else {
            set_memo_text(mpc_transfer->memo,
                          sizeof(mpc_transfer->memo));
        }
    }
}

