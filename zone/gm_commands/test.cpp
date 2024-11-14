
void command_test(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;

	for (auto &e : entity_list.GetMobList()) {
		auto mob = e.second;
		if (Distance(c->GetPosition(), mob->GetPosition()) > 100) {
			mob->SendAppearancePacket(AppearanceType::Invisibility, 3001);
		} else {
			mob->SendAppearancePacket(AppearanceType::Invisibility, 0);
		}
	}
}
