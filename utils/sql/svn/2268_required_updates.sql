/* Akkadius */

-- Heap Corruption Fix --
DELETE FROM `aa_effects` WHERE `id`=3346 LIMIT 1;

-- QGlobal Changes --
ALTER TABLE `quest_globals`
MODIFY COLUMN `value`  varchar(128) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '?' AFTER `name`;

ALTER TABLE `quest_globals`
DROP COLUMN `id`,
MODIFY COLUMN `charid`  int(11) NOT NULL DEFAULT 0 FIRST ,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`charid`, `npcid`, `zoneid`, `name`);

-- Add First Logon Status, required for EVENT_CONNECT in player.pl --
ALTER TABLE `character_`
ADD COLUMN `firstlogon`  tinyint(3) NOT NULL DEFAULT 0 AFTER `xtargets`;

-- QueryServ Rules --
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'QueryServ:PlayerLogNPCKills', 'false', '');
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'QueryServ:PlayerLogTrades', 'false', '');

-- Flow Perl Commands to EVENT_SAY in player.pl/global_player.pl --
INSERT INTO `rule_values` VALUES ('1', 'Chat:FlowCommandstoPerl_EVENT_SAY', 'true', '');