#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

#if defined(HAVE_SHA256)
#include "lcx_sha256.h"
#endif

/**
 * Length of the identifier part of the blockchain_address_s.
 */
#define IDENTIFIER_LEN 20
/**
 * Length of blockchain_address_s.
 */
#define ADDRESS_LEN (IDENTIFIER_LEN + 1)

/**
 * Distinguishes the types of contract addresses that exists.
 */
typedef uint8_t blockchain_address_type_t;

/**
 * blockchain_address_type_t for users that can create external interactions with
 * the blockchain.
 */
#define BLOCKCHAIN_ADDRESS_ACCOUNT ((blockchain_address_type_t) 0)
/** blockchain_address_type_t for system contracts. */
#define BLOCKCHAIN_ADDRESS_CONTRACT_SYSTEM ((blockchain_address_type_t) 1)
/** blockchain_address_type_t for public contracts. */
#define BLOCKCHAIN_ADDRESS_CONTRACT_PUBLIC ((blockchain_address_type_t) 2)
/** blockchain_address_type_t for zero-knowledge contracts. */
#define BLOCKCHAIN_ADDRESS_CONTRACT_ZK ((blockchain_address_type_t) 3)
/** blockchain_address_type_t for governance contracts. */
#define BLOCKCHAIN_ADDRESS_CONTRACT_GOVERNANCE ((blockchain_address_type_t) 4)

/**
 * An address on the blockchain. Includes a type field to distinguish accounts
 * and different kinds of contracts.
 */
typedef struct {
    /** Unique identifier part of the address. */
    uint8_t raw_bytes[ADDRESS_LEN];
} blockchain_address_s;

#if defined(HAVE_SHA256)
/**
 * Convert public key to address.
 *
 * Only available when platform has HAVE_SHA256.
 *
 * address = '00' .. SHA256(public_key)[12:32] (21 bytes)
 *
 * @param[in]  public_key
 *   Pointer to byte buffer with public key.
 *   The public key is represented as 65 bytes with 1 byte for format and 32 bytes for
 *   each coordinate.
 * @param[out] out
 *   Pointer to output byte buffer for address.
 *
 * @return true if success, false otherwise.
 */
bool blockchain_address_from_pubkey(const uint8_t public_key[static 65], blockchain_address_s *out);
#endif

/**
 * Checks whether two blockchain_address_s are equals.
 */
bool blockchain_address_is_equal(blockchain_address_s *a, blockchain_address_s *b);
