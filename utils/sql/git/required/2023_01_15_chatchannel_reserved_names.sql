CREATE TABLE `chatchannel_reserved_names`
(
    `id`   int(11) NOT NULL AUTO_INCREMENT,
    `name` varchar(64) NOT NULL,
    PRIMARY KEY (`id`) USING BTREE,
    UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

ALTER TABLE `chatchannels`
    ADD COLUMN `id` int(11) NOT NULL AUTO_INCREMENT FIRST,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`id`) USING BTREE,
ADD UNIQUE INDEX(`name`)
