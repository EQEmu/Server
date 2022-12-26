#include "../client.h"

void command_xptoggle(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;

	if (arguments == 0) {
		c->Message(Chat::White, "Command Syntax: #xptoggle [on/off]");
		return;
	}
	else {
		std::string arugment = Strings::ToLower(sep->argplus[1]);
		if (!arugment.empty()) {
			if (arugment == "on") {
				double xpModifier = getStoredXPModifier(*c); // Retrieve previously saved xpModifier (if any)
				if (xpModifier >= 0) {
					c->SetEXPModifier(0, xpModifier); // If valid modifier retrieved, restore it.
				} 
				else {
					c->SetEXPModifier(0, 100); // If no valid modifier retrieved, set modifier to 100%
				}
				return;
			}
			else if (arugment == "off") {
				double currentModifier = c->GetEXPModifier(0); // Retrieve current global client xp modifier
				saveStoredXPModifier(*c, currentModifier); // save current global client modifier
				c->SetEXPModifier(0, 0); // Set global client xp modifier to 0
				return;
			}
			else {
				c->Message(Chat::Red, "Invalid Command Syntax");
				c->Message(Chat::White, "Command Syntax: #xptoggle [on/off]");
				return;
			}
		}
	}

	// If the player had an xp modifier before they toggled xp off, retrieve it.
	double getStoredXPModifier(Client *c) {
		double storedModifier = std::stod(c->GetBucket("xpmodifier"));
		if (storedModifier >= 0) {
			return storedModifier;
		}
		else {
			return -1;
		}
	}

	// If the player has an xp modifier before they toggle xp off, save it.
	double saveStoredXPModifier(Client *c, double modifier) {
		c->SetBucket("xpmodifier", std::to_string(currentModifier));
	}
