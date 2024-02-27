#pragma once

#include "parser.h"

/** SIGN_TX: Parameter 1 to indicate the first APDU chunk. */
#define P1_FIRST_CHUNK 0x00
/** SIGN_TX: Parameter 1 to indicate any non-first APDU chunks. */
#define P1_NOT_FIRST_CHUNK 0x01
/** GET_ADDRESS: Parameter 1 to skip screen confirmation. */
#define P1_SILENT 0x00
/** GET_ADDRESS: Parameter 1 for screen confirmation */
#define P1_CONFIRM 0x01

/** SIGN_TX: Parameter 2 to indicate the last APDU chunk for the current SIGN_TX command. */
#define P2_LAST_CHUNK 0x00
/** SIGN_TX: Parameter 2 to indicate that there is not APDU chunks to parse for
 * the current SIGN_TX command. */
#define P2_NOT_LAST_CHUNK 0x80

/**
 * Dispatch APDU command received to the right handler.
 *
 * @param[in] cmd
 *   Structured APDU command (CLA, INS, P1, P2, Lc, Command data).
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
WARN_UNUSED_RESULT
int apdu_dispatcher(const command_t *cmd);
