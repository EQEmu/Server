-- rename the instance_lockout tables to instance_list. They have nothing to do with lockouts.
ALTER TABLE `instance_lockout` RENAME TO  `instance_list` ;
ALTER TABLE `instance_lockout_player` RENAME TO  `instance_list_player` ;