#!/usr/bin/perl

do $ARGV[0];

print "#include \"idc.idc\"\n\nstatic main() {\n";

foreach my $sig(@signatures) {
	my $off = $sig->{offset} + 0x00400000;
	my $name = $sig->{name};
	my $op = $sig->{opcode};
	printf("MakeName(0x%x, \"${name}_0x%x\");\n", $off, $op);
}

print "}\n\n";
