ALTER TABLE instance_lockout ADD never_expires TINYINT UNSIGNED DEFAULT '0' NOT NULL AFTER duration;
ALTER TABLE instance_lockout ADD is_global TINYINT UNSIGNED DEFAULT '0' NOT NULL AFTER version;