
#include <ctype.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <future>
#include <thread>
#include <chrono>

#include "dotnet_quest_interface.h"
#include "../common/spdat.h"
#include "masterentity.h"
#include "questmgr.h"
#include "zone.h"
#include "zone_config.h"
#include "dotnet_runtime.h"

extern Zone *zone;
extern EntityList entity_list;

DotnetParser::DotnetParser()
{
}

void DotnetParser::DotnetLoad()
{
    if (!zone)
    {
        return;
    }
    initialize(zone, &entity_list, &worldserver, &LogSys);
    reload_quests();
}

int DotnetParser::EventNPC(QuestEventID evt, NPC *npc, Mob *init, std::string data, uint32 extra_data,
                           std::vector<std::any> *extra_pointers)
{

    Client *c = (init && init->IsClient()) ? init->CastToClient() : nullptr;
    quest_manager.StartQuest(npc, c);
    event(evt, npc, init, data, extra_data, extra_pointers, false);
    quest_manager.EndQuest();

    return 0;
}

int DotnetParser::EventGlobalNPC(QuestEventID evt, NPC *npc, Mob *init, std::string data, uint32 extra_data,
                                 std::vector<std::any> *extra_pointers)
{
    Client *c = (init && init->IsClient()) ? init->CastToClient() : nullptr;
    quest_manager.StartQuest(npc, c);
    event(evt, npc, init, data, extra_data, extra_pointers, false);
    quest_manager.EndQuest();
}

int DotnetParser::EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
                              std::vector<std::any> *extra_pointers)
{
    quest_manager.StartQuest(client, client);
    event(evt, nullptr, client, data, extra_data, extra_pointers, true);
    quest_manager.EndQuest();

    return 0;
}

int DotnetParser::EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
                                    std::vector<std::any> *extra_pointers)
{
    quest_manager.StartQuest(client, client);
    event(evt, nullptr, client, data, extra_data, extra_pointers, true);
    quest_manager.EndQuest();

    return 0;
}

int DotnetParser::EventItem(QuestEventID evt, Client *client, EQ::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
                            std::vector<std::any> *extra_pointers)
{
    quest_manager.StartQuest(client, client, item);
    quest_manager.EndQuest();

    return 0;
}

int DotnetParser::EventSpell(QuestEventID evt, Mob *mob, Client *client, uint32 spell_id, std::string data, uint32 extra_data,
                             std::vector<std::any> *extra_pointers)
{

    quest_manager.StartQuest(mob, client, nullptr, const_cast<SPDat_Spell_Struct *>(&spells[spell_id]));
    quest_manager.EndQuest();
    return 0;
}

int DotnetParser::EventEncounter(QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers)
{
    // quest_manager.StartQuest(enc, nullptr, nullptr, nullptr, encounter_name);

    // quest_manager.EndQuest();

    return 0;
}

bool DotnetParser::HasQuestSub(uint32 npc_id, QuestEventID evt)
{
    return true;
}

bool DotnetParser::HasGlobalQuestSub(QuestEventID evt)
{
    return true;
}

bool DotnetParser::PlayerHasQuestSub(QuestEventID evt)
{
    return true;
}

bool DotnetParser::GlobalPlayerHasQuestSub(QuestEventID evt)
{
    return true;
}

bool DotnetParser::SpellHasQuestSub(uint32 spell_id, QuestEventID evt)
{
    return true;
}

bool DotnetParser::ItemHasQuestSub(EQ::ItemInstance *itm, QuestEventID evt)
{
    return true;
}

bool DotnetParser::EncounterHasQuestSub(std::string encounter_name, QuestEventID evt)
{
    return true;
}

void DotnetParser::LoadNPCScript(std::string filename, int npc_id)
{
    std::string package_name = "npc_" + std::to_string(npc_id);

    LoadScript(filename, package_name);
}

void DotnetParser::LoadGlobalNPCScript(std::string filename)
{
    LoadScript(filename, "global_npc");
}

void DotnetParser::LoadPlayerScript(std::string filename)
{
    LoadScript(filename, "player");
}

void DotnetParser::LoadGlobalPlayerScript(std::string filename)
{
    LoadScript(filename, "global_player");
}

void DotnetParser::LoadItemScript(std::string filename, EQ::ItemInstance *item)
{
    if (item == nullptr)
        return;
    std::string package_name = "item_";
    package_name += std::to_string(item->GetID());

    LoadScript(filename, package_name);
}

void DotnetParser::LoadSpellScript(std::string filename, uint32 spell_id)
{
    std::string package_name = "spell_" + std::to_string(spell_id);

    LoadScript(filename, package_name);
}

void DotnetParser::LoadEncounterScript(std::string filename, std::string encounter_name)
{
    std::string package_name = "encounter_" + encounter_name;

    LoadScript(filename, package_name);
}

void DotnetParser::AddVar(std::string name, std::string val)
{
    vars_[name] = val;
}

std::string DotnetParser::GetVar(std::string name)
{
    auto iter = vars_.find(name);
    if (iter != vars_.end())
    {
        return iter->second;
    }

    return std::string();
}

void DotnetParser::Init()
{
    ReloadQuests();
}

void DotnetParser::ReloadQuests()
{
    this->DotnetLoad();
}

void DotnetParser::RemoveEncounter(const std::string &name)
{
}

void DotnetParser::LoadScript(std::string filename, std::string package_name)
{
}

bool DotnetParser::HasFunction(std::string subname, std::string package_name)
{
    // std::transform(subname.begin(), subname.end(), subname.begin(), ::tolower);

    auto iter = loaded_.find(package_name);
    if (iter == loaded_.end())
    {
        return false;
    }

    return false;
}

bool DotnetParser::HasEncounterSub(const std::string &package_name, QuestEventID evt)
{

    return false;
}

int DotnetParser::DispatchEventNPC(QuestEventID evt, NPC *npc, Mob *init, std::string data, uint32 extra_data,
                                   std::vector<std::any> *extra_pointers)
{

    if (!npc)
        return 0;

    std::string package_name = "npc_" + std::to_string(npc->GetNPCTypeID());
    int ret = 0;

    return ret;
}

int DotnetParser::DispatchEventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
                                      std::vector<std::any> *extra_pointers)
{

    std::string package_name = "player";

    int ret = 0;

    return ret;
}

int DotnetParser::DispatchEventItem(QuestEventID evt, Client *client, EQ::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
                                    std::vector<std::any> *extra_pointers)
{
    if (!item)
        return 0;

    std::string package_name = "item_";
    package_name += std::to_string(item->GetID());
    int ret = 0;
    return ret;
}

int DotnetParser::DispatchEventSpell(QuestEventID evt, Mob *mob, Client *client, uint32 spell_id, std::string data, uint32 extra_data,
                                     std::vector<std::any> *extra_pointers)
{

    std::string package_name = "spell_" + std::to_string(spell_id);

    int ret = 0;
    return ret;
}

int DotnetParser::EventBot(
    QuestEventID evt,
    Bot *bot,
    Mob *init,
    std::string data,
    uint32 extra_data,
    std::vector<std::any> *extra_pointers)
{

    if (!bot)
    {
        return 0;
    }

    if (!BotHasQuestSub(evt))
    {
        return 0;
    }

    auto *c = (init && init->IsClient()) ? init->CastToClient() : nullptr;
    quest_manager.StartQuest(bot, c);
    quest_manager.EndQuest();

    return 0;
}

int DotnetParser::EventGlobalBot(
    QuestEventID evt,
    Bot *bot,
    Mob *init,
    std::string data,
    uint32 extra_data,
    std::vector<std::any> *extra_pointers)
{

    if (!bot)
    {
        return 0;
    }

    if (!GlobalBotHasQuestSub(evt))
    {
        return 0;
    }

        auto *c = (init && init->IsClient()) ? init->CastToClient() : nullptr;
    quest_manager.StartQuest(bot, c);
    quest_manager.EndQuest();

    return 0;
}

int DotnetParser::DispatchEventBot(
    QuestEventID evt,
    Bot *bot,
    Mob *init,
    std::string data,
    uint32 extra_data,
    std::vector<std::any> *extra_pointers)
{

    return 0;
}

bool DotnetParser::BotHasQuestSub(QuestEventID evt)
{

    return false;
}

bool DotnetParser::GlobalBotHasQuestSub(QuestEventID evt)
{
    return false;
}

void DotnetParser::LoadBotScript(std::string filename)
{
    LoadScript(filename, "bot");
}

void DotnetParser::LoadGlobalBotScript(std::string filename)
{
    LoadScript(filename, "global_bot");
}