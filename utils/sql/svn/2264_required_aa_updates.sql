-- update Improved Familiar to not require allegiant familiar to purchase
UPDATE `altadv_vars` SET `prereq_skill`=0 WHERE `skill_id`='155';
-- Clear Any Prior Stuff on Animation Empathy --
DELETE FROM `aa_effects` WHERE aaid = 580 or aaid = 581 or aaid = 582;
-- Animation Empathy Rank I --
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '1', '267', '1', '4');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '2', '267', '1', '8');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '3', '267', '1', '5');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '4', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '5', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '6', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '7', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '8', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '9', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '10', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '580', '11', '254', '0', '0');
-- Animation Empathy Rank II --
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '1', '267', '1', '4');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '2', '267', '1', '8');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '3', '267', '1', '5');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '4', '267', '1', '2');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '5', '267', '1', '7');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '6', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '7', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '8', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '9', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '10', '254', '0', '0');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '581', '11', '254', '0', '0');
-- Animation Empathy Rank III --
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '1', '267', '1', '4');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '2', '267', '1', '8');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '3', '267', '1', '5');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '4', '267', '1', '2');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '5', '267', '1', '7');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '6', '267', '1', '6');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '7', '267', '1', '9');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '8', '267', '1', '12');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '9', '267', '1', '11');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '10', '267', '1', '28');
INSERT INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '582', '11', '267', '1', '1');