#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "bip32.h"
#include "lcx_sha256.h"

#include "constants.h"
#include "transaction/types.h"

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    /** Instruction to get version of application. */
    GET_VERSION = 0x03,
    /** Instruction to get name of the application. */
    GET_APP_NAME = 0x04,
    /** Instruction to get the public key of the given BIP32 path. */
    GET_PUBLIC_KEY = 0x05,
    /** Instruction to sign the given transaction with the given BIP32 path. */
    SIGN_TX = 0x06
} command_e;

/**
 * Enumeration with parsing state.
 */
typedef enum {
    /** No state. */
    STATE_NONE,
    /** Transaction data parsed. */
    STATE_PARSED,
    /** Transaction data approved. */
    STATE_APPROVED
} state_e;

/**
 * Enumeration with user request type.
 */
typedef enum {
    /** Confirm address derived from public key. */
    CONFIRM_ADDRESS,
    /** Confirm transaction information. */
    CONFIRM_TRANSACTION
} request_type_e;

/**
 * Structure for public key context information.
 */
typedef struct {
    /** Layout: format (1), x-coordinate (32), y-coodinate (32) */
    uint8_t raw_public_key[65];
    /** For public key derivation */
    uint8_t chain_code[32];
} pubkey_ctx_t;

/**
 * Structure for transaction information context.
 */
typedef struct {
    /** Transaction parser state. */
    transaction_parsing_state_t transaction_parser_state;
    /** Parsed transaction. */
    transaction_t transaction;
    /** Which chain the transaction is targeting. */
    chain_id_t chain_id;
    /** Message digest state. */
    cx_sha256_t digest_state;
    /** Message hash digest. */
    uint8_t m_hash[CX_SHA256_SIZE];
    /** Transaction signature encoded in BIP66 DER.
     *
     * @see https://bips.dev/66/
     */
    uint8_t signature[MAX_BIP66_DER_SIGNATURE_LENGTH];
    /** Length of transaction signature. */
    uint8_t signature_len;
    /** Parity of y-coordinate of R in ECDSA signature. */
    uint8_t v;
} transaction_ctx_t;

/**
 * Global state for application.
 */
typedef struct {
    /** state of the context. */
    state_e state;
    union {
        /** public key context. */
        pubkey_ctx_t pk_info;
        /** transaction context. */
        transaction_ctx_t tx_info;
    };
    /** User request. */
    request_type_e req_type;
    /** BIP32 path */
    uint32_t bip32_path[MAX_BIP32_PATH];
    /** length of BIP32 path */
    uint8_t bip32_path_len;
} global_ctx_t;
