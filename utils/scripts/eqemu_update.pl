#!/usr/bin/perl

###########################################################
#::: Automatic (Database) Upgrade Script
#::: Author: Akkadius
#::: Purpose: To upgrade databases with ease and maintain versioning
###########################################################

$menu_displayed = 0;

use Config;
$console_output .= "	Operating System is: $Config{osname}\n";
if($Config{osname}=~/linux/i){ $OS = "Linux"; }
if($Config{osname}=~/Win|MS/i){ $OS = "Windows"; }

#::: If current version is less than what world is reporting, then download a new one...
$current_version = 2;

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

my $confile = "eqemu_config.xml"; #default
open(F, "<$confile") or die "Unable to open config: $confile\n";
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
if(trim(GetMySQLResult("SHOW COLUMNS FROM db_version LIKE 'Revision'")) ne ""){
	print GetMySQLResult("DROP TABLE db_version");
	print "Old db_version table present, dropping...\n\n";
}

if(GetMySQLResult("SHOW TABLES LIKE 'db_version'") eq ""){
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
	print $console_output; 
}


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
# GetRemoteFile("https://dl.dropboxusercontent.com/u/50023467/dl/db_update_manifest.txt", "db_update/db_update_manifest.txt");

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


sub ShowMenuPrompt {
    my %dispatch = (
        1 => \&database_dump,
        2 => \&database_dump_compress,
        3 => \&Run_Database_Check,
        4 => \&AA_Fetch,
        5 => \&OpCodes_Fetch,
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
		$option[3] = "Check for pending REQUIRED Database updates 
		Stages updates for automatic upgrade...";
	}

return <<EO_MENU;
Database Management Menu (Please Select):
	1) Backup Database - (Saves to Backups folder) 
		Ideal to perform before performing updates
	2) Backup Database Compressed - (Saves to Backups folder)
		Ideal to perform before performing updates
	3) $option[3]
	4) AAs - Get Latest AA's from PEQ (This deletes AA's already in the database)
	5) OPCodes - Download latest opcodes from repository
	0) Exit
	
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
	if($OS eq "Windows"){ return `"$path" --host $host --user $user --password="$pass" $db -N -B -e "$run_query"`; }
	if($OS eq "Linux"){ 
		$run_query =~s/`//g;
		return `$path --user="$user" --host $host --password="$pass" $db -N -B -e "$run_query"`; 
	}
}

sub GetMySQLResultFromFile{
	my $update_file = $_[0];
	if($OS eq "Windows"){ return `"$path" --host $host --user $user --password="$pass" --force $db < $update_file`;  }
	if($OS eq "Linux"){ return `"$path" --host $host --user $user --password="$pass" --force $db < $update_file`;  }
}

#::: Gets Remote File based on URL (1st Arg), and saves to destination file (2nd Arg)
#::: Example: GetRemoteFile("https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/db_update_manifest.txt", "db_update/db_update_manifest.txt");
sub GetRemoteFile{
	my $URL = $_[0];
	my $Dest_File = $_[1];
	
	if($OS eq "Windows"){ 
		require LWP::UserAgent; 
		my $ua = LWP::UserAgent->new;
		$ua->timeout(10);
		$ua->env_proxy; 
		my $response = $ua->get($URL);

		if ($response->is_success){
			open (FILE, '> ' . $Dest_File . '');
			print FILE $response->decoded_content;
			close (FILE); 
			print "	URL:	" . $URL . "\n";
			print "	Saved:	" . $Dest_File . " \n";
		}
		else {
			print "Error, no connection to the internet...\n\n";
			die $response->status_line;
		}
	}
	if($OS eq "Linux"){ 
		#::: wget -O db_update/db_update_manifest.txt https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/db_update_manifest.txt
		$wget = `wget --no-check-certificate --quiet -O $Dest_File $URL`;
		print "	URL:	" . $URL . "\n";
		print "	Saved:	" . $Dest_File . " \n";
		if($wget=~/unable to resolve/i){
			print "Error, no connection to the internet...\n\n";
			die;
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

#::: Responsible for Database Upgrade Routines
sub Run_Database_Check{ 
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
