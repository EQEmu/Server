#!/usr/bin/perl

###########################################################
#::: General EQEmu Server Administration Script
#::: Purpose - Handles:
#::: 	Automatic database versioning (bots and normal DB)
#::: 	Updating server assets (binary, opcodes, maps, configuration files)
#::: Original Author: Akkadius
#::: 	Contributors: Uleat, Ali
#::: Purpose: To upgrade databases with ease and maintain versioning
###########################################################

use Config;
use File::Copy qw(copy);
use POSIX qw(strftime);
use File::Path;
use File::Find;
use Time::HiRes qw(usleep);

#############################################
# variables
#############################################
my $install_repository_request_url = "https://raw.githubusercontent.com/Akkadius/eqemu-install-v2/master/";
my $eqemu_repository_request_url   = "https://raw.githubusercontent.com/EQEmu/Server/master/";
my $opcodes_path                   = "";
my $patches_path                   = "";
my $time_stamp                     = strftime('%m-%d-%Y', gmtime());
my $bin_dir                        = "";

#############################################
# os
#############################################
my $OS        = "";
my $os_flavor = "";
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

if (-e "../eqemu_config.json") {
    print "[Info] Config is up one level, let's set current directory up one level...\n";
    chdir("../");
}

#############################################
# internet check
#############################################
my $has_internet_connection = check_internet_connection();
if (-e "skip_internet_connection_check.txt") {
    $has_internet_connection = 1;
}

#############################################
# skip self update
#############################################
my $skip_self_update_check = 0;
if (-e "eqemu_server_skip_update.txt" || defined($ENV{'EQEMU_SERVER_SKIP_UPDATE'})) {
    print "[Info] Skipping self check\n";
    $skip_self_update_check = 1;
}

#############################################
# skip maps update
#############################################
my $skip_self_maps_update_check = 0;
if (-e "eqemu_server_skip_maps_update.txt" || defined($ENV{'EQEMU_SERVER_SKIP_MAPS_UPDATE'})) {
    print "[Info] Skipping maps update\n";
    $skip_self_maps_update_check = 1;
}

#############################################
# bin dir
#############################################
if (-d "bin") {
    $bin_dir = "bin/";
}

#############################################
# run routines
#############################################
get_windows_wget();
do_self_update_check_routine() if !$skip_self_update_check;
get_perl_version();
if (-e "eqemu_config.json") {
    read_eqemu_config_json();
}

get_mysql_path();

#::: Remove old eqemu_update.pl
if (-e "eqemu_update.pl") {
    unlink("eqemu_update.pl");
}

print "[Info] For EQEmu Server management utilities - run eqemu_server.pl\n" if $ARGV[0] eq "ran_from_world";

my $skip_checks = 0;
if ($ARGV[0] && ($ARGV[0] eq "new_server")) {
    $skip_checks = 1;
}

show_menu_prompt();

sub urlencode
{
    my ($rv) = @_;
    $rv =~ s/([^A-Za-z0-9])/sprintf("%%%2.2X", ord($1)) /ge;
    return $rv;
}

sub urldecode
{
    my ($rv) = @_;
    $rv =~ s/\+/ /g;
    $rv =~ s/%(..)/pack("c", hex($1)) /ge;
    return $rv;
}

sub analytics_insertion
{
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

sub show_install_summary_info
{
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
        print " - server_start.sh           	Starts EQEmu server (Quiet) with 30 dynamic zones, UCS & Queryserv, dynamic zones\n";
        print " - server_start_with_login.sh	Starts EQEmu server (Quiet) with 30 dynamic zones, UCS & Queryserv, dynamic zones\n";
        print " - server_start_dev.sh       	Starts EQEmu server with 10 dynamic zones, UCS & Queryserv, dynamic zones all verbose\n";
        print " - server_stop.sh            	Stops EQEmu Server (No warning)\n";
        print " - server_status.sh          	Prints the status of the EQEmu Server processes\n";
    }

    print "[Configure] eqemu_config.json 		Edit to change server settings and name\n";

    analytics_insertion("install_complete", "null");
}

sub new_server
{
    $build_options = $_[0];
    $file_count    = 0;
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

    get_installation_variables();

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

            $check_connection =
                `mysql -u $database_user -p$database_password -N -B -e "SHOW PROCESSLIST" > mysqlcheck.txt`;
            $mysql_pass = 0;
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

            if ($database_name eq "" && !-e "install_variables.txt" && !-e "../install_variables.txt") {
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

            # This shouldn't be necessary, as we call do_linux_login_server_setup as the last step in do_installer_routines()
            # if ($OS eq "Linux") {
            #     build_linux_source("login");
            # }

            do_installer_routines($build_options);

            if ($OS eq "Linux") {
                print `chmod 755 *.sh`;
            }

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

sub build_linux_source
{
    $build_options = $_[0];

    $cmake_options          = "";
    $source_folder_post_fix = "";

    $current_directory = `pwd`;
    @directories       = split('/', $current_directory);
    foreach my $val (@directories) {
        if (trim($val) ne "") {
            $last_directory = trim($val);
        }
    }
    my $eqemu_server_directory = "/home/eqemu";
    # source between bots and not is the same, just different build results, so use the same source folder, different build folders
    my $source_dir = $eqemu_server_directory . '/' . $last_directory . '_source';
    my $build_dir  = $eqemu_server_directory . '/' . $last_directory . '_build' . $source_folder_post_fix;

    $current_directory = trim($current_directory);

    mkdir($source_dir) if (!-e $source_dir);

    # print 'server_dir: ' . $eqemu_server_directory . "\n";
    # print 'source_dir: ' . $source_dir . "\n";
    # print 'current_dir: \'' . $current_directory . "'\n";

    chdir($source_dir);

    if (!-d "$source_dir/.git") {
        print `git clone --recurse-submodules https://github.com/EQEmu/Server.git $source_dir`;
    }
    else {
        print `git pull --recurse-submodules`;
    }

    mkdir($build_dir) if (!-e $build_dir);
    chdir($build_dir);

    print "Generating CMake build files...\n";
    if ($os_flavor eq "fedora_core") {
        print `cmake $cmake_options -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_LUA=ON -DLUA_INCLUDE_DIR=/usr/include/lua-5.1/ -G "Unix Makefiles" $source_dir`;
    }
    else {
        print `cmake $cmake_options -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_LUA=ON -G "Unix Makefiles" $source_dir`;
    }
    print "Building EQEmu Server code. This will take a while.";

    #::: Build
    print `make`;

    chdir($current_directory);

    print `ln -s -f $build_dir/bin/eqlaunch .`;
    print `ln -s -f $build_dir/bin/export_client_files .`;
    print `ln -s -f $build_dir/bin/import_client_files .`;
    print `ln -s -f $build_dir/bin/libcommon.a .`;
    print `ln -s -f $build_dir/bin/libluabind.a .`;
    print `ln -s -f $build_dir/bin/queryserv .`;
    print `ln -s -f $build_dir/bin/shared_memory .`;
    print `ln -s -f $build_dir/bin/ucs .`;
    print `ln -s -f $build_dir/bin/world .`;
    print `ln -s -f $build_dir/bin/zone .`;
    print `ln -s -f $build_dir/bin/loginserver .`;
}

sub do_installer_routines
{
    $build_options = $_[0];
    print "[Install] EQEmu Server Installer... LOADING... PLEASE WAIT...\n";

    #::: Make some local server directories...
    mkdir('logs');
    mkdir('updates_staged');
    mkdir('shared');
    mkdir('bin');

    $bin_dir = "bin/";

    do_install_config_json();
    read_eqemu_config_json();
    get_installation_variables();

    $db_name = "peq";
    if ($installation_variables{"mysql_eqemu_db_name"}) {
        $db_name = $installation_variables{"mysql_eqemu_db_name"};
    }

    #::: Download assets
    if ($OS eq "Windows") {
        fetch_latest_windows_appveyor();
    }

    map_files_fetch_bulk() if !$skip_self_maps_update_check;
    opcodes_fetch();
    plugins_fetch();
    quest_files_fetch();
    lua_modules_fetch();
    fetch_utility_scripts();

    #::: Database Routines
    $root_user     = $user;
    $root_password = $pass;
    print "[Database] Creating Database '" . $db_name . "'\n";
    if (defined($ENV{'MYSQL_ROOT_PASSWORD'})) {
        # In the case that the user doesn't have privileges to create databases, support passing in the root password during setup
        print "[Database] Using 'root' for database management.\n";
        $root_user     = "root";
        $root_password = $ENV{'MYSQL_ROOT_PASSWORD'};
    }
    print `"$path" --host $host --user $root_user --password="$root_password" -N -B -e "DROP DATABASE IF EXISTS $db_name;"`;
    print `"$path" --host $host --user $root_user --password="$root_password" -N -B -e "CREATE DATABASE $db_name"`;
    if (defined($ENV{'MYSQL_ROOT_PASSWORD'})) {
        # If we used root, make sure $user has permissions on db
        print "[Database] Assigning ALL PRIVILEGES to $user on $db_name.\n";
        print `"$path" --host $host --user $root_user --password="$root_password" -N -B -e "GRANT ALL PRIVILEGES ON $db_name.* TO '$user.%'"`;
        print `"$path" --host $host --user $root_user --password="$root_password" -N -B -e "FLUSH PRIVILEGES"`;
    }

    #::: Download PEQ latest
    fetch_peq_db_full();
    print "[Database] Fetching and Applying Latest Database Updates...\n";

    if ($OS eq "Windows") {
        check_windows_firewall_rules();
        do_windows_login_server_setup();
    }
    if ($OS eq "Linux") {
        do_linux_login_server_setup($build_options);
    }
}

sub check_for_input
{
    print "[Input] " . $_[0];
    $input = <STDIN>;
    chomp $input;
}

sub check_internet_connection
{
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

sub get_perl_version
{
    #::: Check Perl version
    $perl_version = $^V;
    $perl_version =~ s/v//g;
    print "[Update] Perl Version is " . $perl_version . "\n" if $debug;
    if ($perl_version > 5.12) {
        no warnings 'uninitialized';
    }
    no warnings;
}

sub get_windows_wget
{
    if (!-e "bin/wget.exe" && $OS eq "Windows") {
        if (!-d "bin") {
            mkdir("bin");
        }
        eval "use LWP::Simple qw(getstore);";
        getstore("https://raw.githubusercontent.com/Akkadius/eqemu-install-v2/master/windows/wget.exe", "bin\\wget.exe");
    }
}

sub do_self_update_check_routine
{

    #::: Check for internet connection before updating
    if (!$has_internet_connection) {
        print "[Update] Cannot check self-update without internet connection...\n";
        return;
    }

    #::: Check for script changes :: eqemu_server.pl
    get_remote_file($eqemu_repository_request_url . "utils/scripts/eqemu_server.pl",
        "updates_staged/eqemu_server.pl",
        0,
        1,
        1
    );

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
                    print "[Install] Installing [" . $destination_file . "]\n";
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

sub get_installation_variables
{
    # Read installation variables from the ENV if set, but override them with install_variables.txt
    if ($ENV{"MYSQL_HOST"}) { $installation_variables{"mysql_host"}               = $ENV{"MYSQL_HOST"}; }
    if ($ENV{"MYSQL_DATABASE"}) { $installation_variables{"mysql_eqemu_db_name"}  = $ENV{"MYSQL_DATABASE"}; }
    if ($ENV{"MYSQL_USER"}) { $installation_variables{"mysql_eqemu_user"}         = $ENV{"MYSQL_USER"} }
    if ($ENV{"MYSQL_PASSWORD"}) { $installation_variables{"mysql_eqemu_password"} = $ENV{"MYSQL_PASSWORD"} }

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

sub do_install_config_json
{
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

    if ($installation_variables{"mysql_host"}) {
        $host = $installation_variables{"mysql_host"};
    }
    else {
        $host = "127.0.0.1";
    }

    $config->{"server"}{"database"}{"host"}       = $host;
    $config->{"server"}{"database"}{"username"}   = $installation_variables{"mysql_eqemu_user"};
    $config->{"server"}{"database"}{"password"}   = $installation_variables{"mysql_eqemu_password"};
    $config->{"server"}{"database"}{"db"}         = $db_name;
    $config->{"server"}{"qsdatabase"}{"host"}     = $host;
    $config->{"server"}{"qsdatabase"}{"username"} = $installation_variables{"mysql_eqemu_user"};
    $config->{"server"}{"qsdatabase"}{"password"} = $installation_variables{"mysql_eqemu_password"};
    $config->{"server"}{"qsdatabase"}{"db"}       = $db_name;

    $json->canonical(1);

    open(my $fh, '>', 'eqemu_config.json');
    print $fh $json->pretty->utf8->encode($config);
    close $fh;

    unlink("eqemu_config_template.json");
}

sub do_install_config_login_json
{
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

    if ($installation_variables{"mysql_host"}) {
        $host = $installation_variables{"mysql_host"};
    }
    else {
        $host = "127.0.0.1";
    }

    $config->{"database"}{"host"}                         = $host;
    $config->{"database"}{"user"}                         = $installation_variables{"mysql_eqemu_user"};
    $config->{"database"}{"password"}                     = $installation_variables{"mysql_eqemu_password"};
    $config->{"database"}{"db"}                           = $db_name;
    $config->{"client_configuration"}{"titanium_opcodes"} = $opcodes_path . "login_opcodes.conf";
    $config->{"client_configuration"}{"sod_opcodes"}      = $opcodes_path . "login_opcodes_sod.conf";

    $json->canonical(1);

    open(my $fh, '>', 'login.json');
    print $fh $json->pretty->utf8->encode($config);
    close $fh;

    unlink("login_template.json");
}

sub fetch_utility_scripts
{
    if ($OS eq "Windows") {

        opendir(DIR, "bin/");
        my @files = grep(/\.exe$/, readdir(DIR));
        closedir(DIR);

        foreach my $file (@files) {
            my $full_file = "bin/" . $file;

            if ($file =~ /test|launch/i) {
                next;
            }

            print "Creating Symbolic Link for [$file] from [$full_file]\n";
            system("del start_$file >nul 2>&1");
            system("powershell.exe \"New-Item -ItemType SymbolicLink -Name 'start_$file' -Value '$full_file'\" >nul 2>&1");
        }

        get_remote_file($install_repository_request_url . "windows/t_database_backup.bat", "t_database_backup.bat");
        get_remote_file($install_repository_request_url . "windows/t_start_server.bat", "t_start_server.bat");
        get_remote_file($install_repository_request_url . "windows/t_server_update_binaries_no_bots.bat",
            "t_server_update_binaries_no_bots.bat");
        get_remote_file($install_repository_request_url . "windows/t_start_server_with_login_server.bat",
            "t_start_server_with_login_server.bat");
        get_remote_file($install_repository_request_url . "windows/t_stop_server.bat", "t_stop_server.bat");
        get_remote_file($install_repository_request_url . "windows/t_server_crash_report.pl", "t_server_crash_report.pl");
        get_remote_file($install_repository_request_url . "windows/win_server_launcher.pl", "win_server_launcher.pl");
        get_remote_file($install_repository_request_url . "windows/t_start_server_with_login_server.bat",
            "t_start_server_with_login_server.bat");
        get_remote_file(
            $install_repository_request_url . "windows/t_set_gm_account.bat",
            "t_set_gm_account.bat"
        );
        get_remote_file(
            $install_repository_request_url . "windows/windows_server_readme.html",
            "windows_server_readme.html"
        );
    }
    else {
        get_remote_file($install_repository_request_url . "linux/server_launcher.pl", "server_launcher.pl");
        get_remote_file($install_repository_request_url . "linux/server_start.sh", "server_start.sh");
        get_remote_file($install_repository_request_url . "linux/server_start_dev.sh", "server_start_dev.sh");
        get_remote_file($install_repository_request_url . "linux/server_status.sh", "server_status.sh");
        get_remote_file($install_repository_request_url . "linux/server_stop.sh", "server_stop.sh");
    }
}

sub setup_bots
{
    my $command = get_world_command();
    print `$command bots:bootstrap`;

    print "Bots should be setup, run your server and the bot command should be available in-game (type '^help')\n";
}

sub show_menu_prompt
{

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
            print " \n";
            print " [drop_bots_db_schema]		Removes bot database schema\n";

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
            fetch_latest_windows_appveyor();
            $dc = 1;
        }
        elsif ($input eq "windows_server_latest") {
            fetch_latest_windows_appveyor();
            $dc = 1;
        }
        elsif ($input eq "utility_scripts") {
            fetch_utility_scripts();
            $dc = 1;
        }
        elsif ($input eq "check_db_updates") {
            db_update_check();
            $dc = 1;
        }
        elsif ($input eq "check_bot_db_updates") {
            db_update_check();
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

sub print_main_menu
{
    print "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
    print ">>> EQEmu Server Main Menu >>>>>>>>>>>>\n";
    print ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n";
    print " [database]              Enter database management menu \n";
    print " [assets]                Manage server assets \n";
    print " [new_server]            New folder EQEmu/PEQ install - Assumes MySQL/Perl installed \n";
    print " [setup_bots]            Enables bots on server - builds code and database requirements \n";
    print "\n";
    print " exit \n";
    print "\n";
    print "Enter a command #> ";
}

sub get_mysql_path
{
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
        if ($path eq "") {
            my @files;
            my $start_dir = trim(`echo %programfiles%`);
            find(
                sub {
                    if ($#files > 0) {
                        return;
                    }
                    push @files, $File::Find::name unless $File::Find::name!~/mysql.exe/i;
                },
                $start_dir
            );
            for my $file (@files) {
                if ($file=~/mysql.exe/i) {
                    $path = $file;
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

sub get_world_path
{
    if (-e "world") {
        return "world";
    }
    elsif (-e "world.exe") {
        return "world.exe";
    }
    elsif (-e "bin/world") {
        return "bin/world";
    }
    elsif (-e "bin/world.exe") {
        return "bin/world.exe";
    }
}

sub get_world_command
{
    my $command    = "";
    my $world_path = get_world_path();
    if ($OS eq "Windows") {
        $command = "\"$world_path\"";
    }
    if ($OS eq "Linux") {
        $command = "./$world_path";
    }

    return $command;
}

sub database_dump
{
    print "[Database] Performing database backup....\n";
    my $command = get_world_command();
    print `$command database:dump --all`;
}

sub db_update_check
{
    my $command = get_world_command();
    print `$command database:updates`;
}

sub database_dump_player_tables
{
    print "[Database] Performing database backup of player tables....\n";
    my $command = get_world_command();
    print `$command database:dump --player-tables`;
}

sub database_dump_compress
{
    print "[Database] Performing database backup....\n";
    my $command = get_world_command();
    print `$command database:dump --all --compress`;
}

sub script_exit
{
    #::: Cleanup staged folder...
    rmtree("updates_staged/");
    exit;
}

#::: Returns Tab Delimited MySQL Result from Command Line
sub get_mysql_result
{
    my $run_query = $_[0];
    if (!$db) { return; }
    if ($OS eq "Windows") { return `"$path" --host $host --user $user --password="$pass" $db -N -B -e "$run_query"`; }
    if ($OS eq "Linux") {
        $run_query =~ s/`//g;
        return `$path --user="$user" --host $host --password="$pass" $db -N -B -e "$run_query"`;
    }
}

sub get_mysql_result_from_file
{
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
sub get_remote_file
{
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
                print "[Copy] folder doesn't exist, creating [" . $build_path . "]\n";
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
    if ($OS eq "Linux") {
        $wget = `wget -N --no-cache --cache=no --no-check-certificate --quiet -O $destination_file $request_url`;
    }
    elsif ($OS eq "Windows") {
        $wget = `bin\\wget.exe -N --no-cache --cache=no --no-check-certificate --quiet -O $destination_file $request_url`;
    }
    print "[Download] Saved [" . $destination_file . "] from [" . $request_url . "]\n" if !$silent_download;
    if ($wget =~ /unable to resolve/i) {
        print "Error, no connection or failed request...\n\n";
        #die;
    }

}

#::: Trim Whitespaces
sub trim
{
    my $string = $_[0];
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}

sub read_eqemu_config_json
{
    use JSON;
    my $json = new JSON();

    my $content;
    open(my $fh, '<', "eqemu_config.json") or die "cannot open file $filename"; {
    local $/;
    $content = <$fh>;
}
    close($fh);

    $config = $json->decode($content);

    $db           = $config->{"server"}{"database"}{"db"};
    $host         = $config->{"server"}{"database"}{"host"};
    $user         = $config->{"server"}{"database"}{"username"};
    $pass         = $config->{"server"}{"database"}{"password"};
    $opcodes_path = $config->{"server"}{"directories"}{"opcodes"};
    $patches_path = $config->{"server"}{"directories"}{"patches"};
}

#::: Fetch Latest Opcodes
sub opcodes_fetch
{
    print "[Update] Pulling down latest opcodes...\n";
    my %opcodes = (
        1 => [ "opcodes", $eqemu_repository_request_url . "utils/patches/opcodes.conf" ],
        2 => [ "mail_opcodes", $eqemu_repository_request_url . "utils/patches/mail_opcodes.conf" ],
        3 => [ "Titanium", $eqemu_repository_request_url . "utils/patches/patch_Titanium.conf" ],
        4 => [ "Secrets of Faydwer", $eqemu_repository_request_url . "utils/patches/patch_SoF.conf" ],
        5 => [ "Seeds of Destruction", $eqemu_repository_request_url . "utils/patches/patch_SoD.conf" ],
        6 => [ "Underfoot", $eqemu_repository_request_url . "utils/patches/patch_UF.conf" ],
        7 => [ "Rain of Fear", $eqemu_repository_request_url . "utils/patches/patch_RoF.conf" ],
        8 => [ "Rain of Fear 2", $eqemu_repository_request_url . "utils/patches/patch_RoF2.conf" ],
    );
    my $loop = 1;
    while ($opcodes{$loop}[0]) {
        #::: Split the request_url by the patches folder to get the file name from request_url
        my @real_file = split("patches/", $opcodes{$loop}[1]);
        my $find      = 0;
        while ($real_file[$find]) {
            $file_name = $real_file[$find];
            $find++;
        }

        my $file_path = $opcodes_path;
        if ($file_name =~ /patch_/i) {
            $file_path = $patches_path;
        }

        get_remote_file($opcodes{$loop}[1], $file_path . $file_name);
        $loop++;
    }
    print "[Update] Done...\n";
}

sub copy_file
{
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

sub fetch_latest_windows_appveyor
{
    print "[Update] Fetching Latest Windows Binaries (unstable) from Appveyor... \n";
    get_remote_file("https://github.com/eqemu/server/releases/latest/download/eqemu-server-windows-x64.zip",
        "updates_staged/eqemu-server-windows-x64.zip",
        1
    );

    print "[Update] Fetched Latest Windows Binaries (unstable) from Appveyor... \n";
    print "[Update] Extracting... --- \n";
    unzip('updates_staged/eqemu-server-windows-x64.zip', 'updates_staged/binaries/');
    my @files;
    my $start_dir = "updates_staged/binaries";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        my $destination_file = $file;
        $destination_file =~ s/updates_staged\/binaries\///g;
        print "[Update] Installing [" . $bin_dir . $destination_file . "]\n";
        copy_file($file, $bin_dir . $destination_file);
    }
    print "[Update] Done\n";

    rmtree('updates_staged');
}

sub do_windows_login_server_setup
{
    print "[Install] Pulling down Loginserver database tables...\n";
    get_remote_file(
        $eqemu_repository_request_url . "loginserver/login_util/login_schema.sql",
        "db_update/login_schema.sql"
    );

    get_remote_file(
        $eqemu_repository_request_url . "loginserver/login_util/login_opcodes_sod.conf",
        $opcodes_path . "login_opcodes_sod.conf"
    );

    get_remote_file(
        $eqemu_repository_request_url . "loginserver/login_util/login_opcodes.conf",
        $opcodes_path . "login_opcodes.conf"
    );

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

sub do_linux_login_server_setup
{
    build_linux_source($_[0]);

    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/updates_staged\/login_server\///g;
        print "[Install] Installing [" . $destination_file . "]\n";
        copy_file($file, $destination_file);
    }
    print "\n Done... \n";

    print "[Install] Pulling down Loginserver database tables...\n";
    get_remote_file($eqemu_repository_request_url . "loginserver/login_util/login_schema.sql",
        "db_update/login_schema.sql");
    print "[Install] Installing Loginserver tables...\n";
    print get_mysql_result_from_file("db_update/login_schema.sql");
    print "[Install] Done...\n\n";

    print "[Install] Pulling and initializing Loginserver configuration files...\n";
    do_install_config_login_json();
    print "[Install] Done...\n";

    rmtree('updates_staged');
    rmtree('db_update');

    get_remote_file($install_repository_request_url . "linux/login_opcodes.conf", $opcodes_path . "login_opcodes.conf");
    get_remote_file($install_repository_request_url . "linux/login_opcodes_sod.conf",
        $opcodes_path . "login_opcodes_sod.conf");
    get_remote_file($install_repository_request_url . "linux/server_start_with_login.sh", "server_start_with_login.sh");
    system("chmod 755 *.sh");

    print "[Install] Press any key to continue...\n";

    <>; #Read from STDIN

}

sub add_login_server_firewall_rules
{
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
                    print "[Install] Found existing rule [" . trim($val) . "]\n";
                }
                if ($val =~ /EQEmu Loginserver/i && $val =~ /SOD/i) {
                    $has_loginserver_rules_sod = 1;
                    print "[Install] Found existing rule [" . trim($val) . "]\n";
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

sub check_windows_firewall_rules
{
    $output          = `netsh advfirewall firewall show rule name=all`;
    @output_buffer   = split("\n", $output);
    $has_world_rules = 0;
    $has_zone_rules  = 0;
    foreach my $val (@output_buffer) {
        if ($val =~ /Rule Name/i) {
            $val =~ s/Rule Name://g;
            if ($val =~ /EQEmu World/i) {
                $has_world_rules = 1;
                print "[Install] Found existing rule [" . trim($val) . "]\n";
            }
            if ($val =~ /EQEmu Zone/i) {
                $has_zone_rules = 1;
                print "[Install] Found existing rule [" . trim($val) . "]\n";
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

sub fetch_peq_db_full
{
    print "[Install] Downloading latest PEQ Database... Please wait...\n";
    get_remote_file("http://db.projecteq.net/api/v1/dump/latest", "updates_staged/peq-latest.zip", 1);
    print "[Install] Downloaded latest PEQ Database... Extracting...\n";
    unzip('updates_staged/peq-latest.zip', 'updates_staged/peq_db/');
    my $start_dir = "updates_staged/peq_db/peq-dump";
    find(
        sub { push @files, $File::Find::name unless -d; },
        $start_dir
    );
    for my $file (@files) {
        $destination_file = $file;
        $destination_file =~ s/updates_staged\/peq_db\/peq-dump\///g;
        if ($file =~ /create_tables_content|create_tables_login|create_tables_player|create_tables_queryserv|create_tables_state|create_tables_system/i) {
            print "[Install] Database sourcing [" . $destination_file . "]\n";
            get_mysql_result_from_file($file);
        }
    }
}

sub map_files_fetch_bulk
{
    print "[Install] Fetching Latest Maps... (This could take a few minutes...)\n";
    get_remote_file("https://github.com/Akkadius/eqemu-maps/releases/latest/download/maps.zip", "maps/maps.zip", 1);
    unzip('maps/maps.zip', 'maps/');
    print "[Install] Fetched Latest Maps\n";
    unlink('maps/maps.zip');
}

sub quest_files_fetch
{
    if (!-e "updates_staged/projecteqquests-master/") {
        print "[Update] Fetching Latest Quests --- \n";
        get_remote_file("https://codeload.github.com/ProjectEQ/projecteqquests/zip/master",
            "updates_staged/projecteqquests-master.zip",
            1);
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
                print "[Install] Installing [" . $destination_file . "]\n";
                $fc++;
            }
            else {
                $directory_indexff = do_file_diff($destination_file, $staged_file);
                if ($directory_indexff ne "") {
                    $backup_dest = "updates_backups/" . $time_stamp . "/" . $destination_file;

                    print $directory_indexff . "\n";
                    print "[Update] File Different [" . $destination_file . "]\n";
                    print "[Update] Do you wish to update this Quest? '" . $destination_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
                    my $input = <STDIN>;
                    if ($input =~ /N/i) {}
                    else {
                        #::: Make a backup
                        copy_file($destination_file, $backup_dest);
                        #::: Copy staged to running
                        copy($staged_file, $destination_file);
                        print "[Install] Installing [" . $destination_file . "]\n\n";
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

sub lua_modules_fetch
{
    if (!-e "updates_staged/projecteqquests-master/") {
        print "[Update] Fetching Latest lua modules --- \n";
        get_remote_file("https://codeload.github.com/ProjectEQ/projecteqquests/zip/master",
            "updates_staged/projecteqquests-master.zip",
            1);
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
                print "[Install] Installing [" . $destination_file . "]\n";
                $fc++;
            }
            else {
                $directory_indexff = do_file_diff($destination_file, $staged_file);
                if ($directory_indexff ne "") {
                    $backup_dest = "updates_backups/" . $time_stamp . "/" . $destination_file;
                    print $directory_indexff . "\n";
                    print "[Update] File Different [" . $destination_file . "]\n";
                    print "[Update] Do you wish to update this LUA Module? '" . $destination_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
                    my $input = <STDIN>;
                    if ($input =~ /N/i) {}
                    else {
                        #::: Make a backup
                        copy_file($destination_file, $backup_dest);
                        #::: Copy staged to running
                        copy($staged_file, $destination_file);
                        print "[Install] Installing [" . $destination_file . "]\n\n";
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

sub plugins_fetch
{
    if (!-e "updates_staged/projecteqquests-master/") {
        print "[Update] Fetching Latest plugins --- \n";
        get_remote_file("https://codeload.github.com/ProjectEQ/projecteqquests/zip/master",
            "updates_staged/projecteqquests-master.zip",
            1);
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
                print "[Install] Installing [" . $destination_file . "]\n";
                $fc++;
            }
            else {
                $directory_indexff = do_file_diff($destination_file, $staged_file);
                if ($directory_indexff ne "") {
                    $backup_dest = "updates_backups/" . $time_stamp . "/" . $destination_file;
                    print $directory_indexff . "\n";
                    print "[Update] File Different [" . $destination_file . "]\n";
                    print "[Update] Do you wish to update this Plugin? '" . $destination_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
                    my $input = <STDIN>;
                    if ($input =~ /N/i) {}
                    else {
                        #::: Make a backup
                        copy_file($destination_file, $backup_dest);
                        #::: Copy staged to running
                        copy($staged_file, $destination_file);
                        print "[Install] Installing [" . $destination_file . "]\n\n";
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

sub do_file_diff
{
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

sub unzip
{
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

sub do_bots_db_schema_drop
{
    print `$command bots:disable`;
}

sub clear_database_runs
{
    # print "DEBUG :: clear_database_runs\n\n";
    #::: Clear manifest data...
    %m_d = ();
    #::: Clear updates...
    @total_updates = ();
}

sub print_break
{
    if (!$debug) { return; }
    print "\n==============================================\n";
}

sub generate_random_password
{
    my $passwordsize = shift;
    my @alphanumeric = ('a' .. 'z', 'A' .. 'Z', 0 .. 9);
    my $randpassword = join '',
        map $alphanumeric[rand @alphanumeric], 0 .. $passwordsize;

    return $randpassword;
}
