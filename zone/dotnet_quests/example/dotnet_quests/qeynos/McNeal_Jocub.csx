#r "../../bin/dotnet/DotNetTypes.dll"

using System;
using System.Collections;

public void Say(NpcEvent e)
{
   	String msg = e.data.ToLower();
    if(msg.Contains("hail")) {
        e.npc.Say("Good ta see ya! Now start showin' these poodlewalkers how a real fish drinks!");
    } else if (msg.Contains("low")) {
        e.npc.Say("This is going to sound crazy, but my main supplier of [Blackburrow Stout] is one of the brewers themselves. I have run too low on the fine brew and need someone to [pick up my shipment].");
    } else if (msg.Contains("shipment")) {
        e.npc.Say("Take this note to the Qeynos Hills. Somewhere there, you shall find a gnoll at night called Gnasher. Give him the note. Now, get moving!");
        e.mob.CastToClient().SummonItem(18800); // Item: Tattered Note
    } else if (msg.Contains("blackburrow stout")) {
        e.npc.Say("Keep it down!! So you've heard of Blackburrow Stout? We sell it here in Fish's Backroom. If the Qeynos Guards knew, well.. it wouldn't be such a good thing. The stout is illegal, It's made by the gnolls. It is one of the finest brews you will ever taste. If you want any.. slide me a [moonstone].");
    } else if (msg.Contains("Moonstone")) {
        e.npc.Say("Looking for moonstones, are we? The only way I know of getting a moonstone is to hunt gnolls for Captain Tillin of the Qeynos Guards.");
    }
}

public void Signal(NpcEvent e) 
{
    e.npc.Say("You shall never take me alive!");
    e.npc.Attack(e.entityList.GetNPCByName("Executioner"));
}

public void WaypointArrive(NpcEvent e) {
    if (e.data == "5") {
        e.npc.Say("Blast!! We are running [low]!!");
    }
}

public void Trade(NpcEvent e)
{
    var client = e.mob.CastToClient();
    foreach (ItemInstance item in e.itemList)
    {
        if (item.GetID() == 13131)
        {
            e.npc.Say("Good work, pal. Here's a little dough to spend, just don't spend it at any other bar.");
            e.questManager.givecash(9, 3, 0, 0);
            e.questManager.ding();
            e.questManager.faction(345,1,0); // Faction: Karana Residents
 			e.questManager.faction(262,1,0); // Faction: Guards of Qeynos
 			e.questManager.faction(341,1,0); // Faction: Priests of Life
 			e.questManager.faction(280,1,0); // Faction: Knights of Thunder
        } else if (item.GetID() == 10070) {
            e.npc.Say("Here you go then. Don't go tellin' no Guards where that came from, I would hate to rid myself of a good paying customer.");
            e.questManager.ding();
            e.questManager.faction(345,1,0); // Faction: Karana Residents
 			e.questManager.faction(262,1,0); // Faction: Guards of Qeynos
 			e.questManager.faction(341,1,0); // Faction: Priests of Life
 			e.questManager.faction(280,1,0); // Faction: Knights of Thunder
            e.mob.CastToClient().AddEXP(500);
            e.mob.CastToClient().SummonItem(13107, 2);
        }
    }
}