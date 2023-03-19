ALTER TABLE `tasks`
  ADD COLUMN `lock_activity_id` INT NOT NULL DEFAULT '-1' AFTER `request_timer_seconds`;
