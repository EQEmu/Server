#!/usr/bin/perl

############################################################
#::: Script: db_dumper.pl
#::: Purpose: Utility to easily manage database backups and compress.
#:::	Export Individual DB Tables...
#:::	Export specific databases...
#::: 	Built for both Windows and Linux
#::: Windows uses WinRar or 7-Zip for compression
#::: Linux uses tar for compression
#::: Author: Akkadius
############################################################ 

$localdrive = "C:"; #::: Where Windows and all Install Programs are...
$linesep = "---------------------------------------";

use POSIX qw(strftime);
my $date = strftime "%m_%d_%Y", localtime;
print "\nTodays Date: " . $date . "\n";

use Config;
print "Operating System is: $Config{osname}\n";
if($Config{osname}=~/linux/i){ $OS = "Linux"; }
if($Config{osname}=~/Win|MS/i){ $OS = "Windows"; }

if(!$ARGV[0]){
	print "\nERROR! Need arguments\n";
	print "#::: Help :::#\n";
	print "######################################################\n";
	print "Arguments\n";
	print "	loc=\"C:\\File Location\" 	- File path location to backup...\n";
	print "	database=\"dbname\" 	- Manually specify databasename, default is database in eqemu_config.xml\n";
	print "	tables=\"table1,table2,table3\" 	- Manually specify tables, default is to dump all tables from database\n";
	print "	compress 		- Compress Database with 7-ZIP, will fallback to WinRAR depending on what is installed (Must be installed to default program dir)...\n";
	print "	nolock 		- Does not lock tables, meant for backuping while the server is running..\n";
	print "	backup_name=\"name\" - Sets database backup prefix name\n";
	print '	Example: perl DB_Dumper.pl Loc="E:\Backups"' . "\n";
	print "######################################################\n";
	exit;
}

sub read_eqemu_config_json {
	use JSON;
	my $json = new JSON();

	my $content;
	open(my $fh, '<', "eqemu_config.json") or die "Unable to open config: eqemu_config.json - This must be in your EQEmu Server Folder\n"; {
		local $/;
		$content = <$fh>;
	}
	close($fh);

	$config = $json->decode($content);
	
	$db = $config->{"server"}{"database"}{"db"};
	$host = $config->{"server"}{"database"}{"host"};
	$user = $config->{"server"}{"database"}{"username"};
	$pass = $config->{"server"}{"database"}{"password"};
	$long_name = $config->{"server"}{"world"}{"longname"};

}

read_eqemu_config_json();

$Debug = 0;
print "[db_dumper.pl] Arguments\n" if $Debug;
$n = 0;
while($ARGV[$n]){
	print $n . ': ' . $ARGV[$n] . "\n" if $Debug;
	if($ARGV[$n]=~/nolock/i){ 
		$no_lock = 1;
	}
	if($ARGV[$n]=~/compress/i){ 
		print "[db_dumper.pl] Compression SET\n"; 
		$Compress = 1;
	}
	if($ARGV[$n]=~/database=/i){ 
		@DB_NAME = split('=', $ARGV[$n]); 
		print "[db_dumper.pl] Database is " . $DB_NAME[1] . "\n"; 
		$db = $DB_NAME[1];
	}
	if($ARGV[$n]=~/backup_name=/i){ 
		@data = split('=', $ARGV[$n]); 
		print "[db_dumper.pl] Backup Name is " . $data[1] . "\n"; 
		$backup_name = $data[1];
	}
	if($ARGV[$n]=~/loc=/i){ 
		@backup_location = split('=', $ARGV[$n]); 
		print "[db_dumper.pl] Backup Directory: " . $backup_location[1] . "\n"; 
	}
	if($ARGV[$n]=~/tables=/i){ 
		@Tables = split('=', $ARGV[$n]); @TList = split(',', $Tables[1]);
		foreach my $tables (@TList){
			$t_tables .= $tables . " ";
			$t_tables_l .= $tables . "_";
			$t_tables_p .= $tables . "\n";
		}
		print "[db_dumper.pl] Backing up tables: \n############################\n" . $t_tables_p . "############################\n"; 
	}
	$n++;
}

#::: Check for Backup Directory existence, if doesn't exist then create...
if (-d $backup_location[1]) {
	print "[db_dumper.pl] Directory currently exists... Adding files to it...\n";
}
elsif($backup_location[1] ne ""){
	print "[db_dumper.pl] Directory does NOT exist! Creating...\n";
	mkdir($backup_location[1]) or die 'Failed to create folder, maybe created the folder manually at "' . $backup_location[1]. '" ?';
}
else{
	print "[db_dumper.pl] No save location specified... Saving to folder script is running in...\n";
}
if($backup_location[1] ne ""){ 
	if($OS eq "Windows"){ $file_app = "\\"; } 
	if($OS eq "Linux"){ $file_app = "/"; } 
} 
else { 
	$file_app = ""; 
}

if($t_tables ne ""){
	$tables_f_l = substr($t_tables_l, 0, 20) . '-';
	if($backup_name){
		$target_file = $backup_name . '_' . $date . ''; 
	}
	else {
		$target_file = '' . $tables_f_l . '_' . $date . ''; 
	}
	
	print "[db_dumper.pl] Performing table based backup...\n";
	#::: Backup Database... 
	print "[db_dumper.pl] Backing up Database " . $db . "... \n"; 
	if($no_lock == 1){
		$added_parameters .= " --skip-lock-tables ";
	}
	$cmd = 'mysqldump -u' . $user . ' --host ' . $host . ' ' . $added_parameters . ' --max_allowed_packet=512M --password="' . $pass . '" ' . $db . ' ' . $t_tables . ' > "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.sql"'; 
	printcmd($cmd);
	system($cmd); 
}
else{ #::: Entire DB Backup
	
	if($backup_name){
		$target_file = $backup_name . '_' . $db . '_' . $date . ''; 
	}
	else {
		$target_file = '' . $db . '_' . $date . ''; 
	}
	
	#::: Backup Database... 
	print "[db_dumper.pl] Backing up Database " . $db . "... \n";  
	if($no_lock == 1){
		$added_parameters .= " --skip-lock-tables ";
	}
	$cmd = 'mysqldump -u' . $user . ' --host ' . $host . ' ' . $added_parameters . ' --max_allowed_packet=512M --password="' . $pass . '" ' . $db . ' > "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.sql"'; 
	printcmd($cmd);
	system($cmd);
}

#::: Get File Size 
$fileloc = '' . $backup_location[1] . '' . $file_app . '' . $target_file . '.sql';
$filesize = -s $fileloc;
if($filesize < 1000){ print "[db_dumper.pl] " . 'Error occurred... exiting...' . "\n"; exit; }
print "[db_dumper.pl] Backup DONE... DB Backup File Size '" . $filesize . "' (" . get_filesize_str($fileloc) . ")\n"; 

#::: WinRar Get, check compression flag
if($Compress == 1){
	if($OS eq "Windows"){
		if(-d $localdrive . "\\Program Files\\7-Zip"){
			print "[db_dumper.pl]  ::: You have 7-Zip installed as 64 Bit...\n";
			$S_ZIP = $localdrive . "\\Program Files\\7-Zip";
		}
		elsif(-d $localdrive . "\\Program Files (x86)\\7-Zip"){
			print "[db_dumper.pl]  ::: You have 7-Zip installed as 32 Bit...\n";
			$S_ZIP = $localdrive . "\\Program Files (x86)\\7-Zip";
		}
		elsif(-d $localdrive . "\\Program Files (x86)\\WinRAR"){
			print "[db_dumper.pl]  ::: You have WinRAR installed as 32 Bit...\n";
			$WinRar = $localdrive . "\\Program Files (x86)\\WinRAR";
		}
		elsif(-d $localdrive . "\\Program Files\\WinRAR"){
			print "[db_dumper.pl]  ::: You have WinRAR installed as 64 Bit...\n";
			$WinRar = $localdrive . "\\Program Files\\WinRAR";
		}
		else{
			print "[db_dumper.pl] No WinRAR installed... Will not compress...\n";
		}
		if($S_ZIP ne ""){
			print "[db_dumper.pl] Compressing Database with 7-ZIP... \n"; 
			$cmd = '"' . $S_ZIP . '\\7z" a -t7z -m0=lzma -mx=9 "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.7z" "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.sql" ';
			printcmd($cmd);
			system($cmd); 
			print "[db_dumper.pl] \nDeleting RAW .sql Dump... \n"; 
			$cmd = 'del "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.sql" ';
			printcmd($cmd);
			system($cmd); 
			$final_file = $target_file . ".7z";
		}
		elsif($WinRar ne ""){
			print "[db_dumper.pl] Compressing Database with WinRAR... \n"; 
			$cmd = '"' . $WinRar . '\\rar" a "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.rar" "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.sql" ';
			printcmd($cmd);
			system($cmd); 
			print "[db_dumper.pl] \nDeleting RAW .sql Dump... \n"; 
			$cmd = 'del "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.sql" '; 
			printcmd($cmd);
			system($cmd); 
			$final_file = $target_file . ".rar";
		}
	}
	if($OS eq "Linux"){
		print "[db_dumper.pl] Compressing Database with Tarball... \n"; 
		$cmd = 'tar -zcvf "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.tar.gz" "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.sql" ';
		printcmd($cmd); 
		system($cmd);  
		print "[db_dumper.pl] \nDeleting RAW .sql Dump... \n"; 
		$cmd = 'rm "' . $backup_location[1] . '' . $file_app . '' . $target_file . '.sql" '; 
		printcmd($cmd);
		system($cmd); 
		$final_file = $target_file . ".tar.gz";
	}
}
else {
	$final_file = $target_file . ".sql";
}

#::: Get Final File Location for display
if($backup_location[1] ne ""){ $final_loc = $backup_location[1] . '' . $file_app . ""; }
else{ 
	if($OS eq "Windows"){
		$final_loc = `echo %cd%`;
	}
	elsif($OS eq "Linux"){
		$final_loc = `pwd`;
	}
}

print "[db_dumper.pl] Final file located: " . $final_loc . "" . $final_file . "\n"; 

sub printcmd{
	print "[db_dumper.pl] Command [" . $_[0] . "]\n";
}

sub get_filesize_str{
    my $file = shift();
    my $size = (stat($file))[7] || die "stat($file): $!\n";
    if ($size > 1099511627776)	{	return sprintf("%.2f TiB", $size / 1099511627776);  }
    elsif ($size > 1073741824)  {	return sprintf("%.2f GiB", $size / 1073741824);  }
    elsif ($size > 1048576) 	{   return sprintf("%.2f MiB", $size / 1048576);   }
    elsif ($size > 1024) 		{	return sprintf("%.2f KiB", $size / 1024);  }
    else  { return "$size byte" . ($size == 1 ? "" : "s"); }
}
