
#include <string.h>  // memset
#include "format.h"
#include "io.h"

#include "common.h"
#include "../address.h"
#include "../types.h"

// Text buffer for transaction gas cost
char g_gas_cost[PRIu64_MAX_LENGTH + 1];
// Text buffer for MPC transfer amounts
char g_transfer_amount[TOKEN_SUFFIX_LEN + 1 + PRIu64_MAX_LENGTH + 1];
// Text buffer for MPC transfer recipient or contract address
char g_address[2 * ADDRESS_LEN + 1];
// Text buffer for MPC transfer memo
char g_memo[MEMO_MAX_LENGTH + 1];
// Text buffer for Chain Id
char g_chain_id[CHAIN_ID_MAX_LENGTH + 1];

/**
 * Formats a blockchain_address_s as a hex string.
 */
WARN_UNUSED_RESULT
static bool blockchain_address_format(blockchain_address_s* address, char* out, size_t out_len) {
    memset(g_address, 0, sizeof(g_address));
    return format_hex(address->raw_bytes, ADDRESS_LEN, out, out_len) != -1;
}

/**
 * Replaces non-ASCII and non-readable characters with ?.
 */
static void replace_unreadable(char* str, size_t str_len) {
    for (size_t i = 0; i < str_len; i++) {
        if (str[i] == 0) {
            continue;
        } else if (str[i] < ' ' || '~' < str[i]) {
            str[i] = '?';
        }
    }
}

/**
 * Sets the memo text.
 */
static void set_g_memo_text(uint8_t* text, size_t text_len) {
    size_t g_memo_len = sizeof(g_memo);
    size_t copy_amount = text_len < g_memo_len ? text_len : g_memo_len;

    memcpy(g_memo, text, copy_amount);
    replace_unreadable(g_memo, copy_amount);
    g_memo[copy_amount - 1] = 0;
}

WARN_UNUSED_RESULT
bool set_g_address(blockchain_address_s* address) {
    memset(g_address, 0, sizeof(g_address));
    return blockchain_address_format(address, g_address, sizeof(g_address));
}

WARN_UNUSED_RESULT
bool set_g_chain_id(chain_id_t* chain_id) {
    int num_written_chars = snprintf(g_chain_id,
                                     sizeof(g_chain_id),
                                     "%.*s",
                                     (int) chain_id->length,
                                     chain_id->raw_bytes);
    if (!(0 <= num_written_chars && (size_t) num_written_chars < sizeof(g_chain_id))) {
        return false;
    }
    replace_unreadable(g_chain_id, sizeof(g_chain_id));
    return true;
}

WARN_UNUSED_RESULT
bool set_g_token_amount(char* out,
                        size_t out_size,
                        const char suffix[const TOKEN_SUFFIX_LEN + 1],
                        uint64_t amount,
                        uint8_t decimals) {
    memset(out, 0, out_size);
    char number_buffer[30] = {0};
    if (decimals == 0) {
        if (!format_u64(number_buffer, sizeof(number_buffer), amount)) {
            return false;
        }
    } else {
        if (!format_fpu64(number_buffer, sizeof(number_buffer), amount, decimals)) {
            return false;
        }
    }

    int num_written_chars = snprintf(out, out_size, "%s %s", number_buffer, suffix);
    return 0 <= num_written_chars && (size_t) num_written_chars < out_size;
}

WARN_UNUSED_RESULT
bool set_g_fields_for_mpc_transfer(mpc_transfer_transaction_type_s* mpc_transfer) {
    // Display recipient
    if (!set_g_address(&mpc_transfer->recipient_address)) {
        return false;
    }

    // Display token transfer amount
    if (!set_g_token_amount(g_transfer_amount,
                            sizeof(g_transfer_amount),
                            "MPC",
                            mpc_transfer->token_amount_10000ths,
                            MPC_TOKEN_DECIMALS)) {
        return false;
    }

    // Display Memo
    if (mpc_transfer->memo_length > 0) {
        if (mpc_transfer->has_u64_memo) {
            return set_g_token_amount(g_memo, sizeof(g_memo), "   ", mpc_transfer->memo_u64, 0);
        } else {
            set_g_memo_text(mpc_transfer->memo, sizeof(mpc_transfer->memo));
        }
    }

    return true;
}
