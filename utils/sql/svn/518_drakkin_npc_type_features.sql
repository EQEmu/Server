ALTER TABLE `npc_types` ADD `drakkin_heritage` int(10) NOT NULL default '0' AFTER `luclin_beard`;
ALTER TABLE `npc_types` ADD `drakkin_tattoo` int(10) NOT NULL default '0' AFTER `drakkin_heritage`;
ALTER TABLE `npc_types` ADD `drakkin_details` int(10) NOT NULL default '0' AFTER `drakkin_tattoo`;