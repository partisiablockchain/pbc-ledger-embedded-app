#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "transaction/deserialize.h"
#include "transaction/types.h"
#include "format.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    buffer_t buf = {.ptr = data, .size = size, .offset = 0};
    transaction_t tx;
    parser_status_e status;

    char nonce[21] = {0};
    char contract[ADDRESS_LEN * 2 + 1] = {0};
    char gas_cost[21] = {0};
    char valid_to_time[21] = {0};

    memset(&tx, 0, sizeof(tx));

    transaction_parsing_state_t state;
    transaction_parser_init(&state);
    status = transaction_parser_update(&state, &buf, &tx);

    if (status == PARSING_DONE || status == PARSING_CONTINUE) {
        format_u64(nonce, sizeof(nonce), tx.basic.nonce);
        printf("nonce: %s\n", nonce);

        format_hex(tx.basic.contract_address.raw_bytes, ADDRESS_LEN, contract, sizeof(contract));
        printf("contract: %s\n", contract);

        format_u64(gas_cost, sizeof(gas_cost), tx.basic.gas_cost);
        printf("gas_cost: %s\n", gas_cost);

        format_u64(valid_to_time, sizeof(valid_to_time), tx.basic.gas_cost);
        printf("valid_to_time: %s\n", valid_to_time);
    }

    return 0;
}
