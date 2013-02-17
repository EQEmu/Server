#! /usr/bin/perl

use DBI;
use Getopt::Std;

getopts('d:h');
$conf = "eqemu_config.xml";
if($opt_h) {
	die "Usage: load_13thfloor_items.pl [-d path/to/eqemu_config.xml]\n";
}
if($opt_d) {
	$conf = $opt_d;
}

$db = "eq";
$user = "eq";
$pass = "eq";
$host = "localhost";
open(F, "<$conf") or die "Unable to open config $conf\n";
$indb = 0;
while(<F>) {
	s/\r//g;
	if(/<database>/i) {
		$indb = 1;
	}
	next unless($indb == 1);
	if(/<\/database>/i) {
		$indb = 0;
		last;
	}
	if(/<host>(.*)<\/host>/i) {
		$host = $1;
	} elsif(/<username>(.*)<\/username>/i) {
		$user = $1;
	} elsif(/<password>(.*)<\/password>/i) {
		$pass = $1;
	} elsif(/<db>(.*)<\/db>/i) {
		$db = $1;
	}
}
if(!$db || !$user || !$pass || !$host) {
	die "Invalid db.ini, missing one of: host, user, password, database\n";
}

$source="DBI:mysql:database=$db;host=$host";

my $dbh = DBI->connect($source, $user, $pass) || die "Could not create db handle\n";

$_=<STDIN>;
chomp();
s/'/\\'/g;
@fields=split("(?<!\\\\)\\|", $_);

%conversions = (
	"itemtype" => "itemuse"
);

$insert="replace into items (".join(",",@fields).",source,updated) values ('";
$insert=~s/UNK130/potionbeltslots/;
$insert=~s/UNK133/stackable/;

#select(STDOUT); $|=1;
while(<STDIN>) {
	chomp();
	s/'/\\'/g;
	@f=split("(?<!\\\\)\\|", $_);
	$insert2=join("','",@f);
	$#f--;
	grep(s/\\\|/\\\\\|/g,@f);
	grep(s/"/\\\\"/g,@f);
	$statement=sprintf("%s%s','13THFLOOR',now())",$insert,$insert2,join('|',@f));
	$dbh->do($statement);
	printf("Processing: %d %s                        \r",$f[4],$f[1]);
	++$count;
}
printf("Processed: %d items(s)                     \n",$count);

