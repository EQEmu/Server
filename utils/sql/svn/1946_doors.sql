ALTER TABLE doors DROP INDEX DoorIndex;
ALTER TABLE doors ADD CONSTRAINT DoorIndex UNIQUE KEY (zone, doorid, version);
