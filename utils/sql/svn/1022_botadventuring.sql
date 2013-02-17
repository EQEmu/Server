DELIMITER $$

DROP FUNCTION IF EXISTS `GetMobType` $$
CREATE FUNCTION `GetMobType` (mobname VARCHAR(64)) RETURNS CHAR(1)
BEGIN
    DECLARE Result CHAR(1);

    SET Result = NULL;

    IF (select count(*) from character_ where name = mobname) > 0 THEN
      SET Result = 'C';
    ELSEIF (select count(*) from bots where Name = mobname) > 0 THEN
      SET Result = 'B';
    END IF;

    RETURN Result;
END $$

DELIMITER ;

DROP VIEW IF EXISTS `vwGroups`;
CREATE VIEW `vwGroups` AS
  select g.groupid as groupid,
GetMobType(g.name) as mobtype,
g.name as name,
g.charid as mobid,
ifnull(c.level, b.BotLevel) as level
from group_id as g
left join character_ as c on g.name = c.name
left join bots as b on g.name = b.Name;