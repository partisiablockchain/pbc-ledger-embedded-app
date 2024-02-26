#pragma once

#include "decorators.h" // WARN_UNUSED_RESULT

/**
 * Helper to send APDU response with blockchain address.
 *
 * response = G_context.pk_info.address (21)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
WARN_UNUSED_RESULT
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
WARN_UNUSED_RESULT
int helper_send_response_sig(void);
