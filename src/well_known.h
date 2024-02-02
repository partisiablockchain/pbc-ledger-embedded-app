#pragma once

#include "address.h"

/*! \file This file contains various well known contract addresses to enable special
 * handling of these contracts. Most of them are governance contracts.
 *
 * @see documentation:
 * https://partisiablockchain.gitlab.io/documentation/pbc-fundamentals/governance-system-smart-contracts-overview.html
 */

/**
 * Address of the MPC token contract.
 *
 * @see contract reference:
 * https://partisiablockchain.gitlab.io/governance/mpc-token/com/partisiablockchain/governance/mpctoken/MpcTokenContract.html
 * @see browser:
 * https://browser.partisiablockchain.com/contracts/01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881
 */
#define MPC_TOKEN_ADDRESS                                                           \
    ((blockchain_address_s){.raw_bytes = {0x01, 0xa4, 0x08, 0x2d, 0x9d, 0x56, 0x07, \
                                          0x49, 0xec, 0xd0, 0xff, 0xa1, 0xdc, 0xaa, \
                                          0xae, 0xe2, 0xc2, 0xcb, 0x25, 0xd8, 0x81}})

/** Byte shortname of the MPC transfer invocation. */
#define MPC_TOKEN_SHORTNAME_TRANSFER            3
/** Byte shortname of the MPC transfer with small memo invocation. */
#define MPC_TOKEN_SHORTNAME_TRANSFER_MEMO_SMALL 13
/** Byte shortname of the MPC transfer with large memo invocation. */
#define MPC_TOKEN_SHORTNAME_TRANSFER_MEMO_LARGE 23
