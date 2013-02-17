ALTER TABLE `discovered_items` ADD COLUMN `account_status` int(11) NOT NULL DEFAULT '0' AFTER `discovered_date`;
UPDATE discovered_items, account, character_ SET discovered_items.account_status = account.status WHERE character_.account_id = account.id AND character_.name = discovered_items.char_name;
