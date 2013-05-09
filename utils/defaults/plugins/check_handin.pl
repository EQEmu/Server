# plugin::check_handin($item1 => #required_amount,...);
# autoreturns extra unused items on success
sub check_handin {
    my $hashref = shift;
    my %required = @_;
    foreach my $req (keys %required) {
	if ((!defined $hashref->{$req}) || ($hashref->{$req} != $required{$req})) {
            return(0);
	}
    }
     foreach my $req (keys %required) {
         if ($required{$req} < $hashref->{$req}) {
             $hashref->{$req} -= $required{$req};
         } else {
             delete $hashref->{$req};
         }
     }
     quest::clearhandin();
     return 1;
}

sub check_mq_handin {
    my $hashref = shift;
    my %required = @_;
    	quest::resethandin();
    foreach my $req (keys %required) {
	$charges = $required{$req};
	if ( !quest::handleturnin($req,$charges) )
	{
		return(0);
	}
    }
     quest::completehandin();
     return 1;
}

sub return_items {
	my $hashref = plugin::var('$itemcount');
	my $client = plugin::val('$client');
	my $items_returned = 0;

	my %ItemHash = (
		0 => [ plugin::val('$item1'), plugin::val('$item1_charges'), plugin::val('$item1_attuned') ],
		1 => [ plugin::val('$item2'), plugin::val('$item2_charges'), plugin::val('$item2_attuned') ],
		2 => [ plugin::val('$item3'), plugin::val('$item3_charges'), plugin::val('$item3_attuned') ],
		3 => [ plugin::val('$item4'), plugin::val('$item4_charges'), plugin::val('$item4_attuned') ],
	);

	foreach my $k (keys(%{$hashref}))
	{
		next if($k == 0);
		my $rcount = $hashref->{$k};
		my $r;
		for ($r = 0; $r < 4; $r++)
		{
			if ($rcount > 0 && $ItemHash{$r}[0] && $ItemHash{$r}[0] == $k)
			{
				if ($client)
				{
					$client->SummonItem($k, $ItemHash{$r}[1], $ItemHash{$r}[2]);
					$items_returned = 1;
				}
				else
				{
					# This shouldn't be needed, but just in case
					quest::summonitem($k, 0);
					$items_returned = 1;
				}
				$rcount--;
			}
		}
		delete $hashref->{$k};
	}
	quest::clearhandin();
	# Return true if items were returned
	return $items_returned;

}