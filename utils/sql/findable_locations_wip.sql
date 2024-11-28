CREATE TABLE `findable_location` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`zone` VARCHAR(32) NOT NULL COLLATE 'utf8mb4_general_ci',
	`version` INT(11) NOT NULL DEFAULT '0',
	`findable_id` INT(11) NOT NULL DEFAULT '0',
	`findable_sub_id` INT(11) NOT NULL DEFAULT '0',
	`type` INT(11) NOT NULL DEFAULT '0',
	`zone_id` INT(11) NOT NULL DEFAULT '0',
	`zone_id_index` INT(11) NOT NULL DEFAULT '0',
	`x` FLOAT NOT NULL DEFAULT '0',
	`y` FLOAT NOT NULL DEFAULT '0',
	`z` FLOAT NOT NULL DEFAULT '0',
	`min_expansion` TINYINT(4) NOT NULL DEFAULT '-1',
	`max_expansion` TINYINT(4) NOT NULL DEFAULT '-1',
	`content_flags` VARCHAR(100) NULL DEFAULT NULL COLLATE 'utf8mb4_general_ci',
	`content_flags_disabled` VARCHAR(100) NULL DEFAULT NULL COLLATE 'utf8mb4_general_ci',
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `zone_version` (`zone`, `version`) USING BTREE
);

INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 77, -1, 6, 202, 0, 484.484130859375, 183.69752502441406, 0.0020000000949949026);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 16, 0, 7, 45, 1, 342.86285400390625, 188.9244384765625, -181.92721557617188);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 17, 0, 7, 1, 1, -6.997137069702148, -174.92999267578125, 15.993850708007812);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 18, 0, 7, 1, 2, 356.8572692871094, -48.98040771484375, 15.993560791015625);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 19, 0, 7, 4, 1, 566.7733154296875, 1699.3203125, 54.97816467285156);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 19, 2, 7, 4, 2, 230.90762329101562, 1699.3203125, 59.97674560546875);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 20, 0, 7, 4, 3, 141.9432373046875, 1699.3203125, 20.986679077148438);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 21, 0, 7, 4, 4, -3.9984021186828613, 1699.3203125, 20.986663818359375);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 21, 3, 7, 4, 5, -333.866455078125, 1699.3203125, 59.97560119628906);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 22, 0, 7, 4, 6, 899.6401977539062, 1699.3203125, 54.97816467285156);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 23, 0, 7, 4, 7, -799.6800537109375, 1699.3203125, 54.97825622558594);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 53, 0, 7, 45, 2, 76.9692153930664, 174.9300537109375, -34.986000061035156);
INSERT INTO `findable_location` (zone, version, findable_id, findable_sub_id, type, zone_id, zone_id_index, x, y, z) VALUES ('qeynos2', 0, 54, 0, 7, 45, 3, -160.9356231689453, 300.879638671875, -139.94403076171875);