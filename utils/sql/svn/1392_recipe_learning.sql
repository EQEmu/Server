create table `char_recipe_list` (
  `char_id` int NOT NULL,
  `recipe_id` int NOT NULL,
  `madecount` int NOT NULL default 0,
  primary key (`char_id`, `recipe_id`)
) Engine=InnoDB;

alter table `tradeskill_recipe` add column `must_learn` tinyint not null default 0;

insert into rule_values (ruleset_id, rule_name, rule_value, notes) values
(1, 'Skills:UseLimitTradeskillSearchSkillDiff', 'true', 'Enables the limit for the maximum difference between trivial and skill for recipe searches and favorites.'),
(1, 'Skills:MaxTradeskillSearchSkillDiff', '50', 'The maximum difference in skill between the trivial of an item and the skill of the player if the trivial is higher than the skill. Recipes that have not been learnt or made at least once via the Experiment mode will be removed from searches based on this criteria.');