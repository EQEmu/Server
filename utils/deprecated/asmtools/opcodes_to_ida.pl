#!/usr/bin/perl

print "#include \"idc.idc\"\n\nstatic main() {\n";

print "\tauto id;\n";
print "\tDelEnum(GetEnum(\"EQOpcode\"));\n";
print "\tid = AddEnum(GetEnumQty(), \"EQOpcode\", 0);\n";

while(<>) {
	next unless(/^(OP_[^= \t]+)=(0x[0-9a-fA-F]+)/);
	next if($2 eq "0x0000" || hex($2) == 0);
	printf("\tAddConstEx(id, \"$1\", $2, -1);\n");
}

print "}\n\n";


