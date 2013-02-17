#!/usr/bin/perl

#reads the output of locate_stringids.pl from stdin
#produces a .IDC file on stdout which we can feed to IDA

print "#include \"idc.idc\"\n\nstatic main() {\n";

while(<>) {
	s/\r?\n//g;
	next unless(/^([0-9a-fA-F]+) Sends (.*)/);
	next if(hex($1) == 0);
	my $off = hex($1);
	my $str = substr($2, 0, 200);
	$str =~ s/"/\\"/g;
	printf("\tMakeComm($off, \"$str\");\n");
}

print "}\n\n";


