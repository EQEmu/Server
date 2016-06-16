#!/usr/bin/perl

###########################################################
#::: Automatic (Database) Upgrade Script
#::: Author: Akkadius
#::: Purpose: To upgrade databases with ease and maintain versioning
###########################################################

$menu_displayed = 0;

use Config;
use File::Copy qw(copy);
use POSIX qw(strftime);
use File::Path;
use File::Find;
use URI::Escape;
use Time::HiRes qw(usleep);

$time_stamp = strftime('%m-%d-%Y', gmtime());

$console_output .= "	Operating System is: $Config{osname}\n";
if($Config{osname}=~/linux/i){ $OS = "Linux"; }
if($Config{osname}=~/Win|MS/i){ $OS = "Windows"; }

#::: If current version is less than what world is reporting, then download a new one...
$current_version = 14;

if($ARGV[0] eq "V"){
	if($ARGV[1] > $current_version){ 
		print "eqemu_update.pl Automatic Database Upgrade Needs updating...\n";
		print "	Current version: " . $current_version . "\n"; 
		print "	New version: " . $ARGV[1] . "\n";  
		get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_update.pl", "eqemu_update.pl");
		exit;
	}
	else{
		print "[Upgrade Script] No script update necessary \n";
	}
	exit;
}

#::: Sets database run stage check 
$db_run_stage = 0;

$perl_version = $^V;
$perl_version =~s/v//g;
print "Perl Version is " . $perl_version . "\n";
if($perl_version > 5.12){ no warnings 'uninitialized';  }
no warnings;

($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime();

my $confile = "eqemu_config.xml"; #default
open(F, "<$confile");
my $indb = 0;
while(<F>) {
	s/\r//g;
	if(/<database>/i) { $indb = 1; }
	next unless($indb == 1);
	if(/<\/database>/i) { $indb = 0; last; }
	if(/<host>(.*)<\/host>/i) { $host = $1; } 
	elsif(/<username>(.*)<\/username>/i) { $user = $1; } 
	elsif(/<password>(.*)<\/password>/i) { $pass = $1; } 
	elsif(/<db>(.*)<\/db>/i) { $db = $1; } 
}

$console_output = 
"============================================================
           EQEmu: Automatic Upgrade Check         
============================================================
";

if($OS eq "Windows"){
	$has_mysql_path = `echo %PATH%`;
	if($has_mysql_path=~/MySQL|MariaDB/i){ 
		@mysql = split(';', $has_mysql_path);
		foreach my $v (@mysql){
			if($v=~/MySQL|MariaDB/i){ 
				$v =~s/\n//g; 
				$path = trim($v) . "/mysql";
				last;
			}
		}
		$console_output .= "	(Windows) MySQL is in system path \n";
		$console_output .= "	Path = " . $path . "\n";
		$console_output .= "============================================================\n";
	}
}

#::: Linux Check
if($OS eq "Linux"){
	$path = `which mysql`; 
	if ($path eq "") {
		$path = `which mariadb`;
	}
	$path =~s/\n//g; 
	
	$console_output .= "	(Linux) MySQL is in system path \n";
	$console_output .= "	Path = " . $path . "\n";
	$console_output .= "============================================================\n";
}

#::: Path not found, error and exit
if($path eq ""){
	print "MySQL path not found, please add the path for automatic database upgrading to continue... \n\n";
	print "script_exiting...\n";
	exit;
}

if($ARGV[0] eq "install_peq_db"){
	
	$db_name = "peq";
	if($ARGV[1]){
		$db_name = $ARGV[1];
	}
	
	$db = $db_name;

	#::: Database Routines
	print "MariaDB :: Creating Database '" . $db_name . "'\n";
	print `"$path" --host $host --user $user --password="$pass" -N -B -e "DROP DATABASE IF EXISTS $db_name;"`;
	print `"$path" --host $host --user $user --password="$pass" -N -B -e "CREATE DATABASE $db_name"`;
	if($OS eq "Windows"){ @db_version = split(': ', `world db_version`); } 
	if($OS eq "Linux"){ @db_version = split(': ', `./world db_version`); }  
	$bin_db_ver = trim($db_version[1]);
	check_db_version_table();
	$local_db_ver = trim(get_mysql_result("SELECT version FROM db_version LIMIT 1"));
	fetch_peq_db_full();
	print "\nFetching Latest Database Updates...\n";
	main_db_management();
	print "\nApplying Latest Database Updates...\n";
	main_db_management();
	
	print get_mysql_result("UPDATE `launcher` SET `dynamics` = 30 WHERE `name` = 'zone'");
}

if($ARGV[0] eq "remove_duplicate_rules"){
	remove_duplicate_rule_values();	
	exit;
}

if($ARGV[0] eq "installer"){
	print "Running EQEmu Server installer routines...\n";
	mkdir('logs');
	mkdir('updates_staged');
	mkdir('shared');
	fetch_latest_windows_binaries();
	map_files_fetch_bulk();
	opcodes_fetch();
	plugins_fetch();
	quest_files_fetch();
	lua_modules_fetch();
	
	#::: Binary dll's
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/lua51.dll", "lua51.dll", 1);
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/zlib1.dll", "zlib1.dll", 1);
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/libmysql.dll", "libmysql.dll", 1);
	
	#::: Server scripts
	fetch_utility_scripts();
	
	#::: Database Routines
	print "MariaDB :: Creating Database 'peq'\n";
	print `"$path" --host $host --user $user --password="$pass" -N -B -e "DROP DATABASE IF EXISTS peq;"`;
	print `"$path" --host $host --user $user --password="$pass" -N -B -e "CREATE DATABASE peq"`;
	if($OS eq "Windows"){ @db_version = split(': ', `world db_version`); } 
	if($OS eq "Linux"){ @db_version = split(': ', `./world db_version`); }  
	$bin_db_ver = trim($db_version[1]);
	check_db_version_table();
	$local_db_ver = trim(get_mysql_result("SELECT version FROM db_version LIMIT 1"));
	fetch_peq_db_full();
	print "\nFetching Latest Database Updates...\n";
	main_db_management();
	print "\nApplying Latest Database Updates...\n";
	main_db_management();
	
	print get_mysql_result("UPDATE `launcher` SET `dynamics` = 30 WHERE `name` = 'zone'");
	
	if($OS eq "Windows"){
		check_windows_firewall_rules();
		do_windows_login_server_setup();
	}
	exit;
}

if($ARGV[0] eq "db_dump_compress"){ database_dump_compress(); exit; }
if($ARGV[0] eq "login_server_setup"){
	do_windows_login_server_setup();	
	exit;
}

#::: Create db_update working directory if not created
mkdir('db_update'); 

#::: Check if db_version table exists... 
if(trim(get_mysql_result("SHOW COLUMNS FROM db_version LIKE 'Revision'")) ne "" && $db){
	print get_mysql_result("DROP TABLE db_version");
	print "Old db_version table present, dropping...\n\n";
}

sub check_db_version_table{
	if(get_mysql_result("SHOW TABLES LIKE 'db_version'") eq "" && $db){
		print get_mysql_result("
			CREATE TABLE db_version (
			  version int(11) DEFAULT '0'
			) ENGINE=InnoDB DEFAULT CHARSET=latin1;
			INSERT INTO db_version (version) VALUES ('1000');");
		print "Table 'db_version' does not exists.... Creating...\n\n";
	}
}

check_db_version_table();

if($OS eq "Windows"){ @db_version = split(': ', `world db_version`); }
if($OS eq "Linux"){ @db_version = split(': ', `./world db_version`); }  

$bin_db_ver = trim($db_version[1]);
$local_db_ver = trim(get_mysql_result("SELECT version FROM db_version LIMIT 1"));

#::: If ran from Linux startup script, supress output
if($bin_db_ver == $local_db_ver && $ARGV[0] eq "ran_from_start"){ 
	print "Database up to date...\n"; 
	exit; 
}
else{ 
	print $console_output if $db; 
}

if($db){
	print "	Binary Revision / Local: (" . $bin_db_ver . " / " . $local_db_ver . ")\n";
	
	#::: Bots
	#::: Make sure we're running a bots binary to begin with
	if(trim($db_version[2]) > 0){
		$bots_local_db_version = get_bots_db_version();
		if($bots_local_db_version > 0){
			print "	(Bots) Binary Revision / Local: (" . trim($db_version[2]) . " / " . $bots_local_db_version . ")\n";
		}
	}

	#::: If World ran this script, and our version is up to date, continue...
	if($bin_db_ver <= $local_db_ver && $ARGV[0] eq "ran_from_world"){  
		print "	Database up to Date: Continuing World Bootup...\n";
		print "============================================================\n";
		exit; 
	}

}

if($local_db_ver < $bin_db_ver && $ARGV[0] eq "ran_from_world"){
	print "You have missing database updates, type 1 or 2 to backup your database before running them as recommended...\n\n";
	#::: Display Menu 
	show_menu_prompt();
}
else{
	#::: Most likely ran standalone
	print "\n";
	show_menu_prompt();
}

sub do_update_self{
	get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_update.pl", "eqemu_update.pl");
	die "Rerun eqemu_update.pl";
}

sub fetch_utility_scripts {
	if($OS eq "Windows"){
		get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/t_database_backup.bat", "t_database_backup.bat");
		get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/t_start_server.bat", "t_start_server.bat");
		get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/t_start_server_with_login_server.bat", "t_start_server_with_login_server.bat");
		get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/t_stop_server.bat", "t_stop_server.bat");
		get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/t_server_crash_report.pl", "t_server_crash_report.pl");
		get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/win_server_launcher.pl", "win_server_launcher.pl");
		get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/t_start_server_with_login_server.bat", "t_start_server_with_login_server.bat");
	}
	else {
		print "No scripts found for OS: " . $OS . "...\n";
	}
}

sub show_menu_prompt {
    my %dispatch = (
        1 => \&database_dump,
        2 => \&database_dump_compress,
        3 => \&main_db_management,
        4 => \&bots_db_management,
        5 => \&opcodes_fetch,
        6 => \&map_files_fetch,
        7 => \&plugins_fetch,
        8 => \&quest_files_fetch,
        9 => \&lua_modules_fetch,
		10 => \&aa_fetch,
		11 => \&fetch_latest_windows_binaries,
		12 => \&fetch_server_dlls,
		13 => \&do_windows_login_server_setup,
		14 => \&remove_duplicate_rule_values,
		15 => \&fetch_utility_scripts,
		19 => \&do_bots_db_schema_drop,
        20 => \&do_update_self,
        0 => \&script_exit,
    );

    while (1) { 
		{
			local $| = 1;
			if(!$menu_show && ($ARGV[0] eq "ran_from_world" || $ARGV[0] eq "ran_from_start")){ 
				$menu_show++;
				next;
			}
			print menu_options(), '> ';
			$menu_displayed++;
			if($menu_displayed > 50){
				print "Safety: Menu looping too many times, exiting...\n"; 
				exit;
			}
		}

		my $choice = <>;

		$choice =~ s/\A\s+//;
		$choice =~ s/\s+\z//;

		if (defined(my $handler = $dispatch{$choice})) {
			my $result = $handler->();
			unless (defined $result) {
				exit 0;
			}
		}
		else {
			if($ARGV[0] ne "ran_from_world"){
				# warn "\n\nInvalid selection\n\n";
			}
		}
	}
}

sub menu_options {
	if(@total_updates){ 
		if($bots_db_management == 1){
			$option[3] = "Check and stage pending REQUIRED Database updates";
			$bots_management = "Run pending REQUIRED updates... (" . scalar (@total_updates) . ")";
		}
		else{
			$option[3] = "Run pending REQUIRED updates... (" . scalar (@total_updates) . ")";
			if(get_mysql_result("SHOW TABLES LIKE 'bots'") eq ""){
				$bots_management = "Install bots database pre-requisites (Requires bots server binaries)";
			}
			else{
				$bots_management = "Check for Bot pending REQUIRED database updates... (Must have bots enabled)";
			}
		}
	}
	else{
		$option[3] = "Check and stage pending REQUIRED Database updates";
		$bots_management = "Check for Bot REQUIRED database updates... (Must have bots enabled)";
	}

return <<EO_MENU;
============================================================
#::: EQEmu Update Utility Menu: (eqemu_update.pl)
============================================================
 1) [Backup Database] :: (Saves to Backups folder)
 2) [Backup Database Compressed] :: (Saves to Backups folder)
 3) [EQEmu DB Schema] :: $option[3]
 4) [EQEmu DB Bots Schema] $bots_management
 5) [OPCodes] :: Download latest opcodes for each EQ Client
 6) [Maps] :: Download latest map and water files
 7) [Plugins (Perl)] :: Download latest Perl plugins
 8) [Quests (Perl/LUA)] :: Download latest PEQ quests and stage updates
 9) [LUA Modules] :: Download latest LUA Modules (Required for Lua)
 10) [DB Data : Alternate Advancement] :: Download Latest AA's from PEQ (This overwrites existing data)
 11) [Windows Server Build] :: Download Latest and Stable Server Build (Overwrites existing .exe's, includes .dll's)
 12) [Windows Server .dll's] :: Download Pre-Requisite Server .dll's
 13) [Windows Server Loginserver Setup] :: Download and install Windows Loginserver
 14) [Remove Duplicate Rule Values] :: Looks for redundant rule_values entries and removes them
 15) [Fetch Utility Scripts] :: Fetches server management utility scripts
 19) [EQEmu DB Drop Bots Schema] :: Remove Bots schema and return database to normal state
 20) [Update the updater] Force update this script (Redownload)
 0) Exit
 
 Enter numbered option and press enter...	
	
EO_MENU
}

sub check_for_database_dump_script{
	if(`perl db_dumper.pl`=~/Need arguments/i){
		return; 
	}
	else{
		print "db_dumper.pl not found... retrieving...\n\n";
		get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/db_dumper.pl", "db_dumper.pl");
	}
}

sub ran_from_world { 
	print "Running from world...\n";
}

sub database_dump { 
	check_for_database_dump_script();
	print "Performing database backup....\n";
	print `perl db_dumper.pl database="$db" loc="backups"`;
}
sub database_dump_compress { 
	check_for_database_dump_script();
	print "Performing database backup....\n";
	print `perl db_dumper.pl database="$db"  loc="backups" compress`;
}

sub script_exit{ 
	#::: Cleanup staged folder...
	rmtree("updates_staged/");
	exit;
}

#::: Returns Tab Delimited MySQL Result from Command Line
sub get_mysql_result{
	my $run_query = $_[0];
	if(!$db){ return; }
	if($OS eq "Windows"){ return `"$path" --host $host --user $user --password="$pass" $db -N -B -e "$run_query"`; }
	if($OS eq "Linux"){ 
		$run_query =~s/`//g;
		return `$path --user="$user" --host $host --password="$pass" $db -N -B -e "$run_query"`; 
	}
}

sub get_mysql_result_from_file{
	my $update_file = $_[0];
	if(!$db){ return; }
	if($OS eq "Windows"){ return `"$path" --host $host --user $user --password="$pass" --force $db < $update_file`;  }
	if($OS eq "Linux"){ return `"$path" --host $host --user $user --password="$pass" --force $db < $update_file`;  }
}

#::: Gets Remote File based on URL (1st Arg), and saves to destination file (2nd Arg)
#::: Example: get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/db_update_manifest.txt", "db_update/db_update_manifest.txt");
sub get_remote_file{
	my $URL = $_[0];
	my $Dest_File = $_[1];
	my $content_type = $_[2];
	
	#::: Build file path of the destination file so that we may check for the folder's existence and make it if necessary
	if($Dest_File=~/\//i){
		my @dir_path = split('/', $Dest_File);
		$build_path = "";
		$di = 0;
		while($dir_path[$di]){
			$build_path .= $dir_path[$di] . "/";	
			#::: If path does not exist, create the directory...
			if (!-d $build_path) {
				mkdir($build_path);
			}
			if(!$dir_path[$di + 2] && $dir_path[$di + 1]){
				# print $actual_path . "\n";
				$actual_path = $build_path;
				last;
			}
			$di++;
		}
	}
	
	if($OS eq "Windows"){ 
		#::: For non-text type requests...
		if($content_type == 1){
			$break = 0;
			while($break == 0) {
				use LWP::Simple qw(getstore);
				if(!getstore($URL, $Dest_File)){
					# print "Error, no connection or failed request...\n\n";
				}
				# sleep(1);
				#::: Make sure the file exists before continuing...
				if(-e $Dest_File) { 
					$break = 1;
					print " [URL] :: " . $URL . "\n";
					print "	[Saved] :: " . $Dest_File . "\n";
				} else { $break = 0; }
				usleep(500);
			}
		}
		else{
			$break = 0;
			while($break == 0) {
				require LWP::UserAgent; 
				my $ua = LWP::UserAgent->new;
				$ua->timeout(10);
				$ua->env_proxy; 
				my $response = $ua->get($URL);
				if ($response->is_success){
					open (FILE, '> ' . $Dest_File . '');
					print FILE $response->decoded_content;
					close (FILE); 
				}
				else {
					# print "Error, no connection or failed request...\n\n";
				}
				if(-e $Dest_File) { 
					$break = 1;
					print " [URL] :: " . $URL . "\n";
					print "	[Saved] :: " . $Dest_File . "\n";
				} else { $break = 0; }
				usleep(500);
			}
		}
	}
	if($OS eq "Linux"){
		#::: wget -O db_update/db_update_manifest.txt https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/db_update_manifest.txt
		$wget = `wget --no-check-certificate --quiet -O $Dest_File $URL`;
		print " o URL: (" . $URL . ")\n";
		print " o Saved: (" . $Dest_File . ") \n";
		if($wget=~/unable to resolve/i){
			print "Error, no connection or failed request...\n\n";
			#die;
		}
	}
}

#::: Trim Whitespaces
sub trim { 
	my $string = $_[0]; 
	$string =~ s/^\s+//; 
	$string =~ s/\s+$//; 
	return $string; 
}

#::: Fetch Latest PEQ AA's
sub aa_fetch{
	if(!$db){
		print "No database present, check your eqemu_config.xml for proper MySQL/MariaDB configuration...\n";
		return;
	}

	print "Pulling down PEQ AA Tables...\n";
	get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/peq_aa_tables_post_rework.sql", "db_update/peq_aa_tables_post_rework.sql");
	print "\n\nInstalling AA Tables...\n";
	print get_mysql_result_from_file("db_update/peq_aa_tables_post_rework.sql");
	print "\nDone...\n\n";
}

#::: Fetch Latest Opcodes
sub opcodes_fetch{
	print "Pulling down latest opcodes...\n"; 
	%opcodes = (
		1 => ["opcodes", "https://raw.githubusercontent.com/EQEmu/Server/master/utils/patches/opcodes.conf"],
		2 => ["mail_opcodes", "https://raw.githubusercontent.com/EQEmu/Server/master/utils/patches/mail_opcodes.conf"],
		3 => ["Titanium", "https://raw.githubusercontent.com/EQEmu/Server/master/utils/patches/patch_Titanium.conf"],
		4 => ["Secrets of Faydwer", "https://raw.githubusercontent.com/EQEmu/Server/master/utils/patches/patch_SoF.conf"],
		5 => ["Seeds of Destruction", "https://raw.githubusercontent.com/EQEmu/Server/master/utils/patches/patch_SoD.conf"],
		6 => ["Underfoot", "https://raw.githubusercontent.com/EQEmu/Server/master/utils/patches/patch_UF.conf"],
		7 => ["Rain of Fear", "https://raw.githubusercontent.com/EQEmu/Server/master/utils/patches/patch_RoF.conf"],
		8 => ["Rain of Fear 2", "https://raw.githubusercontent.com/EQEmu/Server/master/utils/patches/patch_RoF2.conf"],
	);
	$loop = 1;
	while($opcodes{$loop}[0]){ 
		#::: Split the URL by the patches folder to get the file name from URL
		@real_file = split("patches/", $opcodes{$loop}[1]);
		$find = 0;
		while($real_file[$find]){
			$file_name = $real_file[$find]; 
			$find++;
		}
		
		print "\nDownloading (" . $opcodes{$loop}[0] . ") File: '" . $file_name . "'...\n\n"; 
		get_remote_file($opcodes{$loop}[1], $file_name);
		$loop++; 
	}
	print "\nDone...\n\n";
}

sub remove_duplicate_rule_values{
	$ruleset_id = trim(get_mysql_result("SELECT `ruleset_id` FROM `rule_sets` WHERE `name` = 'default'"));
	print "Default Ruleset ID: " . $ruleset_id . "\n";
	
	$total_removed = 0;
	#::: Store Default values...
	$mysql_result = get_mysql_result("SELECT * FROM `rule_values` WHERE `ruleset_id` = " . $ruleset_id);
	my @lines = split("\n", $mysql_result);
	foreach my $val (@lines){
		my @values = split("\t", $val);
		$rule_set_values{$values[1]}[0] = $values[2];
	}
	#::: Compare default values against other rulesets to check for duplicates...
	$mysql_result = get_mysql_result("SELECT * FROM `rule_values` WHERE `ruleset_id` != " . $ruleset_id);
	my @lines = split("\n", $mysql_result);
	foreach my $val (@lines){
		my @values = split("\t", $val);
		if($values[2] == $rule_set_values{$values[1]}[0]){
			print "DUPLICATE : " . $values[1] . " (Ruleset (" . $values[0] . ")) matches default value of : " . $values[2] . ", removing...\n";
			get_mysql_result("DELETE FROM `rule_values` WHERE `ruleset_id` = " .  $values[0] . " AND `rule_name` = '" . $values[1] . "'");
			$total_removed++;
		}
	}
	
	print "Total duplicate rules removed... " . $total_removed . "\n";
}

sub copy_file{
	$l_source_file = $_[0];
	$l_dest_file = $_[1];
	if($l_dest_file=~/\//i){
		my @dir_path = split('/', $l_dest_file);
		$build_path = "";
		$di = 0;
		while($dir_path[$di]){
			$build_path .= $dir_path[$di] . "/";	
			#::: If path does not exist, create the directory...
			if (!-d $build_path) {
				mkdir($build_path);
			}
			if(!$dir_path[$di + 2] && $dir_path[$di + 1]){
				# print $actual_path . "\n";
				$actual_path = $build_path;
				last;
			}
			$di++;
		}
	}
	copy $l_source_file, $l_dest_file;
}

sub fetch_latest_windows_binaries{
	print "\n --- Fetching Latest Windows Binaries... --- \n";
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/master_windows_build.zip", "updates_staged/master_windows_build.zip", 1);
	print "\n --- Fetched Latest Windows Binaries... --- \n";
	print "\n --- Extracting... --- \n";
	unzip('updates_staged/master_windows_build.zip', 'updates_staged/binaries/');
	my @files;
	my $start_dir = "updates_staged/binaries";
	find( 
		sub { push @files, $File::Find::name unless -d; }, 
		$start_dir
	);
	for my $file (@files) {
		$dest_file = $file;
		$dest_file =~s/updates_staged\/binaries\///g;
		print "Installing :: " . $dest_file . "\n";
		copy_file($file, $dest_file);
	}
	print "\n --- Done... --- \n";
	
	rmtree('updates_staged');
}

sub do_windows_login_server_setup{
	print "\n --- Fetching Loginserver... --- \n";
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/login_server.zip", "updates_staged/login_server.zip", 1);
	print "\n --- Extracting... --- \n";
	unzip('updates_staged/login_server.zip', 'updates_staged/login_server/');
	my @files;
	my $start_dir = "updates_staged/login_server";
	find( 
		sub { push @files, $File::Find::name unless -d; }, 
		$start_dir
	);
	for my $file (@files) {
		$dest_file = $file;
		$dest_file =~s/updates_staged\/login_server\///g;
		print "Installing :: " . $dest_file . "\n";
		copy_file($file, $dest_file);
	}
	print "\n Done... \n";
	
	print "Pulling down Loginserver database tables...\n";
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/login_server_tables.sql", "db_update/login_server_tables.sql");
	print "\n\nInstalling Loginserver tables...\n";
	print get_mysql_result_from_file("db_update/login_server_tables.sql");
	print "\nDone...\n\n";
	
	add_login_server_firewall_rules();
	
	rmtree('updates_staged');
	rmtree('db_update');
	
	print "\nPress any key to continue...\n";
	
	<>; #Read from STDIN
	
}

sub add_login_server_firewall_rules{
	#::: Check Loginserver Firewall install for Windows
	if($OS eq "Windows"){
		$output = `netsh advfirewall firewall show rule name=all`;
		@output_buffer = split("\n", $output);
		$has_loginserver_rules_titanium = 0;
		$has_loginserver_rules_sod = 0;
		foreach my $val (@output_buffer){
			if($val=~/Rule Name/i){
				$val=~s/Rule Name://g;
				if($val=~/EQEmu Loginserver/i && $val=~/Titanium/i){
					$has_loginserver_rules_titanium = 1;
					print "Found existing rule :: " . trim($val) . "\n";
				}
				if($val=~/EQEmu Loginserver/i && $val=~/SOD/i){
					$has_loginserver_rules_sod = 1;
					print "Found existing rule :: " . trim($val) . "\n";
				}
			}
		}
		
		if($has_loginserver_rules_titanium == 0){
			print "Attempting to add EQEmu Loginserver Firewall Rules (Titanium) (TCP) port 5998 \n";
			print `netsh advfirewall firewall add rule name="EQEmu Loginserver (Titanium) (5998) TCP" dir=in action=allow protocol=TCP localport=5998`;
			print "Attempting to add EQEmu Loginserver Firewall Rules (Titanium) (UDP) port 5998 \n";
			print `netsh advfirewall firewall add rule name="EQEmu Loginserver (Titanium) (5998) UDP" dir=in action=allow protocol=UDP localport=5998`;
		}
		if($has_loginserver_rules_sod == 0){
			print "Attempting to add EQEmu Loginserver Firewall Rules (SOD+) (TCP) port 5999 \n";
			print `netsh advfirewall firewall add rule name="EQEmu Loginserver (SOD+) (5999) TCP" dir=in action=allow protocol=TCP localport=5999`;
			print "Attempting to add EQEmu Loginserver Firewall Rules (SOD+) (UDP) port 5999 \n";
			print `netsh advfirewall firewall add rule name="EQEmu Loginserver (SOD+) (5999) UDP" dir=in action=allow protocol=UDP localport=5999`;
		}
		
		print "If firewall rules don't add you must run this script (eqemu_update.pl) as administrator\n";
		print "\n";
		print "#::: Instructions \n";
		print "In order to connect your server to the loginserver you must point your eqemu_config.xml to your local server similar to the following:\n";
		print "
	<loginserver1>
		<host>login.eqemulator.net</host>
		<port>5998</port>
		<account></account>
		<password></password>
	</loginserver1>
	<loginserver2>
		<host>127.0.0.1</host>
		<port>5998</port>
		<account></account>
		<password></password>
	</loginserver2>
		";
		print "\nWhen done, make sure your EverQuest client points to your loginserver's IP (In this case it would be 127.0.0.1) in the eqhosts.txt file\n";
	}
}

sub check_windows_firewall_rules{
	$output = `netsh advfirewall firewall show rule name=all`;
	@output_buffer = split("\n", $output);
	$has_world_rules = 0;
	$has_zone_rules = 0;
	foreach my $val (@output_buffer){
		if($val=~/Rule Name/i){
			$val=~s/Rule Name://g;
			if($val=~/EQEmu World/i){
				$has_world_rules = 1;
				print "Found existing rule :: " . trim($val) . "\n";
			}
			if($val=~/EQEmu Zone/i){
				$has_zone_rules = 1;
				print "Found existing rule :: " . trim($val) . "\n";
			}
		}
	}
	
	if($has_world_rules == 0){
		print "Attempting to add EQEmu World Firewall Rules (TCP) port 9000 \n";
		print `netsh advfirewall firewall add rule name="EQEmu World (9000) TCP" dir=in action=allow protocol=TCP localport=9000`;
		print "Attempting to add EQEmu World Firewall Rules (UDP) port 9000 \n";
		print `netsh advfirewall firewall add rule name="EQEmu World (9000) UDP" dir=in action=allow protocol=UDP localport=9000`;
	}
	if($has_zone_rules == 0){
		print "Attempting to add EQEmu Zones (7000-7500) TCP \n";
		print `netsh advfirewall firewall add rule name="EQEmu Zones (7000-7500) TCP" dir=in action=allow protocol=TCP localport=7000-7500`;
		print "Attempting to add EQEmu Zones (7000-7500) UDP \n";
		print `netsh advfirewall firewall add rule name="EQEmu Zones (7000-7500) UDP" dir=in action=allow protocol=UDP localport=7000-7500`;
	}
}

sub fetch_server_dlls{
	print "Fetching lua51.dll, zlib1.dll, libmysql.dll...\n";
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/lua51.dll", "lua51.dll", 1);
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/zlib1.dll", "zlib1.dll", 1);
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuInstall/master/libmysql.dll", "libmysql.dll", 1);
}

sub fetch_peq_db_full{
	print "Downloading latest PEQ Database... Please wait...\n";
	get_remote_file("http://edit.peqtgc.com/weekly/peq_beta.zip", "updates_staged/peq_beta.zip", 1);
	print "Downloaded latest PEQ Database... Extracting...\n";
	unzip('updates_staged/peq_beta.zip', 'updates_staged/peq_db/');
	my $start_dir = "updates_staged\\peq_db";
	find( 
		sub { push @files, $File::Find::name unless -d; }, 
		$start_dir
	);
	for my $file (@files) {
		$dest_file = $file;
		$dest_file =~s/updates_staged\\peq_db\///g;
		if($file=~/peqbeta|player_tables/i){
			print "MariaDB :: Installing :: " . $dest_file . "\n";
			get_mysql_result_from_file($file);
		}
		if($file=~/eqtime/i){
			print "Installing eqtime.cfg\n";
			copy_file($file, "eqtime.cfg");
		}
	}
}

sub map_files_fetch_bulk{
	print "\n --- Fetching Latest Maps... (This could take a few minutes...) --- \n";
	get_remote_file("http://github.com/Akkadius/EQEmuMaps/archive/master.zip", "maps/maps.zip", 1);
	unzip('maps/maps.zip', 'maps/');
	my @files;
	my $start_dir = "maps\\EQEmuMaps-master\\maps";
	find( 
		sub { push @files, $File::Find::name unless -d; }, 
		$start_dir
	);
	for my $file (@files) {
		$dest_file = $file;
		$dest_file =~s/maps\\EQEmuMaps-master\\maps\///g;
		print "Installing :: " . $dest_file . "\n";
		copy_file($file, "maps/" . $new_file);
	}
	print "\n --- Fetched Latest Maps... --- \n";
	
	rmtree('maps/EQEmuMaps-master');
	unlink('maps/maps.zip');
}

sub map_files_fetch{
	print "\n --- Fetching Latest Maps --- \n";
	
	get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuMaps/master/!eqemu_maps_manifest.txt", "updates_staged/eqemu_maps_manifest.txt");
	
	#::: Get Data from manifest
	open (FILE, "updates_staged/eqemu_maps_manifest.txt");
	$i = 0;
	while (<FILE>){
		chomp;
		$o = $_;
		@manifest_map_data = split(',', $o);
		if($manifest_map_data[0] ne ""){
			$maps_manifest[$i] = [$manifest_map_data[0], $manifest_map_data[1]];
			$i++;
		}
	}
	
	#::: Download  
	$fc = 0;
	for($m = 0; $m <= $i; $m++){
		my $file_existing = $maps_manifest[$m][0];
		my $file_existing_size = (stat $file_existing)[7];
		if($file_existing_size != $maps_manifest[$m][1]){
			print "Updating: '" . $maps_manifest[$m][0] . "'\n";
			get_remote_file("https://raw.githubusercontent.com/Akkadius/EQEmuMaps/master/" .  $maps_manifest[$m][0], $maps_manifest[$m][0], 1);
			$fc++;
		}
	}
	
	if($fc == 0){
		print "\nNo Map Updates found... \n\n";
	}
}

sub quest_files_fetch{
	if (!-e "updates_staged/Quests-Plugins-master/quests/") {
		print "\n --- Fetching Latest Quests --- \n";
		get_remote_file("https://github.com/EQEmu/Quests-Plugins/archive/master.zip", "updates_staged/Quests-Plugins-master.zip", 1);
		print "\nFetched latest quests...\n";
		mkdir('updates_staged');
		unzip('updates_staged/Quests-Plugins-master.zip', 'updates_staged/');
	}
	
	$fc = 0;
	use File::Find;
	use File::Compare;
	
	my @files;
	my $start_dir = "updates_staged/Quests-Plugins-master/quests/";
	find( 
		sub { push @files, $File::Find::name unless -d; }, 
		$start_dir
	);
	for my $file (@files) {
		if($file=~/\.pl|\.lua|\.ext/i){
			$staged_file = $file;
			$dest_file = $file;
			$dest_file =~s/updates_staged\/Quests-Plugins-master\///g;
			
			if (!-e $dest_file) {
				copy_file($staged_file, $dest_file);
				print "Installing :: '" . $dest_file . "'\n";
				$fc++;
			}
			else{
				$diff = do_file_diff($dest_file, $staged_file);
				if($diff ne ""){
					$backup_dest = "updates_backups/" . $time_stamp . "/" . $dest_file;
				
					print $diff . "\n";
					print "\nFile Different :: '" . $dest_file . "'\n";
					print "\nDo you wish to update this Quest? '" . $dest_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
					my $input = <STDIN>;
					if($input=~/N/i){}
					else{
						#::: Make a backup
						copy_file($dest_file, $backup_dest);
						#::: Copy staged to running
						copy($staged_file, $dest_file);
						print "Installing :: '" . $dest_file . "'\n\n";
					}
					$fc++;
				}
			}
		}
	}
	
	rmtree('updates_staged');
	
	if($fc == 0){
		print "\nNo Quest Updates found... \n\n";
	}
}

sub lua_modules_fetch{
	if (!-e "updates_staged/Quests-Plugins-master/quests/lua_modules/") {
		print "\n --- Fetching Latest LUA Modules --- \n";
		get_remote_file("https://github.com/EQEmu/Quests-Plugins/archive/master.zip", "updates_staged/Quests-Plugins-master.zip", 1);
		print "\nFetched latest LUA Modules...\n";
		unzip('updates_staged/Quests-Plugins-master.zip', 'updates_staged/');
	}
	
	$fc = 0;
	use File::Find;
	use File::Compare;
	
	my @files;
	my $start_dir = "updates_staged/Quests-Plugins-master/quests/lua_modules/";
	find( 
		sub { push @files, $File::Find::name unless -d; }, 
		$start_dir
	);
	for my $file (@files) {
		if($file=~/\.pl|\.lua|\.ext/i){
			$staged_file = $file;
			$dest_file = $file;
			$dest_file =~s/updates_staged\/Quests-Plugins-master\/quests\///g;
			
			if (!-e $dest_file) {
				copy_file($staged_file, $dest_file);
				print "Installing :: '" . $dest_file . "'\n";
				$fc++;
			}
			else{
				$diff = do_file_diff($dest_file, $staged_file);
				if($diff ne ""){
					$backup_dest = "updates_backups/" . $time_stamp . "/" . $dest_file;
					print $diff . "\n";
					print "\nFile Different :: '" . $dest_file . "'\n";
					print "\nDo you wish to update this LUA Module? '" . $dest_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
					my $input = <STDIN>;
					if($input=~/N/i){}
					else{
						#::: Make a backup
						copy_file($dest_file, $backup_dest);
						#::: Copy staged to running
						copy($staged_file, $dest_file);
						print "Installing :: '" . $dest_file . "'\n\n";
					}
					$fc++;
				}
			}
		}
	}
	
	if($fc == 0){
		print "\nNo LUA Modules Updates found... \n\n";
	}	
}

sub plugins_fetch{
	if (!-e "updates_staged/Quests-Plugins-master/plugins/") {
		print "\n --- Fetching Latest Plugins --- \n";
		get_remote_file("https://github.com/EQEmu/Quests-Plugins/archive/master.zip", "updates_staged/Quests-Plugins-master.zip", 1);
		print "\nFetched latest plugins...\n";
		unzip('updates_staged/Quests-Plugins-master.zip', 'updates_staged/');
	}
	
	$fc = 0;
	use File::Find;
	use File::Compare;
	
	my @files;
	my $start_dir = "updates_staged/Quests-Plugins-master/plugins/";
	find( 
		sub { push @files, $File::Find::name unless -d; }, 
		$start_dir
	);
	for my $file (@files) {
		if($file=~/\.pl|\.lua|\.ext/i){
			$staged_file = $file;
			$dest_file = $file;
			$dest_file =~s/updates_staged\/Quests-Plugins-master\///g;
			
			if (!-e $dest_file) {
				copy_file($staged_file, $dest_file);
				print "Installing :: '" . $dest_file . "'\n";
				$fc++;
			}
			else{
				$diff = do_file_diff($dest_file, $staged_file);
				if($diff ne ""){
					$backup_dest = "updates_backups/" . $time_stamp . "/" . $dest_file;
					print $diff . "\n";
					print "\nFile Different :: '" . $dest_file . "'\n";
					print "\nDo you wish to update this Plugin? '" . $dest_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
					my $input = <STDIN>;
					if($input=~/N/i){}
					else{
						#::: Make a backup
						copy_file($dest_file, $backup_dest);
						#::: Copy staged to running
						copy($staged_file, $dest_file);
						print "Installing :: '" . $dest_file . "'\n\n";
					}
					$fc++;
				}
			}
		}
	}

	if($fc == 0){
		print "\nNo Plugin Updates found... \n\n";
	}	
}

sub do_file_diff{
	$file_1 = $_[0];
	$file_2 = $_[1];
	if($OS eq "Windows"){
		eval "use Text::Diff";
		$diff = diff($file_1, $file_2, { STYLE => "Unified" });
		return $diff;
	}
	if($OS eq "Linux"){
		# print 'diff -u "$file_1" "$file_2"' . "\n";
		return `diff -u "$file_1" "$file_2"`;
	}
}

sub unzip{
	$archive_to_unzip = $_[0];
	$dest_folder = $_[1];
	
	if($OS eq "Windows"){ 
		eval "use Archive::Zip qw( :ERROR_CODES :CONSTANTS )";
		my $zip = Archive::Zip->new();
		unless ( $zip->read($archive_to_unzip) == AZ_OK ) {
			die 'read error';
		}
		print "Extracting...\n";
		$zip->extractTree('', $dest_folder);
	}
	if($OS eq "Linux"){
		print `unzip -o "$archive_to_unzip" -d "$dest_folder"`;
	}
}

sub are_file_sizes_different{
	$file_1 = $_[0];
	$file_2 = $_[1];
	my $file_1 = (stat $file_1)[7];
	my $file_2 = (stat $file_2)[7];
	# print $file_1 . " :: " . $file_2 . "\n";
	if($file_1 != $file_2){
		return 1;
	}
	return;
}

sub do_bots_db_schema_drop{
	#"drop_bots.sql" is run before reverting database back to 'normal'
	print "Fetching drop_bots.sql...\n";
	get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/git/bots/drop_bots.sql", "db_update/drop_bots.sql");
	print get_mysql_result_from_file("db_update/drop_bots.sql");
	
	print "Restoring normality...\n";
	print get_mysql_result("DELETE FROM `rule_values` WHERE `rule_name` LIKE 'Bots:%';");
	
	if(get_mysql_result("SHOW TABLES LIKE 'commands'") ne "" && $db){
		print get_mysql_result("DELETE FROM `commands` WHERE `command` LIKE 'bot';");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'command_settings'") ne "" && $db){
		print get_mysql_result("DELETE FROM `command_settings` WHERE `command` LIKE 'bot';");
	}
	
	if(get_mysql_result("SHOW KEYS FROM `group_id` WHERE `Key_name` LIKE 'PRIMARY'") ne "" && $db){
		print get_mysql_result("ALTER TABLE `group_id` DROP PRIMARY KEY;");
	}
	print get_mysql_result("ALTER TABLE `group_id` ADD PRIMARY KEY (`groupid`, `charid`, `ismerc`);");
	
	if(get_mysql_result("SHOW KEYS FROM `guild_members` WHERE `Key_name` LIKE 'PRIMARY'") ne "" && $db){
		print get_mysql_result("ALTER TABLE `guild_members` DROP PRIMARY KEY;");
	}
	print get_mysql_result("ALTER TABLE `guild_members` ADD PRIMARY KEY (`char_id`);");
	
	print get_mysql_result("UPDATE `spawn2` SET `enabled` = 0 WHERE `id` IN (59297,59298);");
	
	if(get_mysql_result("SHOW COLUMNS FROM `db_version` LIKE 'bots_version'") ne "" && $db){
		print get_mysql_result("UPDATE `db_version` SET `bots_version` = 0;");
	}
}

sub modify_db_for_bots{
	#Called after the db bots schema (2015_09_30_bots.sql) has been loaded
	print "Modifying database for bots...\n";
	print get_mysql_result("UPDATE `spawn2` SET `enabled` = 1 WHERE `id` IN (59297,59298);");
	
	if(get_mysql_result("SHOW KEYS FROM `guild_members` WHERE `Key_name` LIKE 'PRIMARY'") ne "" && $db){
		print get_mysql_result("ALTER TABLE `guild_members` DROP PRIMARY KEY;");
	}
	
	if(get_mysql_result("SHOW KEYS FROM `group_id` WHERE `Key_name` LIKE 'PRIMARY'") ne "" && $db){
		print get_mysql_result("ALTER TABLE `group_id` DROP PRIMARY KEY;");
	}
	print get_mysql_result("ALTER TABLE `group_id` ADD PRIMARY KEY USING BTREE(`groupid`, `charid`, `name`, `ismerc`);");
	
	if(get_mysql_result("SHOW TABLES LIKE 'command_settings'") ne "" && get_mysql_result("SELECT `command` FROM `command_settings` WHERE `command` LIKE 'bot'") eq "" && $db){
		print get_mysql_result("INSERT INTO `command_settings` VALUES ('bot', '0', '');");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'commands'") ne "" && get_mysql_result("SELECT `command` FROM `commands` WHERE `command` LIKE 'bot'") eq "" && $db){
		print get_mysql_result("INSERT INTO `commands` VALUES ('bot', '0');");
	}
	
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotAAExpansion'") ne "" && $db){
		print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:AAExpansion' WHERE `rule_name` LIKE 'Bots:BotAAExpansion';");
	}	
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:AAExpansion'") eq "" && $db){
		print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:AAExpansion', '8', 'The expansion through which bots will obtain AAs');");
	}
	
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:CreateBotCount'") ne "" && $db){
		print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:CreationLimit' WHERE `rule_name` LIKE 'Bots:CreateBotCount';");
	}
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:CreationLimit'") eq "" && $db){
		print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:CreationLimit', '150', 'Number of bots that each account can create');");
	}
	
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotFinishBuffing'") ne "" && $db){
		print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:FinishBuffing' WHERE `rule_name` LIKE 'Bots:BotFinishBuffing';");
	}
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:FinishBuffing'") eq "" && $db){
		print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:FinishBuffing', 'false', 'Allow for buffs to complete even if the bot caster is out of mana.  Only affects buffing out of combat.');");
	}
	
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotGroupBuffing'") ne "" && $db){
		print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:GroupBuffing' WHERE `rule_name` LIKE 'Bots:BotGroupBuffing';");
	}
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:GroupBuffing'") eq "" && $db){
		print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:GroupBuffing', 'false', 'Bots will cast single target buffs as group buffs, default is false for single. Does not make single target buffs work for MGB.');");
	}
	
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotManaRegen'") ne "" && $db){
		print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:ManaRegen' WHERE `rule_name` LIKE 'Bots:BotManaRegen';");
	}
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:ManaRegen'") eq "" && $db){
		print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:ManaRegen', '3.0', 'Adjust mana regen for bots, 1 is fast and higher numbers slow it down 3 is about the same as players.');");
	}
	
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotQuest'") ne "" && $db){
		print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:QuestableSpawnLimit' WHERE `rule_name` LIKE 'Bots:BotQuest';");
	}
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:QuestableSpawnLimit'") eq "" && $db){
		print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:QuestableSpawnLimit', 'false', 'Optional quest method to manage bot spawn limits using the quest_globals name bot_spawn_limit, see: /bazaar/Aediles_Thrall.pl');");
	}
	
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotSpellQuest'") ne "" && $db){
		print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:QuestableSpells' WHERE `rule_name` LIKE 'Bots:BotSpellQuest';");
	}
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:QuestableSpells'") eq "" && $db){
		print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:QuestableSpells', 'false', 'Anita Thrall\\\'s (Anita_Thrall.pl) Bot Spell Scriber quests.');");
	}
	
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:SpawnBotCount'") ne "" && $db){
		print get_mysql_result("UPDATE `rule_values` SET `rule_name` = 'Bots:SpawnLimit' WHERE `rule_name` LIKE 'Bots:SpawnBotCount';");
	}
	if(get_mysql_result("SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:SpawnLimit'") eq "" && $db){
		print get_mysql_result("INSERT INTO `rule_values` VALUES ('1', 'Bots:SpawnLimit', '71', 'Number of bots a character can have spawned at one time, You + 71 bots is a 12 group raid');");
	}
	
	convert_existing_bot_data();
}

sub convert_existing_bot_data{
	if(get_mysql_result("SHOW TABLES LIKE 'bots'") ne "" && $db){
		print "Converting existing bot data...\n";
		print get_mysql_result("INSERT INTO `bot_data` (`bot_id`, `owner_id`, `spells_id`, `name`, `last_name`, `zone_id`, `gender`, `race`, `class`, `level`, `creation_day`, `last_spawn`, `time_spawned`, `size`, `face`, `hair_color`, `hair_style`, `beard`, `beard_color`, `eye_color_1`, `eye_color_2`, `drakkin_heritage`, `drakkin_tattoo`, `drakkin_details`, `ac`, `atk`, `hp`, `mana`, `str`, `sta`, `cha`, `dex`, `int`, `agi`, `wis`, `fire`, `cold`, `magic`, `poison`, `disease`, `corruption`) SELECT `BotID`, `BotOwnerCharacterID`, `BotSpellsID`, `Name`, `LastName`, `LastZoneId`, `Gender`, `Race`, `Class`, `BotLevel`, UNIX_TIMESTAMP(`BotCreateDate`), UNIX_TIMESTAMP(`LastSpawnDate`), `TotalPlayTime`, `Size`, `Face`, `LuclinHairColor`, `LuclinHairStyle`, `LuclinBeard`, `LuclinBeardColor`, `LuclinEyeColor`, `LuclinEyeColor2`, `DrakkinHeritage`, `DrakkinTattoo`, `DrakkinDetails`, `AC`, `ATK`, `HP`, `Mana`, `STR`, `STA`, `CHA`, `DEX`, `_INT`, `AGI`, `WIS`, `FR`, `CR`, `MR`, `PR`, `DR`, `Corrup` FROM `bots`;");
		
		print get_mysql_result("INSERT INTO `bot_inspect_messages` (`bot_id`, `inspect_message`) SELECT `BotID`, `BotInspectMessage` FROM `bots`;");
		
		print get_mysql_result("RENAME TABLE `bots` TO `bots_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'botstances'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_stances` (`bot_id`, `stance_id`) SELECT bs.`BotID`, bs.`StanceID` FROM `botstances` bs INNER JOIN `bot_data` bd ON bs.`BotID` = bd.`bot_id`;");
		
		print get_mysql_result("RENAME TABLE `botstances` TO `botstances_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'bottimers'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_timers` (`bot_id`, `timer_id`, `timer_value`) SELECT bt.`BotID`, bt.`TimerID`, bt.`Value` FROM `bottimers` bt INNER JOIN `bot_data` bd ON bt.`BotID` = bd.`bot_id`;");
		
		print get_mysql_result("RENAME TABLE `bottimers` TO `bottimers_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'botbuffs'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_buffs` (`buffs_index`, `bot_id`, `spell_id`, `caster_level`, `duration_formula`, `tics_remaining`, `poison_counters`, `disease_counters`, `curse_counters`, `corruption_counters`, `numhits`, `melee_rune`, `magic_rune`, `persistent`) SELECT bb.`BotBuffId`, bb.`BotId`, bb.`SpellId`, bb.`CasterLevel`, bb.`DurationFormula`, bb.`TicsRemaining`, bb.`PoisonCounters`, bb.`DiseaseCounters`, bb.`CurseCounters`, bb.`CorruptionCounters`, bb.`HitCount`, bb.`MeleeRune`, bb.`MagicRune`, bb.`Persistent` FROM `botbuffs` bb INNER JOIN `bot_data` bd ON bb.`BotId` = bd.`bot_id`;");
		
		if(get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'dot_rune'") ne "" && $db){
			print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`dot_rune` = bbo.`dot_rune` WHERE bb.`bot_id` = bbo.`BotID`;");
		}
		
		if(get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'caston_x'") ne "" && $db){
			print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`caston_x` = bbo.`caston_x` WHERE bb.`bot_id` = bbo.`BotID`;");
		}
		
		if(get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'caston_y'") ne "" && $db){
			print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`caston_y` = bbo.`caston_y` WHERE bb.`bot_id` = bbo.`BotID`;");
		}
		
		if(get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'caston_z'") ne "" && $db){
			print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`caston_z` = bbo.`caston_z` WHERE bb.`bot_id` = bbo.`BotID`;");
		}
		
		if(get_mysql_result("SHOW COLUMNS FROM `botbuffs` LIKE 'ExtraDIChance'") ne "" && $db){
			print get_mysql_result("UPDATE `bot_buffs` bb INNER JOIN `botbuffs` bbo ON bb.`buffs_index` = bbo.`BotBuffId` SET bb.`extra_di_chance` = bbo.`ExtraDIChance` WHERE bb.`bot_id` = bbo.`BotID`;");
		}
		
		print get_mysql_result("RENAME TABLE `botbuffs` TO `botbuffs_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'botinventory'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_inventories` (`inventories_index`, `bot_id`, `slot_id`, `item_id`, `inst_charges`, `inst_color`, `inst_no_drop`, `augment_1`, `augment_2`, `augment_3`, `augment_4`, `augment_5`) SELECT bi.`BotInventoryID`, bi.`BotID`, bi.`SlotID`, bi.`ItemID`, bi.`charges`, bi.`color`, bi.`instnodrop`, bi.`augslot1`, bi.`augslot2`, bi.`augslot3`, bi.`augslot4`, bi.`augslot5` FROM `botinventory` bi INNER JOIN `bot_data` bd ON bi.`BotID` = bd.`bot_id`;");
		
		if(get_mysql_result("SHOW COLUMNS FROM `botinventory` LIKE 'augslot6'") ne "" && $db){
			print get_mysql_result("UPDATE `bot_inventories` bi INNER JOIN `botinventory` bio ON bi.`inventories_index` = bio.`BotInventoryID` SET bi.`augment_6` = bio.`augslot6` 	WHERE bi.`bot_id` = bio.`BotID`;");
		}
		
		print get_mysql_result("RENAME TABLE `botinventory` TO `botinventory_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'botpets'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_pets` (`pets_index`, `pet_id`, `bot_id`, `name`, `mana`, `hp`) SELECT bp.`BotPetsId`, bp.`PetId`, bp.`BotId`, bp.`Name`, bp.`Mana`, bp.`HitPoints` FROM `botpets` bp INNER JOIN `bot_data` bd ON bp.`BotId` = bd.`bot_id`;");
		
		print get_mysql_result("RENAME TABLE `botpets` TO `botpets_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'botpetbuffs'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_pet_buffs` (`pet_buffs_index`, `pets_index`, `spell_id`, `caster_level`, `duration`) SELECT bpb.`BotPetBuffId`, bpb.`BotPetsId`, bpb.`SpellId`, bpb.`CasterLevel`, bpb.`Duration` FROM `botpetbuffs` bpb INNER JOIN `bot_pets` bp ON bpb.`BotPetsId` = bp.`pets_index`;");
		
		print get_mysql_result("RENAME TABLE `botpetbuffs` TO `botpetbuffs_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'botpetinventory'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_pet_inventories` (`pet_inventories_index`, `pets_index`, `item_id`) SELECT bpi.`BotPetInventoryId`, bpi.`BotPetsId`, bpi.`ItemId` FROM `botpetinventory` bpi INNER JOIN `bot_pets` bp ON bpi.`BotPetsId` = bp.`pets_index`;");
		
		print get_mysql_result("RENAME TABLE `botpetinventory` TO `botpetinventory_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'botgroup'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_groups` (`groups_index`, `group_leader_id`, `group_name`) SELECT bg.`BotGroupId`, bg.`BotGroupLeaderBotId`, bg.`BotGroupName` FROM  `botgroup` bg INNER JOIN `bot_data` bd ON bg.`BotGroupLeaderBotId` = bd.`bot_id`;");
		
		print get_mysql_result("RENAME TABLE `botgroup` TO `botgroup_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'botgroupmembers'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_group_members` (`group_members_index`, `groups_index`, `bot_id`) SELECT bgm.`BotGroupMemberId`, bgm.`BotGroupId`, bgm.`BotId` FROM `botgroupmembers` bgm INNER JOIN `bot_groups` bg ON bgm.`BotGroupId` = bg.`groups_index` INNER JOIN `bot_data` bd ON bgm.`BotId` = bd.`bot_id`;");
		
		print get_mysql_result("RENAME TABLE `botgroupmembers` TO `botgroupmembers_old`;");
	}
	
	if(get_mysql_result("SHOW TABLES LIKE 'botguildmembers'") ne "" && $db){
		print get_mysql_result("INSERT INTO `bot_guild_members` (`bot_id`, `guild_id`, `rank`, `tribute_enable`, `total_tribute`, `last_tribute`, `banker`, `public_note`, `alt`) SELECT bgm.`char_id`, bgm.`guild_id`, bgm.`rank`, bgm.`tribute_enable`, bgm.`total_tribute`, bgm.`last_tribute`, bgm.`banker`, bgm.`public_note`, bgm.`alt` FROM `botguildmembers` bgm INNER JOIN `guilds` g ON bgm.`guild_id` = g.`id` INNER JOIN `bot_data` bd ON bgm.`char_id` = bd.`bot_id`;");
		
		print get_mysql_result("RENAME TABLE `botguildmembers` TO `botguildmembers_old`;");
	}
}

sub get_bots_db_version{
	#::: Check if bots_version column exists...
	if(get_mysql_result("SHOW COLUMNS FROM db_version LIKE 'bots_version'") eq "" && $db){
	   print get_mysql_result("ALTER TABLE db_version ADD bots_version int(11) DEFAULT '0' AFTER version;");
	   print "\nColumn 'bots_version' does not exists.... Adding to 'db_version' table...\n\n";
	}
	$bots_local_db_version = trim(get_mysql_result("SELECT bots_version FROM db_version LIMIT 1"));
	return $bots_local_db_version;
}

sub bots_db_management{
	#::: Main Binary Database version
	$bin_db_ver = trim($db_version[2]);
	
	#::: If we have stale data from main db run
	if($db_run_stage > 0 && $bots_db_management == 0){
		clear_database_runs();
	}

	if($bin_db_ver == 0){
		print "Your server binaries (world/zone) are not compiled for bots...\n";
		return;
	}
	
	#::: Set on flag for running bot updates...
	$bots_db_management = 1;
	
	$bots_local_db_version = get_bots_db_version();
	
	run_database_check();
}

sub main_db_management{
	#::: If we have stale data from bots db run
	if($db_run_stage > 0 && $bots_db_management == 1){
		clear_database_runs();
	}

	#::: Main Binary Database version
	$bin_db_ver = trim($db_version[1]);
	
	$bots_db_management = 0;
	run_database_check();
}

sub clear_database_runs{
	# print "DEBUG :: clear_database_runs\n\n";
	#::: Clear manifest data...
	%m_d = ();
	#::: Clear updates...
	@total_updates = ();
	#::: Clear stage
	$db_run_stage = 0;
}

#::: Responsible for Database Upgrade Routines
sub run_database_check{ 

	if(!$db){
		print "No database present, check your eqemu_config.xml for proper MySQL/MariaDB configuration...\n";
		return;
	}
	
	if(!@total_updates){
		#::: Pull down bots database manifest
		if($bots_db_management == 1){
			print "Retrieving latest bots database manifest...\n";
			get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/git/bots/bots_db_update_manifest.txt", "db_update/db_update_manifest.txt"); 
		}
		#::: Pull down mainstream database manifest
		else{
			print "Retrieving latest database manifest...\n";
			get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/db_update_manifest.txt", "db_update/db_update_manifest.txt");
		}
	}

	#::: Run 2 - Running pending updates...
	if(@total_updates){
		@total_updates = sort @total_updates;
		foreach my $val (@total_updates){
			$file_name 		= trim($m_d{$val}[1]);
			print "Running Update: " . $val . " - " . $file_name . "\n";
			print get_mysql_result_from_file("db_update/$file_name");
			print get_mysql_result("UPDATE db_version SET version = $val WHERE version < $val");
			
			if($bots_db_management == 1 && $val == 9000){
				modify_db_for_bots();
			}
		}
		$db_run_stage = 2;
	}
	#::: Run 1 - Initial checking of needed updates...
	else{
		print "Reading manifest...\n\n";
		use Data::Dumper;
		open (FILE, "db_update/db_update_manifest.txt");
		while (<FILE>) { 
			chomp;
			$o = $_;
			if($o=~/#/i){ next; }
			@manifest = split('\|', $o);
			$m_d{$manifest[0]} = [@manifest];
		}
		#::: Setting Manifest stage...
		$db_run_stage = 1;
	}
	
	@total_updates = ();
	
	#::: This is where we set checkpoints for where a database might be so we don't check so far back in the manifest...
	$revision_check = 1000;
	if(get_mysql_result("SHOW TABLES LIKE 'character_data'") ne ""){
		$revision_check = 9000;
	}
	
	#::: Iterate through Manifest backwards from binary version down to local version...
	for($i = $bin_db_ver; $i > $revision_check; $i--){ 
		if(!defined($m_d{$i}[0])){ next; } 
		
		$file_name 		= trim($m_d{$i}[1]);
		$query_check 	= trim($m_d{$i}[2]);
		$match_type 	= trim($m_d{$i}[3]);
		$match_text 	= trim($m_d{$i}[4]);
		
		#::: Match type update
		if($match_type eq "contains"){
			if(trim(get_mysql_result($query_check))=~/$match_text/i){
				print "Missing DB Update " . $i . " '" . $file_name . "' \n";
				fetch_missing_db_update($i, $file_name);
				push(@total_updates, $i);
			}
			else{
				print "DB up to date with: " . $i . " - '" . $file_name . "' \n";
			}
			print_match_debug();
			print_break();
		}
		if($match_type eq "missing"){
			if(get_mysql_result($query_check)=~/$match_text/i){  
				print "DB up to date with: " . $i . " - '" . $file_name . "' \n";
				next; 
			}
			else{
				print "Missing DB Update " . $i . " '" . $file_name . "' \n";
				fetch_missing_db_update($i, $file_name);
				push(@total_updates, $i);
			}
			print_match_debug();
			print_break();
		}
		if($match_type eq "empty"){
			if(get_mysql_result($query_check) eq ""){
				print "Missing DB Update " . $i . " '" . $file_name . "' \n";
				fetch_missing_db_update($i, $file_name);
				push(@total_updates, $i);
			}
			else{
				print "DB up to date with: " . $i . " - '" . $file_name . "' \n";
			}
			print_match_debug();
			print_break();
		}
		if($match_type eq "not_empty"){
			if(get_mysql_result($query_check) ne ""){
				print "Missing DB Update " . $i . " '" . $file_name . "' \n";
				fetch_missing_db_update($i, $file_name);
				push(@total_updates, $i);
			}
			else{
				print "DB up to date with: " . $i . " - '" . $file_name . "' \n";
			}
			print_match_debug();
			print_break();
		}
	}
	print "\n"; 
	
	if(scalar (@total_updates) == 0 && $db_run_stage == 2){
		print "No updates need to be run...\n";
		if($bots_db_management == 1){
			print "Setting Database to Bots Binary Version (" . $bin_db_ver . ") if not already...\n\n";
			get_mysql_result("UPDATE db_version SET bots_version = $bin_db_ver"); 
		}
		else{
			print "Setting Database to Binary Version (" . $bin_db_ver . ") if not already...\n\n";
			get_mysql_result("UPDATE db_version SET version = $bin_db_ver"); 
		}
		
		clear_database_runs();
	}
}

sub fetch_missing_db_update{
	$db_update = $_[0];
	$update_file = $_[1];
	if($db_update >= 9000){
		if($bots_db_management == 1){
			get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/git/bots/required/" . $update_file, "db_update/" . $update_file . "");
		}
		else{
			get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/git/required/" . $update_file, "db_update/" . $update_file . "");
		}
	}
	elsif($db_update >= 5000 && $db_update <= 9000){
		get_remote_file("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/svn/" . $update_file, "db_update/" . $update_file . "");
	}
}

sub print_match_debug{ 
	if(!$debug){ return; }
	print "	Match Type: '" . $match_type . "'\n";
	print "	Match Text: '" . $match_text . "'\n";
	print "	Query Check: '" . $query_check . "'\n";
	print "	Result: '" . trim(get_mysql_result($query_check)) . "'\n";
}
sub print_break{ 
	if(!$debug){ return; } 
	print "\n==============================================\n"; 
}
