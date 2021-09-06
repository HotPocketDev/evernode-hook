/**
 * This hook just accepts any transaction coming through it
 */
#include "../lib/hookapi.h"
#include "evernode.h"

// Executed when an emitted transaction is successfully accepted into a ledger
// or when an emitted transaction cannot be accepted into any ledger (with what = 1),
int64_t cbak(int64_t reserved)
{
    return 0;
}
// maximum tx blob
#define MAX_MEMO_SIZE 4096

// Executed whenever a transaction comes into or leaves from the account the Hook is set on.
int64_t hook(int64_t reserved)
{
    uint32_t host_count = 0;
    if (state(SBUF(&host_count), SBUF(STK_HOST_COUNT)) == DOESNT_EXIST)
    {
        if (state_set(SBUF(&host_count), SBUF(STK_HOST_COUNT)) < 0)
            rollback(SBUF("Evernode: Could not set default state for host count."), 1);
    }
    TRACEVAR(host_count);

    uint32_t auditor_count = 0;
    if (state(SBUF(&auditor_count), SBUF(STK_AUDITOR_COUNT)) == DOESNT_EXIST)
    {
        if (state_set(SBUF(&auditor_count), SBUF(STK_AUDITOR_COUNT)) < 0)
            rollback(SBUF("Evernode: Could not set default state for host count."), 1);
    }
    TRACEVAR(auditor_count);

    // Setting and loading configuration values from the hook state.
    uint8_t K_CONF_MOMENT_SIZE[32];
    CONF_KEY(K_CONF_MOMENT_SIZE, CONF_MOMENT_SIZE);
    uint16_t conf_moment_size;
    if (state(SBUF(&conf_moment_size), SBUF(K_CONF_MOMENT_SIZE)) == DOESNT_EXIST)
    {
        if (state_set(SBUF(&DEF_MOMENT_SIZE), SBUF(K_CONF_MOMENT_SIZE)) < 0)
            rollback(SBUF("Evernode: Could not set default state for moment size."), 1);

        conf_moment_size = DEF_MOMENT_SIZE;
    }
    TRACEVAR(conf_moment_size);

    uint8_t K_CONF_MINT_LIMIT[32];
    CONF_KEY(K_CONF_MINT_LIMIT, CONF_MINT_LIMIT);
    uint64_t conf_mint_limit;
    if (state(SBUF(&conf_mint_limit), SBUF(K_CONF_MINT_LIMIT)) == DOESNT_EXIST)
    {
        if (state_set(SBUF(&DEF_MINT_LIMIT), SBUF(K_CONF_MINT_LIMIT)) < 0)
            rollback(SBUF("Evernode: Could not set default state for mint limit."), 1);

        conf_mint_limit = DEF_MINT_LIMIT;
    }
    TRACEVAR(conf_mint_limit);

    uint8_t K_CONF_HOST_REG_FEE[32];
    CONF_KEY(K_CONF_HOST_REG_FEE, CONF_HOST_REG_FEE);
    uint16_t conf_host_reg_fee;
    if (state(SBUF(&conf_host_reg_fee), SBUF(K_CONF_HOST_REG_FEE)) == DOESNT_EXIST)
    {
        if (state_set(SBUF(&DEF_HOST_REG_FEE), SBUF(K_CONF_HOST_REG_FEE)) < 0)
            rollback(SBUF("Evernode: Could not set default state for host reg fee."), 1);

        conf_host_reg_fee = DEF_HOST_REG_FEE;
    }
    TRACEVAR(conf_host_reg_fee);

    uint8_t K_CONF_MIN_REDEEM[32];
    CONF_KEY(K_CONF_MIN_REDEEM, CONF_MIN_REDEEM);
    uint16_t conf_min_redeem;
    if (state(SBUF(&conf_min_redeem), SBUF(K_CONF_MIN_REDEEM)) == DOESNT_EXIST)
    {
        if (state_set(SBUF(&DEF_MIN_REDEEM), SBUF(K_CONF_MIN_REDEEM)) < 0)
            rollback(SBUF("Evernode: Could not set default state for min redeem."), 1);

        conf_min_redeem = DEF_MIN_REDEEM;
    }
    TRACEVAR(conf_min_redeem);

    uint8_t K_CONF_REDEEM_WINDOW[32];
    CONF_KEY(K_CONF_REDEEM_WINDOW, CONF_REDEEM_WINDOW);
    uint16_t conf_redeem_window;
    if (state(SBUF(&conf_redeem_window), SBUF(K_CONF_REDEEM_WINDOW)) == DOESNT_EXIST)
    {
        if (state_set(SBUF(&DEF_REDEEM_WINDOW), SBUF(K_CONF_REDEEM_WINDOW)) < 0)
            rollback(SBUF("Evernode: Could not set default state for redeem window."), 1);

        conf_redeem_window = DEF_REDEEM_WINDOW;
    }
    TRACEVAR(conf_redeem_window);

    uint8_t K_CONF_HOST_REWARD[32];
    CONF_KEY(K_CONF_HOST_REWARD, CONF_HOST_REWARD);
    uint16_t conf_host_reward;
    if (state(SBUF(&conf_host_reward), SBUF(K_CONF_HOST_REWARD)) == DOESNT_EXIST)
    {
        if (state_set(SBUF(&DEF_HOST_REWARD), SBUF(K_CONF_HOST_REWARD)) < 0)
            rollback(SBUF("Evernode: Could not set default state for host reward."), 1);

        conf_host_reward = DEF_HOST_REWARD;
    }
    TRACEVAR(conf_host_reward);

    // TRACESTR("Evernode hook called.");

    // Getting the hook account id.
    unsigned char hook_accid[20];
    hook_account((uint32_t)hook_accid, 20);

    // Next fetch the sfAccount field from the originating transaction
    uint8_t account_field[20];
    uint8_t dest_field[20];
    int32_t account_field_len = otxn_field(SBUF(account_field), sfAccount);
    int32_t dest_field_len = otxn_field(SBUF(dest_field), sfDestination);
    if (account_field_len < 20 || dest_field_len < 20)
        rollback(SBUF("Evernode: sfAccount or sfAccount field missing!!!"), 10);

    // specifically we're interested in the amount sent
    int64_t oslot = otxn_slot(0);
    if (oslot < 0)
        rollback(SBUF("Evernode: Could not slot originating txn."), 1);

    int64_t amt_slot = slot_subfield(oslot, sfAmount, 0);

    if (amt_slot < 0)
        rollback(SBUF("Evernode: Could not slot otxn.sfAmount"), 2);

    int64_t is_xrp = slot_type(amt_slot, 1);
    if (is_xrp < 0)
        rollback(SBUF("Evernode: Could not determine sent amount type"), 3);

    if (is_xrp)
        accept(SBUF("Evernode: XRP transaction detected."), 0);

    // Host registration logic.
    // Checking transaction type.
    int64_t txn_type = otxn_type();
    TRACEVAR(txn_type);
    if (txn_type != ttPAYMENT)
        rollback(SBUF("Evernode: Transaction should be of type 'Payement' for host registration."), 2);

    int is_dest_hook = 0;
    BUFFER_EQUAL(is_dest_hook, hook_accid, dest_field, 20);
    if (!is_dest_hook)
        rollback(SBUF("Evernode: Destination should be hook account for host registration."), 2);

    int64_t amt = slot_float(amt_slot);
    if (amt < 0)
        rollback(SBUF("Evernode: Could not parse amount."), 1);

    uint8_t amount_buffer[48];
    if (slot(SBUF(amount_buffer), amt_slot) != 48)
        rollback(SBUF("Evernode: Could not dump sfAmount"), 1);

    // Get amount received in drops
    int64_t amount_val_drops = float_int(amt, 6, 0);
    TRACEVAR(amount_val_drops);

    if (amount_val_drops < (conf_host_reg_fee * 1000000))
        rollback(SBUF("Evernode: Amount sent is less than the minimum fee."), 1);

    // Currency should be EVR.
    int is_evr;
    IS_EVR(is_evr, amount_buffer, currency, hook_accid);
    if (!is_evr)
        rollback(SBUF("Evernode: Currency should be EVR for host registration."), 1);

    // Memos
    uint8_t memos[MAX_MEMO_SIZE];
    int64_t memos_len = otxn_field(SBUF(memos), sfMemos);

    if (!memos_len)
        rollback(SBUF("Evernode: No memos found for host registration."), 1);

    if (memos_len > 0)
    {
        // since our memos are in a buffer inside the hook (as opposed to being a slot) we use the sto api with it
        // the sto apis probe into a serialized object returning offsets and lengths of subfields or array entries
        int64_t memo_lookup = sto_subarray(memos, memos_len, 0);

        uint8_t *memo_ptr = SUB_OFFSET(memo_lookup) + memos;
        uint32_t memo_len = SUB_LENGTH(memo_lookup);

        // memos are nested inside an actual memo object, so we need to subfield
        // equivalently in JSON this would look like memo_array[i]["Memo"]
        memo_lookup = sto_subfield(memo_ptr, memo_len, sfMemo);
        memo_ptr = SUB_OFFSET(memo_lookup) + memo_ptr;
        memo_len = SUB_LENGTH(memo_lookup);

        if (memo_lookup < 0)
            rollback(SBUF("Echo: Incoming txn had a blank sfMemos, abort."), 1);

        int64_t type_lookup = sto_subfield(memo_ptr, memo_len, sfMemoType);
        uint8_t *type_ptr = SUB_OFFSET(type_lookup) + memo_ptr;
        uint32_t type_len = SUB_LENGTH(type_lookup);
        trace(SBUF("type in hex: "), type_ptr, type_len, 1);
        int is_type_match = 0;
        BUFFER_EQUAL_STR_GUARD(is_type_match, type_ptr, type_len, "evnHostReg", 1);
        if (!is_type_match)
            rollback(SBUF("Evernode: Memo type should be evnHostReg."), 50);

        // Checking whether this host is already registered.
        uint8_t K_HOST_ADDR[32];
        STATE_KEY(K_HOST_ADDR, STP_HOST_ADDR, account_field, account_field_len);
        uint8_t host_addr[7]; // <host_id(4)><hosting_token(3)>

        if (state(SBUF(host_addr), SBUF(K_HOST_ADDR)) != DOESNT_EXIST)
            rollback(SBUF("Evernode: Host already registered."), 1);

        int64_t format_lookup = sto_subfield(memo_ptr, memo_len, sfMemoFormat);
        uint8_t *format_ptr = SUB_OFFSET(format_lookup) + memo_ptr;
        uint32_t format_len = SUB_LENGTH(format_lookup);
        trace(SBUF("format in hex: "), format_ptr, format_len, 1);
        int is_format_match = 0;
        BUFFER_EQUAL_STR_GUARD(is_format_match, format_ptr, format_len, "text/plain", 1);
        if (!is_format_match)
            rollback(SBUF("Evernode: Memo format should be text/plain."), 50);

        int64_t data_lookup = sto_subfield(memo_ptr, memo_len, sfMemoData);
        uint8_t *data_ptr = SUB_OFFSET(data_lookup) + memo_ptr;
        uint32_t data_len = SUB_LENGTH(data_lookup);
        trace(SBUF("data in hex: "), data_ptr, data_len, 1); // Text data is in hex format.
        // Generate transaction with following properties.
        /**
            Transaction type: Trust Set
            Source: Hook
            Destination: Host
            Currency: hosting_token
            Limit: 999999999 
        */ 

        // Reserving one transaction.
        etxn_reserve(1);
        // Calculate fee for trustline transaction.
        int64_t fee = etxn_fee_base(PREPARE_SIMPLE_TRUSTLINE_SIZE);

        uint8_t hosting_token[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, data_ptr[0], data_ptr[1], data_ptr[2], 0, 0, 0, 0, 0};

        uint8_t amt_out[48];
        int64_t token_limit = float_sum(float_set(9, 1), float_negate(float_one())); // 999999999
        // we need to dump the iou amount into a buffer
        // by supplying -1 as the fieldcode we tell float_sto not to prefix an actual STO header on the field
        if (float_sto(SBUF(amt_out), SBUF(hosting_token), SBUF(account_field), token_limit, -1) < 0)
            rollback(SBUF("Evernode: Could not dump hosting token amount into sto"), 1);

        // set the currency code and issuer in the amount field
        for (int i = 0; GUARD(20), i < 20; ++i)
        {
            amt_out[i + 28] = account_field[i];
            amt_out[i + 8] = hosting_token[i];
        }

        // Preparing trustline transaction.
        uint8_t txn_out[PREPARE_SIMPLE_TRUSTLINE_SIZE];
        PREPARE_SIMPLE_TRUSTLINE(txn_out, amt_out, fee, account_field);

        uint8_t emithash[32];
        if (emit(SBUF(emithash), SBUF(txn_out)) < 0)
            rollback(SBUF("Evernode: Emitting txn failed"), 1);

        trace(SBUF("emit hash: "), SBUF(emithash), 1);

        uint8_t K_HOST_ID[32];
        uint32_t host_id = host_count + 1;
        uint8_t host_id_arr[4];
        UINT32_TO_BYTES(host_id_arr, host_id);
        STATE_KEY(K_HOST_ID, STP_HOST_ID, host_id_arr, sizeof(host_id_arr));
        if (state_set(SBUF(account_field), SBUF(K_HOST_ID)) < 0)
            rollback(SBUF("Evernode: Could not set state for host_id."), 1);

        // uint8_t K_HOST_ADDR[32];
        // STATE_KEY(K_HOST_ADDR, STP_HOST_ADDR, account_field, account_field_len);
        // uint8_t host_addr[7]; // <host_id(4)><hosting_token(3)>
        host_addr[0] = host_id_arr[0];
        host_addr[1] = host_id_arr[1];
        host_addr[2] = host_id_arr[2];
        host_addr[3] = host_id_arr[3];
        host_addr[4] = data_ptr[0];
        host_addr[5] = data_ptr[1];
        host_addr[6] = data_ptr[2];
        if (state_set(SBUF(host_addr), SBUF(K_HOST_ADDR)) < 0)
            rollback(SBUF("Evernode: Could not set state for host_addr."), 1);

        host_count += 1;
        if (state_set(SBUF(&host_count), SBUF(STK_HOST_COUNT)) < 0)
            rollback(SBUF("Evernode: Could not set default state for host count."), 1);
    }
    accept(0, 0, 0);

    _g(1, 1); // every hook needs to import guard function and use it at least once
    // unreachable
    return 0;
}