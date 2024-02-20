#!/usr/bin/env python3
'''
Small utility for produces corpus for the fuzzing process from the example
transactions in the functional tests.
'''

import transaction_examples

CORPUS_PATH = '../fuzzing/corpus/valid-examples'

for transaction_name, transaction in transaction_examples.VALID_TRANSACTIONS:
    print(transaction_name, transaction)

    with open('{}/{}'.format(CORPUS_PATH, transaction_name), 'wb') as f:
        f.write(transaction.serialize())
