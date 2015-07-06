DELIMITER $$
DROP PROCEDURE IF EXISTS GrantTripleAttack$$
CREATE PROCEDURE GrantTripleAttack()
	BEGIN
		DECLARE finished INT;
		DECLARE char_id INT;
		DECLARE skill_max INT;
		DECLARE cur CURSOR FOR SELECT character_data.id, skill_caps.cap FROM `character_data` LEFT JOIN `skill_caps` ON character_data.`level` = skill_caps.`level` AND character_data.class = skill_caps.class AND skill_caps.skillID = 76;
		DECLARE CONTINUE HANDLER FOR NOT FOUND SET finished = 1;
		
		OPEN cur;
		
		SET finished = 0;
		REPEAT
			FETCH cur INTO char_id, skill_max;
			
			IF skill_max IS NOT NULL AND skill_max > 0 THEN
				REPLACE INTO `character_skills` (`id`, `skill_id`, `value`) VALUES(char_id, 76, skill_max);
			END IF;
		UNTIL finished END REPEAT;
		
		CLOSE cur;
	END$$
DELIMITER ;

CALL GrantTripleAttack();
DROP PROCEDURE GrantTripleAttack;
