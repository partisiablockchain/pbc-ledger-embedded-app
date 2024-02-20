#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "transaction/deserialize.h"
#include "transaction/types.h"
#include "format.h"

static bool check_status_invariants(parser_status_e status) {
  // Zero is reserved.
  if (status == 0) {
    return false;
  }

  // Status must be known
  return status == PARSING_DONE || status == PARSING_CONTINUE || PARSING_FAILED_MPC_MEMO <= status;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    buffer_t buf = {.ptr = data, .size = size, .offset = 0};

    transaction_t tx;
    memset(&tx, 0, sizeof(tx));

    transaction_parsing_state_t state;
    transaction_parser_init(&state);
    parser_status_e status = transaction_parser_update(&state, &buf, &tx);

    // Check that status cannot lie outside the expected set.
    if (!check_status_invariants(status)) {
      exit(1);
    }

    return 0;
}
