#!/usr/bin/perl

###########################################################
#::: General EQEmu Server Administration Script
#::: Purpose - Handles:
#::: 	Automatic database versioning (bots and normal DB)
#::: 	Updating server assets (binary, opcodes, maps, configuration files)
#::: Original Author: Akkadius
#::: 	Contributors: Uleat
#::: Purpose: To upgrade databases with ease and maintain versioning
###########################################################

use Config;
use File::Copy qw(copy);
use POSIX qw(strftime);
use File::Path;
use File::Find;
use Time::HiRes qw(usleep);

#::: Variables
$install_repository_request_url = "https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/";
$eqemu_repository_request_url   = "https://raw.githubusercontent.com/EQEmu/Server/master/";

#::: Globals
$time_stamp   = strftime('%m-%d-%Y', gmtime());
$db_run_stage = 0; #::: Sets database run stage check
if ($Config{osname} =~ /freebsd|linux/i) {
    $OS        = "Linux";
    $os_flavor = "";
    if (-e "/etc/debian_version") {
        $os_flavor = "debian";
    }
    elsif (-e "/etc/fedora-release") {
        $os_flavor = "fedora_core";
    }
    elsif (-e "/etc/redhat-release") {
        $os_flavor = "red_hat";
    }
}
if ($Config{osname} =~ /Win|MS/i) {
    $OS = "Windows";
}

$has_internet_connection = check_internet_connection();
if (-e "skip_internet_connection_check.txt") {
    $has_internet_connection = 1;
}

($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime();

if (-e "eqemu_server_skip_update.txt") {
    $skip_self_update_check = 1;
}

#::: Check for script self update
check_xml_to_json_conversion() if $ARGV[0] eq "convert_xml";
do_self_update_check_routine() if !$skip_self_update_check;
get_windows_wget();
get_perl_version();
if (-e "eqemu_config.json") {
    read_eqemu_config_json();
}
else {
    #::: This will need to stay for servers who simply haven't updated yet
    # This script can still update without the server bins being updated
    read_eqemu_config_xml();
}
get_mysql_path();

#::: Remove old eqemu_update.pl
if (-e "eqemu_update.pl") {
    unlink("eqemu_update.pl");
}

print "[Info] For EQEmu Server management utilities - run eqemu_server.pl\n" if $ARGV[0] eq "ran_from_world";

check_db_version_table();

#::: Check if db_version table exists...
if (trim(get_mysql_result("SHOW COLUMNS FROM db_version LIKE 'Revision'")) ne "" && $db) {
    print get_mysql_result("DROP TABLE db_version");
    print "[Database] Old db_version table present, dropping...\n\n";
}

check_for_world_bootup_database_update();

sub urlencode {
    my ($rv) = @_;
    $rv =~ s/([^A-Za-z0-9])/sprintf("%%%2.2X", ord($1))/ge;
    return $rv;
}

sub urldecode {
    my ($rv) = @_;
    $rv =~ s/\+/ /g;
    $rv =~ s/%(..)/pack("c", hex($1))/ge;
    return $rv;
}

sub analytics_insertion {
    $event_name = urlencode($_[0]);
    $event_data = urlencode($_[1]);

    #::: Check for internet connection before doing analytics
    if (!$has_internet_connection || $can_see_analytics_server == -1) {
        return;
    }

    #::: Check for analytics server connectivity so that the script doesn't break when its offline
    if (!$can_see_analytics_server) {
        if ($OS eq "Linux") {
            $count = "c";
        }
        if ($OS eq "Windows") {
            $count = "n";
        }

        if (`ping analytics.akkadius.com -$count 1 -w 500` =~ /Reply from|1 received/i) {
            $can_see_analytics_server = 1;
        }
        else {
            $can_see_analytics_server = -1;
        }
    }

    $server_name = "";
    if ($long_name) {
        $server_name = "&server_name=" . urlencode($long_name);
    }

    if (!$extended_os) {
        if ($OS eq "Linux") {
            $extended_os = `cat /proc/version`;
            $extended_os = trim($extended_os);
        }
        if ($OS eq "Windows") {
            my $output     = `ver`;
            my @os_version = split("\n", $output);
            foreach my $val (@os_version) {
                if ($val =~ /Windows/i) {
                    $extended_os = trim($val);
                }
            }
        }
    }

    $url = "http://analytics.akkadius.com/";
    $url .= "?api_key=24a0bde2e5bacd65bcab06a9ac40b62c";
    $url .= "&event=" . $event_name;
    $url .= "&event_data=" . $event_data;
    $url .= "&OS=" . urlencode($OS);
    $url .= "&extended_os=" . urlencode($extended_os);
    $url .= $server_name;

    # print "Calling url :: '" . $url . "'\n";

    if ($OS eq "Windows") {
        eval('require LWP::UserAgent;');
        my $ua = LWP::UserAgent->new;
        $ua->timeout(1);
        $ua->env_proxy;
        my $response = $ua->get($url);
    }
    if ($OS eq "Linux") {
        $api_call = `curl -s "$url"`;
    }
}

sub show_install_summary_info {
    print "[Install] Installation complete...\n";
    print "[Install] Server Info (Save somewhere if needed):\n";

    if (-e "install_variables.txt") {
        $file_to_open = "install_variables.txt";
    }
    elsif (-e "../install_variables.txt") {
        $file_to_open = "../install_variables.txt";
    }
    open(INSTALL_VARS, $file_to_open);
    while (<INSTALL_VARS>) {
        chomp;
        $o    = $_;
        @data = split(":", $o);
        print " - " . $data[0] . "\t" . $data[1] . "\n";
    }
    close(INSTALL_VARS);

    if ($OS eq "Windows") {
        print "[Install] Windows Utility Scripts:\n";
        print " - t_start_server.bat			Starts EQEmu server with 30 dynamic zones, UCS & Queryserv, dynamic zones\n";
        print " - t_start_server_with_loginserver.bat	Starts EQEmu server with 30 zones with loginserver\n";
        print " - t_stop_server.bat			Stops EQEmu Server (No warning)\n";
        print " - t_database_backup.bat		Backs up the Database to backups/ folder - do not run during server is online\n";
        print " - t_server_crash_report.pl 		Will parse any zone crashes for reporting to developers\n";
    }
    if ($OS eq "Linux") {
        print "[Install] Linux Utility Scripts:\n";
        print " - server_start.sh			Starts EQEmu server (Quiet) with 30 dynamic zones, UCS & Queryserv, dynamic zones\n";
        print " - server_start_dev.sh			Starts EQEmu server with 10 dynamic zones, UCS & Queryserv, dynamic zones all verbose\n";
        print " - server_stop.sh			Stops EQEmu Server (No warning)\n";
        print " - server_status.sh			Prints the status of the EQEmu Server processes\n";
    }

    print "[Configure] eqemu_config.xml 		Edit to change server settings and name\n";

    analytics_insertion("install_complete", "null");
}

sub new_server {
    $file_count = 0;
    opendir(DIR, ".") or die $!;
    while (my $file = readdir(DIR)) {
        next if ($file =~ m/^\./);
        $file_count++;
    }
    closedir(DIR);

    if ($file_count > 4 && (!-e "install_variables.txt" && !-e "../install_variables.txt")) {
        print "[New Server] ERROR: You must run eqemu_server.pl in an empty directory\n";
        <>;
        exit;
    }

    if (-e "install_variables.txt" || -e "../install_variables.txt") {
        get_installation_variables();
    }

    while (1) {

        $database_name     = $installation_variables{"mysql_eqemu_db_name"};
        $database_user     = $installation_variables{"mysql_eqemu_user"};
        $database_password = $installation_variables{"mysql_eqemu_password"};

        if ($database_name ne "") {
            $mysql_pass = 1;
        }
        else {

            print "\n";
            print "[New Server] For a new server folder install, we assume Perl and MySQL are configured\n";
            print "[New Server] This will install a fresh PEQ Database, with all server assets\n";
            print "[New Server] You will need to supply database credentials to get started...\n\n";

            check_for_input("MySQL User: ");
            $database_user = trim($input);

            check_for_input("MySQL Password: ");
            $database_password = trim($input);

            $check_connection = `mysql -u $database_user -p$database_password -N -B -e "SHOW PROCESSLIST" > mysqlcheck.txt`;
            $mysql_pass       = 0;
            open(MYSQL_CHECK, "mysqlcheck.txt");
            while (<MYSQL_CHECK>) {
                chomp;
                $o                                           = $_;
                if ($o =~ /Error/i) { $mysql_pass            = 0; }
                if ($o =~ /SHOW PROCESSLIST/i) { $mysql_pass = 1; }
            }
            close(MYSQL_CHECK);
            unlink("mysqlcheck.txt");
        }

        if ($mysql_pass == 1) {

            if ((!-e "install_variables.txt" && !-e "../install_variables.txt")) {
                print "[New Server] Success! We have a database connection\n";

                check_for_input("Specify a NEW database name that PEQ will be installed to: ");
                $database_name = trim($input);

                #::: Write install vars
                open(INSTALL_VARS, '>', 'install_variables.txt');
                print INSTALL_VARS "";
                print INSTALL_VARS "mysql_eqemu_db_name:" . $database_name . "\n";
                print INSTALL_VARS "mysql_eqemu_user:" . $database_user . "\n";
                print INSTALL_VARS "mysql_eqemu_password:" . $database_password . "\n";
                close(INSTALL_VARS);
            }
            analytics_insertion("new_server::install", $database_name);

            if ($OS eq "Linux") {
                build_linux_source("login");
            }

            do_installer_routines();

            if ($OS eq "Linux") {
                print `chmod 755 *.sh`;
            }

            analytics_insertion("new_server::install_complete", $database_name . " :: Binary DB Version / Local DB Version :: " . $binary_database_version . " / " . $local_database_version);

            print "[New Server] New server folder install complete\n";
            print "[New Server] Below is your installation info:\n";

            show_install_summary_info();

            if ($OS eq "Linux") {
                unlink('/home/eqemu/install_variables.txt');
            }

            rmtree('updates_staged');

            return;
        }
        else {
            print "[New Server] MySQL authorization failed or no MySQL installed\n";
        }
    }
}

sub check_xml_to_json_conversion {
    if (-e "eqemu_config.xml" && !-e "eqemu_config.json") {

        if ($OS eq "Windows") {
            get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/xmltojson/xmltojson-windows-x86.exe", "xmltojson.exe");
            print "Converting eqemu_config.xml to eqemu_config.json\n";
            print `xmltojson eqemu_config.xml`;
        }
        if ($OS eq "Linux") {
            get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/xmltojson/xmltojson-linux-x86", "xmltojson");
            print "Converting eqemu_config.xml to eqemu_config.json\n";
            print `chmod 755 xmltojson`;
            print `./xmltojson eqemu_config.xml`;
        }

        #::: Prettify and alpha order the config
        use JSON;
        my $json = new JSON();

        my $content;
        open(my $fh, '<', "eqemu_config.json") or die "cannot open file $filename"; {
            local $/;
            $content = <$fh>;
        }
        close($fh);

        $result = $json->decode($content);
        $json->canonical(1);

        print $json->pretty->indent_length(5)->utf8->encode($result), "\n";

        open(my $fh, '>', 'eqemu_config.json');
        print $fh $json->pretty->indent_length(5)->utf8->encode($result);
        close $fh;

        mkdir('backups');
        copy_file("eqemu_config.xml", "backups/eqemu_config.xml");
        unlink('eqemu_config.xml');
        unlink('db_dumper.pl');

        print "[Server Maintenance] eqemu_config.xml is now DEPRECATED \n";
        print "[Server Maintenance] eqemu_config.json is now the new Server config format \n";
        print " A backup of this old config is located in the backups folder of your server directory\n";
        print " --- \n";
        print " You may have some plugins and/or applications that still require reference of this config file\n";
        print " Please update these plugins/applications to use the new configuration format if needed\n";
        print " --- \n";
        print " Thanks for your understanding\n";
        print " The EQEmulator Team\n\n";

        exit;
    }

}

sub build_linux_source {

    $build_options = $_[0];

    $cmake_options          = "";
    $source_folder_post_fix = "";

    if ($build_options =~ /bots/i) {
        $cmake_options          .= " -DEQEMU_ENABLE_BOTS=ON";
        $source_folder_post_fix = "_bots";
    }

    $current_directory = `pwd`;
    @directories       = split('/', $current_directory);
    foreach my $val (@directories) {
        if (trim($val) ne "") {
            $last_directory = trim($val);
        }
    }
    my $eqemu_server_directory = "/home/eqemu";
    my $source_dir             = $eqemu_server_directory . '/' . $last_directory . '_source' . $source_folder_post_fix;

    $current_directory = trim($current_directory);

    mkdir($source_dir) if (!-e $source_dir);

    # print 'server_dir: ' . $eqemu_server_directory . "\n";
    # print 'source_dir: ' . $source_dir . "\n";
    # print 'current_dir: \'' . $current_directory . "'\n";

    chdir($source_dir);

    print `git clone https://github.com/EQEmu/Server.git`;

    mkdir($source_dir . "/Server/build") if (!-e $source_dir . "/Server/build");
    chdir($source_dir . "/Server");

    print `git submodule init`;
    print `git submodule update`;

    chdir($source_dir . "/Server/build");
    
    print "Generating CMake build files...\n";
    if ($os_flavor eq "fedora_core") {
        print `cmake $cmake_options -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_LUA=ON -DLUA_INCLUDE_DIR=/usr/include/lua-5.1/ -G "Unix Makefiles" ..`;
    }
    else {
        print `cmake $cmake_options -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_LUA=ON -G "Unix Makefiles" ..`;
    }
    print "Building EQEmu Server code. This will take a while.";

    #::: Build
    print `make`;

    chdir($current_directory);

    print `ln -s -f $source_dir/Server/build/bin/eqlaunch .`;
    print `ln -s -f $source_dir/Server/build/bin/export_client_files .`;
    print `ln -s -f $source_dir/Server/build/bin/import_client_files .`;
    print `ln -s -f $source_dir/Server/build/bin/libcommon.a .`;
    print `ln -s -f $source_dir/Server/build/bin/libluabind.a .`;
    print `ln -s -f $source_dir/Server/build/bin/queryserv .`;
    print `ln -s -f $source_dir/Server/build/bin/shared_memory .`;
    print `ln -s -f $source_dir/Server/build/bin/ucs .`;
    print `ln -s -f $source_dir/Server/build/bin/world .`;
    print `ln -s -f $source_dir/Server/build/bin/zone .`;
    print `ln -s -f $source_dir/Server/build/bin/loginserver .`;
}

sub do_installer_routines {
    print "[Install] EQEmu Server Installer... LOADING... PLEASE WAIT...\n";

    #::: Make some local server directories...
    mkdir('logs');
    mkdir('updates_staged');
    mkdir('shared');

    do_install_config_json();
    read_eqemu_config_json();
    get_installation_variables();

    $db_name = "peq";
    if ($installation_variables{"mysql_eqemu_db_name"}) {
        $db_name = $installation_variables{"mysql_eqemu_db_name"};
    }

    #::: Download assets
    if ($OS eq "Windows") {
        # fetch_latest_windows_binaries();
        fetch_latest_windows_appveyor();
        get_remote_file($install_repository_request_url . "lua51.dll", "lua51.dll", 1);
        get_remote_file($install_repository_request_url . "zlib1.dll", "zlib1.dll", 1);
		get_remote_file($install_repository_request_url . "zlib1.pdb", "zlib1.pdb", 1);
        get_remote_file($install_repository_request_url . "libmysql.dll", "libmysql.dll", 1);
    }

    map_files_fetch_bulk();
    opcodes_fetch();
    plugins_fetch();
    quest_files_fetch();
    lua_modules_fetch();
    fetch_utility_scripts();

    #::: Database Routines
    print "[Database] Creating Database '" . $db_name . "'\n";
    print `"$path" --host $host --user $user --password="$pass" -N -B -e "DROP DATABASE IF EXISTS $db_name;"`;
    print `"$path" --host $host --user $user --password="$pass" -N -B -e "CREATE DATABASE $db_name"`;

    my $world_path = "world";
    if (-e "bin/world") {
        $world_path = "bin/world";
    }

    #::: Get Binary DB version
    if ($OS eq "Windows") {
        @db_version = split(': ', `$world_path db_version`);
    }
    if ($OS eq "Linux") {
        @db_version = split(': ', `./$world_path db_version`);
    }

    $binary_database_version            = trim($db_version[1]);

    #::: Local DB Version
    check_db_version_table();
    $local_database_version = trim(get_mysql_result("SELECT version FROM db_version LIMIT 1"));

    #::: Download PEQ latest
    fetch_peq_db_full();
    print "[Database] Fetching Latest Database Updates...\n";
    main_db_management();
    print "[Database] Applying Latest Database Updates...\n";
    main_db_management();

    remove_duplicate_rule_values();

    if ($OS eq "Windows") {
        check_windows_firewall_rules();
        do_windows_login_server_setup();
    }
    if ($OS eq "Linux") {
        do_linux_login_server_setup();
    }
}

sub check_for_input {
    print "[Input] " . $_[0];
    $input = <STDIN>;
    chomp $input;
}

sub check_for_world_bootup_database_update {

    my $world_path = "world";
    if (-e "bin/world") {
        $world_path = "bin/world";
    }

    #::: Get Binary DB version
    if ($OS eq "Windows") {
        @db_version = split(': ', `$world_path db_version`);
    }
    if ($OS eq "Linux") {
        @db_version = split(': ', `./$world_path db_version`);
    }
    
    $binary_database_version = trim($db_version[1]);
    $local_database_version  = trim(get_mysql_result("SELECT version FROM db_version LIMIT 1"));

    #::: Bots
    $bots_binary_version = trim($db_version[2]);
    if ($bots_binary_version > 0) {
        $bots_local_db_version = get_bots_db_version();
        #::: We ran world - Database needs to update, lets backup and run updates and continue world bootup

        if ($bots_local_db_version < $bots_binary_version && $ARGV[0] eq "ran_from_world") {
            print "[Update] Bots Database not up to date with binaries... Automatically updating...\n";
            print "[Update] Issuing database backup first...\n";
            database_dump_compress();
            print "[Update] Updating bots database...\n";
            sleep(1);
            bots_db_management();
            run_database_check();
            print "[Update] Continuing bootup\n";
            analytics_insertion("auto database bots upgrade world", $db . " :: Binary DB Version / Local DB Version :: " . $binary_database_version . " / " . $local_database_version);

            exit;
        }
        else {
            print "[Update] Bots database up to Date: Continuing World Bootup...\n";
        }
    }

    if ($binary_database_version == $local_database_version && $ARGV[0] eq "ran_from_world") {
        print "[Update] Database up to date...\n";
        exit;
    }
    else {
        #::: We ran world - Database needs to update, lets backup and run updates and continue world bootup
        if ($local_database_version < $binary_database_version && $ARGV[0] eq "ran_from_world") {
            print "[Update] Database not up to date with binaries... Automatically updating...\n";
            print "[Update] Issuing database backup first...\n";
            database_dump_compress();
            print "[Update] Updating database...\n";
            sleep(1);
            main_db_management();
            main_db_management();
            print "[Update] Continuing bootup\n";
            analytics_insertion("auto database upgrade world", $db . " :: Binary DB Version / Local DB Version :: " . $binary_database_version . " / " . $local_database_version);

            exit;
        }

        #::: Make sure that we didn't pass any arugments to the script
        else {
            if (!$db) { print "[eqemu_server.pl] No database connection found... Running without\n"; }
            show_menu_prompt();
        }
    }
}

sub check_internet_connection {
    if ($OS eq "Linux") {
        $count = "c";
    }
    if ($OS eq "Windows") {
        $count = "n";
    }

    if (`ping 8.8.8.8 -$count 1 -w 500` =~ /TTL|1 received/i) {
        # print "[Update] We have a connection to the internet, continuing...\n";
        return 1;
    }
    elsif (`ping 4.2.2.2 -$count 1 -w 500` =~ /TTL|1 received/i) {
        # print "[Update] We have a connection to the internet, continuing...\n";
        return 1;
    }
    else {
        print "[Update] No connection to the internet, can't check update\n";
        return;
    }
}

sub get_perl_version {
    #::: Check Perl version
    $perl_version = $^V;
    $perl_version =~ s/v//g;
    print "[Update] Perl Version is " . $perl_version . "\n" if $debug;
    if ($perl_version > 5.12) {
        no warnings 'uninitialized';
    }
    no warnings;
}

sub get_windows_wget {
    if (!-e "wget.exe" && $OS eq "Windows") {
        eval "use LWP::Simple qw(getstore);";
        getstore("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/wget.exe", "wget.exe");
    }
}

sub do_self_update_check_routine {

    #::: Check for internet connection before updating
    if (!$has_internet_connection) {
        print "[Update] Cannot check update without internet connection...\n";
        return;
    }

    #::: Check for script changes :: eqemu_server.pl
    get_remote_file($eqemu_repository_request_url . "utils/scripts/eqemu_server.pl", "updates_staged/eqemu_server.pl", 0, 1, 1);

    if (-e "updates_staged/eqemu_server.pl") {

        my $remote_script_size = -s "updates_staged/eqemu_server.pl";
        my $local_script_size  = -s "eqemu_server.pl";

        if ($remote_script_size != $local_script_size) {
            print "[Update] Script has been updated, updating...\n";

            my @files;
            my $start_dir = "updates_staged/";
            find(
                sub { push @files, $File::Find::name unless -d; },
                $start_dir
            );
            for my $file (@files) {
                if ($file =~ /eqemu_server/i) {
                    $destination_file = $file;
                    $destination_file =~ s/updates_staged\///g;
                    print "[Install] Installing :: " . $destination_file . "\n";
                    unlink($destination_file);
                    copy_file($file, $destination_file);
                    if ($OS eq "Linux") {
                        system("chmod 755 eqemu_server.pl");
                    }
                    exec("perl eqemu_server.pl ran_from_world");
                }
            }
            print "[Install] Done\n";
        }
        else {
            print "[Update] No script update necessary...\n";

            if (-e "db_update") {
                unlink("db_update");
            }

            if (-e "updates_staged") {
                unlink("updates_staged");
            }
        }

        unlink("updates_staged/eqemu_server.pl");
        unlink("updates_staged");
    }
}

sub get_installation_variables {
    #::: Fetch installation variables before building the config
    if ($OS eq "Linux") {
        if (-e "../install_variables.txt") {
            open(INSTALL_VARS, "../install_variables.txt");
        }
        elsif (-e "install_variables.txt") {
            open(INSTALL_VARS, "./install_variables.txt");
        }
    }
    if ($OS eq "Windows") {
        open(INSTALL_VARS, "install_variables.txt");
    }
    while (<INSTALL_VARS>) {
        chomp;
        $o                                      = $_;
        @data                                   = split(":", $o);
        $installation_variables{trim($data[0])} = trim($data[1]);
    }
    close(INSTALL_VARS);
}

sub do_install_config_json {
    get_installation_variables();

    #::: Fetch json template
    get_remote_file($install_repository_request_url . "eqemu_config.json", "eqemu_config_template.json");

    use JSON;
    my $json = new JSON();

    my $content;
    open(my $fh, '<', "eqemu_config_template.json") or die "cannot open file $filename"; {
        local $/;
        $content = <$fh>;
    }
    close($fh);

    $config = $json->decode($content);

    $long_name                               = "Akkas " . $OS . " PEQ Installer (" . generate_random_password(5) . ')';
    $config->{"server"}{"world"}{"longname"} = $long_name;
    $config->{"server"}{"world"}{"key"}      = generate_random_password(30);

    if ($installation_variables{"mysql_eqemu_db_name"}) {
        $db_name = $installation_variables{"mysql_eqemu_db_name"};
    }
    else {
        $db_name = "peq";
    }

    $config->{"server"}{"database"}{"username"} = $installation_variables{"mysql_eqemu_user"};
    $config->{"server"}{"database"}{"password"} = $installation_variables{"mysql_eqemu_password"};
    $config->{"server"}{"database"}{"db"}       = $db_name;

    $config->{"server"}{"qsdatabase"}{"username"} = $installation_variables{"mysql_eqemu_user"};
    $config->{"server"}{"qsdatabase"}{"password"} = $installation_variables{"mysql_eqemu_password"};
    $config->{"server"}{"qsdatabase"}{"db"}       = $db_name;

    $json->canonical(1);
    $json->indent_length(5);

    open(my $fh, '>', 'eqemu_config.json');
    print $fh $json->pretty->indent_length(5)->utf8->encode($config);
    close $fh;

    unlink("eqemu_config_template.json");
}

sub do_install_config_login_json {
    get_installation_variables();

    #::: Fetch json template
    get_remote_file($eqemu_repository_request_url . "loginserver/login_util/login.json", "login_template.json");

    use JSON;
    my $json = new JSON();

    my $content;
    open(my $fh, '<', "login_template.json") or die "cannot open file $filename"; {
        local $/;
        $content = <$fh>;
    }
    close($fh);

    $config = $json->decode($content);

    if ($installation_variables{"mysql_eqemu_db_name"}) {
        $db_name = $installation_variables{"mysql_eqemu_db_name"};
    }
    else {
        $db_name = "peq";
    }

    $config->{"database"}{"host"} = "127.0.0.1";
    $config->{"database"}{"user"} = $installation_variables{"mysql_eqemu_user"};
    $config->{"database"}{"password"} = $installation_variables{"mysql_eqemu_password"};
    $config->{"database"}{"db"}       = $db_name;

    $json->canonical(1);
    $json->indent_length(5);

    open(my $fh, '>', 'login.json');
    print $fh $json->pretty->indent_length(5)->utf8->encode($config);
    close $fh;

    unlink("login_template.json");
}

sub fetch_utility_scripts {
    if ($OS eq "Windows") {
        get_remote_file($install_repository_request_url . "t_database_backup.bat", "t_database_backup.bat");
        get_remote_file($install_repository_request_url . "t_start_server.bat", "t_start_server.bat");
        get_remote_file($install_repository_request_url . "t_server_update_binaries_no_bots.bat", "t_server_update_binaries_no_bots.bat");
        get_remote_file($install_repository_request_url . "t_start_server_with_login_server.bat", "t_start_server_with_login_server.bat");
        get_remote_file($install_repository_request_url . "t_stop_server.bat", "t_stop_server.bat");
        get_remote_file($install_repository_request_url . "t_server_crash_report.pl", "t_server_crash_report.pl");
        get_remote_file($install_repository_request_url . "win_server_launcher.pl", "win_server_launcher.pl");
        get_remote_file($install_repository_request_url . "t_start_server_with_login_server.bat", "t_start_server_with_login_server.bat");
    }
    else {
        get_remote_file($install_repository_request_url . "linux/server_launcher.pl", "server_launcher.pl");
        get_remote_file($install_repository_request_url . "linux/server_start.sh", "server_start.sh");
        get_remote_file($install_repository_request_url . "linux/server_start_dev.sh", "server_start_dev.sh");
        get_remote_file($install_repository_request_url . "linux/server_status.sh", "server_status.sh");
        get_remote_file($install_repository_request_url . "linux/server_stop.sh", "server_stop.sh");
    }
}

sub setup_bots {
    if ($OS eq "Windows") {
        fetch_latest_windows_appveyor_bots();
    }
    if ($OS eq "Linux") {
        build_linux_source("bots");
    }
    bots_db_management();
    run_database_check();

    print "Bots should be setup, run your server and the bot command should be available in-game (type '^help')\n";
}

sub show_menu_prompt {

    $dc = 0;
    while (1) {

        if ($ARGV[0] ne "") {
            $input = trim($ARGV[0]);
        }
        else {
            $input = trim($input);
        }

        $errored_command = 0;

        if ($input eq "database") {
            print "\n>>> Database Menu\n\n";
            print " [backup_database]		Back up database to backups/ directory\n";
            print " [backup_player_tables]		Back up player tables to backups/ directory\n";
            print " [backup_database_compressed]	Back up database compressed to backups/ directory\n";
            print " \n";
            print " [check_db_updates]		Checks for database updates manually\n";
            print " [check_bot_db_updates]		Checks for bot database updates\n";
            print " \n";
            print " [aa_tables]			Downloads and installs clean slate AA data from PEQ\n";
            print " [remove_duplicate_rules]	Removes duplicate rules from rule_values table\n";
            print " [drop_bots_db_schema]		Removes bot database schema\n";

            print " \n> main - go back to main menu\n";
            print "Enter a command #> ";
            $last_menu = trim($input);
        }
        elsif ($input eq "conversions") {
            print "\n>>> Conversions Menu\n\n";
            print " [quest_heading_convert] Converts old heading format in quest scripts to new (live format)\n";
            print " [quest_faction_convert] Converts to new faction values imported from client\n";
            print " \n> main - go back to main menu\n";
            print "Enter a command #> ";
            $last_menu = trim($input);
        }
        elsif ($input eq "assets") {
            print "\n>>> Server Assets Menu\n\n";
            print " [maps]			Download latest maps\n";
            print " [opcodes]		Download opcodes (Patches for eq clients)\n";
            print " [quests]		Download latest quests\n";
            print " [plugins]		Download latest plugins\n";
            print " [lua_modules]		Download latest lua_modules\n";
            print " [utility_scripts]	Download utility scripts to run and operate the EQEmu Server\n";
            if ($OS eq "Windows") {
                print ">>> Windows\n";
                print " [windows_server_download]	Updates server via latest 'stable' code\n";
                print " [windows_server_latest]	Updates server via latest commit 'unstable'\n";
                print " [windows_server_download_bots]	Updates server (bots) via latest 'stable'\n";
                print " [windows_server_latest_bots]	Updates server (bots) via latest commit 'unstable'\n";
                print " [fetch_dlls]			Grabs dll's needed to run windows binaries\n";
                print " [setup_loginserver]		Sets up loginserver for Windows\n";
            }
            print " \n> main - go back to main menu\n";
            print "Enter a command #> ";
            $last_menu = trim($input);
        }
        elsif ($input eq "backup_database") {
            database_dump();
            $dc = 1;
        }
        elsif ($input eq "backup_player_tables") {
            database_dump_player_tables();
            $dc = 1;
        }
        elsif ($input eq "backup_database_compressed") {
            database_dump_compress();
            $dc = 1;
        }
        elsif ($input eq "drop_bots_db_schema") {
            do_bots_db_schema_drop();
            $dc = 1;
        }
        elsif ($input eq "aa_tables") {
            aa_fetch();
            $dc = 1;
        }
        elsif ($input eq "remove_duplicate_rules") {
            remove_duplicate_rule_values();
            $dc = 1;
        }
        elsif ($input eq "maps") {
            map_files_fetch_bulk();
            $dc = 1;
        }
        elsif ($input eq "opcodes") {
            opcodes_fetch();
            $dc = 1;
        }
        elsif ($input eq "plugins") {
            plugins_fetch();
            $dc = 1;
        }
        elsif ($input eq "quests") {
            quest_files_fetch();
            $dc = 1;
        }
        elsif ($input eq "lua_modules") {
            lua_modules_fetch();
            $dc = 1;
        }
        elsif ($input eq "windows_server_download") {
            fetch_latest_windows_binaries();
            $dc = 1;
        }
        elsif ($input eq "windows_server_latest") {
            fetch_latest_windows_appveyor();
            $dc = 1;
        }
        elsif ($input eq "windows_server_download_bots") {
            fetch_latest_windows_binaries_bots();
            $dc = 1;
        }
        elsif ($input eq "windows_server_latest_bots") {
            fetch_latest_windows_appveyor_bots();
            $dc = 1;
        }
        elsif ($input eq "fetch_dlls") {
            fetch_server_dlls();
            $dc = 1;
        }
        elsif ($input eq "utility_scripts") {
            fetch_utility_scripts();
            $dc = 1;
        }
        elsif ($input eq "check_db_updates") {
            main_db_management();
            main_db_management();
            $dc = 1;
        }
        elsif ($input eq "check_bot_db_updates") {
            bots_db_management();
            run_database_check();
            $dc = 1;
        }
        elsif ($input eq "setup_loginserver") {
            do_windows_login_server_setup();
            $dc = 1;
        }
        elsif ($input eq "new_server") {
            new_server();
            $dc = 1;
        }
        elsif ($input eq "setup_bots") {
            setup_bots();
            $dc = 1;
        }
        elsif ($input eq "linux_login_server_setup") {
            do_linux_login_server_setup();
            $dc = 1;
        }
        elsif ($input eq "quest_heading_convert") {
            quest_heading_convert();
            $dc = 1;
        }
        elsif ($input eq "quest_faction_convert") {
            quest_faction_convert();
            $dc = 1;
        }
        elsif ($input eq "source_peq_db") {
            fetch_peq_db_full();
            $dc = 1;
        }
        elsif ($input eq "exit") {
            exit;
        }
        elsif ($input eq "main") {
            print "Returning to main menu...\n";
            print_main_menu();
            $last_menu = trim($input);
        }
        elsif ($input eq "" && $last_menu ne "") {
            $errored_command = 1;
        }
        elsif ($input ne "") {
            # print "Invalid command '" . $input . "'\n";
            $errored_command = 1;
        }
        else {
            print_main_menu();
        }

        #::: Errored command checking
        if ($errored_command == 1) {
            $input = $last_menu;
        }
        elsif ($dc == 1) {
            analytics_insertion("menu", trim($input));
            $dc    = 0;
            $input = "";
        }
        else {
            $input = <>;
        }

        #::: If we're processing a CLI command, kill the loop
        if ($ARGV[0] ne "") {
            analytics_insertion("cli", trim($input));
            $input   = "";
            $ARGV[0] = "";
            exit;
        }
    }
}

sub print_main_menu {
    print "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
    print ">>> EQEmu Server Main Menu >>>>>>>>>>>>\n";
    print ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n";
    print " [database]	Enter database management menu \n";
    print " [assets]	Manage server assets \n";
    print " [new_server]	New folder EQEmu/PEQ install - Assumes MySQL/Perl installed \n";
    print " [setup_bots]	Enables bots on server - builds code and database requirements \n";
    print " [conversions]	Routines used for conversion of scripts/data \n";
    print "\n";
    print " exit \n";
    print "\n";
    print "Enter a command #> ";
}

sub get_mysql_path {
    if ($OS eq "Windows") {
        $has_mysql_path = `echo %PATH%`;
        if ($has_mysql_path =~ /MySQL|MariaDB/i) {
            @mysql = split(';', $has_mysql_path);
            foreach my $v (@mysql) {
                if ($v =~ /MySQL|MariaDB/i) {
                    $v =~ s/\n//g;
                    $path = trim($v) . "/mysql";
                    last;
                }
            }
        }
    }
    if ($OS eq "Linux") {
        $path = `which mysql`;
        if ($path eq "") {
            $path = `which mariadb`;
        }
        $path =~ s/\n//g;
    }

    #::: Path not found, error and exit
    if ($path eq "") {
        print "[Error:eqemu_server.pl] MySQL path not found, please add the path for automatic database upgrading to continue... \n\n";
        exit;
    }
}

sub check_for_database_dump_script {
    #::: Check for internet connection before updating
    if (!$has_internet_connection) {
        print "[Update] Cannot check update without internet connection...\n";
        return;
    }

    #::: Check for script changes :: database_dumper.pl
    get_remote_file($eqemu_repository_request_url . "utils/scripts/database_dumper.pl", "updates_staged/database_dumper.pl", 0, 1, 1);

    if (-e "updates_staged/database_dumper.pl") {

        my $remote_script_size = -s "updates_staged/database_dumper.pl";
        my $local_script_size  = -s "database_dumper.pl";

        if ($remote_script_size != $local_script_size) {
            print "[Update] Script has been updated, updating...\n";

            my @files;
            my $start_dir = "updates_staged/";
            find(
                sub { push @files, $File::Find::name unless -d; },
                $start_dir
            );
            for my $file (@files) {
                if ($file =~ /database_dumper/i) {
                    $destination_file = $file;
                    $destination_file =~ s/updates_staged\///g;
                    print "[Install] Installing :: " . $destination_file . "\n";
                    unlink($destination_file);
                    copy_file($file, $destination_file);
                    if ($OS eq "Linux") {
                        system("chmod 755 database_dumper.pl");
                    }
                }
            }
            print "[Install] Done\n";
        }
        else {
            print "[Update] No script update necessary...\n";
        }

        unlink("updates_staged/database_dumper.pl");
    }

    return;

}

sub database_dump {
    check_for_database_dump_script();
    print "[Database] Performing database backup....\n";
    print `perl database_dumper.pl database="$db" loc="backups"`;
}

sub database_dump_player_tables {
    check_for_database_dump_script();
    print "[Database] Performing database backup of player tables....\n";
    get_remote_file($eqemu_repository_request_url . "utils/sql/character_table_list.txt", "backups/character_table_list.txt");

    $tables = "";
    open(FILE, "backups/character_table_list.txt");
    $i = 0;
    while (<FILE>) {
        chomp;
        $o      = $_;
        $tables .= $o . ",";
    }
    $tables = substr($tables, 0, -1);

    print `perl database_dumper.pl database="$db" loc="backups" tables="$tables" backup_name="player_tables_export" nolock`;

    print "[Database] Press any key to continue...\n";

    <>; #Read from STDIN

}

sub database_dump_compress {
    check_for_database_dump_script();
    print "[Database] Performing database backup....\n";
    print `perl database_dumper.pl database="$db"  loc="backups" compress`;
}

sub script_exit {
    #::: Cleanup staged folder...
    rmtree("updates_staged/");
    exit;
}

sub check_db_version_table {
    if (get_mysql_result("SHOW TABLES LIKE 'db_version'") eq "" && $db) {
        print get_mysql_result("
			CREATE TABLE db_version (
			  version int(11) DEFAULT '0'
			) ENGINE=InnoDB DEFAULT CHARSET=latin1;
			INSERT INTO db_version (version) VALUES ('1000');");
        print "[Database] Table 'db_version' does not exist.... Creating...\n\n";
    }
}

#::: Returns Tab Delimited MySQL Result from Command Line
sub get_mysql_result {
    my $run_query = $_[0];
    if (!$db) { return; }
    if ($OS eq "Windows") { return `"$path" --host $host --user $user --password="$pass" $db -N -B -e "$run_query"`; }
    if ($OS eq "Linux") {
        $run_query =~ s/`//g;
        return `$path --user="$user" --host $host --password="$pass" $db -N -B -e "$run_query"`;
    }
}

sub get_mysql_result_from_file {
    my $update_file = $_[0];
    if (!$db) {
        return;
    }

    if ($OS eq "Windows") {
        return `"$path" --host $host --user $user --password="$pass" --force $db < $update_file`;
    }

    if ($OS eq "Linux") {
        return `"$path" --host $host --user $user --password="$pass" --force $db < $update_file`;
    }
}

#::: Gets Remote File based on request_url (1st Arg), and saves to destination file (2nd Arg)
#::: Example: get_remote_file($eqemu_repository_request_url . "utils/sql/db_update_manifest.txt", "db_update/db_update_manifest.txt");
sub get_remote_file {
    my $request_url      = $_[0];
    my $destination_file = $_[1];
    my $content_type     = $_[2];
    my $no_retry         = $_[3];
    my $silent_download  = $_[4];

    if (!$has_internet_connection) {
        print "[Download] Cannot download without internet connection...\n";
        return;
    }

    #::: Build file path of the destination file so that we may check for the folder's existence and make it if necessary

    if ($destination_file =~ /\//i) {
        my @directory_path = split('/', $destination_file);
        $build_path        = "";
        $directory_index   = 0;
        while ($directory_path[$directory_index] && $directory_path[$directory_index + 1]) {
            $build_path .= $directory_path[$directory_index] . "/";
            # print "checking '" . $build_path . "'\n";
            #::: If path does not exist, create the directory...
            if (!-d $build_path) {
                print "[Copy] folder doesn't exist, creating '" . $build_path . "'\n";
                mkdir($build_path);
            }
            if (!$directory_indexr_path[$directory_index + 2] && $directory_indexr_path[$directory_index + 1]) {
                # print $actual_path . "\n";
                $actual_path = $build_path;
                last;
            }
            $directory_index++;
        }
    }

    #::: wget -O db_update/db_update_manifest.txt https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/db_update_manifest.txt
    $wget = `wget -N --no-cache --cache=no --no-check-certificate --quiet -O $destination_file $request_url`;
    print "[Download] Saved: (" . $destination_file . ") from " . $request_url . "\n" if !$silent_download;
    if ($wget =~ /unable to resolve/i) {
        print "Error, no connection or failed request...\n\n";
        #die;
    }

}

#::: Trim Whitespaces
sub trim {
    my $string = $_[0];
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}

sub read_eqemu_config_xml {
    open(CONFIG, "eqemu_config.xml");
    while (<CONFIG>) {
        chomp;
        $o = $_;

        if ($o =~ /\<\!--/i) {
            next;
        }

        if ($o =~ /database/i && $o =~ /\<\//i) {
            $in_database_tag = 0;
        }
        if ($o =~ /<database>/i) {
            print "IN DATABASE TAG\n" if $debug;
            $in_database_tag = 1;
        }
        if ($o =~ /<longname>/i) {
            ($long_name) = $o =~ /<longname>(.*)<\/longname>/;
            print "Long Name: '" . $long_name . "'\n" if $debug;
        }
        if ($in_database_tag == 1) {
            @left     = split(">", $o);
            @right    = split("<", $left[1]);
            $tag_data = trim($right[0]);

            if ($o =~ /<username>/i && $in_database_tag) {
                $user = $tag_data;
                print "Database User: '" . $user . "'\n" if $debug;
            }
            if ($o =~ /<password>/i && $in_database_tag) {
                $pass = $tag_data;
                print "Database Pass: '" . $pass . "'\n" if $debug;
            }
            if ($o =~ /<db>/i) {
                $db = $tag_data;
                print "Database Name: '" . $db . "'\n" if $debug;
            }
            if ($o =~ /<host>/i) {
                $host = $tag_data;
                print "Database Host: '" . $host . "'\n" if $debug;
            }
        }
    }
    close(CONFIG);
}

sub read_eqemu_config_json {
    use JSON;
    my $json = new JSON();

    my $content;
    open(my $fh, '<', "eqemu_config.json") or die "cannot open file $filename"; {
        local $/;
        $content = <$fh>;
    }
    close($fh);

    $config = $json->decode($content);

    $db   = $config->{"server"}{"database"}{"db"};
    $host = $config->{"server"}{"database"}{"host"};
    $user = $config->{"server"}{"database"}{"username"};
    $pass = $config->{"server"}{"database"}{"password"};

}

#::: Fetch Latest PEQ AA's
sub aa_fetch {
    if (!$db) {
        print "No database present, check your eqemu_config.xml for proper MySQL/MariaDB configuration...\n";
        return;
    }

    print "[Install] Pulling down PEQ AA Tables...\n";
    get_remote_file($eqemu_repository_request_url . "utils/sql/peq_aa_tables_post_rework.sql", "db_update/peq_aa_tables_post_rework.sql");
    print "[Install] Installing AA Tables...\n";
    print get_mysql_result_from_file("db_update/peq_aa_tables_post_rework.sql");
    print "[Install] Done...\n\n";
}

#::: Fetch Latest Opcodes
sub opcodes_fetch {
    print "[Update] Pulling down latest opcodes...\n";
    %opcodes = (
        1 => [ "opcodes", $eqemu_repository_request_url . "utils/patches/opcodes.conf" ],
        2 => [ "mail_opcodes", $eqemu_repository_request_url . "utils/patches/mail_opcodes.conf" ],
        3 => [ "Titanium", $eqemu_repository_request_url . "utils/patches/patch_Titanium.conf" ],
        4 => [ "Secrets of Faydwer", $eqemu_repository_request_url . "utils/patches/patch_SoF.conf" ],
        5 => [ "Seeds of Destruction", $eqemu_repository_request_url . "utils/patches/patch_SoD.conf" ],
        6 => [ "Underfoot", $eqemu_repository_request_url . "utils/patches/patch_UF.conf" ],
        7 => [ "Rain of Fear", $eqemu_repository_request_url . "utils/patches/patch_RoF.conf" ],
        8 => [ "Rain of Fear 2", $eqemu_repository_request_url . "utils/patches/patch_RoF2.conf" ],
    );
    $loop = 1;
    while ($opcodes{$loop}[0]) {
        #::: Split the request_url by the patches folder to get the file name from request_url
        @real_file = split("patches/", $opcodes{$loop}[1]);
        $find      = 0;
        while ($real_file[$find]) {
            $file_name = $real_file[$find];
            $find++;
        }

        get_remote_file($opcodes{$loop}[1], $file_name);
        $loop++;
    }
    print "[Update] Done...\n";
}

sub remove_duplicate_rule_values {
    $ruleset_id = trim(get_mysql_result("SELECT `ruleset_id` FROM `rule_sets` WHERE `name` = 'default'"));
    print "[Database] Default Ruleset ID: " . $ruleset_id . "\n";

    $total_removed = 0;

    #::: Store Default values...
    $mysql_result = get_mysql_result("SELECT * FROM `rule_values` WHERE `ruleset_id` = " . $ruleset_id);
    my @lines     = split("\n", $mysql_result);
    foreach my $val (@lines) {
        my @values                      = split("\t", $val);
        $rule_set_values{$values[1]}[0] = $values[2];
    }

    #::: Compare default values against other rulesets to check for duplicates...
    $mysql_result = get_mysql_result("SELECT * FROM `rule_values` WHERE `ruleset_id` != " . $ruleset_id);
    my @lines     = split("\n", $mysql_result);
    foreach my $val (@lines) {
        my @values = split("\t", $val);
        if ($values[2] == $rule_set_values{$values[1]}[0]) {
            print "[Database] Removing duplicate : " . $values[1] . " (Ruleset (" . $values[0] . ")) matches default value of : " . $values[2] . "\n";
            get_mysql_result("DELETE FROM `rule_values` WHERE `ruleset_id` = " . $values[0] . " AND `rule_name` = '" . $values[1] . "'");
            $total_removed++;
        }
    }

    print "[Database] Total duplicate rules removed... " . $total_removed . "\n";
}

sub copy_file {
    $l_source_file      = $_[0];
    $l_destination_file = $_[1];
    if ($l_destination_file =~ /\//i) {
        my @directory_path = split('/', $l_destination_file);
        $build_path        = "";
        $directory_index   = 0;
        while ($directory_path[$directory_index]) {
            $build_path .= $directory_path[$directory_index] . "/";
            #::: If path does not exist, create the directory...
            if (!-d $build_path) {
                mkdir($build_path);
            }
            if (!$directory_path[$directory_index + 2] && $directory_path[$directory_index + 1]) {
                # print $actual_path . "\n";
                $actual_path = $build_path;
                last;
            }
            $directory_index++;
        }
    }
    copy $l_source_file, $l_destination_file;
}

sub fetch_latest_windows_appveyor {
    print "[Update] Fetching Latest Windows Binaries (unstable) from Appveyor... \n";
    get_remote_file("https://ci.appveyor.com/api/projects/KimLS/server/artifacts/eqemu-x86-no-bots.zip", "updates_staged/eqemu-x86-no-bots.zip", 1);

    print "[Update] Fetched Latest Windows Binaries (unstable) from Appveyor... \n";
    print "[Update] Extracting... --- \n";
    unzip('updates_staged/eqemu-x86-no-bots.zip', 'updates_staged/binaries/');
    my @files;
    my $start_dir = "updates_staged/binaries";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/updates_staged\/binaries\///g;
        print "[Update] Installing :: " . $destination_file . "\n";
        copy_file($file, $destination_file);
    }
    print "[Update] Done\n";

    rmtree('updates_staged');
}

sub fetch_latest_windows_binaries {
    print "[Update] Fetching Latest Windows Binaries... \n";
    get_remote_file($install_repository_request_url . "master_windows_build.zip", "updates_staged/master_windows_build.zip", 1);
    print "[Update] Fetched Latest Windows Binaries... \n";
    print "[Update] Extracting... --- \n";
    unzip('updates_staged/master_windows_build.zip', 'updates_staged/binaries/');
    my @files;
    my $start_dir = "updates_staged/binaries";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/updates_staged\/binaries\///g;
        print "[Update] Installing :: " . $destination_file . "\n";
        copy_file($file, $destination_file);
    }
    print "[Update] Done\n";

    rmtree('updates_staged');
}

sub fetch_latest_windows_appveyor_bots {
    print "[Update] Fetching Latest Windows Binaries with Bots (unstable) from Appveyor... \n";
    get_remote_file("https://ci.appveyor.com/api/projects/KimLS/server/artifacts/eqemu-x86-bots.zip", "updates_staged/eqemu-x86-bots.zip", 1);

    print "[Update] Fetched Latest Windows Binaries (unstable) from Appveyor... \n";
    print "[Update] Extracting... --- \n";
    unzip('updates_staged/eqemu-x86-bots.zip', 'updates_staged/binaries/');
    my @files;
    my $start_dir = "updates_staged/binaries";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/updates_staged\/binaries\///g;
        print "[Update] Installing :: " . $destination_file . "\n";
        copy_file($file, $destination_file);
    }
    print "[Update] Done\n";

    rmtree('updates_staged');
}

sub fetch_latest_windows_binaries_bots {
    print "[Update] Fetching Latest Windows Binaries with Bots...\n";
    get_remote_file($install_repository_request_url . "master_windows_build_bots.zip", "updates_staged/master_windows_build_bots.zip", 1);

    print "[Update] Fetched Latest Windows Binaries with Bots...\n";
    print "[Update] Extracting...\n";
    unzip('updates_staged/master_windows_build_bots.zip', 'updates_staged/binaries/');
    my @files;
    my $start_dir = "updates_staged/binaries";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/updates_staged\/binaries\///g;
        print "[Install] Installing :: " . $destination_file . "\n";
        copy_file($file, $destination_file);
    }
    print "[Update] Done...\n";

    rmtree('updates_staged');
}

sub do_windows_login_server_setup {
    print "[Install] Fetching Loginserver... \n";
    get_remote_file($install_repository_request_url . "login_server.zip", "updates_staged/login_server.zip", 1);
    print "[Install] Extracting... \n";
    unzip('updates_staged/login_server.zip', 'updates_staged/login_server/');
    my @files;
    my $start_dir = "updates_staged/login_server";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/updates_staged\/login_server\///g;
        print "[Install] Installing :: " . $destination_file . "\n";
        copy_file($file, $destination_file);
    }
    print "[Install] Done... \n";

    print "[Install] Pulling down Loginserver database tables...\n";
    get_remote_file($eqemu_repository_request_url . "loginserver/login_util/login_schema.sql", "db_update/login_schema.sql");
    print "[Install] Installing Loginserver tables...\n";
    print get_mysql_result_from_file("db_update/login_schema.sql");
    print "[Install] Done...\n";

    print "[Install] Pulling and initializing Loginserver configuration files...\n";
    do_install_config_login_json();
    print "[Install] Done...\n";

    add_login_server_firewall_rules();

    rmtree('updates_staged');
    rmtree('db_update');

    print "[Install] Press any key to continue...\n";

    <>; #Read from STDIN

}

sub do_linux_login_server_setup {

    build_linux_source();

    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/updates_staged\/login_server\///g;
        print "[Install] Installing :: " . $destination_file . "\n";
        copy_file($file, $destination_file);
    }
    print "\n Done... \n";

    print "[Install] Pulling down Loginserver database tables...\n";
    get_remote_file($eqemu_repository_request_url . "loginserver/login_util/login_schema.sql", "db_update/login_schema.sql");
    print "[Install] Installing Loginserver tables...\n";
    print get_mysql_result_from_file("db_update/login_schema.sql");
    print "[Install] Done...\n\n";

    print "[Install] Pulling and initializing Loginserver configuration files...\n";
    do_install_config_login_json();
    print "[Install] Done...\n";

    rmtree('updates_staged');
    rmtree('db_update');

    get_remote_file($install_repository_request_url . "linux/login_opcodes.conf", "login_opcodes.conf");
    get_remote_file($install_repository_request_url . "linux/login_opcodes_sod.conf", "login_opcodes_sod.conf");
    get_remote_file($install_repository_request_url . "linux/server_start_with_login.sh", "server_start_with_login.sh");
    system("chmod 755 *.sh");

    print "[Install] Press any key to continue...\n";

    <>; #Read from STDIN

}

sub add_login_server_firewall_rules {
    #::: Check Loginserver Firewall install for Windows
    if ($OS eq "Windows") {
        $output                         = `netsh advfirewall firewall show rule name=all`;
        @output_buffer                  = split("\n", $output);
        $has_loginserver_rules_titanium = 0;
        $has_loginserver_rules_sod      = 0;
        foreach my $val (@output_buffer) {
            if ($val =~ /Rule Name/i) {
                $val =~ s/Rule Name://g;
                if ($val =~ /EQEmu Loginserver/i && $val =~ /Titanium/i) {
                    $has_loginserver_rules_titanium = 1;
                    print "[Install] Found existing rule :: " . trim($val) . "\n";
                }
                if ($val =~ /EQEmu Loginserver/i && $val =~ /SOD/i) {
                    $has_loginserver_rules_sod = 1;
                    print "[Install] Found existing rule :: " . trim($val) . "\n";
                }
            }
        }

        if ($has_loginserver_rules_titanium == 0) {
            print "[Install] Attempting to add EQEmu Loginserver Firewall Rules (Titanium) (TCP) port 5998 \n";
            print `netsh advfirewall firewall add rule name="EQEmu Loginserver (Titanium) (5998) TCP" dir=in action=allow protocol=TCP localport=5998`;
            print "[Install] Attempting to add EQEmu Loginserver Firewall Rules (Titanium) (UDP) port 5998 \n";
            print `netsh advfirewall firewall add rule name="EQEmu Loginserver (Titanium) (5998) UDP" dir=in action=allow protocol=UDP localport=5998`;
        }
        if ($has_loginserver_rules_sod == 0) {
            print "[Install] Attempting to add EQEmu Loginserver Firewall Rules (SOD+) (TCP) port 5999 \n";
            print `netsh advfirewall firewall add rule name="EQEmu Loginserver (SOD+) (5999) TCP" dir=in action=allow protocol=TCP localport=5999`;
            print "[Install] Attempting to add EQEmu Loginserver Firewall Rules (SOD+) (UDP) port 5999 \n";
            print `netsh advfirewall firewall add rule name="EQEmu Loginserver (SOD+) (5999) UDP" dir=in action=allow protocol=UDP localport=5999`;
        }

        print "If firewall rules don't add you must run this script (eqemu_server.pl) as administrator\n";
        print "\n";
        print "[Install] Instructions \n";
        print "[Install] In order to connect your server to the loginserver you must point your eqemu_config.json to your local server similar to the following:\n";
        print "
	\"loginserver1\" : {
		\"account\" : \"\",
		\"host\" : \"login.eqemulator.net\",
		\"password\" : \"\",
		\"port\" : \"5998\",
		\"legacy\": \"1\"
	},
	\"loginserver2\" : {
		\"account\" : \"\",
		\"host\" : \"192.168.197.129\",
		\"password\" : \"\",
		\"port\" : \"5998\"
	},
	\"localaddress\" : \"192.168.197.129\",
		";
        print "[Install] When done, make sure your EverQuest client points to your loginserver's IP (In this case it would be 192.168.197.129) in the eqhosts.txt file\n";
    }
}

sub check_windows_firewall_rules {
    $output          = `netsh advfirewall firewall show rule name=all`;
    @output_buffer   = split("\n", $output);
    $has_world_rules = 0;
    $has_zone_rules  = 0;
    foreach my $val (@output_buffer) {
        if ($val =~ /Rule Name/i) {
            $val =~ s/Rule Name://g;
            if ($val =~ /EQEmu World/i) {
                $has_world_rules = 1;
                print "[Install] Found existing rule :: " . trim($val) . "\n";
            }
            if ($val =~ /EQEmu Zone/i) {
                $has_zone_rules = 1;
                print "[Install] Found existing rule :: " . trim($val) . "\n";
            }
        }
    }

    if ($has_world_rules == 0) {
        print "[Install] Attempting to add EQEmu World Firewall Rules (TCP) port 9000 \n";
        print `netsh advfirewall firewall add rule name="EQEmu World (9000) TCP" dir=in action=allow protocol=TCP localport=9000`;
        print "[Install] Attempting to add EQEmu World Firewall Rules (UDP) port 9000 \n";
        print `netsh advfirewall firewall add rule name="EQEmu World (9000) UDP" dir=in action=allow protocol=UDP localport=9000`;
    }
    if ($has_zone_rules == 0) {
        print "[Install] Attempting to add EQEmu Zones (7000-7500) TCP \n";
        print `netsh advfirewall firewall add rule name="EQEmu Zones (7000-7500) TCP" dir=in action=allow protocol=TCP localport=7000-7500`;
        print "[Install] Attempting to add EQEmu Zones (7000-7500) UDP \n";
        print `netsh advfirewall firewall add rule name="EQEmu Zones (7000-7500) UDP" dir=in action=allow protocol=UDP localport=7000-7500`;
    }
}

sub fetch_server_dlls {
    print "[Download] Fetching lua51.dll, zlib1.dll, zlib1.pdb, libmysql.dll...\n";
    get_remote_file($install_repository_request_url . "lua51.dll", "lua51.dll", 1);
    get_remote_file($install_repository_request_url . "zlib1.dll", "zlib1.dll", 1);
	get_remote_file($install_repository_request_url . "zlib1.pdb", "zlib1.pdb", 1);
    get_remote_file($install_repository_request_url . "libmysql.dll", "libmysql.dll", 1);
}

sub fetch_peq_db_full {
    print "[Install] Downloading latest PEQ Database... Please wait...\n";
    get_remote_file("http://edit.projecteq.net/weekly/peq_beta.zip", "updates_staged/peq_beta.zip", 1);
    print "[Install] Downloaded latest PEQ Database... Extracting...\n";
    unzip('updates_staged/peq_beta.zip', 'updates_staged/peq_db/');
    my $start_dir = "updates_staged/peq_db";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/updates_staged\/peq_db\///g;
        if ($file =~ /peqbeta|player_tables/i) {
            print "[Install] DB :: Installing :: " . $destination_file . "\n";
            get_mysql_result_from_file($file);
        }
    }

    #::: PEQ DB baseline version
    print get_mysql_result("DELETE FROM db_version");
    print get_mysql_result("INSERT INTO `db_version` (`version`) VALUES (9130);");
}

sub map_files_fetch_bulk {
    print "[Install] Fetching Latest Maps... (This could take a few minutes...)\n";
    get_remote_file("http://github.com/Akkadius/EQEmuMaps/archive/master.zip", "maps/maps.zip", 1);
    unzip('maps/maps.zip', 'maps/');
    my @files;
    my $start_dir = "maps/EQEmuMaps-master/";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/maps\/EQEmuMaps-master\///g;
        print "[Install] Installing :: " . $destination_file . "\n";
        copy_file($file, "maps/" . $destination_file);
    }
    print "[Install] Fetched Latest Maps\n";

    rmtree('maps/EQEmuMaps-master');
    unlink('maps/maps.zip');
}

sub map_files_fetch {
    print "[Install] Fetching Latest Maps --- \n";

    get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuMaps/master/!eqemu_maps_manifest.txt", "updates_staged/eqemu_maps_manifest.txt");

    #::: Get Data from manifest
    open(FILE, "updates_staged/eqemu_maps_manifest.txt");
    $i = 0;
    while (<FILE>) {
        chomp;
        $o                 = $_;
        @manifest_map_data = split(',', $o);
        if ($manifest_map_data[0] ne "") {
            $maps_manifest[$i] = [ $manifest_map_data[0], $manifest_map_data[1] ];
            $i++;
        }
    }

    #::: Download
    $fc                        = 0;
    for ($m                    = 0; $m <= $i; $m++) {
        my $file_existing      = $maps_manifest[$m][0];
        my $file_existing_size = (stat $file_existing)[7];
        if ($file_existing_size != $maps_manifest[$m][1]) {
            print "[Install] Updating: '" . $maps_manifest[$m][0] . "'\n";
            get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuMaps/master/" . $maps_manifest[$m][0], $maps_manifest[$m][0], 1);
            $fc++;
        }
    }

    if ($fc == 0) {
        print "[Install] No Map Updates found... \n\n";
    }
}

sub quest_files_fetch {
    if (!-e "updates_staged/projecteqquests-master/") {
        print "[Update] Fetching Latest Quests --- \n";
        get_remote_file("https://codeload.github.com/ProjectEQ/projecteqquests/zip/master", "updates_staged/projecteqquests-master.zip", 1);
        print "[Install] Fetched latest quests...\n";
        mkdir('updates_staged');
        unzip('updates_staged/projecteqquests-master.zip', 'updates_staged/');
    }

    $fc = 0;
    use File::Find;
    use File::Compare;

    my @files;
    my $start_dir = "updates_staged/projecteqquests-master/";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        if ($file =~ /\.pl|\.lua|\.ext/i) {
            $staged_file      = $file;
            $destination_file = $file;
            $destination_file =~ s/updates_staged\/projecteqquests-master\//quests\//g;

            if (!-e $destination_file) {
                copy_file($staged_file, $destination_file);
                print "[Install] Installing :: '" . $destination_file . "'\n";
                $fc++;
            }
            else {
                $directory_indexff = do_file_diff($destination_file, $staged_file);
                if ($directory_indexff ne "") {
                    $backup_dest = "updates_backups/" . $time_stamp . "/" . $destination_file;

                    print $directory_indexff . "\n";
                    print "[Update] File Different :: '" . $destination_file . "'\n";
                    print "[Update] Do you wish to update this Quest? '" . $destination_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
                    my $input = <STDIN>;
                    if ($input =~ /N/i) {}
                    else {
                        #::: Make a backup
                        copy_file($destination_file, $backup_dest);
                        #::: Copy staged to running
                        copy($staged_file, $destination_file);
                        print "[Install] Installing :: '" . $destination_file . "'\n\n";
                    }
                    $fc++;
                }
            }
        }
    }

    if ($fc == 0) {
        print "[Update] No Quest Updates found... \n\n";
    }
	
    rmtree("updates_staged/");
}

sub lua_modules_fetch {
    if (!-e "updates_staged/projecteqquests-master/") {
        print "[Update] Fetching Latest lua modules --- \n";
        get_remote_file("https://codeload.github.com/ProjectEQ/projecteqquests/zip/master", "updates_staged/projecteqquests-master.zip", 1);
        print "[Install] Fetched latest lua modules...\n";
        mkdir('updates_staged');
        unzip('updates_staged/projecteqquests-master.zip', 'updates_staged/');
    }

    $fc = 0;
    use File::Find;
    use File::Compare;

    mkdir('lua_modules');

    my @files;
    my $start_dir = "updates_staged/projecteqquests-master/lua_modules/";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        if ($file =~ /\.pl|\.lua|\.ext/i) {
            $staged_file      = $file;
            $destination_file = $file;
            $destination_file =~ s/updates_staged\/projecteqquests-master\/lua_modules\//lua_modules\//g;

            if (!-e $destination_file) {
                copy_file($staged_file, $destination_file);
                print "[Install] Installing :: '" . $destination_file . "'\n";
                $fc++;
            }
            else {
                $directory_indexff = do_file_diff($destination_file, $staged_file);
                if ($directory_indexff ne "") {
                    $backup_dest = "updates_backups/" . $time_stamp . "/" . $destination_file;
                    print $directory_indexff . "\n";
                    print "[Update] File Different :: '" . $destination_file . "'\n";
                    print "[Update] Do you wish to update this LUA Module? '" . $destination_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
                    my $input = <STDIN>;
                    if ($input =~ /N/i) {}
                    else {
                        #::: Make a backup
                        copy_file($destination_file, $backup_dest);
                        #::: Copy staged to running
                        copy($staged_file, $destination_file);
                        print "[Install] Installing :: '" . $destination_file . "'\n\n";
                    }
                    $fc++;
                }
            }
        }
    }

    if ($fc == 0) {
        print "[Update] No LUA Modules Updates found... \n\n";
    }
}

sub plugins_fetch {
    if (!-e "updates_staged/projecteqquests-master/") {
        print "[Update] Fetching Latest plugins --- \n";
        get_remote_file("https://codeload.github.com/ProjectEQ/projecteqquests/zip/master", "updates_staged/projecteqquests-master.zip", 1);
        print "[Install] Fetched latest plugins...\n";
        mkdir('updates_staged');
        unzip('updates_staged/projecteqquests-master.zip', 'updates_staged/');
    }

    $fc = 0;
    use File::Find;
    use File::Compare;

    mkdir('plugins');

    my @files;
    my $start_dir = "updates_staged/projecteqquests-master/plugins/";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        if ($file =~ /\.pl|\.lua|\.ext/i) {
            $staged_file      = $file;
            $destination_file = $file;
            $destination_file =~ s/updates_staged\/projecteqquests-master\///g;

            if (!-e $destination_file) {
                copy_file($staged_file, $destination_file);
                print "[Install] Installing :: '" . $destination_file . "'\n";
                $fc++;
            }
            else {
                $directory_indexff = do_file_diff($destination_file, $staged_file);
                if ($directory_indexff ne "") {
                    $backup_dest = "updates_backups/" . $time_stamp . "/" . $destination_file;
                    print $directory_indexff . "\n";
                    print "[Update] File Different :: '" . $destination_file . "'\n";
                    print "[Update] Do you wish to update this Plugin? '" . $destination_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
                    my $input = <STDIN>;
                    if ($input =~ /N/i) {}
                    else {
                        #::: Make a backup
                        copy_file($destination_file, $backup_dest);
                        #::: Copy staged to running
                        copy($staged_file, $destination_file);
                        print "[Install] Installing :: '" . $destination_file . "'\n\n";
                    }
                    $fc++;
                }
            }
        }
    }

    if ($fc == 0) {
        print "[Update] No Plugin Updates found... \n\n";
    }
}

sub do_file_diff {
    $file_1 = $_[0];
    $file_2 = $_[1];
    if ($OS eq "Windows") {
        eval "use Text::Diff";
        $directory_indexff = diff($file_1, $file_2, { STYLE => "Unified" });
        return $directory_indexff;
    }
    if ($OS eq "Linux") {
        # print 'diff -u "$file_1" "$file_2"' . "\n";
        return `diff -u "$file_1" "$file_2"`;
    }
}

sub unzip {
    $archive_to_unzip = $_[0];
    $dest_folder      = $_[1];

    if ($OS eq "Windows") {
        eval "use Archive::Zip qw( :ERROR_CODES :CONSTANTS )";
        my $zip = Archive::Zip->new();
        unless ($zip->read($archive_to_unzip) == AZ_OK) {
            die 'read error';
        }
        print "[Unzip] Extracting...\n";
        $zip->extractTree('', $dest_folder);
    }
    if ($OS eq "Linux") {
        print `unzip -o -q "$archive_to_unzip" -d "$dest_folder"`;
    }
}

sub are_file_sizes_different {
    $file_1    = $_[0];
    $file_2    = $_[1];
    my $file_1 = (stat $file_1)[7];
    my $file_2 = (stat $file_2)[7];
    # print $file_1 . " :: " . $file_2 . "\n";
    if ($file_1 != $file_2) {
        return 1;
    }
    return;
}

sub do_bots_db_schema_drop {
    #"drop_bots.sql" is run before reverting database back to 'normal'
    print "[Database] Fetching drop_bots.sql...\n";
    get_remote_file($eqemu_repository_request_url . "utils/sql/git/bots/drop_bots.sql", "db_update/drop_bots.sql");
    print get_mysql_result_from_file("db_update/drop_bots.sql");

    print "[Database] Removing bot database tables...\n";
    print get_mysql_result("DELETE FROM `rule_values` WHERE `rule_name` LIKE 'Bots:%';");

    if (get_mysql_result("SHOW TABLES LIKE 'commands'") ne "" && $db) {
        print get_mysql_result("DELETE FROM `commands` WHERE `command` LIKE 'bot';");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'command_settings'") ne "" && $db) {
        print get_mysql_result("DELETE FROM `command_settings` WHERE `command` LIKE 'bot';");
    }

    if (get_mysql_result("SHOW KEYS FROM `group_id` WHERE `Key_name` LIKE 'PRIMARY'") ne "" && $db) {
        print get_mysql_result("ALTER TABLE `group_id` DROP PRIMARY KEY;");
    }
    print get_mysql_result("ALTER TABLE `group_id` ADD PRIMARY KEY (`groupid`, `charid`, `ismerc`);");

    if (get_mysql_result("SHOW KEYS FROM `guild_members` WHERE `Key_name` LIKE 'PRIMARY'") ne "" && $db) {
        print get_mysql_result("ALTER TABLE `guild_members` DROP PRIMARY KEY;");
    }
    print get_mysql_result("ALTER TABLE `guild_members` ADD PRIMARY KEY (`char_id`);");

    print get_mysql_result("UPDATE `spawn2` SET `enabled` = 0 WHERE `id` IN (59297,59298);");

    if (get_mysql_result("SHOW COLUMNS FROM `db_version` LIKE 'bots_version'") ne "" && $db) {
        print get_mysql_result("UPDATE `db_version` SET `bots_version` = 0;");
    }
    print "[Database] Done...\n";
}

sub modify_db_for_bots {
    #Called after the db bots schema (2015_09_30_bots.sql) has been loaded
    print "[Database] Modifying database for bots...\n";
    print get_mysql_result("UPDATE `spawn2` SET `enabled` = 1 WHERE `id` IN (59297,59298);");

    if (get_mysql_result("SHOW KEYS FROM `guild_members` WHERE `Key_name` LIKE 'PRIMARY'") ne "" && $db) {
        print get_mysql_result("ALTER TABLE `guild_members` DROP PRIMARY KEY;");
    }

    if (get_mysql_result("SHOW KEYS FROM `group_id` WHERE `Key_name` LIKE 'PRIMARY'") ne "" && $db) {
        print get_mysql_result("ALTER TABLE `group_id` DROP PRIMARY KEY;");
    }
    print get_mysql_result("ALTER TABLE `group_id` ADD PRIMARY KEY USING BTREE(`groupid`, `charid`, `name`, `ismerc`);");

    if (get_mysql_result("SHOW TABLES LIKE 'command_settings'") ne "" && get_mysql_result("SELECT `command` FROM `command_settings` WHERE `command` LIKE 'bot'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `command_settings` VALUES ('bot', '0', '');");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'commands'") ne "" && get_mysql_result("SELECT `command` FROM `commands` WHERE `command` LIKE 'bot'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `commands` VALUES ('bot', '0');");
    }

    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotAAExpansion'") ne "" && $db) {
        print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:AAExpansion' WHERE `rule_name` LIKE 'Bots:BotAAExpansion';");
    }
    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:AAExpansion'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:AAExpansion', '8', 'The expansion through which bots will obtain AAs');");
    }

    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:CreateBotCount'") ne "" && $db) {
        print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:CreationLimit' WHERE `rule_name` LIKE 'Bots:CreateBotCount';");
    }
    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:CreationLimit'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:CreationLimit', '150', 'Number of bots that each account can create');");
    }

    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotFinishBuffing'") ne "" && $db) {
        print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:FinishBuffing' WHERE `rule_name` LIKE 'Bots:BotFinishBuffing';");
    }
    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:FinishBuffing'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:FinishBuffing', 'false', 'Allow for buffs to complete even if the bot caster is out of mana.  Only affects buffing out of combat.');");
    }

    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotGroupBuffing'") ne "" && $db) {
        print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:GroupBuffing' WHERE `rule_name` LIKE 'Bots:BotGroupBuffing';");
    }
    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:GroupBuffing'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:GroupBuffing', 'false', 'Bots will cast single target buffs as group buffs, default is false for single. Does not make single target buffs work for MGB.');");
    }

    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotManaRegen'") ne "" && $db) {
        print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:ManaRegen' WHERE `rule_name` LIKE 'Bots:BotManaRegen';");
    }
    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:ManaRegen'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:ManaRegen', '3.0', 'Adjust mana regen for bots, 1 is fast and higher numbers slow it down 3 is about the same as players.');");
    }

    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotQuest'") ne "" && $db) {
        print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:QuestableSpawnLimit' WHERE `rule_name` LIKE 'Bots:BotQuest';");
    }
    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:QuestableSpawnLimit'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:QuestableSpawnLimit', 'false', 'Optional quest method to manage bot spawn limits using the quest_globals name bot_spawn_limit, see: /bazaar/Aediles_Thrall.pl');");
    }

    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotSpellQuest'") ne "" && $db) {
        print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:QuestableSpells' WHERE `rule_name` LIKE 'Bots:BotSpellQuest';");
    }
    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:QuestableSpells'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:QuestableSpells', 'false', 'Anita Thrall\\\'s (Anita_Thrall.pl) Bot Spell Scriber quests.');");
    }

    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:SpawnBotCount'") ne "" && $db) {
        print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:SpawnLimit' WHERE `rule_name` LIKE 'Bots:SpawnBotCount';");
    }
    if (get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:SpawnLimit'") eq "" && $db) {
        print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:SpawnLimit', '71', 'Number of bots a character can have spawned at one time, You + 71 bots is a 12 group raid');");
    }

    convert_existing_bot_data();
}

sub convert_existing_bot_data {
    if (get_mysql_result("SHOW TABLES LIKE 'bots'") ne "" && $db) {
        print "[Database] Converting existing bot data...\n";
        print get_mysql_result("INSERT INTO `bot_data` (`bot_id`, `owner_id`, `spells_id`, `name`, `last_name`, `zone_id`, `gender`, `race`, `class`, `level`, `creation_day`, `last_spawn`, `time_spawned`, `size`, `face`, `hair_color`, `hair_style`, `beard`, `beard_color`, `eye_color_1`, `eye_color_2`, `drakkin_heritage`, `drakkin_tattoo`, `drakkin_details`, `ac`, `atk`, `hp`, `mana`, `str`, `sta`, `cha`, `dex`, `int`, `agi`, `wis`, `fire`, `cold`, `magic`, `poison`, `disease`, `corruption`) SELECT `BotID`, `BotOwnerCharacterID`, `BotSpellsID`, `Name`, `LastName`, `LastZoneId`, `Gender`, `Race`, `Class`, `BotLevel`, UNIX_TIMESTAMP(`BotCreateDate`), UNIX_TIMESTAMP(`LastSpawnDate`), `TotalPlayTime`, `Size`, `Face`, `LuclinHairColor`, `LuclinHairStyle`, `LuclinBeard`, `LuclinBeardColor`, `LuclinEyeColor`, `LuclinEyeColor2`, `DrakkinHeritage`, `DrakkinTattoo`, `DrakkinDetails`, `AC`, `ATK`, `HP`, `Mana`, `STR`, `STA`, `CHA`, `DEX`, `_INT`, `AGI`, `WIS`, `FR`, `CR`, `MR`, `PR`, `DR`, `Corrup` FROM `bots`;");

        print get_mysql_result("INSERT INTO `bot_inspect_messages` (`bot_id`, `inspect_message`) SELECT `BotID`, `BotInspectMessage` FROM `bots`;");

        print get_mysql_result("RENAME TABLE `bots` TO `bots_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'botstances'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_stances` (`bot_id`, `stance_id`) SELECT bs.`BotID`, bs.`StanceID` FROM `botstances` bs INNER JOIN `bot_data` bd ON bs.`BotID` = bd.`bot_id`;");

        print get_mysql_result("RENAME TABLE `botstances` TO `botstances_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'bottimers'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_timers` (`bot_id`, `timer_id`, `timer_value`) SELECT bt.`BotID`, bt.`TimerID`, bt.`Value` FROM `bottimers` bt INNER JOIN `bot_data` bd ON bt.`BotID` = bd.`bot_id`;");

        print get_mysql_result("RENAME TABLE `bottimers` TO `bottimers_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'botbuffs'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_buffs` (`buffs_index`, `bot_id`, `spell_id`, `caster_level`, `duration_formula`, `tics_remaining`, `poison_counters`, `disease_counters`, `curse_counters`, `corruption_counters`, `numhits`, `melee_rune`, `magic_rune`, `persistent`) SELECT bb.`BotBuffId`, bb.`BotId`, bb.`SpellId`, bb.`CasterLevel`, bb.`DurationFormula`, bb.`TicsRemaining`, bb.`PoisonCounters`, bb.`DiseaseCounters`, bb.`CurseCounters`, bb.`CorruptionCounters`, bb.`HitCount`, bb.`MeleeRune`, bb.`MagicRune`, bb.`Persistent` FROM `botbuffs` bb INNER JOIN `bot_data` bd ON bb.`BotId` = bd.`bot_id`;");

        if (get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'dot_rune'") ne "" && $db) {
            print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`dot_rune` = bbo.`dot_rune` WHERE bb.`bot_id` = bbo.`BotID`;");
        }

        if (get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'caston_x'") ne "" && $db) {
            print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`caston_x` = bbo.`caston_x` WHERE bb.`bot_id` = bbo.`BotID`;");
        }

        if (get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'caston_y'") ne "" && $db) {
            print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`caston_y` = bbo.`caston_y` WHERE bb.`bot_id` = bbo.`BotID`;");
        }

        if (get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'caston_z'") ne "" && $db) {
            print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`caston_z` = bbo.`caston_z` WHERE bb.`bot_id` = bbo.`BotID`;");
        }

        if (get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'ExtraDIChance'") ne "" && $db) {
            print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`extra_di_chance` = bbo.`ExtraDIChance` WHERE bb.`bot_id` = bbo.`BotID`;");
        }

        print get_mysql_result("RENAME TABLE `botbuffs` TO `botbuffs_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'botinventory'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_inventories` (`inventories_index`, `bot_id`, `slot_id`, `item_id`, `inst_charges`, `inst_color`, `inst_no_drop`, `augment_1`, `augment_2`, `augment_3`, `augment_4`, `augment_5`) SELECT bi.`BotInventoryID`, bi.`BotID`, bi.`SlotID`, bi.`ItemID`, bi.`charges`, bi.`color`, bi.`instnodrop`, bi.`augslot1`, bi.`augslot2`, bi.`augslot3`, bi.`augslot4`, bi.`augslot5` FROM `botinventory` bi INNER JOIN `bot_data` bd ON bi.`BotID` = bd.`bot_id`;");

        if (get_mysql_result("SHOW COLUMNS FROM `botinventory` LIKE 'augslot6'") ne "" && $db) {
            print get_mysql_result("UPDATE `bot_inventories` bi INNER JOIN `botinventory` bio ON bi.`inventories_index` = bio.`BotInventoryID` SET bi.`augment_6` = bio.`augslot6` 	WHERE bi.`bot_id` = bio.`BotID`;");
        }

        print get_mysql_result("RENAME TABLE `botinventory` TO `botinventory_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'botpets'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_pets` (`pets_index`, `pet_id`, `bot_id`, `name`, `mana`, `hp`) SELECT bp.`BotPetsId`, bp.`PetId`, bp.`BotId`, bp.`Name`, bp.`Mana`, bp.`HitPoints` FROM `botpets` bp INNER JOIN `bot_data` bd ON bp.`BotId` = bd.`bot_id`;");

        print get_mysql_result("RENAME TABLE `botpets` TO `botpets_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'botpetbuffs'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_pet_buffs` (`pet_buffs_index`, `pets_index`, `spell_id`, `caster_level`, `duration`) SELECT bpb.`BotPetBuffId`, bpb.`BotPetsId`, bpb.`SpellId`, bpb.`CasterLevel`, bpb.`Duration` FROM `botpetbuffs` bpb INNER JOIN `bot_pets` bp ON bpb.`BotPetsId` = bp.`pets_index`;");

        print get_mysql_result("RENAME TABLE `botpetbuffs` TO `botpetbuffs_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'botpetinventory'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_pet_inventories` (`pet_inventories_index`, `pets_index`, `item_id`) SELECT bpi.`BotPetInventoryId`, bpi.`BotPetsId`, bpi.`ItemId` FROM `botpetinventory` bpi INNER JOIN `bot_pets` bp ON bpi.`BotPetsId` = bp.`pets_index`;");

        print get_mysql_result("RENAME TABLE `botpetinventory` TO `botpetinventory_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'botgroup'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_groups` (`groups_index`, `group_leader_id`, `group_name`) SELECT bg.`BotGroupId`, bg.`BotGroupLeaderBotId`, bg.`BotGroupName` FROM  `botgroup` bg INNER JOIN `bot_data` bd ON bg.`BotGroupLeaderBotId` = bd.`bot_id`;");

        print get_mysql_result("RENAME TABLE `botgroup` TO `botgroup_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'botgroupmembers'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_group_members` (`group_members_index`, `groups_index`, `bot_id`) SELECT bgm.`BotGroupMemberId`, bgm.`BotGroupId`, bgm.`BotId` FROM `botgroupmembers` bgm INNER JOIN `bot_groups` bg ON bgm.`BotGroupId` = bg.`groups_index` INNER JOIN `bot_data` bd ON bgm.`BotId` = bd.`bot_id`;");

        print get_mysql_result("RENAME TABLE `botgroupmembers` TO `botgroupmembers_old`;");
    }

    if (get_mysql_result("SHOW TABLES LIKE 'botguildmembers'") ne "" && $db) {
        print get_mysql_result("INSERT INTO `bot_guild_members` (`bot_id`, `guild_id`, `rank`, `tribute_enable`, `total_tribute`, `last_tribute`, `banker`, `public_note`, `alt`) SELECT bgm.`char_id`, bgm.`guild_id`, bgm.`rank`, bgm.`tribute_enable`, bgm.`total_tribute`, bgm.`last_tribute`, bgm.`banker`, bgm.`public_note`, bgm.`alt` FROM `botguildmembers` bgm INNER JOIN `guilds` g ON bgm.`guild_id` = g.`id` INNER JOIN `bot_data` bd ON bgm.`char_id` = bd.`bot_id`;");

        print get_mysql_result("RENAME TABLE `botguildmembers` TO `botguildmembers_old`;");
    }
}

sub get_bots_db_version {
    #::: Check if bots_version column exists...
    if (get_mysql_result("SHOW COLUMNS FROM db_version LIKE 'bots_version'") eq "" && $db) {
        print get_mysql_result("ALTER TABLE db_version ADD bots_version int(11) DEFAULT '0' AFTER version;");
        print "[Database] Column 'bots_version' does not exists.... Adding to 'db_version' table...\n\n";
    }
    $bots_local_db_version = trim(get_mysql_result("SELECT bots_version FROM db_version LIMIT 1"));
    return $bots_local_db_version;
}

sub bots_db_management {

    my $world_path = "world";
    if (-e "bin/world") {
        $world_path = "bin/world";
    }

    #::: Get Binary DB version
    if ($OS eq "Windows") {
        @db_version = split(': ', `$world_path db_version`);
    }
    if ($OS eq "Linux") {
        @db_version = split(': ', `./$world_path db_version`);
    }

    #::: Main Binary Database version
    $binary_database_version = trim($db_version[2]);

    #::: If we have stale data from main db run
    if ($db_run_stage > 0 && $bots_db_management == 0) {
        clear_database_runs();
    }

    if ($binary_database_version == 0) {
        print "[Database] Your server binaries (world/zone) are not compiled for bots...\n\n";
        return;
    }

    #::: Set on flag for running bot updates...
    $bots_db_management = 1;

    $bots_local_db_version = get_bots_db_version();

    $local_database_version = $bots_local_db_version;

    run_database_check();
}

sub main_db_management {
    #::: If we have stale data from bots db run
    if ($db_run_stage > 0 && $bots_db_management == 1) {
        clear_database_runs();
    }

    #::: Main Binary Database version
    $binary_database_version = trim($db_version[1]);

    $bots_db_management = 0;
    run_database_check();
}

sub clear_database_runs {
    # print "DEBUG :: clear_database_runs\n\n";
    #::: Clear manifest data...
    %m_d = ();
    #::: Clear updates...
    @total_updates = ();
    #::: Clear stage
    $db_run_stage = 0;
}

#::: Responsible for Database Upgrade Routines
sub run_database_check {

    if (!$db) {
        print "No database present, check your eqemu_config.xml for proper MySQL/MariaDB configuration...\n";
        return;
    }

    if (!@total_updates) {
        #::: Pull down bots database manifest
        if ($bots_db_management == 1) {
            print "[Database] Retrieving latest bots database manifest...\n";
            get_remote_file($eqemu_repository_request_url . "utils/sql/git/bots/bots_db_update_manifest.txt", "db_update/db_update_manifest.txt");
        }
        #::: Pull down mainstream database manifest
        else {
            print "[Database] Retrieving latest database manifest...\n";
            get_remote_file($eqemu_repository_request_url . "utils/sql/db_update_manifest.txt", "db_update/db_update_manifest.txt");
        }
    }

    #::: Run 2 - Running pending updates...
    if (@total_updates || $db_run_stage == 1) {
        @total_updates = sort @total_updates;
        foreach my $val (@total_updates) {
            $file_name = trim($m_d{$val}[1]);
            print "[Database] Running Update: " . $val . " - " . $file_name . "\n";
            print get_mysql_result_from_file("db_update/$file_name");
            print get_mysql_result("UPDATE db_version SET version = $val WHERE version < $val");

            if ($bots_db_management == 1 && $val == 9000) {
                modify_db_for_bots();
            }

            if ($val == 9138) {
                fix_quest_factions();
            }
        }
        $db_run_stage = 2;
    }
    #::: Run 1 - Initial checking of needed updates...
    else {
        print "[Database] Reading manifest...\n";

        use Data::Dumper;
        open(FILE, "db_update/db_update_manifest.txt");
        while (<FILE>) {
            chomp;
            $o = $_;
            if ($o =~ /#/i) {
                next;
            }

            @manifest          = split('\|', $o);
            $m_d{$manifest[0]} = [ @manifest ];
        }
        #::: Setting Manifest stage...
        $db_run_stage = 1;
    }

    @total_updates = ();

    #::: This is where we set checkpoints for where a database might be so we don't check so far back in the manifest...
    if ($local_database_version >= 9000) {
        $revision_check = $local_database_version;
    }
    else {
        $revision_check = 1000;
        if (get_mysql_result("SHOW TABLES LIKE 'character_data'") ne "") {
            $revision_check = 8999;
        }
    }

    #::: Iterate through Manifest backwards from binary version down to local version...
    for ($i = $revision_check; $i <= $binary_database_version; $i++) {
        if (!defined($m_d{$i}[0])) {
            next;
        }

        $file_name   = trim($m_d{$i}[1]);
        $query_check = trim($m_d{$i}[2]);
        $match_type  = trim($m_d{$i}[3]);
        $match_text  = trim($m_d{$i}[4]);

        #::: Match type update
        if ($match_type eq "contains") {
            if (trim(get_mysql_result($query_check)) =~ /$match_text/i) {
                print "[Database] missing update: " . $i . " '" . $file_name . "' \n";
                fetch_missing_db_update($i, $file_name);
                push(@total_updates, $i);
            }
            else {
                print "[Database] has update (" . $i . ") '" . $file_name . "' \n";
            }
            print_match_debug();
            print_break();
        }
        if ($match_type eq "missing") {
            if (get_mysql_result($query_check) =~ /$match_text/i) {
                print "[Database] has update (" . $i . ") '" . $file_name . "' \n";
                next;
            }
            else {
                print "[Database] missing update: " . $i . " '" . $file_name . "' \n";
                fetch_missing_db_update($i, $file_name);
                push(@total_updates, $i);
            }
            print_match_debug();
            print_break();
        }
        if ($match_type eq "empty") {
            if (get_mysql_result($query_check) eq "") {
                print "[Database] missing update: " . $i . " '" . $file_name . "' \n";
                fetch_missing_db_update($i, $file_name);
                push(@total_updates, $i);
            }
            else {
                print "[Database] has update (" . $i . ") '" . $file_name . "' \n";
            }
            print_match_debug();
            print_break();
        }
        if ($match_type eq "not_empty") {
            if (get_mysql_result($query_check) ne "") {
                print "[Database] missing update: " . $i . " '" . $file_name . "' \n";
                fetch_missing_db_update($i, $file_name);
                push(@total_updates, $i);
            }
            else {
                print "[Database] has update (" . $i . ") '" . $file_name . "' \n";
            }
            print_match_debug();
            print_break();
        }
    }
    print "\n";

    if (scalar(@total_updates) == 0 && $db_run_stage == 2) {
        print "[Database] No updates need to be run...\n";
        if ($bots_db_management == 1) {
            print "[Database] Setting Database to Bots Binary Version (" . $binary_database_version . ") if not already...\n\n";
            get_mysql_result("UPDATE db_version SET bots_version = $binary_database_version ");
        }
        else {
            print "[Database] Setting Database to Binary Version (" . $binary_database_version . ") if not already...\n\n";
            get_mysql_result("UPDATE db_version SET version = $binary_database_version ");
        }

        clear_database_runs();
    }
}

sub fetch_missing_db_update {
    $db_update   = $_[0];
    $update_file = $_[1];
    if ($db_update >= 9000) {
        if ($bots_db_management == 1) {
            get_remote_file($eqemu_repository_request_url . "utils/sql/git/bots/required/" . $update_file, "db_update/" . $update_file . "");
        }
        else {
            get_remote_file($eqemu_repository_request_url . "utils/sql/git/required/" . $update_file, "db_update/" . $update_file . "");
        }
    }
    elsif ($db_update >= 5000 && $db_update <= 9000) {
        get_remote_file($eqemu_repository_request_url . "utils/sql/svn/" . $update_file, "db_update/" . $update_file . "");
    }
}

sub print_match_debug {
    if (!$debug) { return; }
    print "	Match Type: '" . $match_type . "'\n";
    print "	Match Text: '" . $match_text . "'\n";
    print "	Query Check: '" . $query_check . "'\n";
    print "	Result: '" . trim(get_mysql_result($query_check)) . "'\n";
}

sub print_break {
    if (!$debug) { return; }
    print "\n==============================================\n";
}

sub generate_random_password {
    my $passwordsize = shift;
    my @alphanumeric = ('a' .. 'z', 'A' .. 'Z', 0 .. 9);
    my $randpassword = join '',
        map $alphanumeric[rand @alphanumeric], 0 .. $passwordsize;

    return $randpassword;
}

sub quest_heading_convert {

    if (trim(get_mysql_result("SELECT value FROM variables WHERE varname = 'new_heading_conversion'")) eq "true") {
        print "Conversion script has already ran... doing this again would skew proper heading values in function calls...\n";
        exit;
    }

    %matches = (
        0 => [ "quest::spawn2", 6 ],
        1 => [ "eq.spawn2", 6 ],
        2 => [ "eq.unique_spawn", 6 ],
        3 => [ "quest::unique_spawn", 6 ],
        4 => [ "GMMove", 3 ],
        5 => [ "MovePCInstance", 5 ],
        6 => [ "MovePC", 4 ],
        7 => [ "moveto", 3 ],
    );

    $total_matches = 0;

    use Scalar::Util qw(looks_like_number);

    my @files;
    my $start_dir = "quests/.";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {

        #::: Skip non script files
        if ($file !~ /lua|pl/i) { next; }

        if ($file =~ /lua|pl/i) {
            $print_buffer = "";

            $changes_made = 0;

            #::: Open and read line by line
            open(FILE, $file);
            while (<FILE>) {
                chomp;
                $line = $_;

                #::: Loop through matches
                foreach my $key (sort (keys %matches)) {
                    $argument_position = $matches{$key}[1];
                    $match             = $matches{$key}[0];

                    if ($line =~ /$match/i) {
                        $line_temp = $line;
                        $line_temp =~ s/$match\(//g;
                        $line_temp =~ s/\(.*?\)//gs;
                        $line_temp =~ s/\);.*//;
                        $line_temp =~ s/\).*//;
                        $line_temp =~ s/\):.*//;
                        $line_temp =~ s/\);//g;

                        @line_data = split(",", $line_temp);

                        # use Data::Dumper;
                        # print Dumper(\@line_data);

                        $heading_value        = $line_data[$argument_position];
                        $heading_value_clean  = trim($heading_value);
                        $heading_value_raw    = $line_data[$argument_position];
                        $heading_value_before = $line_data[$argument_position - 1];

                        if (looks_like_number($heading_value) && $heading_value != 0 && ($heading_value * 2) <= 512) {
                            $heading_value_new = $heading_value * 2;

                            $heading_value =~ s/$heading_value_clean/$heading_value_new/g;

                            $heading_value_search  = quotemeta($heading_value_before . "," . $heading_value_raw);
                            $heading_value_replace = $heading_value_before . "," . $heading_value;

                            print $file . "\n";
                            print $line . "\n";
                            $line =~ s/$heading_value_search/$heading_value_replace/g;
                            print $line . "\n";
                            print "\n";

                            $changes_made = 1;
                        }
                        elsif ($heading_value == 0) {}                         #::: Do nothing
                        elsif ($heading_value =~ /GetHeading|heading|\$h/i) {} #::: Do nothing
                        else {
                            if ($file =~ /\.pl/i) {
                                if ($line_temp =~ /#/i) {
                                    $line .= " - needs_heading_validation";
                                }
                                else {
                                    $line .= " # needs_heading_validation";
                                }
                            }
                            elsif ($file =~ /\.lua/i) {
                                if ($line_temp =~ /--/i) {
                                    $line .= " - needs_heading_validation";
                                }
                                else {
                                    $line .= " -- needs_heading_validation";
                                }
                            }

                            $changes_made = 1;

                            print $line . "\n";
                        }

                        $total_matches++;
                    }
                }

                $print_buffer .= $line . "\n";
            }
            close(FILE);

            if ($changes_made == 1) {
                #::: Write changes
                open(NEW_FILE, '>', $file);
                print NEW_FILE $print_buffer;
                close NEW_FILE;
            }
        }
    }

    #::: Mark conversion as ran
    print get_mysql_result("INSERT INTO `variables` (varname, value, information, ts) VALUES ('new_heading_conversion', 'true', 'Script ran against quests folder to convert new heading values', NOW())");

    print "Total matches: " . $total_matches . "\n";
}


sub quest_faction_convert {

	if(trim(get_mysql_result("SELECT value FROM variables WHERE varname = 'new_faction_conversion'")) eq "true") {
	 	print "Conversion script has already ran... doing this again would skew proper faction values in function calls...\n";
	 	exit;
	 }

	%matches = (
		0 => [ "GetCharacterFactionLevel", 0],
		1 => [ "GetModCharacterFactionLevel", 0],
		2 => [ "SetFactionLevel2", 1],
		3 => [ "GetFactionLevel", 5 ],
		4 => [ "CheckNPCFactionAlly", 0 ],
		5 => [ ":Faction", 0 ],
	);

	$total_matches = 0;

	use Scalar::Util qw(looks_like_number);

	my @files;
	my $start_dir = "quests/.";
	find(
		sub {push @files, $File::Find::name unless -d;},
		$start_dir
	);
	for my $file (@files) {

		#::: Skip non script files
		if ($file !~ /lua|pl/i) {
			next;
		}

		if ($file =~ /lua|pl/i) {
			$print_buffer = "";
			$changes_made = 0;

			#::: Open and read line by line
			open(FILE, $file);
			while (<FILE>) {
				chomp;
				$line = $_;

				#::: Loop through matches
				foreach my $key (sort (keys %matches)) {
					$argument_position = $matches{$key}[1];
					$match             = $matches{$key}[0];

					if ($line =~ /$match\(/i || $line =~ /$match \(/i) {
						$line_temp =  $line;
						$line_temp =~ s/^.*$match\(//gi;
						$line_temp =~ s/^.*$match \(//gi;
						$line_temp =~ s/"//g;
						$line_temp =~ s/\);.*//;

						@line_data = split(",", $line_temp);

						$faction_value        = $line_data[$argument_position];
						$faction_value_clean  = trim($faction_value);

						if (looks_like_number($faction_value_clean)) {
							$new_faction = get_mysql_result("select clientid from client_server_faction_map where serverid = $faction_value_clean");
							chomp $new_faction;
							if ($new_faction == 0) {
								$new_faction = get_mysql_result("select new_faction from custom_faction_mappings where old_faction = $faction_value_clean");
								chomp $new_faction;
							}
							if ($new_faction > 0) {
								print "BEFORE: " . $line . "\n";
								$line =~ s/$faction_value_clean/$new_faction/g;
								print "AFTER: " . $line . "\n";
								$changes_made = 1;	
							}
							else {
								print "Unknown Faction: '$match' FACTION VALUE: '" . $faction_value_clean . "'\n";
							}
						}

						$total_matches++;
					}
				}

				$print_buffer .= $line . "\n";
			}
			close(FILE);

			#::: Write changes
			if ($changes_made == 1) {
				open(NEW_FILE, '>', $file);
			 	print NEW_FILE $print_buffer;
			 	close NEW_FILE;
			}
		}
	}

	#::: Mark conversion as ran
	print get_mysql_result("INSERT INTO `variables` (varname, value, information, ts) VALUES ('new_faction_conversion', 'true', 'Script ran against quests folder to convert new faction values', NOW())");

	print "Total matches: " . $total_matches . "\n";
}

sub fix_quest_factions {
	# Backup the quests
	mkdir('backups');
	my @files;
	my $start_dir = "quests/";
	find(
		sub { push @files, $File::Find::name unless -d; },
		$start_dir
	);
	for my $file (@files) {
		$destination_file = $file;
		my $date = strftime "%m-%d-%Y", localtime;
		$destination_file =~ s/quests/quests-$date/;
		print "Backing up :: " . $destination_file . "\n";
#		unlink($destination_file);
		copy_file($file, 'backups/' . $destination_file);
	}

	# Fix the factions
	quest_faction_convert();	
}
