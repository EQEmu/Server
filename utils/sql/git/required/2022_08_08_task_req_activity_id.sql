ALTER TABLE `task_activities`
  ADD COLUMN `req_activity_id` INT SIGNED NOT NULL DEFAULT '-1' AFTER `activityid`;
