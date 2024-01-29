#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

#if defined(HAVE_SHA256)
#include "lcx_sha256.h"
#endif

#define IDENTIFIER_LEN 20
#define ADDRESS_LEN (IDENTIFIER_LEN+1)

/**
 * Distinguishes the types of contract addresses that exists.
 */
typedef uint8_t blockchain_address_type_t;

#define BLOCKCHAIN_ADDRESS_ACCOUNT            0
#define BLOCKCHAIN_ADDRESS_CONTRACT_SYSTEM    1
#define BLOCKCHAIN_ADDRESS_CONTRACT_PUBLIC    2
#define BLOCKCHAIN_ADDRESS_CONTRACT_ZK        3
#define BLOCKCHAIN_ADDRESS_CONTRACT_GOVERANCE 4

/**
 * An address on the blockchain. Includes a type field to distinguish accounts
 * and different kinds of contracts.
 */
typedef struct {
    uint8_t raw_bytes[ADDRESS_LEN];  // Unique identifier part of the address
} blockchain_address_s;

#if defined(HAVE_SHA256)
/**
 * Convert public key to address.
 *
 * Only available when platform has HAVE_SHA256.
 *
 * address = SHA256(public_key)[12:32] (20 bytes) TODO
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
bool blockchain_address_is_equal(blockchain_address_s *a, blockchain_address_s* b);
