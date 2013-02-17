-- These will throw up warnings if you have an outdated schema. 
alter table items drop column augslot1visible;
alter table items drop column augslot2visible;
alter table items drop column augslot3visible;
alter table items drop column augslot4visible;
alter table items drop column augslot5visible;

alter table items change column `augslot1unk` `augslot1visible` tinyint(3);
alter table items change column `augslot2unk` `augslot2visible` tinyint(3);
alter table items change column `augslot3unk` `augslot3visible` tinyint(3);
alter table items change column `augslot4unk` `augslot4visible` tinyint(3);
alter table items change column `augslot5unk` `augslot5visible` tinyint(3);