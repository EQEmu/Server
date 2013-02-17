-- MISC AA fixes mostly display related

-- Fixed for siezed opporunity
UPDATE altadv_vars SET sof_next_id = 1539 WHERE skill_id = 878;
UPDATE altadv_vars SET sof_current_level = 3 WHERE skill_id = 1539;
UPDATE altadv_vars SET sof_type = 3 WHERE skill_id = 1539;

-- Triple backstab
UPDATE altadv_vars SET sof_next_id = 1301 WHERE skill_id = 846;
UPDATE altadv_vars SET sof_current_level = 3 WHERE skill_id = 1301;
UPDATE altadv_vars SET sof_type = 3 WHERE skill_id = 1301;

/* Prolonged Destruction prereq should be 931 Frenzied  Devastation not 741 Touch of the Divine */
UPDATE altadv_vars SET prereq_skill=931 WHERE skill_id=1337;
/* Ro's Greater Familiar prereq should be 921 Ro's Flaming Familiar not 724 BL Warder's Fury */
UPDATE altadv_vars SET prereq_skill=921 WHERE skill_id=1340;
/* E'ci's Greater Familiar prereq should be 922 E'ci's Icy Familiar not 729 BL Warder's Alacrity  */
UPDATE altadv_vars SET prereq_skill=922 WHERE skill_id=1341;
/* Druzzil's Greater Familiar prereq should be 923 Druzzil's Mystical Familiar not 734 BL Pet Affinity */
UPDATE altadv_vars SET prereq_skill=923 WHERE skill_id=1342;
/* Devoted prereq should be 533 Allegiant Familiar not 290 BL Frenzy of Spirit */
UPDATE altadv_vars SET prereq_skill=533 WHERE skill_id=1344;

-- Fix extend ingenunity
UPDATE altadv_vars SET sof_next_id = 8232 WHERE skill_id = 8261;

-- Hasty Exit
UPDATE altadv_vars SET sof_next_id = 886 WHERE skill_id = 498;
UPDATE altadv_vars SET sof_current_level = 2 WHERE skill_id = 886;

-- Pet Disc -- still semi bugged it will show 0/3 after buying first rank but when you buy it will go to 2/3
UPDATE altadv_vars SET sof_type = 2 WHERE skill_id = 288;
UPDATE altadv_vars SET cost_inc = 2 WHERE skill_id = 1129;

