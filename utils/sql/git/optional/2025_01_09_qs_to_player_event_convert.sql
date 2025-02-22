-- ================================
-- Conversion of Merchant Transactions
-- ================================
INSERT INTO player_event_merchant_sell (
    npc_id, merchant_name, merchant_type, item_id, item_name, charges, cost, alternate_currency_id,
    player_money_balance, player_currency_balance, created_at
)
SELECT
    r.merchant_id, NULL AS merchant_name, NULL AS merchant_type, e.item_id, NULL AS item_name, e.charges, r.merchant_cp AS cost, NULL AS alternate_currency_id,
    r.char_cp AS player_money_balance, NULL AS player_currency_balance, r.time AS created_at
FROM qs_merchant_transaction_record AS r
         JOIN qs_merchant_transaction_record_entries AS e
              ON r.transaction_id = e.event_id;

-- ================================
-- Conversion of Player Trades
-- ================================
INSERT INTO player_event_trade (
    char1_id, char2_id, char1_copper, char1_silver, char1_gold, char1_platinum,
    char2_copper, char2_silver, char2_gold, char2_platinum, created_at
)
SELECT
    char1_id, char2_id, char1_cp, char1_sp, char1_gp, char1_pp,
    char2_cp, char2_sp, char2_gp, char2_pp, time AS created_at
FROM qs_player_trade_record;

INSERT INTO player_event_trade_entries (
    player_event_trade_id, char_id, slot, item_id, charges, augment_1_id, augment_2_id, augment_3_id,
    augment_4_id, augment_5_id, augment_6_id, in_bag, created_at
)
SELECT
    event_id, from_id AS char_id, from_slot AS slot, item_id, charges, aug_1 AS augment_1_id, aug_2 AS augment_2_id, aug_3 AS augment_3_id,
    aug_4 AS augment_4_id, aug_5 AS augment_5_id, NULL AS augment_6_id, 0 AS in_bag, NULL AS created_at
FROM qs_player_trade_record_entries;

-- ================================
-- Conversion of NPC Hand-ins
-- ================================
INSERT INTO player_event_npc_handin (
    npc_id, npc_name, handin_copper, handin_silver, handin_gold, handin_platinum,
    return_copper, return_silver, return_gold, return_platinum, is_quest_handin, created_at
)
SELECT
    npc_id, NULL AS npc_name, char_cp AS handin_copper, char_sp AS handin_silver, char_gp AS handin_gold, char_pp AS handin_platinum,
    npc_cp AS return_copper, npc_sp AS return_silver, npc_gp AS return_gold, npc_pp AS return_platinum,
    0 AS is_quest_handin, time AS created_at
FROM qs_player_handin_record;

INSERT INTO player_event_npc_handin_entries (
    player_event_npc_handin_id, type, item_id, charges, evolve_level, evolve_amount,
    augment_1_id, augment_2_id, augment_3_id, augment_4_id, augment_5_id, augment_6_id, created_at
)
SELECT
    event_id, NULL AS type, item_id, charges, 0 AS evolve_level, 0 AS evolve_amount,
    aug_1 AS augment_1_id, aug_2 AS augment_2_id, aug_3 AS augment_3_id, aug_4 AS augment_4_id, aug_5 AS augment_5_id, 0 AS augment_6_id, NULL AS created_at
FROM qs_player_handin_record_entries;

-- ================================
-- Conversion of NPC Kill Records
-- ================================
INSERT INTO player_event_killed_npc (
    npc_id, npc_name, combat_time_seconds, total_damage_per_second_taken, total_heal_per_second_taken, created_at
)
SELECT
    npc_id, NULL AS npc_name, NULL AS combat_time_seconds, NULL AS total_damage_per_second_taken, NULL AS total_heal_per_second_taken, time AS created_at
FROM qs_player_npc_kill_record;

INSERT INTO player_event_killed_named_npc (
    npc_id, npc_name, combat_time_seconds, total_damage_per_second_taken, total_heal_per_second_taken, created_at
)
SELECT
    npc_id, NULL AS npc_name, NULL AS combat_time_seconds, NULL AS total_damage_per_second_taken, NULL AS total_heal_per_second_taken, time AS created_at
FROM qs_player_npc_kill_record WHERE type = 1; -- Example condition for named NPCs

-- ================================
-- Conversion of Player Speech
-- ================================
INSERT INTO player_event_speech (
    from_char_id, to_char_id, guild_id, type, min_status, message, created_at
)
SELECT
    `from` AS from_char_id, `to` AS to_char_id, guilddbid AS guild_id, type, minstatus AS min_status, message,
    timerecorded AS created_at
FROM qs_player_speech;

-- ================================
-- Conversion of AA Purchases
-- ================================
INSERT INTO player_event_aa_purchase (
    aa_ability_id, cost, previous_id, next_id, created_at
)
SELECT
    NULL AS aa_ability_id, NULL AS cost, NULL AS previous_id, NULL AS next_id, NULL AS created_at
FROM qs_player_aa_rate_hourly; -- Adjust as necessary for the source table and logic
