
using System.Collections;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Runtime;
using System;
            
public class Captain_Tillin {
    private int counter = 0;
	public void Timer(NpcEvent e) {
	   e.logSys.QuestDebug($"Timer hit 21 {counter++}");
	   // e.npc.Say("Something");
	}
	public void TimerStart(NpcEvent e) {
	    e.logSys.QuestDebug($"Timer start: e {e.data}");
	    Console.WriteLine($"Timer start: e {e.data}");
	}
	public void Say(NpcEvent e)
	{
	    e.npc.Say($"Hi there 123, {e.mob.GetCleanName()}! Spend your time wisely in the city of Qeynos. Do not let your mind wander to thoughts of bravado or crime. My guards can easily put to rest any outbreaks. Good day to you, citizen!");
	    //e.questManager.signal(1068);
	    e.npc.AddToHateList(e.entityList.GetNPCByName("Ebon_Strongbear").CastToMob());
	    e.npc.Attack(e.entityList.GetNPCByName("Ebon_Strongbear"));
	    for (struct_HateList h in e.npc.GetHateList()) {
	        e.logSys.QuestDebug($"On hate list: {h.entity_on_hatelist.GetName()}");
	    }
	    //e.npc.AddToHateList(e.mob);
	   // e.npc.GetHateList().PrintHateListToClient(e.mob);
	}
	public void Spawn(NpcEvent e)
	{
	    Console.WriteLine("Zone bootup - spawn");
	    e.questManager.settimerMS("tillin", 5000);
	}
	public void Signal(NpcEvent e)
	{
	    Console.WriteLine("Hit signal new 1");
	    e.npc.Say("Ah.  Good.  You have arrived.  Executioner, could you please visit McNeal Jocub at Fish's Tavern.  He has violated our laws and the sentence is death.");
	}
	public void Trade(NpcEvent e)
	{
	    var client = e.mob.CastToClient();
	    foreach (ItemInstance item in e.itemList)
	    {
	        if (item.GetID() == 13915)
	        {
	            e.npc.Say("Very good! One less gnoll the people of Qeynos need to fear. Here is your bounty as promised.");
	            e.questManager.faction(219, 2, 0); // Antonius Bayle
	            e.questManager.faction(223, -2, 0); // Circle of Unseen Hands
	            e.questManager.faction(230, -2, 0); // Corrupt Qeynos Guards
	            e.questManager.faction(262, 2, 0); // Guards of Qeynos
	            e.questManager.faction(291, 2, 0); // Merchants of Qeynos
	            e.questManager.summonitem(10070);
	            e.questManager.ding();
	            e.questManager.exp(7000);
	        }
	        else if (item.GetID() == 18815)
	        {
	            e.npc.Say("Wonder what the text should be here");
	            e.questManager.faction(219, 1, 0); // Antonius Bayle
	            e.questManager.faction(223, -1, 0); // Circle of Unseen Hands
	            e.questManager.faction(230, -1, 0); // Corrupt Qeynos Guards
	            e.questManager.faction(262, 1, 0); // Guards of Qeynos
	            e.questManager.faction(291, 1, 0); // Merchants of Qeynos
	            e.questManager.summonitem(13305);
	            e.questManager.ding();
	            e.questManager.exp(500);
	            Random rnd = new Random();
	            client.CashReward((uint)rnd.Next(1, 11), (uint)rnd.Next(1, 11), (uint)rnd.Next(1, 11), (uint)rnd.Next(1, 11));
	            // e.questManager.spawn2(1202, 62, 0, new Vec4(-412, 75, -24, 0)); // NPC: Executioner
	        }
	        else if (item.GetID() == 18912)
	        {
	            e.npc.Say("So, an assassin has been sent to Qeynos! I shall have my guards keep an eye out for any suspicious looking visitors. As for you... you should speak with the Surefall Glade ambassador. Ambassador Gash is staying at the Lion's Mane Inn here in South Qeynos. Inform him that [an assassin has been sent to kill] him. Do not let the assassin near him!");
	            e.questManager.faction(219, 1, 0); // Antonius Bayle
	            e.questManager.faction(223, -1, 0); // Circle of Unseen Hands
	            e.questManager.faction(230, -1, 0); // Corrupt Qeynos Guards
	            e.questManager.faction(262, 1, 0); // Guards of Qeynos
	            e.questManager.faction(291, 1, 0); // Merchants of Qeynos
	            e.questManager.summonitem(10070);
	            e.questManager.ding();
	            e.questManager.exp(500);
	            Random rnd = new Random();
	            client.CashReward((uint)rnd.Next(1, 11), (uint)rnd.Next(1, 11), (uint)rnd.Next(1, 11), (uint)rnd.Next(1, 11));
	        }
	    }
	}
}

public class Executioner {
    public void Say(NpcEvent e)
	{
	    e.npc.Say("I have no time to talk, citizen. Please, step aside! ");
	}
	public void WaypointArrive(NpcEvent e) {
	    var waypoint = Int32.Parse(e.data);
	    e.logSys.QuestDebug($"Executioner reaches waypoint: {waypoint}");
	    if (waypoint == 14) {
	        e.npc.Say("Sir. You called for me?");
	        e.questManager.signal(1068, 1);
	    } else if (waypoint == 40) {
	        e.npc.Say("McNeal Jocub? You have been found guilty of crimes against the city of Qeynos.");
	    } else if (waypoint == 41) {
	        e.questManager.signal(1099, 1);
	    }
	}
	public void Signal(NpcEvent e) 
	{
	    e.npc.Say("Aye sir!");
	}
}

public class McNeal_Jocub {
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
	    e.npc.Say("Ah fuck it, you shall never take me alive!");
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
}

        