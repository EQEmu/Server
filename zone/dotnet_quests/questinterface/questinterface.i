



%module questinterface

%ignore Mob::GetSeeInvisibleLevelFromNPCStat;
%ignore Mob::SpecialAbility::timer;
%ignore Mob::GetSpecialAbilityTimer;
%ignore Mob::GetAIThinkTimer;
%ignore Mob::GetAttackTimer;
%ignore Mob::GetAttackDWTimer;
%ignore Mob::GetAIMovementTimer;
%ignore Mob::mob_close_scan_timer;
%ignore Mob::mob_check_moving_timer;
%ignore Mob::AddAura;
%ignore Mob::AddTrap;
%ignore Mob::AuraInfo::aura;
%ignore Mob::GetCombatRecord;
%ignore NPC::GetRefaceTimer;
%ignore NPC::m_SpawnPoint;
%ignore Doors::GetPosition;
%ignore Mob::GetTargetRingLocation;
%ignore Mob::GetPosition;
%ignore Mob::GetRelativePosition;
%ignore Client::GetLastPositionBeforeBulkUpdate;
%ignore ZoneSpellsBlocked::m_Location;
%ignore ZoneSpellsBlocked::m_Difference;
%ignore Trap::m_Position;
%ignore worldserver;
%ignore spells;

%ignore Merc::GetRawACNoShield;
%ignore Raid::RemoveGroupLeader;
%ignore EQApplicationPacket::combine;

%ignore ZoneGuildManager::ListGuilds;
%ignore SwarmPet::duration;
%ignore Trap::SpellOnTarget;
%ignore Trap::respawn_timer;
%ignore Trap::chkarea_timer;
%ignore Trap::reset_timer;
%ignore Group::SendWorldGroup;
%ignore Client::GetEXPModifiers;
%ignore Client::GetMercTimer;
%ignore Client::GetPickLockTimer;
%ignore Client::m_list_task_timers_rate_limit;
%ignore Client::SetEXPModifiers;
%ignore Client::SendTaskComplete;
%ignore Client::Flurry;
%ignore Client::WhoAll;
%ignore Client::Rampage;
%ignore Client::LoadSpellGroupCache;
%ignore Zone::GetAuth;
%ignore Zone::aa_abilities;
%ignore Zone::aa_ranks;
%ignore Zone::ldon_trap_list;
%ignore Zone::ldon_trap_entry_list;
%ignore Zone::merc_templates;
%ignore Zone::merchanttable;
%ignore Zone::merc_spells_list;
%ignore Zone::merc_stance_list;
%ignore Zone::tmpmerchanttable;
%ignore Zone::adventure_entry_list_flavor;
%ignore Zone::GetAlternateAdvancementAbilityAndRank;
%ignore Zone::expedition_cache;
%ignore Zone::dynamic_zone_cache;
%ignore Zone::dz_template_cache;
%ignore NPC::AIautocastspell_timer;
%ignore Zone::GetInstanceTimer;
%ignore Zone::spawn2_timer;
%ignore Zone::hot_reload_timer;
%ignore Zone::GetInitgridsTimer;
%ignore Spawn2::GetTimer;
%ignore ZoneDatabase::AddLootTableToNPC;
%ignore ZoneDatabase::AddLootDropToNPC;
%ignore ZoneDatabase::GetServerFilters;
%ignore ZoneDatabase::SetServerFilters;
%ignore QuestEventSubroutines;
%ignore QuestManager::ClearTimers;

%ignore RuleManager::InvalidBool;
%ignore RuleManager::InvalidReal;
%ignore RuleManager::InvalidInt;
%ignore RuleManager::InvalidString;
%ignore RuleManager::InvalidCategory;

%ignore ListElement::ListElement();

%ignore ListElement::ListElement(const NewSpawn_Struct*&);
%ignore ListElement::ListElement(const Spawn2*&);
%ignore ListElement::ListElement(const ZonePoint*&);

%ignore ListElement::ListElement(const ListElement<NewSpawn_Struct*>&);
%ignore ListElement::ListElement(const ListElement<Spawn2*>&);
%ignore ListElement::ListElement(const ListElement<ZonePoint*>&);

%{

#include <string>
#include <list>
#include <any>
#include <set>
#include <vector>
#include <memory>
#include <iostream>

// init default values in constructors
#define GLM_FORCE_CTOR_INIT

// these included in math/ofVectorMath.h
// we declare some things manually, so some includes are commented out
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include "../../../common/ruletypes.h"
#include "../../../common/rulesys.h"
#include "../../../common/spdat.h"
#include "../../../common/eq_packet.h"
#include "../../../common/faction.h"
#include "../../../common/net/packet.h"
#include "../../../common/net/daybreak_structs.h"
#include "../../../common/eq_stream_intf.h"
#include "../../../common/net/daybreak_connection.h"
#include "../../../common/expedition_lockout_timer.h"
#include "../../../common/eqemu_logsys_log_aliases.h"
#include "../../../common/linked_list.h"
#include "../../../common/emu_constants.h"
#include "../../../common/dynamic_zone_base.h"
#include "../../../common/eq_constants.h"
#include "../../../common/item_instance.h"
#include "../../../common/item_data.h"
#include "../../../common/eqemu_logsys.h"
#include "../../../common/zone_store.h"
#include "../../../common/servertalk.h"

#include "../../common.h"
#include "../../entity.h"
#include "../../bot.h"
#include "../../mob.h"
#include "../../hate_list.h"
#include "../../merc.h"
#include "../../event_codes.h"
#include "../../npc.h"
#include "../../encounter.h"
#include "../../beacon.h"
#include "../../zone.h"
#include "../../client.h"
#include "../../corpse.h"
#include "../../doors.h"
#include "../../groups.h"
#include "../../object.h"
#include "../../raids.h"
#include "../../trap.h"

#include "../../aa.h"
#include "../../aa_ability.h"
#include "../../aa_rank.h"
#include "../../command.h"
#include "../../data_bucket.h"
#include "../../expedition.h"
#include "../../expedition_database.h"
#include "../../expedition_request.h"
#include "../../forage.h"
#include "../../global_loot_manager.h"
#include "../../guild_mgr.h"
#include "../../horse.h"
#include "../../pets.h"
#include "../../position.h"
#include "../../titles.h"
#include "../../tasks.h"
#include "../../task_manager.h"
#include "../../task_client_state.h"

#include "../../spawn2.h"
#include "../../spawngroup.h"
#include "../../zonedb.h"
#include "../../worldserver.h"
#include "../../questmgr.h"
#include "../../dynamic_zone.h"
#include "../../dotnet_quests/dotnet_runtime.h"



void FreeVec4(glm::vec4* ptr) {
    delete ptr;
}

void FreeVec3(glm::vec3* ptr) {
    delete ptr;
}

%}


// Assuming glm::vec4 is the type you're working with
void FreeVec4(glm::vec4* ptr);
void FreeVec3(glm::vec3* ptr);

%include <std_list.i>
%include <std_except.i>
%include <std_vector.i>
%include <std_string.i>
%include <csharp/std_string.i>
%include <std_unordered_map.i>
%include <std_shared_ptr.i>
%include <stdint.i>

%inline %{
std::string GetRuleValue(const std::string& rule) {
    std::string out("");
    RuleManager::Instance()->GetRule(rule, out);
    return out;
}
SPDat_Spell_Struct GetSpellById(int spell_id) {
    return spells[spell_id];
}

%}

%{
using namespace EQ;
using namespace Logs;
using namespace glm;
using namespace AA;
%}

namespace glm {
    %typedef int length_t;
    %include "glm.i"
} // namespace

%typemap(cstype) glm::vec4 "Vec4"
%typemap(imtype) glm::vec4 "nint"
%typemap(in) glm::vec4 %{
    $1 = *reinterpret_cast<glm::vec4*>($input);
%}
%typemap(out) glm::vec4 {
    $result = new glm::vec4($1);
}
%typemap(csout, excode=SWIGEXCODE) glm::vec4 {
    var ptr = $imcall;
    Vec4 vec = System.Runtime.InteropServices.Marshal.PtrToStructure<Vec4>(ptr);
    questinterfacePINVOKE.FreeVec4(new System.Runtime.InteropServices.HandleRef(null, (IntPtr)ptr)); // Implement this in C++ and expose via P/Invoke
    $excode
    return vec; 
}
%typemap(csin) glm::vec4 %{
    (nint)vec4.getCPtr(new vec4($csinput.x, $csinput.y, $csinput.z, $csinput.w))
%}

%typemap(cstype) glm::vec3 "Vec3"
%typemap(imtype) glm::vec3 "nint"
%typemap(in) glm::vec3 %{
    $1 = *reinterpret_cast<glm::vec3*>($input);
%}
%typemap(out) glm::vec3 {
    $result = new glm::vec3($1);
}
%typemap(csout, excode=SWIGEXCODE) glm::vec3 {
    var ptr = $imcall;
    Vec3 vec = System.Runtime.InteropServices.Marshal.PtrToStructure<Vec3>(ptr);
    questinterfacePINVOKE.FreeVec3(new System.Runtime.InteropServices.HandleRef(null, (IntPtr)ptr)); // Implement this in C++ and expose via P/Invoke
    $excode
    return vec; 
}
%typemap(csin) glm::vec3 %{
    (nint)vec3.getCPtr(new vec3($csinput.x, $csinput.y, $csinput.z))
%}

%include "../../../common/ruletypes.h"
%include "../../../common/eq_packet.h"
%include "../../../common/spdat.h"
%include "../../../common/net/packet.h"
%include "../../../common/net/daybreak_structs.h"
%include "../../../common/eq_stream_intf.h"
%include "../../../common/net/daybreak_connection.h"

%include "../../../common/expedition_lockout_timer.h"
%include "../../../common/eqemu_logsys_log_aliases.h"

%ignore Timer;

%include "../../../common/linked_list.h"
%include "../../../common/emu_constants.h"
%include "../../../common/faction.h"
%include "../../../common/eq_constants.h"
%include "../../../common/dynamic_zone_base.h"
%include "../../../common/zone_store.h"
%include "../../../common/servertalk.h"


%include "../../common.h"
%include "../../entity.h"
%include "../../bot.h"
%include "../../mob.h"
%include "../../hate_list.h"
%include "../../merc.h"
%include "../../event_codes.h"
%include "../../npc.h"
%include "../../encounter.h"
%include "../../beacon.h"
%include "../../client.h"
%include "../../corpse.h"
%include "../../doors.h"
%include "../../groups.h"
%include "../../object.h"
%include "../../raids.h"
%include "../../trap.h"
%include "../../zone.h"
%include "../../spawn2.h"
%include "../../spawngroup.h"
%include "../../zonedb.h"
%include "../../worldserver.h"
%include "../../questmgr.h"
%include "../../dynamic_zone.h"
%include "../../dotnet_quests/dotnet_runtime.h"

%include "../../aa.h"
%include "../../aa_ability.h"
%include "../../aa_rank.h"
%include "../../command.h"
%include "../../data_bucket.h"
%include "../../expedition.h"
%include "../../expedition_database.h"
%include "../../expedition_request.h"
%include "../../forage.h"
%include "../../global_loot_manager.h"
%include "../../guild_mgr.h"
%include "../../horse.h"
%include "../../pets.h"
%include "../../position.h"
%include "../../titles.h"
%include "../../tasks.h"
%include "../../task_manager.h"
%include "../../task_client_state.h"

%include "../../../common/item_instance.h"
%include "../../../common/item_data.h"
%include "../../../common/eqemu_logsys.h"
%include "../../../common/rulesys.h"



// Typedefs
typedef uint8_t byte;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;


%template(LinkedListNewSpawn) LinkedList<NewSpawn_Struct*>;
%template(LinkedListSpawn2) LinkedList<Spawn2*>;
%template(LinkedListZonePoint) LinkedList<ZonePoint*>;

%template(LinkedListElementNewSpawn) ListElement<NewSpawn_Struct*>;
%template(LinkedListElementSpawn2) ListElement<Spawn2*>;
%template(LinkedListElementZonePoint) ListElement<ZonePoint*>;

%extend ListElement<Spawn2*> {
    Spawn2* GetObject() {
        return $self->GetData();
    }
}

%extend ListElement<NewSpawn_Struct*> {
    NewSpawn_Struct* GetObject() {
        return $self->GetData();
    }
}

%template(ExtraDataVector) std::vector<std::any>;
%template(StringVector) std::vector<std::string>;
%template(ItemVector) std::vector<EQ::ItemInstance*>;
%template(MobVector) std::vector<Mob*>;
%template(PacketVector) std::vector<EQApplicationPacket*>;
%template(AltCurrencyDefinitionList) std::list<AltCurrencyDefinition_Struct>;
%template(InternalVeteranRewardList) std::list<InternalVeteranReward>;

%template(IntStringMap) std::unordered_map<int, std::string>;
%template(IntDoubleMap) std::unordered_map<int, double>;
%template(IntExpModifierMap) std::unordered_map<unsigned int, EXPModifier>;
%template(IntNpcMap) std::unordered_map<unsigned int, NPCType*>;
%template(IntClientMap) std::unordered_map<unsigned short, Client*>;
%template(IntMobMap) std::unordered_map<unsigned short, Mob *>;
%template(IntNpcMap2) std::unordered_map<unsigned short, NPC *>;
%template(IntMercMap) std::unordered_map<unsigned short, Merc *>;
%template(IntCorpseMap) std::unordered_map<unsigned short, Corpse *>;
%template(IntObjectMap) std::unordered_map<unsigned short, Object *>;
%template(IntDoorMap) std::unordered_map<unsigned short, Doors *>;
%template(IntTrapMap) std::unordered_map<unsigned short, Trap *>;
%template(IntBeaconMap) std::unordered_map<unsigned short, Beacon *>;
%template(IntEncounterMap) std::unordered_map<unsigned short, Encounter *>;
%template(ProximityList) std::list<NPC *>;
%template(GroupList) std::list<Group *>;
%template(RaidList) std::list<Raid *>;
%template(AreaList) std::list<Area>;
%template(ArrayHateList) std::list<struct_HateList*>;