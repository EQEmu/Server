ALTER TABLE `account`
    ADD COLUMN `invulnerable` TINYINT(4) NULL DEFAULT '0' AFTER `gmspeed`,
    ADD COLUMN `flymode` TINYINT(4) NULL DEFAULT '0' AFTER `invulnerable`,
    ADD COLUMN `ignore_tells` TINYINT(4) NULL DEFAULT '0' AFTER `flymode`;
