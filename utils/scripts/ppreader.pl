#!/usr/bin/perl -W

use strict;

sub usage() {
	print "Usage: ppreader.pl [-o|-n|-p|-r|-c] (old_pp) (old2_pp)\n";
	print "   -o - print offsets for old struct\n";
	print "   -n - print offsets for new struct\n";
	print "   -p - print pp converter from old to new\n";
	print "   -c - print pp converter between two old PPs\n";
	print "   -r - read al alternative header and print offsets\n";
	exit(1);
}

if($#ARGV < 0) {
	usage();
}

my $mode = $ARGV[0];

my $temp = ".temp";



if($mode eq "-r") {
	my $file = $ARGV[1];
	my $struct = $ARGV[2];
	print STDERR "Reading $struct from $file...\n";
	(my $old_order, my $old_fields, my $old_offsets) = readpp($struct, $file);
	foreach my $f(@{$old_order}) {
		my $off = $old_offsets->{$f};
		my $code = $old_fields->{$f};
		printf("/*%04lu*/%s\n", $off, $code);
	}
} elsif($mode eq "-o") {
	print STDERR "Reading old player profile...\n";
	my $old_struct = $ARGV[1];
	(my $old_order, my $old_fields, my $old_offsets) = readpp($old_struct, "../common/eq_old_structs.h");
	foreach my $f(@{$old_order}) {
		my $off = $old_offsets->{$f};
		my $code = $old_fields->{$f};
		printf("/*%04lu*/%s\n", $off, $code);
	}
} elsif($mode eq "-n") {
	print STDERR "Reading new player profile...\n";
	(my $order, my $fields, my $offsets) = readpp("PlayerProfile_Struct", "../common/eq_packet_structs.h");
	foreach my $f(@{$order}) {
		my $off = $offsets->{$f};
		my $code = $fields->{$f};
		printf("/*%04lu*/%s\n", $off, $code);
	}
} elsif($mode eq "-p") {
	if($#ARGV != 1) {
		usage();
	}
	my $old_struct = $ARGV[1];

	print STDERR "Reading old player profile...\n";
	(my $old_order, my $old_fields, my $old_offsets) = readpp($old_struct, "../common/eq_old_structs.h");
	print STDERR "Reading new player profile...\n";
	(my $order, my $fields, my $offsets) = readpp("PlayerProfile_Struct", "../common/eq_packet_structs.h");

	compare_pps($old_order, $old_fields, $old_offsets,
		$order, $fields, $offsets);
} elsif($mode eq "-c") {
	if($#ARGV != 2) {
		usage();
	}
	my $old_struct = $ARGV[1];
	my $old2_struct = $ARGV[2];

	print STDERR "Reading old player profile...\n";
	(my $old_order, my $old_fields, my $old_offsets) = readpp($old_struct, "../common/eq_old_structs.h");
	print STDERR "Reading old2 player profile...\n";
	(my $order, my $fields, my $offsets) = readpp($old2_struct, "../common/eq_old_structs.h");

	compare_pps($old_order, $old_fields, $old_offsets,
		$order, $fields, $offsets);
} else {
	print "Invalid mode specified.";
	exit(1);
}


sub compare_pps {
	(my $old_order, my $old_fields, my $old_offsets, my $order, my $fields, my $offsets) = @_;

	#will not catch order changes very well for now

	my $last_diff = 0;
	my $first = $old_order->[0];
	my $taildrop = 0;
	my $final = "";
	my $f;
	foreach $f(@{$old_order}) {
		if(!defined($offsets->{$f})) {
			if($taildrop eq "") {
				$taildrop = $f;
			}
			my $guess = $old_offsets->{$f} + $last_diff;
#			print STDERR "field $f was lost in new profile, it could be at $guess.\n";
			next;
		}

		my $diff = $offsets->{$f} - $old_offsets->{$f};

		if($last_diff != $diff) {
			#a change in deltas... print a rule for last block
			my $tail = $f;
			if($taildrop ne "") {
				$tail = $taildrop;
			}
# print "// delta change from $last_diff to $diff at field $f ($tail)\n";
			print "\t\tmemcpy(\&pp->$first, \&ops->$first, StructDist(ops, $first, $tail));\n";

			$first = $f;
			$last_diff = $diff;
			$taildrop = "";

		} else {
			#another field with the same delta...
			$final = $f;
			$taildrop = "";
			next;
		}
	}
	#finally do the last rule
	my $tail = $f;
	if($taildrop ne "") {
		$tail = $taildrop;
	}
	print "\t\tmemcpy(\&pp->$first, \&ops->$first, StructDist(ops, $first, $tail));\n";

}



sub readpp {
	(my $sname, my $fname) = @_;
	open(F, "<$fname") || die("Unable to open $fname\n");
	my @order = ();
	my %fields = ();
	my %offsets = ();
	my $in = 0;
	while(<F>) {
		s/\r?\n//g;
		if(/struct\s+$sname/) {
			$in = 1;
		}
		next if(!$in);

		if($_ =~ /\}\s*;/) {
			$in = 0;
			last;
		}
		if($_ !~ /^\/\*[0-9]+\*\/(\s*)([^ \t]+)(\s+)([^ \t;]+);(.*)$/) {
			print STDERR "Unable to parse line '$_'\n";
			next;
		}
		my $field = $4;
		my $array = "";
		my $code = "$1$2$3$4;$5";
		if($field =~ /([^ \t]+)\[(.+)\]\[(.+)\]/) {
			$field = $1."[0][0]";
		} elsif($field =~ /([^ \t]+)\[(.+)\]/) {
			$field = $1."[0]";
			$array = $2;
		}
		$fields{$field} = $code;
		push(@order, $field);
	}
	close(F);

	open(F, ">$temp.cpp") || die("Unable to open $temp.cpp");
	print F <<"EOC";
#include "../common/types.h"
#include "$fname"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() {
	$sname p;
	unsigned long start = (unsigned long) &p;
EOC

	foreach my $field(keys(%fields)) {
		print F "\tprintf(\"$field,%lu\\n\", ((unsigned long) \&p.$field) - start);\n";
	}
	print F "\n\treturn(0);\n}\n\n";
	close(F);

	if(system("g++ $temp.cpp -I. -o $temp") != 0) {
		die("Error compiling $temp.cpp\n");
	}

	system("./$temp >$temp.out");
	open(F, "<$temp.out");
	while(<F>) {
		chomp;
		if($_ !~ /(.+),([0-9]+)/) {
			print "Unable to read our own output '$_'\n";
			next;
		}
		if(!defined($fields{$1})) {
			print "Read invalid field name '$1' from own output.\n";
			next;
		}
		$offsets{$1} = $2;
	}
	close(F);

#	unlink("$temp.cpp");
#	unlink("$temp.out");
#	unlink("$temp");

	return(\@order, \%fields, \%offsets);
}







