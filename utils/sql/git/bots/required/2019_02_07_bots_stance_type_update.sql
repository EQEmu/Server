-- Update `bot_stances`.`stance_id` to new values
UPDATE `bot_stances` SET `stance_id` = '9' WHERE `stance_id` = '6';
UPDATE `bot_stances` SET `stance_id` = '7' WHERE `stance_id` = '5';
UPDATE `bot_stances` SET `stance_id` = (`stance_id` + 1) WHERE `stance_id` in (0,1,2,3,4);

-- Update `bot_spell_casting_chances`.`stance_index` to new values
UPDATE `bot_spell_casting_chances` SET `stance_index` = '8' WHERE `stance_index` = '6';
UPDATE `bot_spell_casting_chances` SET `stance_index` = '6' WHERE `stance_index` = '5';

-- Update `bot_spell_casting_chances` implicit versioning
UPDATE `bot_spell_casting_chances` SET `stance_index` = '1' WHERE `spell_type_index` = '255' AND `class_id` = '255';
