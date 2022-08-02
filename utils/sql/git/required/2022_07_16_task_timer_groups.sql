ALTER TABLE `tasks`
  ADD COLUMN `replay_timer_group` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `completion_emote`,
  ADD COLUMN `request_timer_group` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `replay_timer_seconds`;

ALTER TABLE `character_task_timers`
  ADD COLUMN `timer_group` INT NOT NULL DEFAULT '0' AFTER `timer_type`;
