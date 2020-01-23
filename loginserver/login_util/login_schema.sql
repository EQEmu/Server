DROP TABLE IF EXISTS `login_accounts`;
CREATE TABLE `login_accounts` (
  `id` int(11) unsigned NOT NULL,
  `account_name` varchar(50) NOT NULL,
  `account_password` text NOT NULL,
  `account_email` varchar(100) NOT NULL,
  `source_loginserver` varchar(64) DEFAULT NULL,
  `last_ip_address` varchar(15) NOT NULL,
  `last_login_date` datetime NOT NULL,
  `created_at` datetime DEFAULT NULL,
  `updated_at` datetime DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  UNIQUE KEY `source_loginserver_account_name` (`source_loginserver`,`account_name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `login_server_admins`;
CREATE TABLE `login_server_admins` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account_name` varchar(30) NOT NULL,
  `account_password` varchar(255) NOT NULL,
  `first_name` varchar(50) NOT NULL,
  `last_name` varchar(50) NOT NULL,
  `email` varchar(100) NOT NULL,
  `registration_date` datetime NOT NULL,
  `registration_ip_address` varchar(15) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `login_server_list_types`;
CREATE TABLE `login_server_list_types` (
  `id` int(10) unsigned NOT NULL,
  `description` varchar(60) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO `login_server_list_types` (`id`, `description`) VALUES ('1', 'Legends'),
('2', 'Preferred'),
('3', 'Standard');

DROP TABLE IF EXISTS `login_world_servers`;
CREATE TABLE `login_world_servers` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `long_name` varchar(100) NOT NULL,
  `short_name` varchar(100) NOT NULL,
  `tag_description` varchar(50) NOT NULL DEFAULT '',
  `login_server_list_type_id` int(11) NOT NULL,
  `last_login_date` datetime DEFAULT NULL,
  `last_ip_address` varchar(15) DEFAULT NULL,
  `login_server_admin_id` int(11) NOT NULL,
  `is_server_trusted` int(11) NOT NULL,
  `note` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `login_api_tokens`;
CREATE TABLE `login_api_tokens` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `token` varchar(200) DEFAULT NULL,
  `can_write` int(11) DEFAULT 0,
  `can_read` int(11) DEFAULT 0,
  `created_at` datetime DEFAULT NULL,
  `updated_at` datetime DEFAULT current_timestamp(),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;