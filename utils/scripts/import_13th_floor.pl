#! /usr/bin/perl

########################################################################
#::: 13th floor import script
#::: Current Source: http://items.sodeq.org/download.php
#::: Authors: (Natedog, Akkadius)
########################################################################

use DBI;
use DBD::mysql;

my $database_name = "";
my $total_items = 0;
my $read_items_file = "items.txt"; #default
my $dbh = LoadMysql();

read_items_file_from_13th_floor_text();
update_items_table();

sub LoadMysql{	
	#::: Config Variables
	my $confile = "eqemu_config.xml";
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
	$database_name = $db;
	#::: DATA SOURCE NAME
	$dsn = "dbi:mysql:$db:localhost:3306";
	#::: PERL DBI CONNECT
	$connect = DBI->connect($dsn, $user, $pass);
	return $connect;
}

sub read_items_file_from_13th_floor_text {

	#::: Read from file and place into array
	open(F, "<" . $read_items_file) or die "Unable to open itemfile: " . $read_items_file . "\n";
	my @item_file_lines = <F>;
	close(F);

	#::: Chomp this array...
	my @newitem_file_lines;
	chomp($item_file_lines[0]); 
	@fields = split("(?<!\\\\)\\|", $item_file_lines[0]);
	
	my $sth = $dbh->prepare("SHOW TABLES LIKE 'items_floor'");
	$sth->execute();
	my $has_items_floor = $sth->fetchrow_array();
	
	#::: If we have items_floor
	if ($has_items_floor eq '') {
		$dbh->do("CREATE TABLE `items_floor` (`" . join("` VARCHAR(64) NOT NULL DEFAULT '', `", @fields). "` VARCHAR(64) NOT NULL DEFAULT '', UNIQUE INDEX `ID` (`id`)) COLLATE='latin1_swedish_ci' ENGINE=MyISAM");
		$dbh->do("ALTER TABLE `items_floor` CHANGE `id` `id` INT(11) NOT NULL DEFAULT '0'");
		printf "Database items_floor created\n";
	}
	
	#::: Create REPLACE INTO header and define worker variables...
	$master_insert = "REPLACE INTO `items_floor` (" . join(",", @fields) . ") VALUES ";
	$query_insert_ph = ""; #::: Used for building placeholder values in query Ex: (?, ?, ?)
	@field_values = (); #::: Used for stuffing mysql field values
	$query_count = 0; #::: Used for chunking query updates
	$print_cycle = 0; #::: Counter for console updates
	$start_time = time(); #::: Start time for import
	$total_items_file = scalar(grep $_, @item_file_lines) - 1; #::: Total items in text file

	#::: Iterate through each item in items.txt
	for (1 .. $#item_file_lines) {
		@f = split("(?<!\\\\)\\|", $item_file_lines[$_]);

		#::: Build our individual prepared statement (?, ?) values in the insert_ph
		#::: ?, ? placeholders will be resolved via @field_values in the execute
		$query_insert_ph .= " (";
		foreach (@f) {
			push (@field_values, trim($_));
			$query_insert_ph .= "?, ";
		}
		$query_insert_ph = substr($query_insert_ph, 0, -2);
		$query_insert_ph .= "), ";

		#::: Let's chunk our updates so we can break up the amount of individual queries
		if($query_count > 500){
			$query_insert_ph = substr($query_insert_ph, 0, -2);
			$dbh->prepare($master_insert . " " . $query_insert_ph)->execute(@field_values);
			$query_count = 0;
			$query_insert_ph = "";
			@field_values = ();
		}
		
		#::: Print updates to console
		if($print_cycle > 25){
			print "Processing (" . $read_items_file . ") :: (Items: " . $total_items . "/" . $total_items_file . ") 		\r";
			$print_cycle = 0;
		}

		#::: Counters
		$total_items++;
		$query_count++;
		$print_cycle++;
	}
	
	#::: One last processing print
	print "Processing (" . $read_items_file . ") :: (Items: " . $total_items . "/" . $total_items_file . ") 		\r";
	
	printf "\n" . $total_items . " items added to database... Took " . (time() - $start_time) . " second(s)... \n";
	
	print "Flipping slots 21 and 22...";
	$rows_affected = $dbh->prepare("
		UPDATE `items_floor` 
		SET `slots` = (`slots` ^ 6291456) 
		WHERE (`slots` & 6291456) 
		IN (2097152, 4194304)")->execute();
	print " Rows affected (" . $rows_affected . ")\n";
}

sub update_items_table {

	#::: Keep Items table sane
	$query_handle = $dbh->prepare("
		ALTER TABLE `items`
		MODIFY COLUMN `UNK132`  text CHARACTER SET utf8 COLLATE utf8_general_ci NULL;
	");
	$query_handle->execute();
	
	my @matching_table;
	my @missing_items_table;
	my @missing_items_floor_table;
	
	#::: Get columns from `items`
	my $sth = $dbh->prepare("SHOW COLUMNS FROM `items`;");
	$sth->execute();
	my @items_table;
	while (my @row = $sth->fetchrow_array()) {
		push(@items_table, $row[0]);
	}
	
	#::: Get columns from `items_floor`
	$sth2 = $dbh->prepare("SHOW COLUMNS FROM `items_floor`");
	$sth2->execute();
	my @items_floor_table;
	while (my @row = $sth2->fetchrow_array()) {
		push(@items_floor_table, $row[0]);
	}
	
	#::: Go through the original items table columns and line them up with what columns match on 13th floor
	#::: This is so we can use the matching columns to update and insert item data into `items` table
	foreach $value (@items_table) {
		if ( grep( /^$value$/i, @items_floor_table ) ) {
			push(@matching_table, $value);
		} else {
			#::: What values are we missing from EMU items table..
			push(@missing_items_table, $value);
		}
	}
	
	#::: What values are we missing from.. 13thFloor
	foreach $value (@items_floor_table) {
		if ( grep( /^$value$/i, @items_table ) ) {
			#DO NOTHING...
		} else {
			push(@missing_items_floor_table, $value);
		}
	}

	#::: Go through the matched columns and build our query strings...
	
	my $items_field_list = ""; #::: Build the field list for the INSERT (field1, field2)
	my $items_floor_field_list = ""; #::: What fields we will select from items_floor table to insert into items (matched columns)
	my $update_fields = ""; #::: To update an existing item entry if it exists...

	foreach $match (@matching_table) {
		$match = lc($match);
		$update_fields .= "`" . $match . "` = fi.`" . $match . "`, ";
		$items_field_list .= "`" . $match . "`, ";
		$items_floor_field_list .= "fi.`" . $match . "`, ";
	}
	#::: Trim ', ' off the ends
	$update_fields = substr($update_fields, 0, -2);
	$items_field_list = substr($items_field_list, 0, -2);
	$items_floor_field_list = substr($items_floor_field_list, 0, -2);
	
	#::: Mixed up fields...
	$items_floor_field_list =~ s/booktype/booklang/g; #our booktype is mixed with theirs...
	$update_fields =~ s/`booktype` = fi.`booktype`/`booktype` = fi.`booklang`/g;

	#::: FIELDS THAT DO NOT MATCH GO HERE
	my @items_add = (
		"casttime_", "endur", "range", "attuneable", "evolvinglevel", "herosforgemodel", "scrolltype",
		"scriptfileid", "powersourcecapacity", "augslot1unk2", "augslot2unk2", "augslot3unk2", "augslot4unk2", 
		"augslot5unk2", "augslot6unk2", "recskill", "book"
	);
	my @items_floor_add = (
		"foodduration", "endurance", "therange", "attunable", "evolvl", "heroforge1", "scrolleffecttype", 
		"rightclickscriptid", "powersourcecap",	   "augslot1unk", "augslot2unk", "augslot3unk", "augslot4unk", 
		"augslot5unk", "augslot6unk", "reqskill", "booktype"
	);
	
	#::: Match the mis-matched fields...
	my $spot = 0;
	foreach $value (@items_add) {
		$items_field_list .= ", `" . $value . "`";
		$update_fields .= ", `" . $value . "` = fi.`" . $items_floor_add[$spot] . "`";
		$spot++;
		@missing_items_table = grep {$_ ne $value} @missing_items_table;
	}
	foreach $value (@items_floor_add) {
		$items_floor_field_list .= ", fi.`" . $value . "`";
		@missing_items_floor_table = grep {$_ ne $value} @missing_items_floor_table;
	}
	
	my $update_query = "
		INSERT INTO items (" . $items_field_list . ") 
		SELECT " . $items_floor_field_list . "
		FROM items_floor fi 
		ON DUPLICATE KEY UPDATE " . $update_fields;
	
	#::: Print missing fields to file
	my $write_file = "missing_item_fields.txt";
	
	open(F, ">$write_file") or die "Unable to open questfile: $write_file\n";
	print F "$update_query \n\n";
	print F "EQEMU items Table missing fields\n";
	foreach $value (@missing_items_table) {
		print F "$value\n";
	}
	print F "\n\n13thFloor items Table missing fields\n";
	foreach $value (@missing_items_floor_table) {
		print F "$value\n";
	}
	close(F);
	
	#::: Number of rows affected by query
	$rows = $dbh->do($update_query);
	
	#::: Update stackables
	$dbh->do("UPDATE items i SET i.stackable = 1 WHERE i.stacksize > 1");
	
	print "Added all new items to Items table (" . $rows . ")!\n";
	
}

sub trim($) {
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}