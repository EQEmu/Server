CREATE TABLE `bot_heal_rotations` (
	`heal_rotation_index` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`interval` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`fast_heals` INT(3) UNSIGNED NOT NULL DEFAULT '0',
	`adaptive_targeting` INT(3) UNSIGNED NOT NULL DEFAULT '0',
	`casting_override` INT(3) UNSIGNED NOT NULL DEFAULT '0',
	`safe_hp_base` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	`safe_hp_cloth` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	`safe_hp_leather` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	`safe_hp_chain` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	`safe_hp_plate` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	`critical_hp_base` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	`critical_hp_cloth` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	`critical_hp_leather` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	`critical_hp_chain` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	`critical_hp_plate` FLOAT(11) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`heal_rotation_index`),
	CONSTRAINT `FK_bot_heal_rotations` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `bot_heal_rotation_members` (
	`member_index` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`heal_rotation_index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`member_index`),
	CONSTRAINT `FK_bot_heal_rotation_members_1` FOREIGN KEY (`heal_rotation_index`) REFERENCES `bot_heal_rotations` (`heal_rotation_index`),
	CONSTRAINT `FK_bot_heal_rotation_members_2` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `bot_heal_rotation_targets` (
	`target_index` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`heal_rotation_index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`target_name` varchar(64) NOT NULL DEFAULT '',
	PRIMARY KEY (`target_index`),
	CONSTRAINT `FK_bot_heal_rotation_targets` FOREIGN KEY (`heal_rotation_index`) REFERENCES `bot_heal_rotations` (`heal_rotation_index`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO `bot_command_settings` VALUES
('healrotationdelete', 0, 'hrdelete'),
('healrotationsave', 0, 'hrsave');
