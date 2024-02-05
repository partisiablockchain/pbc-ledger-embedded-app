#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "varint.h"
#include "write.h"
#include "buffer.h"

#include <cmocka.h>

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
    // rpc length (4): 38
    0x00, 0x00, 0x00, 0x26,
    // shortname (1)
    0x0d,
    // recipient (21)
    0x00, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75,
    0xd2, 0x66, 0xbd, 0x02, 0x24, 0x39, 0xb2, 0x2c,
    0xdb, 0x16, 0x50, 0x8c, 0x7a,
    // Token amount (8)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33,
    // Small memo (8)
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

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_tx_serialization_generic),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer_but_too_many_bytes),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer_small_memo),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer_large_memo),
        cmocka_unit_test(test_tx_serialization_mpc_token_transfer_large_multichunk_memo),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
