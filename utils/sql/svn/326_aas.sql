SET @row = 0;
UPDATE altadv_vars a SET prereq_skill = COALESCE((SELECT p.skill_id FROM (SELECT a2.skill_id, @row := @row + 1 AS prereq_index_num FROM altadv_vars a2) AS p WHERE p.prereq_index_num = a.prereq_skill), 0) WHERE prereq_skill < 1000000;
update altadv_vars set prereq_skill = 767 where skill_id = 1099;
update altadv_vars set prereq_skill = 844 where skill_id = 1319;
INSERT INTO `rule_values` VALUES (0, 'Combat:ChanceToHitDivideBy', '1250');