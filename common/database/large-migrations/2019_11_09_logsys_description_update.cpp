std::string _2019_11_09_logsys_description_update = R"(
UPDATE `logsys_categories` SET `log_category_description` = 'Packet :: Client -> Server' WHERE `log_category_id` = '5';
UPDATE `logsys_categories` SET `log_category_description` = 'QS Server' WHERE `log_category_id` = '19';
UPDATE `logsys_categories` SET `log_category_description` = 'TCP Connection' WHERE `log_category_id` = '26';
UPDATE `logsys_categories` SET `log_category_description` = 'UCS Server' WHERE `log_category_id` = '31';
UPDATE `logsys_categories` SET `log_category_description` = 'WebInterface Server' WHERE `log_category_id` = '32';
UPDATE `logsys_categories` SET `log_category_description` = 'World Server' WHERE `log_category_id` = '33';
UPDATE `logsys_categories` SET `log_category_description` = 'MySQL Query' WHERE `log_category_id` = '36';
UPDATE `logsys_categories` SET `log_category_description` = 'Packet :: Server -> Client' WHERE `log_category_id` = '39';
UPDATE `logsys_categories` SET `log_category_description` = 'Packet :: Client -> Server Unhandled' WHERE `log_category_id` = '40';
UPDATE `logsys_categories` SET `log_category_description` = 'Packet :: Server -> Client (Dump)' WHERE `log_category_id` = '41';
UPDATE `logsys_categories` SET `log_category_description` = 'Packet :: Client -> Server (Dump)' WHERE `log_category_id` = '42';

)";