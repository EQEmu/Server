-- update any null columns to non-null value first to avoid data truncation errors
-- this will likely only affect the buffer column
update `doors` set `doors`.`dest_x` = 0 where `doors`.`dest_x` is null;
update `doors` set `doors`.`dest_y` = 0 where `doors`.`dest_y` is null;
update `doors` set `doors`.`dest_z` = 0 where `doors`.`dest_z` is null;
update `doors` set `doors`.`dest_heading` = 0 where `doors`.`dest_heading` is null;
update `doors` set `doors`.`invert_state` = 0 where `doors`.`invert_state` is null;
update `doors` set `doors`.`incline` = 0 where `doors`.`incline` is null;
update `doors` set `doors`.`buffer` = 0 where `doors`.`buffer` is null;

ALTER TABLE `doors`
	CHANGE COLUMN `dest_x` `dest_x` FLOAT NOT NULL DEFAULT '0' AFTER `dest_instance`,
	CHANGE COLUMN `dest_y` `dest_y` FLOAT NOT NULL DEFAULT '0' AFTER `dest_x`,
	CHANGE COLUMN `dest_z` `dest_z` FLOAT NOT NULL DEFAULT '0' AFTER `dest_y`,
	CHANGE COLUMN `dest_heading` `dest_heading` FLOAT NOT NULL DEFAULT '0' AFTER `dest_z`,
	CHANGE COLUMN `invert_state` `invert_state` INT(11) NOT NULL DEFAULT '0' AFTER `dest_heading`,
	CHANGE COLUMN `incline` `incline` INT(11) NOT NULL DEFAULT '0' AFTER `invert_state`,
	CHANGE COLUMN `buffer` `buffer` FLOAT NOT NULL DEFAULT '0' AFTER `size`;
