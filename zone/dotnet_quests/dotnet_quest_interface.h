#ifndef _EQE_DOTNET_PARSER_H
#define _EQE_DOTNET_PARSER_H

#include <thread>
#include <chrono>
#include <string>
#include <list>
#include <map>
#include <exception>

#include "../quest_parser_collection.h"
#include "../quest_interface.h"
#include "../zone_config.h"
#include "../zone.h"

extern const ZoneConfig *Config;

class Client;
class NPC;

namespace EQ
{
    class ItemInstance;
}

 class DotnetParser : public QuestInterface
{
public:
    DotnetParser();
    DotnetParser(const DotnetParser &);
    DotnetParser &operator=(const DotnetParser &);

    virtual int EventNPC(
        QuestEventID evt,
        NPC *npc,
        Mob *init,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int EventGlobalNPC(
        QuestEventID evt,
        NPC *npc,
        Mob *init,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int EventPlayer(
        QuestEventID evt,
        Client *client,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int EventGlobalPlayer(
        QuestEventID evt,
        Client *client,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int EventItem(
        QuestEventID evt,
        Client *client,
        EQ::ItemInstance *item,
        Mob *mob,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int EventSpell(
        QuestEventID evt,
        Mob *mob,
        Client *client,
        uint32 spell_id,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int EventEncounter(
        QuestEventID evt,
        std::string encounter_name,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int EventBot(
        QuestEventID evt,
        Bot *bot,
        Mob *init,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int EventGlobalBot(
        QuestEventID evt,
        Bot *bot,
        Mob *init,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);

    virtual bool HasQuestSub(uint32 npc_id, QuestEventID evt);
    virtual bool HasGlobalQuestSub(QuestEventID evt);
    virtual bool PlayerHasQuestSub(QuestEventID evt);
    virtual bool GlobalPlayerHasQuestSub(QuestEventID evt);
    virtual bool SpellHasQuestSub(uint32 spell_id, QuestEventID evt);
    virtual bool ItemHasQuestSub(EQ::ItemInstance *itm, QuestEventID evt);
    virtual bool EncounterHasQuestSub(std::string encounter_name, QuestEventID evt);
    virtual bool HasEncounterSub(const std::string &package_name, QuestEventID evt);
    virtual bool BotHasQuestSub(QuestEventID evt);
    virtual bool GlobalBotHasQuestSub(QuestEventID evt);

    virtual void LoadNPCScript(std::string filename, int npc_id);
    virtual void LoadGlobalNPCScript(std::string filename);
    virtual void LoadPlayerScript(std::string filename);
    virtual void LoadGlobalPlayerScript(std::string filename);
    virtual void LoadItemScript(std::string filename, EQ::ItemInstance *item);
    virtual void LoadSpellScript(std::string filename, uint32 spell_id);
    virtual void LoadEncounterScript(std::string filename, std::string encounter_name);
    virtual void LoadBotScript(std::string filename);
    virtual void LoadGlobalBotScript(std::string filename);

    virtual void AddVar(std::string name, std::string val);
    virtual std::string GetVar(std::string name);
    virtual void Init();
    virtual void ReloadQuests();
    virtual void RemoveEncounter(const std::string &name);
    virtual uint32 GetIdentifier() { return 0xbaddad; }

    virtual int DispatchEventNPC(
        QuestEventID evt,
        NPC *npc,
        Mob *init,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int DispatchEventPlayer(
        QuestEventID evt,
        Client *client,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int DispatchEventItem(
        QuestEventID evt,
        Client *client,
        EQ::ItemInstance *item,
        Mob *mob,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int DispatchEventSpell(
        QuestEventID evt,
        Mob *mob,
        Client *client,
        uint32 spell_id,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);
    virtual int DispatchEventBot(
        QuestEventID evt,
        Bot *bot,
        Mob *init,
        std::string data,
        uint32 extra_data,
        std::vector<std::any> *extra_pointers);

    static DotnetParser *Instance()
    {
        static DotnetParser inst;
        return &inst;
    }

    bool HasFunction(std::string function, std::string package_name);
    void DotnetLoad();


    void LoadScript(std::string filename, std::string package_name);

    std::map<std::string, std::string> vars_;
    std::map<std::string, bool> loaded_;
    std::map<std::string, Encounter*> encounters_;
    std::thread runtime_thread;
};


#endif