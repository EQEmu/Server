-- Convert Negative values to be less than the max signed value
UPDATE tasks SET xpreward = (xpreward - 2147483649) WHERE xpreward > 4294941695 AND xpreward < 4294967196;

-- Change the xpreward field to be signed
ALTER TABLE tasks CHANGE `xpreward` `xpreward` INT(10) SIGNED NOT NULL DEFAULT '0';

-- Convert the previously converted values into actual negative values
UPDATE tasks SET xpreward = (xpreward - 2147483647) WHERE xpreward > 2147458047 AND xpreward < 2147483548;