#pragma once

#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <stdint.h>   // uint*_t

#include "buffer.h"

/**
 * Handler for #GET_ADDRESS command. If successfully parse BIP32 path,
 * derive G_context.pk_info.address and send APDU response.
 *
 * @see G_context.bip32_path, G_context.pk_info.address
 *
 * @param[in,out] cdata
 *   Command data with BIP32 path.
 * @param[in]     display
 *   Whether to display address on screen or not.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_get_address(buffer_t *cdata, bool display);
