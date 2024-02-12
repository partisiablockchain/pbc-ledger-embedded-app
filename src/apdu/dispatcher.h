#pragma once

#include "parser.h"

#include "../types.h"

/** Parameter 1 for first APDU chunk number. */
#define P1_FIRST_CHUNK 0x00
/** Parameter 1 for maximum APDU chunk number. */
#define P1_NOT_FIRST_CHUNK 0x01
/** GET_ADDRESS: Parameter 1 to skip screen confirmation. */
#define P1_SILENT 0x00
/** GET_ADDRESS: Parameter 1 for screen confirmation */
#define P1_CONFIRM 0x01

/** Parameter 2 for last APDU chunk to receive. */
#define P2_LAST_CHUNK 0x00
/** Parameter 2 for more APDU chunk to receive. */
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
int apdu_dispatcher(const command_t *cmd);
