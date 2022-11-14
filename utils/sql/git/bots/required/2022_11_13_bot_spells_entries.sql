UPDATE bot_spells_entries SET bucket_comparison = 0 WHERE bucket_comparison IS NULL;
UPDATE bot_spells_entries SET min_hp = 0 WHERE min_hp IS NULL;
UPDATE bot_spells_entries SET max_hp = 0 WHERE max_hp IS NULL;
UPDATE bot_spells_entries SET resist_adjust = 0 WHERE resist_adjust IS NULL;

ALTER TABLE `bot_spells_entries`
MODIFY COLUMN `resist_adjust` int(11) NOT NULL DEFAULT 0 AFTER `priority`,
MODIFY COLUMN `min_hp` smallint(5) NOT NULL DEFAULT 0 AFTER `resist_adjust`,
MODIFY COLUMN `max_hp` smallint(5) NOT NULL DEFAULT 0 AFTER `min_hp`,
MODIFY COLUMN `bucket_comparison` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `bucket_value`;
