#!/usr/bin/env bash

world_path=""

if [ -d "bin" ]
then
    world_path="bin/"
fi

world_bin="${world_path}world"

echo "World path is [$world_path] bin is [$world_bin]"

# Run from the context of server directory
#
#  --content-tables
#  --login-tables
#  --player-tables
#  --system-tables
#  --query-serv-tables

#############################################
# dump
#############################################

dump_path=/tmp/peq-dump/

mkdir -p ${dump_path}

#############################################
# generate "drop_" table files
#############################################
bash -c "${world_bin} database:dump --content-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_content.sql"
bash -c "${world_bin} database:dump --login-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_login.sql"
bash -c "${world_bin} database:dump --player-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_player.sql"
bash -c "${world_bin} database:dump --system-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_system.sql"
bash -c "${world_bin} database:dump --query-serv-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_queryserv.sql"

#############################################
# generate "create_" table files
#############################################
bash -c "${world_bin} database:dump --content-tables --dump-output-to-console > ${dump_path}create_tables_content.sql"
bash -c "${world_bin} database:dump --login-tables --table-structure-only --dump-output-to-console > ${dump_path}create_tables_login.sql"
bash -c "${world_bin} database:dump --player-tables --table-structure-only --dump-output-to-console > ${dump_path}create_tables_player.sql"
bash -c "${world_bin} database:dump --system-tables --dump-output-to-console > ${dump_path}create_tables_system.sql"
bash -c "${world_bin} database:dump --query-serv-tables --table-structure-only --dump-output-to-console > ${dump_path}create_tables_queryserv.sql"

