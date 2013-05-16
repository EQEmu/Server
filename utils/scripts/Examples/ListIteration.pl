sub EVENT_SAY
{
	if($text=~/mob/i)
	{
		my @moblist = $entity_list->GetMobList();
		foreach $ent (@moblist)
		{
			$ent->Shout("I'm a mob!");
		}
	}

	if($text=~/npc/i)
	{
		my @npclist = $entity_list->GetNPCList();
		foreach $ent (@npclist)
		{
			$ent->Shout("I'm a npc!");
		}
	}

	if($text=~/client/i)
	{
		my @clientlist = $entity_list->GetClientList();
		foreach $ent (@clientlist)
		{
			$ent->Shout("I'm a client!");
		}
	}

	if($text=~/corpse/i)
	{
		my @corpselist = $entity_list->GetCorpseList();
		my $index = 0;
		my $index_two = 0;
		foreach $ent (@corpselist)
		{
			if($ent->GetOwnerName() eq $client->GetName())
			{
				$index_two++;
			}
			$index++;
		}
		quest::say("There are $index corpses in the zone and $index_two of them belong to you.");
	}

	if($text=~/summon/i)
	{
		my @corpselist = $entity_list->GetCorpseList();
		foreach $ent (@corpselist)
		{
			if($ent->GetOwnerName() eq $client->GetName())
			{
				$ent->Summon($client, 1773);
			}
		}
	}

	if($text=~/rez/i)
	{
		my @corpselist = $entity_list->GetCorpseList();
		foreach $ent (@corpselist)
		{
			if($ent->GetOwnerName() eq $client->GetName())
			{
				$ent->CastRezz(994, $npc);
				return;
			}
		}
	}
}

sub EVENT_AGGRO_SAY
{
	if($text=~/hate/i)
	{
		my @hatelist = $npc->GetHateList();
		foreach $ent (@hatelist)
		{
			my $h_ent = $ent->GetEnt();
			my $h_dmg = $ent->GetDamage();
			my $h_hate = $ent->GetHate();
			if($h_ent)
			{
				my $h_ent_name = $h_ent->GetName();
				quest::say("$h_ent_name is on my hate list with $h_hate hate and $h_dmg damage.");
			}
		}
	}
}
