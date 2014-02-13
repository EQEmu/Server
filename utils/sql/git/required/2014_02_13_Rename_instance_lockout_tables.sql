-- rename the instance_lockout tables to instance_list. They have nothing to do with lockouts.
ALTER TABLE `peq`.`instance_lockout` RENAME TO  `peq`.`instance_list` ;
ALTER TABLE `peq`.`instance_lockout_player` RENAME TO  `peq`.`instance_list_player` ;