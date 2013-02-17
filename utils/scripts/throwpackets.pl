#copy this file into your server's dir
#include this file with: require "throwpackets.pl";
# and add:
# command_add("throwfile", "[opcode name] [filename] - Send a file's hex contents as a packet", 250);
#to your commands_init to enable this


sub throwfile {
	my $op = shift;
	my $file = shift;
	my $p = FileToPacket($op, $file);
	if(!$p) {
		$client->Message(13, "Unable to read file or parse contents.");
		return;
	}
	$p->SendTo($client);
	$client->Message(0, "Sent.");
}

sub HexToPacket {
	my $op = shift;
	my $hex = shift;
	my @lines = split(/\r?\n/, $hex);
	my $body = "";
	my @pieces = ();
	foreach my $l (@lines) {
		if($l =~ /[0-9a-fA-Fx]+:\s*(.*)\s+\|/) {
			$l = $1;
		}
		$l =~ s/\s+-\s+/ /g;
		$body .= $l;
	}
	foreach my $p (split(/\s+/, $body)) {
		push(@pieces, "0x$p");
	}
	my $p = new PerlPacket($op);
	$p->FromArray(\@pieces, $#pieces+1);
	return($p);
}

sub FileToPacket {
	my $op = shift;
	my $file = shift;
	my $c = "";
	open(F, "<$file") || return(undef);
	while(<F>) {
		$c .= $_;
	}
	close(F);
	return(HexToPacket($op, $c));
}


