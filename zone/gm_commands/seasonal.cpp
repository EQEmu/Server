#include "../client.h"

void command_disable_seasonal(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;

	if (!c->IsSeasonal()) {
		c->Message(Chat::White, "This character is not a seasonal character, and cannot use this command");
	}

	if (arguments && strcmp(sep->arg[1], "confirm") == 0) {
		c->DisableSeasonal();
	} else {
		c->Message(Chat::White, "Usage: #disable_seasonal confirm - Permanently remove this character from this Season. This CANNOT be reversed.");
		return;
	}
}

void command_seasoninfo(Client *c, const Seperator *sep)
{
	DataBucketKey k;
	k.character_id 	= c->CharacterID();
	k.key 		 	= "Season-LoginCount";

	int arguments   = sep->argnum;
	int login_count = Strings::ToInt(DataBucket::GetData(k).value);

	c->Message(Chat::White, "Welcome to Retribution's first seasonal event! This is a special, time-limited event with character and time-locked progression, unique rewards, and unique challenges! You may not access certain features as a seasonal character, nor meaningfully interact with non-seasonal characters. See the Discord server for more information! This character was automatically included in the event, but you can remove it at any time using the #disable_seasonal command.");

	c->Message(Chat::Yellow, "You have logged in %d days during this event!", login_count);
}
