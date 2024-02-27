#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "varint.h"
#include "write.h"
#include "buffer.h"

#include <cmocka.h>

#include "well_known.h"
#include "buffer_util.h"
#include "transaction/deserialize.h"

/// Transactions

// clang-format off
static uint8_t TRANSACTION_BYTES_GENERIC_TRANSACTION[] = {
    // nonce (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
    // valid-to time (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04,
    // gas cost (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06,
    // contract address (21)
    0x02, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75,
    0xd2, 0x66, 0xbd, 0x02, 0x24, 0x39, 0xb2, 0x2c,
    0xdb, 0x16, 0x50, 0x8c, 0x7a,
    // rpc length (4)
    0x00, 0x00, 0x00, 0xf1,
    // rpc (var: 241)
    0x54, 0x68, 0x65, 0x20, 0x54, 0x68, 0x65, 0x6f,
    0x72, 0x79, 0x20, 0x6f, 0x66, 0x20, 0x47, 0x72,
    0x6f, 0x75, 0x70, 0x73, 0x20, 0x69, 0x73, 0x20,
    0x61, 0x20, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68,
    0x20, 0x6f, 0x66, 0x20, 0x6d, 0x61, 0x74, 0x68,
    0x65, 0x6d, 0x61, 0x74, 0x69, 0x63, 0x73, 0x20,
    0x69, 0x6e, 0x20, 0x77, 0x68, 0x69, 0x63, 0x68,
    0x20, 0x6f, 0x6e, 0x65, 0x20, 0x64, 0x6f, 0x65,
    0x73, 0x20, 0x73, 0x6f, 0x6d, 0x65, 0x74, 0x68,
    0x69, 0x6e, 0x67, 0x20, 0x74, 0x6f, 0x20, 0x73,
    0x6f, 0x6d, 0x65, 0x74, 0x68, 0x69, 0x6e, 0x67,
    0x20, 0x61, 0x6e, 0x64, 0x20, 0x74, 0x68, 0x65,
    0x6e, 0x20, 0x63, 0x6f, 0x6d, 0x70, 0x61, 0x72,
    0x65, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x72,
    0x65, 0x73, 0x75, 0x6c, 0x74, 0x20, 0x77, 0x69,
    0x74, 0x68, 0x20, 0x74, 0x68, 0x65, 0x20, 0x72,
    0x65, 0x73, 0x75, 0x6c, 0x74, 0x20, 0x6f, 0x62,
    0x74, 0x61, 0x69, 0x6e, 0x65, 0x64, 0x20, 0x66,
    0x72, 0x6f, 0x6d, 0x20, 0x64, 0x6f, 0x69, 0x6e,
    0x67, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x61,
    0x6d, 0x65, 0x20, 0x74, 0x68, 0x69, 0x6e, 0x67,
    0x20, 0x74, 0x6f, 0x20, 0x73, 0x6f, 0x6d, 0x65,
    0x74, 0x68, 0x69, 0x6e, 0x67, 0x20, 0x65, 0x6c,
    0x73, 0x65, 0x2c, 0x20, 0x6f, 0x72, 0x20, 0x73,
    0x6f, 0x6d, 0x65, 0x74, 0x68, 0x69, 0x6e, 0x67,
    0x20, 0x65, 0x6c, 0x73, 0x65, 0x20, 0x74, 0x6f,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x61, 0x6d,
    0x65, 0x20, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x2e,
    0x20, 0x4e, 0x65, 0x77, 0x6d, 0x61, 0x6e, 0x2c,
    0x20, 0x4a, 0x61, 0x6d, 0x65, 0x73, 0x20, 0x52,
    0x2e
};

static uint8_t TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO[] = {
    // nonce (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
    // valid-to time (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04,
    // gas cost (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06,
    // contract address (21): MPC token contract
    0x01, 0xa4, 0x08, 0x2d, 0x9d, 0x56, 0x07, 0x49,
    0xec, 0xd0, 0xff, 0xa1, 0xdc, 0xaa, 0xae, 0xe2,
    0xc2, 0xcb, 0x25, 0xd8, 0x81,
    // rpc length (4): 30
    0x00, 0x00, 0x00, 0x1e,
    // shortname (1)
    0x03,
    // recipient (21)
    0x00, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75,
    0xd2, 0x66, 0xbd, 0x02, 0x24, 0x39, 0xb2, 0x2c,
    0xdb, 0x16, 0x50, 0x8c, 0x7a,
    // Token amount (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33,
};

static uint8_t TRANSACTION_BYTES_MPC_TRANSFER_TOO_MANY_BYTES[] = {
    // nonce (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
    // valid-to time (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04,
    // gas cost (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06,
    // contract address (21): MPC token contract
    0x01, 0xa4, 0x08, 0x2d, 0x9d, 0x56, 0x07, 0x49,
    0xec, 0xd0, 0xff, 0xa1, 0xdc, 0xaa, 0xae, 0xe2,
    0xc2, 0xcb, 0x25, 0xd8, 0x81,
    // rpc length (4): 32
    0x00, 0x00, 0x00, 0x20,
    // shortname (1)
    0x03,
    // recipient (21)
    0x00, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75,
    0xd2, 0x66, 0xbd, 0x02, 0x24, 0x39, 0xb2, 0x2c,
    0xdb, 0x16, 0x50, 0x8c, 0x7a,
    // Token amount (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33,
    // Extranious bytes (2)
    0x00, 0x00,
};

static uint8_t TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO[] = {
    // 0: nonce (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
    // 8: valid-to time (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04,
    // 16: gas cost (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06,
    // 24: contract address (21): MPC token contract
    0x01, 0xa4, 0x08, 0x2d, 0x9d, 0x56, 0x07, 0x49,
    0xec, 0xd0, 0xff, 0xa1, 0xdc, 0xaa, 0xae, 0xe2,
    0xc2, 0xcb, 0x25, 0xd8, 0x81,
    // 45: rpc length (4): 38
    0x00, 0x00, 0x00, 0x26,
    // 49: shortname (1)
    0x0d,
    // 50: recipient (21)
    0x00, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75,
    0xd2, 0x66, 0xbd, 0x02, 0x24, 0x39, 0xb2, 0x2c,
    0xdb, 0x16, 0x50, 0x8c, 0x7a,
    // 71: Token amount (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33,
    // 79: Small memo (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x99,
};

static uint8_t TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO[] = {
    // nonce (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
    // valid-to time (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04,
    // gas cost (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06,
    // contract address (21): MPC token contract
    0x01, 0xa4, 0x08, 0x2d, 0x9d, 0x56, 0x07, 0x49,
    0xec, 0xd0, 0xff, 0xa1, 0xdc, 0xaa, 0xae, 0xe2,
    0xc2, 0xcb, 0x25, 0xd8, 0x81,
    // rpc length (4): 30
    0x00, 0x00, 0x00, 30 + 4 + 15,
    // shortname (1)
    0x17,
    // recipient (21)
    0x00, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75,
    0xd2, 0x66, 0xbd, 0x02, 0x24, 0x39, 0xb2, 0x2c,
    0xdb, 0x16, 0x50, 0x8c, 0x7a,
    // Token amount (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33,
    // Large memo size (4)
    0x00, 0x00, 0x00, 0x0b,
    // Large memo data (var)
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd',
};

static uint8_t TRANSACTION_BYTES_MPC_TRANSFER_VERY_LARGE_MEMO_PART_1[255] = {
    // nonce (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
    // valid-to time (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04,
    // gas cost (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06,
    // contract address (21): MPC token contract
    0x01, 0xa4, 0x08, 0x2d, 0x9d, 0x56, 0x07, 0x49,
    0xec, 0xd0, 0xff, 0xa1, 0xdc, 0xaa, 0xae, 0xe2,
    0xc2, 0xcb, 0x25, 0xd8, 0x81,
    // rpc length (4)
    0x00, 0x00, 0x01, 0x42,
    // shortname (1)
    0x17,
    // recipient (21)
    0x00, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75,
    0xd2, 0x66, 0xbd, 0x02, 0x24, 0x39, 0xb2, 0x2c,
    0xdb, 0x16, 0x50, 0x8c, 0x7a,
    // Token amount (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33,
    // Large memo size (4)
    0x00, 0x00, 0x01, 0x20,
    // Large memo data (var)
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
};

static uint8_t TRANSACTION_BYTES_MPC_TRANSFER_VERY_LARGE_MEMO_PART_2[] = {
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n',
};

static uint8_t TRANSACTION_BYTES_MPC_TRANSFER_UNKNOWN_SHORTNAME[] = {
    // 0: nonce (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
    // 8: valid-to time (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04,
    // 16: gas cost (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06,
    // 24: contract address (21): MPC token contract
    0x01, 0xa4, 0x08, 0x2d, 0x9d, 0x56, 0x07, 0x49,
    0xec, 0xd0, 0xff, 0xa1, 0xdc, 0xaa, 0xae, 0xe2,
    0xc2, 0xcb, 0x25, 0xd8, 0x81,
    // 45: rpc length (4): 38
    0x00, 0x00, 0x00, 0x26,
    // 49: shortname (1)
    0x7f,
};

// clang-format on

static uint8_t ADDRESS_GENERIC_CONTRACT[21] = {
    0x02, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75, 0xd2, 0x66, 0xbd,
    0x02, 0x24, 0x39, 0xb2, 0x2c, 0xdb, 0x16, 0x50, 0x8c, 0x7a,
};

static uint8_t ADDRESS_MPC_TOKEN[21] = {
    0x01, 0xa4, 0x08, 0x2d, 0x9d, 0x56, 0x07, 0x49, 0xec, 0xd0, 0xff,
    0xa1, 0xdc, 0xaa, 0xae, 0xe2, 0xc2, 0xcb, 0x25, 0xd8, 0x81,
};

static uint8_t ADDRESS_RECIPIENT[21] = {
    0x00, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75, 0xd2, 0x66, 0xbd,
    0x02, 0x24, 0x39, 0xb2, 0x2c, 0xdb, 0x16, 0x50, 0x8c, 0x7a,
};

//// Tests

static void test_tx_serialization_generic(void **state) {
    // Setup
    (void) state;
    uint8_t raw_tx[sizeof(TRANSACTION_BYTES_GENERIC_TRANSACTION)];
    memcpy(raw_tx,
           &TRANSACTION_BYTES_GENERIC_TRANSACTION,
           sizeof(TRANSACTION_BYTES_GENERIC_TRANSACTION));
    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};

    // Run test
    transaction_parsing_state_t parsing_state;
    transaction_t tx;

    transaction_parser_init(&parsing_state);
    parser_status_e status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check internal state of parser
    assert_int_equal(status, PARSING_DONE);
    assert_int_equal(parsing_state.rpc_bytes_total, 0xf1);
    assert_int_equal(parsing_state.rpc_bytes_parsed, 0xf1);

    // Check output
    assert_int_equal(tx.basic.nonce, 0x102);
    assert_int_equal(tx.basic.valid_to_time, 0x304);
    assert_int_equal(tx.basic.gas_cost, 0x506);
    assert_memory_equal(tx.basic.contract_address.raw_bytes, ADDRESS_GENERIC_CONTRACT, 21);
    assert_int_equal(tx.type, GENERIC_TRANSACTION);
}

static void test_tx_serialization_mpc_token_transfer(void **state) {
    // Setup
    (void) state;
    uint8_t raw_tx[sizeof(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO)];
    memcpy(raw_tx,
           &TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
           sizeof(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO));
    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};

    // Run test
    transaction_parsing_state_t parsing_state;
    transaction_t tx;

    transaction_parser_init(&parsing_state);
    parser_status_e status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check internal state of parser
    assert_int_equal(status, PARSING_DONE);
    assert_int_equal(parsing_state.rpc_bytes_total, 0x1e);
    assert_int_equal(parsing_state.rpc_bytes_parsed, 0x1e);

    // Check output
    assert_int_equal(tx.basic.nonce, 0x102);
    assert_int_equal(tx.basic.valid_to_time, 0x304);
    assert_int_equal(tx.basic.gas_cost, 0x506);
    assert_memory_equal(tx.basic.contract_address.raw_bytes, ADDRESS_MPC_TOKEN, 21);
    assert_int_equal(tx.type, MPC_TRANSFER);
    assert_memory_equal(tx.mpc_transfer.recipient_address.raw_bytes, ADDRESS_RECIPIENT, 21);
    assert_int_equal(tx.mpc_transfer.token_amount_10000ths, 0x333);
    assert_int_equal(tx.mpc_transfer.memo_length, 0);
}

static void test_tx_serialization_mpc_token_transfer_but_too_many_bytes(void **state) {
    // Setup
    (void) state;
    uint8_t raw_tx[sizeof(TRANSACTION_BYTES_MPC_TRANSFER_TOO_MANY_BYTES)];
    memcpy(raw_tx,
           &TRANSACTION_BYTES_MPC_TRANSFER_TOO_MANY_BYTES,
           sizeof(TRANSACTION_BYTES_MPC_TRANSFER_TOO_MANY_BYTES));
    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};

    // Run test
    transaction_parsing_state_t parsing_state;
    transaction_t tx;

    transaction_parser_init(&parsing_state);
    parser_status_e status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check internal state of parser
    assert_int_equal(status, PARSING_DONE);
    assert_int_equal(parsing_state.rpc_bytes_total, 0x20);
    assert_int_equal(parsing_state.rpc_bytes_parsed, 0x20);

    // Check output
    assert_int_equal(tx.basic.nonce, 0x102);
    assert_int_equal(tx.basic.valid_to_time, 0x304);
    assert_int_equal(tx.basic.gas_cost, 0x506);
    assert_memory_equal(tx.basic.contract_address.raw_bytes, ADDRESS_MPC_TOKEN, 21);
    assert_int_equal(tx.type, GENERIC_TRANSACTION);
}

static void test_tx_serialization_mpc_token_transfer_small_memo(void **state) {
    // Setup
    (void) state;
    uint8_t raw_tx[sizeof(TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO)];
    memcpy(raw_tx,
           &TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO,
           sizeof(TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO));
    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};

    // Run test
    transaction_parsing_state_t parsing_state;
    transaction_t tx;

    transaction_parser_init(&parsing_state);
    parser_status_e status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check internal state of parser
    assert_int_equal(status, PARSING_DONE);
    assert_int_equal(parsing_state.rpc_bytes_total, 0x26);
    assert_int_equal(parsing_state.rpc_bytes_parsed, 0x26);

    // Check output
    assert_int_equal(tx.basic.nonce, 0x102);
    assert_int_equal(tx.basic.valid_to_time, 0x304);
    assert_int_equal(tx.basic.gas_cost, 0x506);
    assert_memory_equal(tx.basic.contract_address.raw_bytes, ADDRESS_MPC_TOKEN, 21);
    assert_int_equal(tx.type, MPC_TRANSFER);
    assert_int_equal(tx.mpc_transfer.memo_length, 8);
    assert_true(tx.mpc_transfer.has_u64_memo);
    assert_int_equal(tx.mpc_transfer.memo_u64, 0x9999);
    assert_memory_equal(tx.mpc_transfer.recipient_address.raw_bytes, ADDRESS_RECIPIENT, 21);
}

static void test_tx_serialization_mpc_token_transfer_large_memo(void **state) {
    // Setup
    (void) state;
    uint8_t raw_tx[sizeof(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO)];
    memcpy(raw_tx,
           &TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
           sizeof(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO));
    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};

    // Run test
    transaction_parsing_state_t parsing_state;
    transaction_t tx;

    transaction_parser_init(&parsing_state);
    parser_status_e status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check internal state of parser
    assert_int_equal(status, PARSING_DONE);
    assert_int_equal(parsing_state.rpc_bytes_total, 0x31);
    assert_int_equal(parsing_state.rpc_bytes_parsed, 0x31);

    // Check output
    assert_int_equal(tx.basic.nonce, 0x102);
    assert_int_equal(tx.basic.valid_to_time, 0x304);
    assert_int_equal(tx.basic.gas_cost, 0x506);
    assert_memory_equal(tx.basic.contract_address.raw_bytes, ADDRESS_MPC_TOKEN, 21);
    assert_int_equal(tx.type, MPC_TRANSFER);
    assert_int_equal(tx.mpc_transfer.memo_length, 11);
    assert_memory_equal(tx.mpc_transfer.recipient_address.raw_bytes, ADDRESS_RECIPIENT, 21);
    assert_false(tx.mpc_transfer.has_u64_memo);
    assert_string_equal(tx.mpc_transfer.memo, "Hello World");
}

static void test_tx_serialization_mpc_token_transfer_large_multichunk_memo(void **state) {
    // Setup first chunk
    (void) state;
    uint8_t raw_tx[255];
    memcpy(raw_tx,
           &TRANSACTION_BYTES_MPC_TRANSFER_VERY_LARGE_MEMO_PART_1,
           sizeof(TRANSACTION_BYTES_MPC_TRANSFER_VERY_LARGE_MEMO_PART_1));
    buffer_t buf = {.ptr = raw_tx,
                    .size = sizeof(TRANSACTION_BYTES_MPC_TRANSFER_VERY_LARGE_MEMO_PART_1),
                    .offset = 0};

    // Run test
    transaction_parsing_state_t parsing_state;
    transaction_t tx;

    transaction_parser_init(&parsing_state);
    parser_status_e status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check internal state of parser
    assert_int_equal(status, PARSING_CONTINUE);
    assert_int_equal(parsing_state.rpc_bytes_total, 30 + 4 + 0x120);
    // Expected: |chunk size| - |nonce| - |valid_to_time| - |gas_cost| - |recipient| - |memo len|
    assert_int_equal(parsing_state.rpc_bytes_parsed, 0xff - 8 - 8 - 8 - 21 - 4);

    // Next chunk
    memcpy(raw_tx,
           &TRANSACTION_BYTES_MPC_TRANSFER_VERY_LARGE_MEMO_PART_2,
           sizeof(TRANSACTION_BYTES_MPC_TRANSFER_VERY_LARGE_MEMO_PART_2));
    buf.ptr = raw_tx;
    buf.size = sizeof(TRANSACTION_BYTES_MPC_TRANSFER_VERY_LARGE_MEMO_PART_2);
    buf.offset = 0;

    status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check internal state of parser
    assert_int_equal(status, PARSING_DONE);
    assert_int_equal(parsing_state.rpc_bytes_total, 30 + 4 + 0x120);
    assert_int_equal(parsing_state.rpc_bytes_parsed, 30 + 4 + 0x120);

    // Check output
    assert_int_equal(tx.basic.nonce, 0x102);
    assert_int_equal(tx.basic.valid_to_time, 0x304);
    assert_int_equal(tx.basic.gas_cost, 0x506);
    assert_memory_equal(tx.basic.contract_address.raw_bytes, ADDRESS_MPC_TOKEN, 21);
    assert_int_equal(tx.type,
                     GENERIC_TRANSACTION);  // Too large to be easily parsed; default to blind-sign
}

/**
 * Variant test that cuts off a part of the transaction bytes, and checks
 * whether parsing it will produce the expected error.
 */
static void test_variant_transaction_cut_off(uint8_t *transaction_bytes,
                                             size_t length,
                                             parser_status_e expected_error) {
    uint8_t raw_tx[length];
    memcpy(raw_tx, transaction_bytes, length);
    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};

    // Run test
    transaction_parsing_state_t parsing_state;
    transaction_t tx;

    transaction_parser_init(&parsing_state);
    parser_status_e status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check internal state of parser
    assert_int_equal(status, expected_error);
}

static void test_cut_off_transactions(void **state) {
    (void) state;
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     0,
                                     PARSING_FAILED_NONCE);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     4,
                                     PARSING_FAILED_NONCE);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     7,
                                     PARSING_FAILED_NONCE);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     8,
                                     PARSING_FAILED_VALID_TO_TIME);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     15,
                                     PARSING_FAILED_VALID_TO_TIME);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     16,
                                     PARSING_FAILED_GAS_COST);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     23,
                                     PARSING_FAILED_GAS_COST);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     24,
                                     PARSING_FAILED_CONTRACT_ADDRESS);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     44,
                                     PARSING_FAILED_CONTRACT_ADDRESS);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     45,
                                     PARSING_FAILED_RPC_LENGTH);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     48,
                                     PARSING_FAILED_RPC_LENGTH);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO, 49, PARSING_CONTINUE);
    test_variant_transaction_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                     sizeof(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO) - 1,
                                     PARSING_CONTINUE);
}

/**
 * Variant test that cuts off a part of the transaction bytes, replacing the
 * the RPC length to fit the new length, and checks
 * whether parsing it will produce the expected error.
 */
static void test_variant_transaction_rpc_cut_off(uint8_t *transaction_bytes,
                                                 size_t length,
                                                 parser_status_e error) {
    uint8_t raw_tx[length];
    memcpy(raw_tx, transaction_bytes, length);
    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};
    raw_tx[45] = 0;
    raw_tx[46] = 0;
    raw_tx[47] = 0;
    raw_tx[48] = (uint8_t) (length - 48);

    // Run test
    transaction_parsing_state_t parsing_state;
    transaction_t tx;

    transaction_parser_init(&parsing_state);
    parser_status_e status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check entire buffer consumed
    assert_int_equal(buf.offset, buf.size);

    // Check internal state of parser
    assert_int_equal(status, PARSING_CONTINUE);
    assert_int_equal(tx.type, GENERIC_TRANSACTION);
    assert_int_equal(tx.rpc_parsing_error, error);
}

static void test_cut_off_rpc_no_memo(void **state) {
    (void) state;

    // No memo
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                         49,
                                         PARSING_FAILED_SHORTNAME);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                         50,
                                         PARSING_FAILED_MPC_RECIPIENT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                         70,
                                         PARSING_FAILED_MPC_RECIPIENT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                         71,
                                         PARSING_FAILED_MPC_TOKEN_AMOUNT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_NO_MEMO,
                                         78,
                                         PARSING_FAILED_MPC_TOKEN_AMOUNT);
}

static void test_cut_off_rpc_small_memo(void **state) {
    (void) state;
    // Small memo
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO,
                                         49,
                                         PARSING_FAILED_SHORTNAME);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO,
                                         50,
                                         PARSING_FAILED_MPC_RECIPIENT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO,
                                         70,
                                         PARSING_FAILED_MPC_RECIPIENT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO,
                                         71,
                                         PARSING_FAILED_MPC_TOKEN_AMOUNT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO,
                                         78,
                                         PARSING_FAILED_MPC_TOKEN_AMOUNT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO,
                                         79,
                                         PARSING_FAILED_MPC_MEMO);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_SMALL_MEMO,
                                         86,
                                         PARSING_FAILED_MPC_MEMO);
}

static void test_cut_off_rpc_large_memo(void **state) {
    (void) state;
    // Large memo
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
                                         49,
                                         PARSING_FAILED_SHORTNAME);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
                                         50,
                                         PARSING_FAILED_MPC_RECIPIENT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
                                         70,
                                         PARSING_FAILED_MPC_RECIPIENT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
                                         71,
                                         PARSING_FAILED_MPC_TOKEN_AMOUNT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
                                         78,
                                         PARSING_FAILED_MPC_TOKEN_AMOUNT);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
                                         79,
                                         PARSING_FAILED_MPC_MEMO);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
                                         83,
                                         PARSING_FAILED_MPC_MEMO);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
                                         84,
                                         PARSING_FAILED_MPC_MEMO);
    test_variant_transaction_rpc_cut_off(TRANSACTION_BYTES_MPC_TRANSFER_LARGE_MEMO,
                                         85,
                                         PARSING_FAILED_MPC_MEMO);
}

static void test_mpc_unknown_shortname() {
    uint8_t raw_tx[sizeof(TRANSACTION_BYTES_MPC_TRANSFER_UNKNOWN_SHORTNAME)];
    memcpy(raw_tx,
           TRANSACTION_BYTES_MPC_TRANSFER_UNKNOWN_SHORTNAME,
           sizeof(TRANSACTION_BYTES_MPC_TRANSFER_UNKNOWN_SHORTNAME));
    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};

    // Run test
    transaction_parsing_state_t parsing_state;
    transaction_t tx;

    transaction_parser_init(&parsing_state);
    parser_status_e status = transaction_parser_update(&parsing_state, &buf, &tx);

    // Check entire buffer consumed
    assert_int_equal(buf.offset, buf.size);

    // Check internal state of parser
    assert_int_equal(status, PARSING_CONTINUE);
    assert_int_equal(tx.type, GENERIC_TRANSACTION);
    assert_int_equal(tx.rpc_parsing_error, PARSING_FAILED_SHORTNAME_UNKNOWN);
}

static void test_blockchain_address_is_equal(void **state) {
    (void) state;

    // Self is equals
    assert_true(blockchain_address_is_equal(&MPC_TOKEN_ADDRESS, &MPC_TOKEN_ADDRESS));

    // Other is equals
    blockchain_address_s address;
    memcpy(address.raw_bytes, &MPC_TOKEN_ADDRESS.raw_bytes, 21);
    assert_true(blockchain_address_is_equal(&address, &MPC_TOKEN_ADDRESS));

    // Altered is not equals
    address.raw_bytes[4] = ~address.raw_bytes[4];
    assert_false(blockchain_address_is_equal(&address, &MPC_TOKEN_ADDRESS));
}

static char CHAIN_ID_MAINNET[] = "Partisia Blockchain";
static char CHAIN_ID_TESTNET[] = "Partisia Blockchain Testnet";
static char CHAIN_ID_TOO_LONG[] = "Partisia Blockchain Unknownnet";
static char CHAIN_ID_SHORT[] = "Smallnet";
static char CHAIN_ID_EMPTY[] = "";

static void test_variant_buffer_read_chain_id(char *str, size_t str_len) {
    uint8_t raw_buffer[4 + str_len];
    memcpy(raw_buffer + 4, str, str_len);
    raw_buffer[0] = 0;
    raw_buffer[1] = 0;
    raw_buffer[2] = 0;
    raw_buffer[3] = (uint8_t) str_len;
    buffer_t buf = {.ptr = raw_buffer, .size = sizeof(raw_buffer), .offset = 0};

    chain_id_t read_chain_id;
    bool success = buffer_read_chain_id(&buf, &read_chain_id);

    assert_true(success);
    assert_int_equal(read_chain_id.length, str_len);
    assert_memory_equal(read_chain_id.raw_bytes, str, str_len);
}

static void test_buffer_read_chain_id_too_long() {
    char *str = CHAIN_ID_TOO_LONG;
    size_t str_len = sizeof(CHAIN_ID_TOO_LONG) - 1;
    uint8_t raw_buffer[4 + str_len];
    memcpy(raw_buffer + 4, str, str_len);
    raw_buffer[0] = 0;
    raw_buffer[1] = 0;
    raw_buffer[2] = 0;
    raw_buffer[3] = (uint8_t) str_len;
    buffer_t buf = {.ptr = raw_buffer, .size = sizeof(raw_buffer), .offset = 0};

    chain_id_t read_chain_id;
    bool success = buffer_read_chain_id(&buf, &read_chain_id);

    assert_false(success);
}

static void test_buffer_read_chain_id_fail_to_read_size() {
    uint8_t raw_buffer[2] = {0, 0};
    buffer_t buf = {.ptr = raw_buffer, .size = 0, .offset = 0};

    chain_id_t read_chain_id;
    bool success = buffer_read_chain_id(&buf, &read_chain_id);
    assert_false(success);
}

static void test_buffer_read_chain_id(void **state) {
    (void) state;

    test_variant_buffer_read_chain_id(CHAIN_ID_EMPTY, sizeof(CHAIN_ID_EMPTY) - 1);
    test_variant_buffer_read_chain_id(CHAIN_ID_SHORT, sizeof(CHAIN_ID_SHORT) - 1);
    test_variant_buffer_read_chain_id(CHAIN_ID_MAINNET, sizeof(CHAIN_ID_MAINNET) - 1);
    test_variant_buffer_read_chain_id(CHAIN_ID_TESTNET, sizeof(CHAIN_ID_TESTNET) - 1);
}

static void test_buffer_read_bytes_out_longer(void **state) {
    (void) state;

    uint8_t raw_buffer[10];
    buffer_t buf = {.ptr = raw_buffer, .size = sizeof(raw_buffer), .offset = 0};

    uint8_t out_buffer[20];
    size_t amount_read = buffer_read_bytes(&buf, out_buffer, sizeof(out_buffer));
    assert_int_equal(amount_read, 10);
}

static void test_buffer_read_bytes_in_longer(void **state) {
    (void) state;

    uint8_t raw_buffer[20];
    buffer_t buf = {.ptr = raw_buffer, .size = sizeof(raw_buffer), .offset = 0};

    uint8_t out_buffer[10];
    size_t amount_read = buffer_read_bytes(&buf, out_buffer, sizeof(out_buffer));
    assert_int_equal(amount_read, 10);
}

static void test_buffer_variant_read_with_offset(size_t offset, size_t expected_read) {
    uint8_t raw_buffer[10];
    buffer_t buf = {.ptr = raw_buffer, .size = sizeof(raw_buffer), .offset = offset};

    uint8_t out_buffer[10];
    size_t amount_read = buffer_read_bytes(&buf, out_buffer, sizeof(out_buffer));
    assert_int_equal(amount_read, expected_read);
}

static void test_buffer_read_bytes_equal_size(void **state) {
    (void) state;

    test_buffer_variant_read_with_offset(0, 10);
}

static void test_buffer_read_bytes_already_read(void **state) {
    (void) state;

    test_buffer_variant_read_with_offset(1, 9);
    test_buffer_variant_read_with_offset(5, 5);
    test_buffer_variant_read_with_offset(9, 1);
    test_buffer_variant_read_with_offset(10, 0);
}

static void test_buffer_read_bytes_already_overflown(void **state) {
    (void) state;

    test_buffer_variant_read_with_offset(11, 0);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_tx_serialization_generic),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer_but_too_many_bytes),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer_small_memo),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer_large_memo),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer_large_multichunk_memo),
        cmocka_unit_test(test_cut_off_transactions),
        cmocka_unit_test(test_cut_off_rpc_no_memo),
        cmocka_unit_test(test_cut_off_rpc_small_memo),
        cmocka_unit_test(test_cut_off_rpc_large_memo),
        cmocka_unit_test(test_blockchain_address_is_equal),
        cmocka_unit_test(test_mpc_unknown_shortname),
        cmocka_unit_test(test_buffer_read_chain_id),
        cmocka_unit_test(test_buffer_read_chain_id_too_long),
        cmocka_unit_test(test_buffer_read_chain_id_fail_to_read_size),
        cmocka_unit_test(test_buffer_read_bytes_out_longer),
        cmocka_unit_test(test_buffer_read_bytes_in_longer),
        cmocka_unit_test(test_buffer_read_bytes_equal_size),
        cmocka_unit_test(test_buffer_read_bytes_already_read),
        cmocka_unit_test(test_buffer_read_bytes_already_overflown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
