This program creates the shared memory files for various database tables.

It only need to be run after you make a change to the table in the database or there is a source change that relates to that tables structure.

Requires a folder named `shared` in the root server folder.

    shared_memory

Creates all the shared memory files

    shared_memory all

Same as above

    shared_memory base_data

Creates shared memory files for base data

    shared_memory items

Creates shared memory files for items

    shared_memory factions

Creates shared memory files for items

    shared_memory loot

Creates shared memory files for loot

    shared_memory skill_caps

Creates shared memory files for skill caps

    shared_memory spells

Creates shared memory files for spells

