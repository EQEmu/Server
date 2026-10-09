#pragma once

class Client;
class EQApplicationPacket;

namespace DragonHoard {

void HandlePacket(Client* client, const EQApplicationPacket* app);

} // namespace DragonHoard
