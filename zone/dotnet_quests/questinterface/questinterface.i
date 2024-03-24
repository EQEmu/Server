



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

%ignore Merc::GetRawACNoShield;
%ignore Raid::RemoveGroupLeader;
%ignore EQApplicationPacket::combine;

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

// init default values in constructors
#define GLM_FORCE_CTOR_INIT

// these included in math/ofVectorMath.h
// we declare some things manually, so some includes are commented out
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include "../../../common/ruletypes.h"
#include "../../../common/eq_packet.h"
#include "../../../common/expedition_lockout_timer.h"
#include "../../../common/eqemu_logsys_log_aliases.h"
#include "../../../common/linked_list.h"


#include "../../common.h"
#include "../../entity.h"
#include "../../mob.h"
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
#include "../../spawn2.h"
#include "../../spawngroup.h"
#include "../../zonedb.h"
#include "../../worldserver.h"
#include "../../questmgr.h"


#include "../../../common/item_instance.h"
#include "../../../common/item_data.h"

#include "../../../common/eqemu_logsys.h"

using namespace EQ;
using namespace Logs;
using namespace glm;
%}

%include <std_list.i>
%include <std_except.i>
%include <std_vector.i>
%include <std_string.i>
%include <std_unordered_map.i>
%include <std_shared_ptr.i>
%include <stdint.i>

namespace glm {
    %typedef int length_t;
    %include "glm.i"
} // namespace



// glm::vec3
// %typemap(cstype) glm::vec3 "Vec3"
// %typemap(imtype) glm::vec3 "Vec3"
// %typemap(csin) glm::vec3 "$csinput"
// %typemap(in) glm::vec3 %{
//     $1 = glm::vec3($input.x, $input.y, $input.z);
// %}
// %typemap(csout) glm::vec3 {
//     return new Vec3($.x, $1.y, $1.z);
// }
// %typemap(out) glm::vec3 %{
//     $result = SWIG_NewPointerObj(new vec3($1.x, $1.y, $1.z), $descriptor(vec3 *), SWIG_POINTER_OWN | SWIG_POINTER_DISOWN);
// %}
// %typemap(out) glm::vec3* {
//     $result = SWIG_NewPointerObj(new glm::vec3($1->x, $1->y, $1->z), $descriptor(glm::vec3 *), SWIG_POINTER_OWN);
// }
// %typemap(out) glm::vec3& {
//     $result = SWIG_NewPointerObj(new vec3($1.x, $1.y, $1.z), $descriptor(vec3 *), SWIG_POINTER_OWN | SWIG_POINTER_DISOWN);
// }

// // glm::vec4
// %typemap(cstype) glm::vec4 "Vec4"
// %typemap(imtype) glm::vec4 "Vec4"
// %typemap(csin) glm::vec4 "$csinput"
// %typemap(in) glm::vec4 %{
//     $1 = glm::vec4($input.x, $input.y, $input.z, $input.w);
// %}
// %typemap(csout) glm::vec4 {
//     return new Vec4($1->x, $1->y, $1->z, $1->w);
// }
// %typemap(out) glm::vec4 %{
//     $result = SWIG_NewPointerObj(new vec4($1.x, $1.y, $1.z, $1.w), $descriptor(Vec4 *), SWIG_POINTER_OWN | SWIG_POINTER_DISOWN);
// %}
// %typemap(out) glm::vec4* {
//     $result = SWIG_NewPointerObj(new glm::vec4($1->x, $1->y, $1->z, $1->w), $descriptor(glm::vec4 *), SWIG_POINTER_OWN);
// }
// %typemap(out) glm::vec4& {
//     $result = SWIG_NewPointerObj(new vec4($1.x, $1.y, $1.z, $1.w), $descriptor(Vec4 *), SWIG_POINTER_OWN | SWIG_POINTER_DISOWN);
// }




%typemap(cstype) glm::vec4*, glm::vec4& "Vec4"
%typemap(imtype) glm::vec4*, glm::vec4& "Vec4"
%typemap(csin) glm::vec4* "new vec4($csinput.x, $csinput.y, $csinput.z, $csinput.w)"
%typemap(csin) glm::vec4& "$csinput"
%typemap(in) glm::vec4* %{
    $1 = new glm::vec4;
    if (!SWIG_IsOK(SWIG_ConvertPtr($input, (void**)$1, $descriptor(glm::vec4 *), 0))) {
        SWIG_exception_fail(SWIG_ArgError(res), "Expected a glm::vec4 object");
    }
%}
%typemap(freearg) glm::vec4* %{
    delete $1;
%}
%typemap(out) glm::vec4* {
    $result = SWIG_NewPointerObj(new glm::vec4($1->x, $1->y, $1->z, $1->w), $descriptor(glm::vec4 *), SWIG_POINTER_OWN);
}
%typemap(out) glm::vec4& {
    $result = SWIG_NewPointerObj(new vec4($1.x, $1.y, $1.z, $1.w), $descriptor(Vec4 *), SWIG_POINTER_OWN | SWIG_POINTER_DISOWN);
}

%typemap(cstype) glm::vec3*, glm::vec3& "Vec3"
%typemap(imtype) glm::vec3*, glm::vec3& "Vec3"
%typemap(csin) glm::vec3* "new vec3($csinput.x, $csinput.y, $csinput.z)"
%typemap(csin) glm::vec3& "$csinput"

%typemap(in) glm::vec3* %{
    $1 = new glm::vec3;
    if (!SWIG_IsOK(SWIG_ConvertPtr($input, (void**)$1, $descriptor(glm::vec3 *), 0))) {
        SWIG_exception_fail(SWIG_ArgError(res), "Expected a glm::vec3 object");
    }
%}
%typemap(freearg) glm::vec3* %{
    delete $1;
%}
%typemap(out) glm::vec3* {
    $result = SWIG_NewPointerObj(new glm::vec3($1->x, $1->y, $1->z), $descriptor(glm::vec3 *), SWIG_POINTER_OWN);
}
%typemap(out) glm::vec3& {
    $result = SWIG_NewPointerObj(new vec3($1.x, $1.y, $1.z), $descriptor(vec3 *), SWIG_POINTER_OWN | SWIG_POINTER_DISOWN);
}

%include "../../../common/ruletypes.h"
%include "../../../common/eq_packet.h"
%include "../../../common/expedition_lockout_timer.h"
%include "../../../common/eqemu_logsys_log_aliases.h"

%ignore Timer;

%include "../../../common/linked_list.h"

//%include "../../../common/emu_constants.h"

%include "../../common.h"
%include "../../entity.h"
%include "../../mob.h"
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

%include "../../../common/item_instance.h"
%include "../../../common/item_data.h"
%include "../../../common/eqemu_logsys.h"


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