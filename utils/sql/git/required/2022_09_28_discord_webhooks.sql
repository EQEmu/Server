ALTER TABLE logsys_categories
    ADD log_to_discord smallint(11) default 0 AFTER log_to_gmsay;
ALTER TABLE logsys_categories
    ADD discord_webhook_id int(11) default 0 AFTER log_to_discord;
