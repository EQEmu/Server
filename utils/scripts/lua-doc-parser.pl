#!/usr/bin/perl

# Author:       Akkadius
# @file:        lua-doc-parser.pl
# @description: Script meant to parse the source code to build the LUA API list

use File::Find;
use Data::Dumper;

sub usage() {
	print "Usage:\n";
	print "   --client          - Prints methods for just client class methods\n";
	print "   --mob             - Prints methods for just mob class methods\n";
	print "   --npc             - Prints methods for just npc class methods\n";
	print "   --entity          - Prints methods for just entity class methods\n";
	print "   --entity_list     - Prints methods for just entity_list class methods\n";
	print "   --door            - Prints methods for just door class methods\n";
	print "   --object          - Prints methods for just object class methods\n";
	print "   --group           - Prints methods for just group class methods\n";
	print "   --raid            - Prints methods for just raid class methods\n";
	print "   --item            - Prints methods for just item class methods\n";
	print "   --iteminst        - Prints methods for just iteminst class methods\n";
	print "   --inventory       - Prints methods for just inventory class methods\n";
	print "   --corpse          - Prints methods for just corpse class methods\n";
	print "   --hate_entry      - Prints methods for just hate_entry class methods\n";
	print "   --quest           - Prints methods for just quest class methods\n";
	print "   --spell           - Prints methods for just spell class methods\n";
	print "   --spawn           - Prints methods for just spawn class methods\n";
	print "   --packet          - Prints methods for just packet class methods\n";
	print "   --stat_bonuses    - Prints methods for just stat_bonuses class methods\n";
	print "   --all             - Prints methods for all classes\n";
	exit(1);
}

if($#ARGV < 0) {
	usage();
}

#::: Open File
my $filename = 'lua-api.md';
open(my $fh, '>', $filename) or die "Could not open file '$filename' $!";

my $export = $ARGV[0];
$export=~s/--//g;

my $export_file_search = $export;

if ($export eq "quest") {
	$export_file_search = "lua_general";
}

my @files;
my $start_dir = "zone/";
find(
    sub { push @files, $File::Find::name unless -d; },
    $start_dir
);
for my $file (@files) {

	#::: Skip non lua.cpp files
	if($file!~/lua_/i || $file!~/cpp/i){ 
		next; 
	}

	#::: If we are specifying a specific class type, skip everything else
	if ($export ne "all" && $export ne "") {
		if ($file!~/$export_file_search\.cpp/i) {
			next;
		}
	}

	@methods = ();
	$split_key = "";
	$object_prefix = "";

	#::: Client Export
	if ($export=~/all|client/i && $file=~/_client/i) {
		$split_key = "Client::";
		$object_prefix = "client:";
	}

	#::: Mob Export
	if ($export=~/all|mob/i && $file=~/_mob/i) {
		$split_key = "Mob::";
		$object_prefix = "mob:";
	}

	#::: NPC Export
	if ($export=~/all|npc/i && $file=~/_npc/i) {
		$split_key = "NPC::";
		$object_prefix = "npc:";
	}

	#::: Object Export
	if ($export=~/all|object/i && $file=~/_object/i) {
		$split_key = "Object::";
		$object_prefix = "object:";
	}

	#::: Door Export
	if ($export=~/all|door/i && $file=~/_door/i) {
		$split_key = "Door::";
		$object_prefix = "door:";
	}

	#::: Entity Export
	if ($export=~/all|entity/i && $file=~/_entity/i) {
		$split_key = "Entity::";
		$object_prefix = "entity:";
	}

	#::: Entity List Export
	if ($export=~/all|entity_list/i && $file=~/_entity_list/i) {
		$split_key = "EntityList::";
		$object_prefix = "entity_list:";
	}

	#::: Group
	if ($export=~/all|group/i && $file=~/_group/i) {
		$split_key = "Group::";
		$object_prefix = "group:";
	}

	#::: Raid
	if ($export=~/all|raid/i && $file=~/_raid/i) {
		$split_key = "Raid::";
		$object_prefix = "raid:";
	}

	#::: Corpse
	if ($export=~/all|corpse/i && $file=~/_corpse/i) {
		$split_key = "Corpse::";
		$object_prefix = "corpse:";
	}

	#::: Hateentry
	if ($export=~/all|hate_entry/i && $file=~/_hate_entry/i) {
		$split_key = "HateEntry::";
		$object_prefix = "hate_entry:";
	}

	#::: Spell
	if ($export=~/all|spell/i && $file=~/_spell/i) {
		$split_key = "Spell::";
		$object_prefix = "spell:";
	}

	#::: Spawn
	if ($export=~/all|spawn/i && $file=~/_spawn/i) {
		$split_key = "Spawn::";
		$object_prefix = "spawn:";
	}

	#::: StatBonuses
	if ($export=~/all|stat_bonuses/i && $file=~/stat_bonuses/i) {
		$split_key = "StatBonuses::";
		$object_prefix = "statbonuses:";
	}

	#::: Item
	if ($export=~/all|item/i && $file=~/_item/i) {
		$split_key = "Item::";
		$object_prefix = "item:";
	}

	#::: ItemInst
	if ($export=~/all|iteminst/i && $file=~/_iteminst/i) {
		$split_key = "ItemInst::";
		$object_prefix = "iteminst:";
	}

	#::: Inventory
	if ($export=~/all|inventory/i && $file=~/_inventory/i) {
		$split_key = "Inventory::";
		$object_prefix = "inventory:";
	}

	#::: Packet
	if ($export=~/all|packet/i && $file=~/_packet/i) {
		$split_key = "Packet::";
		$object_prefix = "packet:";
	}

	#::: Quest
	if ($export=~/all|quest/i && $file=~/lua_general/i) {
		$split_key = " lua_";
		$object_prefix = "eq.";
	}

	#::: Open File
	print "\nOpening '" . $file . "'\n";

	if ($split_key eq "") {
		next;
	}

	open (FILE, $file);
	while (<FILE>) {
		chomp;
		$line = $_;

		@data = split(" ", $line);

		if ((lc(substr($data[1], 0, 4)) eq "lua_") && $line!~/luabind/i && $line!~/return |#ifdef|struct /i) {
			#::: Get return type
			$return_type = trim($data[0]);

			@method_split = split($split_key, $line);
			@method_end = split("{", $method_split[1]);

			$method = $object_prefix . trim($method_end[0]) . "; -- " . $return_type . "\n";

			push @methods, $method;
		}
	}

	#::: Header
	$header = $split_key;
	$header =~s/:://g;

	print $fh "# " . $header . "\n";
	print $fh "```lua\n";

	@methods = sort @methods;
	foreach $method (@methods) {
		print $fh $method;
		print $method;
	}

	print $fh "```\n\n";
}

close $fh;

#::: Trim Whitespaces
sub trim { 
	my $string = $_[0]; 
	$string =~ s/^\s+//; 
	$string =~ s/\s+$//; 
	return $string; 
}