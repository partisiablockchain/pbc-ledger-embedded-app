#pragma once

#include "os.h"
#include "macros.h"

/**
 * Helper to send APDU response with blockchain address.
 *
 * response = G_context.pk_info.address (21)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_address(void);

/**
 * Helper to send APDU response with signature and v (parity of
 * y-coordinate of R).
 *
 * response = G_context.tx_info.signature (65)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_sig(void);
