ALTER TABLE `aa_actions` ADD `redux_aa2` MEDIUMINT( 8 ) UNSIGNED NOT NULL DEFAULT '0',
ADD `redux_rate2` TINYINT( 4 ) NOT NULL DEFAULT '0';
UPDATE `aa_actions` SET `redux_aa2` = '886',
`redux_rate2` = '10' WHERE `aa_actions`.`aaid` =243 AND `aa_actions`.`rank` =0 LIMIT 1 ;
