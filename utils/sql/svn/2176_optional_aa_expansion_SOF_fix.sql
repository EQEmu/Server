-- Corrects the values in aa_expansion field for SOF+ client NOTE: DODH AA will go in OOW on Titanium ATM
-- Lucin AA = 3
UPDATE altadv_vars SET aa_expansion = 3 WHERE special_category = 4294967295 AND skill_id < 291;
-- POP AA = 4
UPDATE altadv_vars SET aa_expansion = 4 WHERE special_category = 4294967295 AND skill_id >= 292 AND skill_id <= 652;
-- GOD AA = 7
UPDATE altadv_vars SET aa_expansion = 7 WHERE special_category = 4294967295 AND skill_id >= 672 AND skill_id <= 931;
-- OOW AA = 8
UPDATE altadv_vars SET aa_expansion = 8 WHERE special_category = 4294967295 AND skill_id >= 1158 AND skill_id <= 1344;
UPDATE altadv_vars SET aa_expansion = 8 WHERE special_category = 4294967295 AND skill_id >= 978 AND skill_id <= 1129;
-- DODH AA = 10
UPDATE altadv_vars SET aa_expansion = 10 WHERE special_category = 4294967295 AND skill_id >= 1378 AND skill_id <= 1627;