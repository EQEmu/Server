#!/usr/bin/perl
# Author:       Akkadius
# @file:        repository-generator.pl
# @description: Script used to generate database repositories
# @example      perl ~/code/utils/scripts/generators/repository-generator.pl ~/server/

#############################################
# modules
#############################################
use warnings FATAL => 'all';
no warnings 'experimental::smartmatch';
use experimental 'smartmatch';
use File::Find;
use Data::Dumper;
use DBI;
use DBD::mysql;
use JSON;
my $json = new JSON();

#############################################
# args
#############################################
my $server_path                 = $ARGV[0];
my $config_path                 = $server_path . "/eqemu_config.json";
my $requested_table_to_generate = $ARGV[1] ? $ARGV[1] : "";

#############################################
# world path
#############################################
my $world_path       = $server_path . "/world";
my $world_path_bin   = $server_path . "/bin/world";
my $found_world_path = "";

if (-e $world_path) {
    $found_world_path = $world_path;
}
elsif (-e $world_path_bin) {
    $found_world_path = $world_path_bin;
}

if ($found_world_path eq "") {
    print "Error! Cannot find world binary!\n";
    exit;
}

#############################################
# validate config
#############################################
if (!-e $config_path) {
    print "Error! Config file [$config_path] not found\n";
    exit;
}

#############################################
# fetch schema from world
#############################################
my $output          = `cd $server_path && $found_world_path database:schema`;
my $database_schema = $json->decode($output);

#############################################
# database
#############################################
my $content;
open(my $fh, '<', $config_path) or die "cannot open file $config_path"; {
    local $/;
    $content = <$fh>;
}
close($fh);

#############################################
# database
#############################################
my $config        = $json->decode($content);
my $database_name = $config->{"server"}{"database"}{"db"};
my $host          = $config->{"server"}{"database"}{"host"};
my $user          = $config->{"server"}{"database"}{"username"};
my $pass          = $config->{"server"}{"database"}{"password"};
my $dsn           = "dbi:mysql:$database_name:$host:3306";
my $connect       = DBI->connect($dsn, $user, $pass);

my @tables = ($requested_table_to_generate);
if ($requested_table_to_generate eq "all" || !$requested_table_to_generate) {
    my $table_names_exec = $connect->prepare(
        "
            SELECT
              TABLE_NAME
            FROM
              INFORMATION_SCHEMA.COLUMNS
            WHERE
              TABLE_SCHEMA = ?
            GROUP BY
              TABLE_NAME
        ");

    $table_names_exec->execute($database_name);

    while (my @row = $table_names_exec->fetchrow_array()) {
        push(@tables, $row[0]);
    }
}

my $generated_repository_files = "";

foreach my $table_to_generate (@tables) {

    my $table_found_in_schema = 0;

    my @categories = (
        "content_tables",
        "version_tables",
        "state_tables",
        "server_tables",
        "player_tables",
        "login_tables",
    );

    # These tables don't have a typical schema
    my @table_ignore_list = (
        "character_enabledtasks",
        "keyring",
        "profanity_list",
        "zone_flags",
    );

    foreach my $category (@categories) {
        if ($table_to_generate ~~ $database_schema->{$category}) {
            $table_found_in_schema = 1;
        }
    }

    if ($table_to_generate ~~ @table_ignore_list) {
        print "Table [$table_to_generate] is on ignore list... skipping...\n";
        $table_found_in_schema = 0;
    }

    if ($table_found_in_schema == 0) {
        print "Table [$table_to_generate] not found in schema, skipping\n";
        next;
    }

    my $ex = $connect->prepare(
        "
        SELECT
          COLUMN_NAME,
          TABLE_NAME,
          DATA_TYPE,
          COLUMN_TYPE,
          ORDINAL_POSITION,
          COLUMN_KEY,
          COLUMN_DEFAULT
        FROM
          INFORMATION_SCHEMA.COLUMNS
        WHERE
          TABLE_SCHEMA = ?
          AND TABLE_NAME = ?
          ORDER BY TABLE_NAME, ORDINAL_POSITION
    "
    );

    $ex->execute($database_name, $table_to_generate);

    my $longest_column_length    = 0;
    my $longest_data_type_length = 0;
    while (my @row               = $ex->fetchrow_array()) {
        my $column_name          = $row[0];
        my $data_type            = $row[2];

        if ($longest_column_length < length($column_name)) {
            $longest_column_length = length($column_name);
        }

        my $struct_data_type = translate_mysql_data_type_to_c($data_type);

        if ($longest_data_type_length < length($struct_data_type)) {
            $longest_data_type_length = length($struct_data_type);
        }
    }

    # 2nd pass
    my $default_entries      = "";
    my $insert_one_entries   = "";
    my $insert_many_entries  = "";
    my $find_one_entries     = "";
    my $column_names_quoted  = "";
    my $table_struct_columns = "";
    my $update_one_entries   = "";
    my $all_entries          = "";
    my $index                = 0;
    my %table_data           = ();
    my %table_primary_key    = ();
    $ex->execute($database_name, $table_to_generate);

    $table_primary_key{$table_to_generate} = "id";

    while (my @row           = $ex->fetchrow_array()) {
        my $column_name      = $row[0];
        my $table_name       = $row[1];
        my $data_type        = $row[2];
        my $column_type      = $row[3];
        my $ordinal_position = $row[4];
        my $column_key       = $row[5];
        my $column_default   = ($row[6] ? $row[6] : "");

        if ($column_key eq "PRI") {
            $table_primary_key{$table_name} = $column_name;
        }

        my $default_value = 0;
        if ($column_default ne "NULL" && $column_default ne "") {
            $default_value = $column_default;
        }
        if ($column_default eq "''") {
            $default_value = '""';
        }
        if (trim($column_default) eq "" && $column_type =~ /text|varchar/i) {
            $default_value = '""';
        }

        my $struct_data_type = translate_mysql_data_type_to_c($data_type);

        # struct
        $table_struct_columns .= sprintf("\t\t\%-${longest_data_type_length}s %s;\n", $struct_data_type, $column_name);

        # new entity
        $default_entries .= sprintf("\t\tentry.%-${longest_column_length}s = %s;\n", $column_name, $default_value);

        # column names (string)
        $column_names_quoted .= sprintf("\t\t\t\"%s\",\n", $column_name);

        # update one
        if ($column_key ne "PRI") {
            my $query_value = sprintf('\'" + EscapeString(%s_entry.%s) + "\'");', $table_name, $column_name);
            if ($data_type =~ /int/) {
                $query_value = sprintf('" + std::to_string(%s_entry.%s));', $table_name, $column_name);
            }

            $update_one_entries .= sprintf(
                "\t\t" . 'update_values.push_back(columns[%s] + " = %s' . "\n",
                $index,
                $query_value
            );
        }

        # insert one
        if ($column_key ne "PRI") {
            my $value = sprintf("\"'\" + EscapeString(%s_entry.%s) + \"'\"", $table_name, $column_name);
            if ($data_type =~ /int/) {
                $value = sprintf('std::to_string(%s_entry.%s)', $table_name, $column_name);
            }

            $insert_one_entries  .= sprintf("\t\tinsert_values.push_back(%s);\n", $value);
            $insert_many_entries .= sprintf("\t\t\tinsert_values.push_back(%s);\n", $value);
        }

        # find one / all (select)
        if ($data_type =~ /int/) {
            $all_entries      .= sprintf("\t\t\tentry.%-${longest_column_length}s = atoi(row[%s]);\n", $column_name, $index);
            $find_one_entries .= sprintf("\t\t\tentry.%-${longest_column_length}s = atoi(row[%s]);\n", $column_name, $index);
        }
        elsif ($data_type =~ /float|double|decimal/) {
            $all_entries      .= sprintf("\t\t\tentry.%-${longest_column_length}s = atof(row[%s]);\n", $column_name, $index);
            $find_one_entries .= sprintf("\t\t\tentry.%-${longest_column_length}s = atof(row[%s]);\n", $column_name, $index);
        }
        else {
            $all_entries      .= sprintf("\t\t\tentry.%-${longest_column_length}s = row[%s];\n", $column_name, $index);
            $find_one_entries .= sprintf("\t\t\tentry.%-${longest_column_length}s = row[%s];\n", $column_name, $index);
        }

        # print $column_name . "\n";

        # print "table_name [$table_name] column_name [$column_name] data_type [$data_type] column_type [$column_type]\n";

        $index++;
    }

    #############################################
    # repository template
    #############################################
    my $repository_template_file = './common/repositories/template/repository.template';
    my $repository_template      = "";
    if (-e $repository_template_file) {
        open(my $fh, '<:encoding(UTF-8)', $repository_template_file) or die "Could not open file '$repository_template_file' $!";

        while (my $line          = <$fh>) {
            $repository_template .= $line;
        }

        close $fh;
    }

    if (trim($repository_template) eq "") {
        print "Repository template not found! [$repository_template_file]\n";
        exit;
    }

    foreach my $column (keys %{$table_data{$table_to_generate}}) {
        my $column_data      = $table_data{$table_to_generate}{$column};
        my $data_type        = $column_data->[0];
        my $column_type      = $column_data->[1];
        my $ordinal_position = $column_data->[2];
        my $column_default   = $column_data->[3];

        # print "Column [$column] data_type [$data_type] column_type [$column_type] ordinal [$ordinal_position]\n";
    }

    my $table_name_camel_case = $table_to_generate;
    my $table_name_upper_case = uc($table_to_generate);
    $table_name_camel_case =~ s#(_|^)(.)#\u$2#g;
    my $primary_key         = ($table_primary_key{$table_to_generate} ? $table_primary_key{$table_to_generate} : "");
    my $database_connection = "database";

    if ($table_to_generate ~~ $database_schema->{"content_tables"}) {
        $database_connection = "content_db";
    }

    chomp($column_names_quoted);
    chomp($table_struct_columns);
    chomp($default_entries);
    chomp($update_one_entries);
    chomp($insert_one_entries);
    chomp($insert_many_entries);
    chomp($all_entries);

    print "Table name CamelCase [$table_name_camel_case]\n";
    print "Table name UPPER_CASE [$table_name_upper_case]\n";
    print "Table PRIMARY KEY [$primary_key]\n";
    print "Database connection [$database_connection]\n";

    my $new_repository = $repository_template;

    $new_repository =~ s/\{\{TABLE_NAME_CLASS}}/$table_name_camel_case/g;
    $new_repository =~ s/\{\{TABLE_NAME_UPPER}}/$table_name_upper_case/g;
    $new_repository =~ s/\{\{PRIMARY_KEY_STRING}}/$primary_key/g;
    $new_repository =~ s/\{\{TABLE_NAME_STRUCT}}/$table_name_camel_case/g;
    $new_repository =~ s/\{\{TABLE_NAME_VAR}}/$table_to_generate/g;
    $new_repository =~ s/\{\{DATABASE_CONNECTION}}/$database_connection/g;
    $new_repository =~ s/\{\{DEFAULT_ENTRIES}}/$default_entries/g;
    $new_repository =~ s/\{\{COLUMNS_LIST_QUOTED}}/$column_names_quoted/g;
    $new_repository =~ s/\{\{TABLE_STRUCT_COLUMNS}}/$table_struct_columns/g;
    $new_repository =~ s/\{\{FIND_ONE_ENTRIES}}/$find_one_entries/g;
    $new_repository =~ s/\{\{UPDATE_ONE_ENTRIES}}/$update_one_entries/g;
    $new_repository =~ s/\{\{INSERT_ONE_ENTRIES}}/$insert_one_entries/g;
    $new_repository =~ s/\{\{INSERT_MANY_ENTRIES}}/$insert_many_entries/g;
    $new_repository =~ s/\{\{ALL_ENTRIES}}/$all_entries/g;

    print $new_repository;

    my $generated_repository      = './common/repositories/' . $table_to_generate . '_repository.h';
    my $cmake_generated_reference = $generated_repository;

    $cmake_generated_reference =~ s/.\/common\///g;

    $generated_repository_files .= $cmake_generated_reference . "\n";

    open(FH, '>', $generated_repository) or die $!;

    print FH $new_repository;

    close(FH);
}

print "\n# Make sure to add generated repositories to common/CMakeLists.txt under the repositories section\n\n";

print $generated_repository_files . "\n";

sub trim {
    my $string = $_[0];
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}

sub translate_mysql_data_type_to_c {
    my $mysql_data_type = $_[0];

    my $struct_data_type = "std::string";
    if ($mysql_data_type =~ /tinyint/) {
        $struct_data_type = 'int8';
    }
    elsif ($mysql_data_type =~ /smallint/) {
        $struct_data_type = 'int16';
    }
    elsif ($mysql_data_type =~ /int/) {
        $struct_data_type = 'int';
    }

    return $struct_data_type;
}
