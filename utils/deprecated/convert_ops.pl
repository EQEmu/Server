#! /usr/bin/perl

print<<EOF;
#include "debug.h"
#include <map>
#include <string>

using namespace std;

map<unsigned long, string> opcode_map;

string get_opcode_name(unsigned long opcode)
{
map<unsigned long, string>::iterator itr;;

	return (itr=opcode_map.find(opcode))!=opcode_map.end() ? itr->second : "OP_Unknown";
}
EOF

printf("void load_opcode_names()\n{\n");
open(OPS,"$ARGV[0]") || die;
while($_=<OPS>) {
	if (/^#define\s+(\S+)\s+(\S+)/) {
		if ($2 ne "0x9999") {
			printf("\topcode_map[%s]=\"%s\";\n",$2,$1);
		}
	}
}
close(OPS);
printf("}\n");
