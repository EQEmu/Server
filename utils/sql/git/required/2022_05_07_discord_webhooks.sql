CREATE TABLE discord_webhooks
(
    id           INT auto_increment primary key NULL,
    webhook_name varchar(100) NULL,
    webhook_url  varchar(255) NULL,
    created_at   DATETIME NULL,
    deleted_at   DATETIME NULL
) ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_general_ci;

ALTER TABLE logsys_categories
    ADD log_to_discord smallint(11) default 0 AFTER log_to_gmsay;
ALTER TABLE logsys_categories
    ADD discord_webhook_id int(11) default 0 AFTER log_to_discord;
