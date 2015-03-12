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

$time_stamp = strftime('%m-%d-%Y', gmtime());

$console_output .= "	Operating System is: $Config{osname}\n";
if($Config{osname}=~/linux/i){ $OS = "Linux"; }
if($Config{osname}=~/Win|MS/i){ $OS = "Windows"; }

#::: If current version is less than what world is reporting, then download a new one...
$current_version = 7;

if($ARGV[0] eq "V"){
	if($ARGV[1] > $current_version){ 
		print "eqemu_update.pl Automatic Database Upgrade Needs updating...\n";
		print "	Current version: " . $current_version . "\n"; 
		print "	New version: " . $ARGV[1] . "\n";  
		GetRemoteFile("https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_update.pl", "eqemu_update.pl");
		exit;
	}
	else{
		print "[Upgrade Script] No script update necessary \n";
	}
	exit;
}

$perl_version = $^V;
$perl_version =~s/v//g;
print "Perl Version is " . $perl_version . "\n";
if($perl_version > 5.12){ no warnings 'uninitialized';  }
no warnings;

($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime();

#::: Cleanup staged folder...
rmtree("updates_staged/");

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
	print "Exiting...\n";
	exit;
}

#::: Create db_update working directory if not created
mkdir('db_update'); 

#::: Check if db_version table exists... 
if(trim(GetMySQLResult("SHOW COLUMNS FROM db_version LIKE 'Revision'")) ne "" && $db){
	print GetMySQLResult("DROP TABLE db_version");
	print "Old db_version table present, dropping...\n\n";
}

if(GetMySQLResult("SHOW TABLES LIKE 'db_version'") eq "" && $db){
	print GetMySQLResult("
		CREATE TABLE db_version (
		  version int(11) DEFAULT '0'
		) ENGINE=InnoDB DEFAULT CHARSET=latin1;
		INSERT INTO db_version (version) VALUES ('1000');");
	print "Table 'db_version' does not exists.... Creating...\n\n";
}

if($OS eq "Windows"){ @db_version = split(': ', `world db_version`); }
if($OS eq "Linux"){ @db_version = split(': ', `./world db_version`); }  

$bin_db_ver = trim($db_version[1]);
$local_db_ver = trim(GetMySQLResult("SELECT version FROM db_version LIMIT 1"));

#::: If ran from Linux startup script, supress output
if($bin_db_ver == $local_db_ver && $ARGV[0] eq "ran_from_start"){ 
	print "Database up to date...\n"; 
	exit; 
}
else{ 
	print $console_output if $db; 
}

if($db){
	print "	Binary Database Version: (" . $bin_db_ver . ")\n";
	print "	Local Database Version: (" . $local_db_ver . ")\n\n";

	#::: If World ran this script, and our version is up to date, continue...
	if($bin_db_ver <= $local_db_ver && $ARGV[0] eq "ran_from_world"){  
		print "	Database up to Date: Continuing World Bootup...\n";
		print "============================================================\n";
		exit; 
	}

	print "Retrieving latest database manifest...\n";
	GetRemoteFile("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/db_update_manifest.txt", "db_update/db_update_manifest.txt");
}

if($local_db_ver < $bin_db_ver && $ARGV[0] eq "ran_from_world"){
	print "You have missing database updates, type 1 or 2 to backup your database before running them as recommended...\n\n";
	#::: Display Menu 
	ShowMenuPrompt();
}
else{
	#::: Most likely ran standalone
	print "\n";
	ShowMenuPrompt();
}

sub UpdateSelf{
	GetRemoteFile("https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_update.pl", "eqemu_update.pl");
	die "Rerun eqemu_update.pl";
}

sub ShowMenuPrompt {
    my %dispatch = (
        1 => \&database_dump,
        2 => \&database_dump_compress,
        3 => \&Run_Database_Check,
        4 => \&AA_Fetch,
        5 => \&OpCodes_Fetch,
        6 => \&MapFiles_Fetch,
        7 => \&Plugins_Fetch,
        8 => \&QuestFiles_Fetch,
        9 => \&LUA_Modules_Fetch,
        20 => \&UpdateSelf,
        0 => \&Exit,
    );

    while (1) { 
		{
			local $| = 1;
			if(!$menu_show && ($ARGV[0] eq "ran_from_world" || $ARGV[0] eq "ran_from_start")){ 
				$menu_show++;
				next;
			}
			print MenuOptions(), '> ';
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

sub MenuOptions {

	if(@total_updates){ 
		$option[3] = "Run pending REQUIRED updates... (" . scalar (@total_updates) . ")";
	}
	else{
		$option[3] = "Check and stage pending REQUIRED Database updates";
	}

return <<EO_MENU;
EQEmu Update Utility Menu:
	1) Backup Database - (Saves to Backups folder)
	2) Backup Database Compressed - (Saves to Backups folder)
	3) $option[3]
	4) AAs - Download Latest AA's from PEQ (This overwrites existing data)
	5) OPCodes - Download latest opcodes
	6) Maps - Download latest map and water files
	7) Plugins - Download latest Perl plugins
	8) Quests - Download latest PEQ quests and stage updates
	9) LUA Modules - Download latest LUA Modules (Required for Lua)
	20) Force update this script (Redownload)
	0) Exit
	
	Enter numbered option and press enter...	
	
EO_MENU
}

sub CheckForDatabaseDumpScript{
	if(`perl db_dumper.pl`=~/Need arguments/i){
		return; 
	}
	else{
		print "db_dumper.pl not found... retrieving...\n\n";
		GetRemoteFile("https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/db_dumper.pl", "db_dumper.pl");
	}
}

sub ran_from_world { 
	print "Running from world...\n";
}

sub database_dump { 
	CheckForDatabaseDumpScript();
	print "Performing database backup....\n";
	print `perl db_dumper.pl database="$db" loc="backups"`;
}
sub database_dump_compress { 
	CheckForDatabaseDumpScript();
	print "Performing database backup....\n";
	print `perl db_dumper.pl database="$db"  loc="backups" compress`;
}
sub Exit{ }

#::: Returns Tab Delimited MySQL Result from Command Line
sub GetMySQLResult{
	my $run_query = $_[0];
	if(!$db){ return; }
	if($OS eq "Windows"){ return `"$path" --host $host --user $user --password="$pass" $db -N -B -e "$run_query"`; }
	if($OS eq "Linux"){ 
		$run_query =~s/`//g;
		return `$path --user="$user" --host $host --password="$pass" $db -N -B -e "$run_query"`; 
	}
}

sub GetMySQLResultFromFile{
	my $update_file = $_[0];
	if(!$db){ return; }
	if($OS eq "Windows"){ return `"$path" --host $host --user $user --password="$pass" --force $db < $update_file`;  }
	if($OS eq "Linux"){ return `"$path" --host $host --user $user --password="$pass" --force $db < $update_file`;  }
}

#::: Gets Remote File based on URL (1st Arg), and saves to destination file (2nd Arg)
#::: Example: GetRemoteFile("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/db_update_manifest.txt", "db_update/db_update_manifest.txt");
sub GetRemoteFile{
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
			use LWP::Simple qw(getstore);
			if(!getstore($URL, $Dest_File)){
				print "Error, no connection or failed request...\n\n";
			}
			else{
				print " o URL: (" . $URL . ")\n";
				print " o Saved: (" . $Dest_File . ") \n";
			}
		}
		else{
			require LWP::UserAgent; 
			my $ua = LWP::UserAgent->new;
			$ua->timeout(10);
			$ua->env_proxy; 
			my $response = $ua->get($URL);

			if ($response->is_success){
				open (FILE, '> ' . $Dest_File . '');
				print FILE $response->decoded_content;
				close (FILE); 
				print " o URL: (" . $URL . ")\n";
				print " o Saved: (" . $Dest_File . ") \n";
			}
			else {
				print "Error, no connection or failed request...\n\n";
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
sub AA_Fetch{
	if(!$db){
		print "No database present, check your eqemu_config.xml for proper MySQL/MariaDB configuration...\n";
		return;
	}

	print "Pulling down PEQ AA Tables...\n";
	GetRemoteFile("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/peq_aa_tables.sql", "db_update/peq_aa_tables.sql");
	print "\n\nInstalling AA Tables...\n";
	print GetMySQLResultFromFile("db_update/peq_aa_tables.sql");
	print "\nDone...\n\n";
}

#::: Fetch Latest Opcodes
sub OpCodes_Fetch{
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
		GetRemoteFile($opcodes{$loop}[1], $file_name);
		$loop++; 
	}
	print "\nDone...\n\n";
}

sub CopyFile{
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

sub MapFiles_Fetch{
	print "\n --- Fetching Latest Maps --- \n";
	
	GetRemoteFile("https://raw.githubusercontent.com/Akkadius/EQEmuMaps/master/!eqemu_maps_manifest.txt", "updates_staged/eqemu_maps_manifest.txt");
	
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
			GetRemoteFile("https://raw.githubusercontent.com/Akkadius/EQEmuMaps/master/" .  $maps_manifest[$m][0], $maps_manifest[$m][0], 1);
			$fc++;
		}
	}
	
	if($fc == 0){
		print "\nNo Map Updates found... \n\n";
	}
}

sub QuestFiles_Fetch{
	if (!-e "updates_staged/Quests-Plugins-master/quests/") {
		print "\n --- Fetching Latest Quests --- \n";
		GetRemoteFile("https://github.com/EQEmu/Quests-Plugins/archive/master.zip", "updates_staged/Quests-Plugins-master.zip", 1);
		print "\nFetched latest quests...\n";
		mkdir('updates_staged');
		UnZip('updates_staged/Quests-Plugins-master.zip', 'updates_staged/');
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
				CopyFile($staged_file, $dest_file);
				print "Installing :: '" . $dest_file . "'\n";
				$fc++;
			}
			else{
				$diff = Diff($dest_file, $staged_file);
				if($diff ne ""){
					$backup_dest = "updates_backups/" . $time_stamp . "/" . $dest_file;
				
					print $diff . "\n";
					print "\nFile Different :: '" . $dest_file . "'\n";
					print "\nDo you wish to update this Quest? '" . $dest_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
					my $input = <STDIN>;
					if($input=~/N/i){}
					else{
						#::: Make a backup
						CopyFile($dest_file, $backup_dest);
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
		print "\nNo Quest Updates found... \n\n";
	}
}

sub LUA_Modules_Fetch{
	if (!-e "updates_staged/Quests-Plugins-master/quests/lua_modules/") {
		print "\n --- Fetching Latest LUA Modules --- \n";
		GetRemoteFile("https://github.com/EQEmu/Quests-Plugins/archive/master.zip", "updates_staged/Quests-Plugins-master.zip", 1);
		print "\nFetched latest LUA Modules...\n";
		UnZip('updates_staged/Quests-Plugins-master.zip', 'updates_staged/');
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
				CopyFile($staged_file, $dest_file);
				print "Installing :: '" . $dest_file . "'\n";
				$fc++;
			}
			else{
				$diff = Diff($dest_file, $staged_file);
				if($diff ne ""){
					$backup_dest = "updates_backups/" . $time_stamp . "/" . $dest_file;
					print $diff . "\n";
					print "\nFile Different :: '" . $dest_file . "'\n";
					print "\nDo you wish to update this LUA Module? '" . $dest_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
					my $input = <STDIN>;
					if($input=~/N/i){}
					else{
						#::: Make a backup
						CopyFile($dest_file, $backup_dest);
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

sub Plugins_Fetch{
	if (!-e "updates_staged/Quests-Plugins-master/plugins/") {
		print "\n --- Fetching Latest Plugins --- \n";
		GetRemoteFile("https://github.com/EQEmu/Quests-Plugins/archive/master.zip", "updates_staged/Quests-Plugins-master.zip", 1);
		print "\nFetched latest plugins...\n";
		UnZip('updates_staged/Quests-Plugins-master.zip', 'updates_staged/');
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
				CopyFile($staged_file, $dest_file);
				print "Installing :: '" . $dest_file . "'\n";
				$fc++;
			}
			else{
				$diff = Diff($dest_file, $staged_file);
				if($diff ne ""){
					$backup_dest = "updates_backups/" . $time_stamp . "/" . $dest_file;
					print $diff . "\n";
					print "\nFile Different :: '" . $dest_file . "'\n";
					print "\nDo you wish to update this Plugin? '" . $dest_file . "' [Yes (Enter) - No (N)] \nA backup will be found in '" . $backup_dest . "'\n";
					my $input = <STDIN>;
					if($input=~/N/i){}
					else{
						#::: Make a backup
						CopyFile($dest_file, $backup_dest);
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

sub Diff{
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

sub UnZip{
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

sub AreFileSizesDifferent{
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

#::: Responsible for Database Upgrade Routines
sub Run_Database_Check{ 

	if(!$db){
		print "No database present, check your eqemu_config.xml for proper MySQL/MariaDB configuration...\n";
		return;
	}

	#::: Run 2 - Running pending updates...
	if(defined(@total_updates)){
		@total_updates = sort @total_updates;
		foreach my $val (@total_updates){
			$file_name 		= trim($m_d{$val}[1]);
			print "Running Update: " . $val . " - " . $file_name . "\n";
			print GetMySQLResultFromFile("db_update/$file_name");
			print GetMySQLResult("UPDATE db_version SET version = $val WHERE version < $val");
		} 
	} 
	
	#::: Run 1 - Initial checking of needed updates...
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
	
	@total_updates = ();
	
	#::: Iterate through Manifest backwards from binary version down to local version...
	for($i = $bin_db_ver; $i > 1000; $i--){ 
		if(!defined($m_d{$i}[0])){ next; } 
		
		$file_name 		= trim($m_d{$i}[1]);
		$query_check 	= trim($m_d{$i}[2]);
		$match_type 	= trim($m_d{$i}[3]);
		$match_text 	= trim($m_d{$i}[4]);
		
		#::: Match type update
		if($match_type eq "contains"){
			if(trim(GetMySQLResult($query_check))=~/$match_text/i){
				print "Missing DB Update " . $i . " '" . $file_name . "' \n";
				FetchMissingUpdate($i, $file_name);
				push(@total_updates, $i);
			}
			else{
				print "DB up to date with: " . $i . " - '" . $file_name . "' \n";
			}
			print_match_debug();
			print_break();
		}
		if($match_type eq "missing"){
			if(GetMySQLResult($query_check)=~/$match_text/i){  
				print "DB up to date with: " . $i . " - '" . $file_name . "' \n";
				next; 
			}
			else{
				print "Missing DB Update " . $i . " '" . $file_name . "' \n";
				FetchMissingUpdate($i, $file_name);
				push(@total_updates, $i);
			}
			print_match_debug();
			print_break();
		}
		if($match_type eq "empty"){
			if(GetMySQLResult($query_check) eq ""){
				print "Missing DB Update " . $i . " '" . $file_name . "' \n";
				FetchMissingUpdate($i, $file_name);
				push(@total_updates, $i);
			}
			else{
				print "DB up to date with: " . $i . " - '" . $file_name . "' \n";
			}
			print_match_debug();
			print_break();
		}
		if($match_type eq "not_empty"){
			if(GetMySQLResult($query_check) ne ""){
				print "Missing DB Update " . $i . " '" . $file_name . "' \n";
				FetchMissingUpdate($i, $file_name);
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
	
	if(scalar (@total_updates) == 0){
		print "No updates need to be run...\n";
		print "Setting Database to Binary Version (" . $bin_db_ver . ") if not already...\n\n";
		GetMySQLResult("UPDATE db_version SET version = $bin_db_ver"); 
	}
}

sub FetchMissingUpdate{
	$db_update = $_[0];
	$update_file = $_[1];
	if($db_update >= 9000){ 
		GetRemoteFile("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/git/required/" . $update_file, "db_update/" . $update_file . "");
	}
	elsif($db_update >= 5000 && $db_update <= 9000){
		GetRemoteFile("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/svn/" . $update_file, "db_update/" . $update_file . "");
	}
}

sub print_match_debug{ 
	if(!$debug){ return; }
	print "	Match Type: '" . $match_type . "'\n";
	print "	Match Text: '" . $match_text . "'\n";
	print "	Query Check: '" . $query_check . "'\n";
	print "	Result: '" . trim(GetMySQLResult($query_check)) . "'\n";
}
sub print_break{ 
	if(!$debug){ return; } 
	print "\n==============================================\n"; 
}
