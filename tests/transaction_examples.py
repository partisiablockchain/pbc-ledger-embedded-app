from application_client.transaction import Transaction, MpcTokenTransfer, from_hex

TRANSACTION_GENERIC_CONTRACT = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01de0b295669a9fd93d5f28d9ec85e40f4cb697bae"),
    rpc=from_hex('0xdeadbeef'),
)

TRANSACTION_GENERIC_CONTRACT_PRECISELY_ONE_CHUNK = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01de0b295669a9fd93d5f28d9ec85e40f4cb697bae"),
    rpc=from_hex('0xff') * 206,
)

TRANSACTION_GENERIC_CONTRACT_PRECISELY_OVER_ONE_CHUNK = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01de0b295669a9fd93d5f28d9ec85e40f4cb697bae"),
    rpc=from_hex('0xff') * 207,
)

TRANSACTION_GENERIC_CONTRACT_HUGE_RPC = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01de0b295669a9fd93d5f28d9ec85e40f4cb697bae"),
    rpc=from_hex('0xf0') * (207 + 255 * 100),
)

TRANSACTION_MPC_TRANSFER_FORGOT_SHORTNAME = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=from_hex(
        '0x000000000000000000000000000000000000012345_0000000000000333'),
)

TRANSACTION_MPC_TRANSFER = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444),
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_SMALL = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444, 1337),
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444,
        b"Hello World"),
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_AND_EMPTY = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444, b""),
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_AND_SMALL = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444,
        b"Hello"),
)

LONG_MEMO = b'''
This is a very long memo.
It will force the app client to send the serialized transaction to be sent in chunk.
As the maximum chunk size is 255 bytes we will make this memo greater than 255 characters.
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus. Suspendisse lectus tortor,
dignissim sit amet, adipiscing nec, ultricies sed, dolor. Cras elementum ultrices diam.
'''

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_PRECISELY_ONE_CHUNK = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'),
        0x444,
        LONG_MEMO[:172],
    ),
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_VERY = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'), 0x444,
        LONG_MEMO),
)

TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_RIDICULOUS = Transaction(
    nonce=0x111,
    valid_to_time=0x222,
    gas_cost=0x333,
    contract_address=from_hex("0x01a4082d9d560749ecd0ffa1dcaaaee2c2cb25d881"),
    rpc=MpcTokenTransfer(
        from_hex('0x000000000000000000000000000000000000012345'),
        0x444,
        b"This is a very long memo; just you know way too long, composed almost exclusively by: "
        + b'A' * 10000,
    ),
)

BLIND_TRANSACTIONS = [
    ('generic', TRANSACTION_GENERIC_CONTRACT),
    ('almost_an_mpc_transfer', TRANSACTION_MPC_TRANSFER_FORGOT_SHORTNAME),
    ('generic_one_chunk', TRANSACTION_GENERIC_CONTRACT_PRECISELY_ONE_CHUNK),
    ('generic_over_one_chunk',
     TRANSACTION_GENERIC_CONTRACT_PRECISELY_OVER_ONE_CHUNK),
    ('generic_huge', TRANSACTION_GENERIC_CONTRACT_HUGE_RPC),

    # MPC transactions with memos that are too large for displaying
    ('mpc_memo_large_just_exactly_one_chunk',
     TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_PRECISELY_ONE_CHUNK),
    ('mpc_memo_large_very', TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_VERY),
    ('mpc_memo_large_ridiculous',
     TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_RIDICULOUS),
]

MPC_TRANSFER_TRANSACTIONS = [
    ('mpc_transfer', TRANSACTION_MPC_TRANSFER),
    ('mpc_memo_small', TRANSACTION_MPC_TRANSFER_WITH_MEMO_SMALL),
    ('mpc_memo_large', TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE),
    ('mpc_memo_large_empty',
     TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_AND_EMPTY),
    ('mpc_memo_large_small',
     TRANSACTION_MPC_TRANSFER_WITH_MEMO_LARGE_AND_SMALL),
]
