#!/usr/bin/perl

#use strict;
use DBI;
use Getopt::Std;

getopts('i:d:h');
$dbini = "db.ini";
if($opt_h) {
	die "Usage: serialize_items.pl [-i path/to/item_fieldlist.h] [-d path/to/db.ini]\n";
}
$itemfields = "item_fieldlist.h";
if($opt_d) {
	$dbini = $opt_d;
}
if($opt_i) {
	$itemfields = $opt_i;
}

@fieldlist = ();
open(F, "<$itemfields") or die "Unable to open field list $itemfields\n";
while(<F>) {
	if(/F\((.+)\)/) {
		push(@fieldlist, $1);
		$sr=$#fieldlist if ($1 =~ /^sellrate$/i);
		$id=$#fieldlist if ($1 =~ /^id$/i);
		$name=$#fieldlist if ($1 =~ /^name$/i);
		$lore=$#fieldlist if ($1 =~ /^lore$/i);
	}
}
close(F);

$db = "";
$user = "";
$pass = "";
$host = "";
open(F, "<$dbini") or die "Unable to open database config $dbini\n";
while(<F>) {
	s/\r//g;
	if(/host\s*=\s*(.*)/) {
		$host = $1;
	} elsif(/user\s*=\s*(.*)/) {
		$user = $1;
	} elsif(/password\s*=\s*(.*)/) {
		$pass = $1;
	} elsif(/database\s*=\s*(.*)/) {
		$db = $1;
	}
}
if(!$db || !$user || !$pass || !$host) {
	die "Invalid db.ini, missing one of: host, user, password, database\n";
}

$source="DBI:mysql:database=$db;host=$host";

my $dbh = DBI->connect($source, $user, $pass) || die "Could not create db handle\n";

select(STDOUT); $|=0;

$sth = $dbh->prepare("select ".join(",",@fieldlist).",serialization from items");
$sth->execute();
while (my @data = $sth->fetchrow_array) {
	$data[$sr]=sprintf("%.6f",$data[$sr]);
	$orig_serialization=$data[$#data];
	$#data--;

	$data[$name]=~s/\|/\\|/g;
	$data[$lore]=~s/\|/\\|/g;
	$serialized=join('|',@data);
	$serialized=~s/"/\\"/g;

	printf("Processing: %d %s",$data[$id],$data[$name]);
	if ($serialized ne $orig_serialization) {
		printf(" (UPDATED)\n");
		$dbh->do("update items set serialized=now(),serialization=".$dbh->quote($serialized)." where id=".$data[$id]);
	} else {
		print "                       \r";
	}
}

print "\n";
