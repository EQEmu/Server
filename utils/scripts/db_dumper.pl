#!/usr/bin/perl

############################################################
#::: Script: DB_Dumper.pl
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
	print "\nERROR! Need arguments\n\n";
	print "#::: Help :::#\n";
	print "######################################################\n\n";
	print "Arguments\n";
	print "	loc=\"C:\\File Location\" 	- File path location to backup...\n";
	print "	database=\"dbname\" 	- Manually specify databasename, default is database in eqemu_config.xml\n";
	print "	tables=\"table1,table2,table3\" 	- Manually specify tables, default is to dump all tables from database\n";
	print "	compress 		- Compress Database with 7-ZIP, will fallback to WinRAR depending on what is installed (Must be installed to default program dir)...\n";
	print "	nolock 		- Does not lock tables, meant for backuping while the server is running..\n";
	print '	Example: perl DB_Dumper.pl Loc="E:\Backups"' . "\n\n";
	print "######################################################\n";
	exit;
}

#::: CONFIG VARIABLES - Parsed from eqemu_config.xml 

my $confile = "eqemu_config.xml"; #default
open(F, "<$confile") or die "Unable to open config: $confile - This must be in your EQEmu Server Folder with your XML config\n";
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

$Debug = 0;
print "Arguments\n" if $Debug;
$n = 0;
while($ARGV[$n]){
	print $n . ': ' . $ARGV[$n] . "\n" if $Debug;
	if($ARGV[$n]=~/nolock/i){ 
		$no_lock = 1;
	}
	if($ARGV[$n]=~/compress/i){ 
		print "Compression SET\n"; 
		$Compress = 1;
	}
	if($ARGV[$n]=~/database=/i){ 
		@DB_NAME = split('=', $ARGV[$n]); 
		print "Database is " . $DB_NAME[1] . "\n"; 
		$db = $DB_NAME[1];
	}
	if($ARGV[$n]=~/loc=/i){ 
		@B_LOC = split('=', $ARGV[$n]); 
		print "Backup Directory: " . $B_LOC[1] . "\n"; 
	}
	if($ARGV[$n]=~/tables=/i){ 
		@Tables = split('=', $ARGV[$n]); @TList = split(',', $Tables[1]);
		foreach my $tables (@TList){
			$t_tables .= $tables . " ";
			$t_tables_l .= $tables . "_";
			$t_tables_p .= $tables . "\n";
		}
		print "Backing up tables: \n\n############################\n" . $t_tables_p . "############################\n\n"; 
	}
	$n++;
}

#::: Check for Backup Directory existence, if doesn't exist then create...
if (-d $B_LOC[1]) {
	print "Directory currently exists... Adding files to it...\n\n";
}
elsif($B_LOC[1] ne ""){
	print "Directory does NOT exist! Creating...\n\n";
	mkdir($B_LOC[1]) or die 'Failed to create folder, maybe created the folder manually at "' . $B_LOC[1]. '" ?';
}
else{
	print "No save location specified... Saving to folder script is running in...\n";
}
if($B_LOC[1] ne ""){ 
	if($OS eq "Windows"){ $file_app = "\\"; } 
	if($OS eq "Linux"){ $file_app = "/"; } 
} 
else { 
	$file_app = ""; 
}

if($t_tables ne ""){
	$tables_f_l = substr($t_tables_l, 0, 20) . '...';
	$target_file = '' . $tables_f_l . '_' . $date . ''; 
	print "Performing table based backup...\n";
	#::: Backup Database... 
	print "Backing up Database " . $db . "... \n\n"; 
	if($no_lock == 1){
		$added_parameters .= " --skip-lock-tables ";
	}
	$cmd = 'mysqldump -u' . $user . ' --host ' . $host . ' ' . $added_parameters . ' --max_allowed_packet=512M --password="' . $pass . '" ' . $db . ' ' . $t_tables . ' > "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.sql"'; 
	printcmd($cmd);
	system($cmd); 
}
else{ #::: Entire DB Backup
	$target_file = '' . $db . '_' . $date . ''; 
	#::: Backup Database... 
	print "Backing up Database " . $db . "... \n\n";  
	if($no_lock == 1){
		$added_parameters .= " --skip-lock-tables ";
	}
	$cmd = 'mysqldump -u' . $user . ' --host ' . $host . ' ' . $added_parameters . ' --max_allowed_packet=512M --password="' . $pass . '" ' . $db . ' > "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.sql"'; 
	printcmd($cmd);
	system($cmd);
}

#::: Get File Size 
$fileloc = '' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.sql';
$filesize = -s $fileloc;
if($filesize < 1000){ print "\n" . 'Error occurred... exiting...' . "\n\n"; exit; }
print "Backup DONE... DB Backup File Size '" . $filesize . "' (" . get_filesize_str($fileloc) . ")\n\n"; 

#::: WinRar Get, check compression flag
if($Compress == 1){
	if($OS eq "Windows"){
		if(-d $localdrive . "\\Program Files\\7-Zip"){
			print " ::: You have 7-Zip installed as 64 Bit...\n\n";
			$S_ZIP = $localdrive . "\\Program Files\\7-Zip";
		}
		elsif(-d $localdrive . "\\Program Files (x86)\\7-Zip"){
			print " ::: You have 7-Zip installed as 32 Bit...\n\n";
			$S_ZIP = $localdrive . "\\Program Files (x86)\\7-Zip";
		}
		elsif(-d $localdrive . "\\Program Files (x86)\\WinRAR"){
			print " ::: You have WinRAR installed as 32 Bit...\n\n";
			$WinRar = $localdrive . "\\Program Files (x86)\\WinRAR";
		}
		elsif(-d $localdrive . "\\Program Files\\WinRAR"){
			print " ::: You have WinRAR installed as 64 Bit...\n\n";
			$WinRar = $localdrive . "\\Program Files\\WinRAR";
		}
		else{
			print "No WinRAR installed... Will not compress...\n";
		}
		if($S_ZIP ne ""){
			print "Compressing Database with 7-ZIP... \n\n"; 
			$cmd = '"' . $S_ZIP . '\\7z" a -t7z -m0=lzma -mx=9 "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.7z" "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.sql" ';
			printcmd($cmd);
			system($cmd); 
			print "\nDeleting RAW .sql Dump... \n\n"; 
			$cmd = 'del "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.sql" ';
			printcmd($cmd);
			system($cmd); 
			$final_file = $target_file . ".7z";
		}
		elsif($WinRar ne ""){
			print "Compressing Database with WinRAR... \n"; 
			$cmd = '"' . $WinRar . '\\rar" a "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.rar" "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.sql" ';
			printcmd($cmd);
			system($cmd); 
			print "\nDeleting RAW .sql Dump... \n\n"; 
			$cmd = 'del "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.sql" '; 
			printcmd($cmd);
			system($cmd); 
			$final_file = $target_file . ".rar";
		}
	}
	if($OS eq "Linux"){
		print "Compressing Database with Tarball... \n"; 
		$cmd = 'tar -zcvf "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.tar.gz" "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.sql" ';
		printcmd($cmd); 
		system($cmd);  
		print "\nDeleting RAW .sql Dump... \n\n"; 
		$cmd = 'rm "' . $B_LOC[1] . '' . $file_app . '' . $target_file . '.sql" '; 
		printcmd($cmd);
		system($cmd); 
		$final_file = $target_file . ".tar.gz";
	}
}

#::: Get Final File Location for display
if($B_LOC[1] ne ""){ $final_loc = $B_LOC[1] . '' . $file_app . ""; }
else{ 
	if($OS eq "Windows"){
		$final_loc = `echo %cd%`;
	}
	elsif($OS eq "Linux"){
		$final_loc = `pwd`;
	}
}

print "Final file located: " . $final_loc . "" . $final_file . "\n\n"; 

sub printcmd{
	print "--- CMD --- \n" . $_[0] . "\n" . $linesep . "\n\n";
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
