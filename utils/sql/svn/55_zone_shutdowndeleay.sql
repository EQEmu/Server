ALTER TABLE `zone` ADD column `shutdowndelay` bigint (16) unsigned NOT NULL default '5000';
INSERT INTO rule_values VALUES (1,'Zone:AutoShutdownDelay', 5000);
