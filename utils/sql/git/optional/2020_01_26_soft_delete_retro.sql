-- Run this to un-reserve deleted characters
UPDATE
  character_data
SET
  name = SUBSTRING(
    CONCAT(name, '-deleted-', UNIX_TIMESTAMP()),
    1,
    64
  )
WHERE
  deleted_at IS NOT NULL
  AND name NOT LIKE '%-deleted-%';