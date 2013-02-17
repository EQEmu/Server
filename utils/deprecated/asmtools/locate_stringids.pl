#! /usr/bin/perl -W

#reads the output of `objdump -D eqgame.exe` from stdin
#reads eqstr_us.txt from .

open(F, "<eqstr_us.txt") or die("Unable to open ./eqstr_us.txt\n");
%strs = ();
while(<F>) {
	s/\r?\n//g;
	next unless(/^([0-9]+)\s+(.+)$/);
	$strs{$1} = $2;
}
close(F);

while(<STDIN>) {
	if (/push\s+(.*)/) {
	        push(@stack,$1);
	        shift @stack if ($#stack>4);
	}
	#address of DisplayStringID
	if (/  ([0-9a-f]+):.*call   0x5DFA00/i) {
		my $loc = $1;
		if($stack[$#stack] =~ /(0x[0-9a-fA-F]+)/) {
			my $sid = hex($1);
			if(defined($strs{$sid})) {
			        printf("%s Sends %s\n",$loc,$strs{$sid});
			} else {
			        printf("%s Sends %s\n",$loc,"UNDEFINED STRING");
			}
		}
	}
}
