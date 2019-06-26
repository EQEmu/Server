#!/usr/bin/perl

# Author:       Akkadius
# @file:        perl-doc-parser.pl
# @description: Script meant to parse the source code to build the Perl API list

use File::Find;
use Data::Dumper;

sub usage() {
	print "Usage:\n";
	print "   --client     - Prints methods for just client class methods\n";
	print "   --mob        - Prints methods for just mob class methods\n";
	print "   --npc        - Prints methods for just npc class methods\n";
	print "   --entity     - Prints methods for just entity class methods\n";
	print "   --door       - Prints methods for just door class methods\n";
	print "   --object     - Prints methods for just object class methods\n";
	print "   --group      - Prints methods for just group class methods\n";
	print "   --raid       - Prints methods for just raid class methods\n";
	print "   --questitem  - Prints methods for just questitem class methods\n";
	print "   --corpse     - Prints methods for just corpse class methods\n";
	print "   --hateentry  - Prints methods for just hateentry class methods\n";
	print "   --quest      - Prints methods for just quest class methods\n";
	print "   --all        - Prints methods for all classes\n";
	exit(1);
}

if($#ARGV < 0) {
	usage();
}

my $export = $ARGV[0];
$export=~s/--//g;

my $export_file_search = $export;

if ($export eq "quest") {
	$export_file_search = "embparser_api";
}

my @files;
my $start_dir = "zone/";
find(
    sub { push @files, $File::Find::name unless -d; },
    $start_dir
);
for my $file (@files) {

	#::: Skip non Perl files
	if($file!~/perl_|embparser_api/i){ 
		next; 
	}

	#::: If we are specifying a specific class type, skip everything else
	if ($export ne "all" && $export ne "") {
		if ($file!~/$export_file_search/i) {
			next;
		}
	}

	@methods = ();
	$split_key = "";
	$object_prefix = "";

	#::: Open File
	print "\nOpening '" . $file . "'\n";
	open (FILE, $file);
	while (<FILE>) {
		chomp;
		$line = $_;

		if ($line=~/Perl_croak/i && $line=~/Usa/i && $line=~/::/i && $line!~/::new/i) {

			#::: Client export
			if ($export=~/all|client/i && $line=~/Client::/i) {
				$split_key = "Client::";
				$object_prefix = "\$client->";
			}

			#::: Mob export
			if ($export=~/all|mob/i && $line=~/Mob::/i) {
				$split_key = "Mob::";
				$object_prefix = "\$mob->";
			}

			#::: NPC export
			if ($export=~/all|npc/i && $line=~/NPC::/i) {
				$split_key = "NPC::";
				$object_prefix = "\$npc->";
			}

			#::: Corpse export
			if ($export=~/all|corpse/i && $line=~/Corpse::/i) {
				$split_key = "Corpse::";
				$object_prefix = "\$corpse->";
			}

			#::: Entity export
			if ($export=~/all|entity/i && $line=~/EntityList::/i) {
				$split_key = "EntityList::";
				$object_prefix = "\$entity_list->";
			}

			#::: Doors export
			if ($export=~/all|door/i && $line=~/Doors::/i) {
				$split_key = "Doors::";
				$object_prefix = "\$door->";
			}

			#::: Object export
			if ($export=~/all|object/i && $line=~/Object::/i) {
				$split_key = "Object::";
				$object_prefix = "\$object->";
			}

			#::: Group export
			if ($export=~/all|group/i && $line=~/Group::/i) {
				$split_key = "Group::";
				$object_prefix = "\$group->";
			}

			#::: Raid export
			if ($export=~/all|raid/i && $line=~/Raid::/i) {
				$split_key = "Raid::";
				$object_prefix = "\$raid->";
			}

			#::: Hateentry export
			if ($export=~/all|hateentry/i && $line=~/HateEntry::/i) {
				$split_key = "HateEntry::";
				$object_prefix = "\$hate_entry->";
			}

			#::: Questitem export
			if ($export=~/all|questitem/i && $line=~/QuestItem::/i) {
				$split_key = "QuestItem::";
				$object_prefix = "\$quest_item->";
			}

			#::: Quest:: exports
			if ($export=~/all|quest/i && $line=~/quest::/i) {
				$split_key = "quest::";
				$object_prefix = "\quest::";
			}

			#::: Split on croak usage
			@data  = split($split_key, $line);
			$usage = trim($data[1]);

			#::: Split out param borders and get method name
			@params_begin = split('\(', $usage);
			$method_name  = trim($params_begin[0]);

			#::: Get params string built
			@params_end    = split('\)', $params_begin[1]);
			$params_string = trim($params_end[0]);
			$params_string =~s/THIS\,//g;
			$params_string =~s/THIS//g;
			$params_string = trim($params_string);

			$method = $object_prefix . $method_name . "(" . lc($params_string) . ")\n";

			push @methods, $method;
		}
	}

	@methods = sort @methods;
	foreach $method (@methods) {
		print $method;
	}
}

#::: Trim Whitespaces
sub trim { 
	my $string = $_[0]; 
	$string =~ s/^\s+//; 
	$string =~ s/\s+$//; 
	return $string; 
}