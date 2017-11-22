/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/spdat.h"
#include "../common/string_util.h"
#include "../common/misc_functions.h"

#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"

#include <limits.h>
#include <math.h>
#include <sstream>
#include <algorithm>

#ifdef BOTS
#include "bot.h"
#endif

extern EntityList entity_list;

extern Zone* zone;
extern WorldServer worldserver;

Mob::Mob(const char* in_name,
		const char* in_lastname,
		int32		in_cur_hp,
		int32		in_max_hp,
		uint8		in_gender,
		uint16		in_race,
		uint8		in_class,
		bodyType	in_bodytype,
		uint8		in_deity,
		uint8		in_level,
		uint32		in_npctype_id,
		float		in_size,
		float		in_runspeed,
		const glm::vec4& position,
		uint8		in_light,
		uint8		in_texture,
		uint8		in_helmtexture,
		uint16		in_ac,
		uint16		in_atk,
		uint16		in_str,
		uint16		in_sta,
		uint16		in_dex,
		uint16		in_agi,
		uint16		in_int,
		uint16		in_wis,
		uint16		in_cha,
		uint8		in_haircolor,
		uint8		in_beardcolor,
		uint8		in_eyecolor1, // the eyecolors always seem to be the same, maybe left and right eye?
		uint8		in_eyecolor2,
		uint8		in_hairstyle,
		uint8		in_luclinface,
		uint8		in_beard,
		uint32		in_drakkin_heritage,
		uint32		in_drakkin_tattoo,
		uint32		in_drakkin_details,
		EQEmu::TintProfile	in_armor_tint,
		uint8		in_aa_title,
		uint8		in_see_invis, // see through invis/ivu
		uint8		in_see_invis_undead,
		uint8		in_see_hide,
		uint8		in_see_improved_hide,
		int32		in_hp_regen,
		int32		in_mana_regen,
		uint8		in_qglobal,
		uint8		in_maxlevel,
		uint32		in_scalerate,
		uint8		in_armtexture,
		uint8		in_bracertexture,
		uint8		in_handtexture,
		uint8		in_legtexture,
		uint8		in_feettexture
	) :
		attack_timer(2000),
		attack_dw_timer(2000),
		ranged_timer(2000),
		tic_timer(6000),
		mana_timer(2000),
		spellend_timer(0),
		rewind_timer(30000),
		bindwound_timer(10000),
		stunned_timer(0),
		spun_timer(0),
		bardsong_timer(6000),
		gravity_timer(1000),
		viral_timer(0),
		m_FearWalkTarget(-999999.0f, -999999.0f, -999999.0f),
		m_TargetLocation(glm::vec3()),
		m_TargetV(glm::vec3()),
		flee_timer(FLEE_CHECK_TIMER), 
		m_Position(position),
		tmHidden(-1),
		mitigation_ac(0),
		m_specialattacks(eSpecialAttacks::None),
		fix_z_timer(300),
		fix_z_timer_engaged(100),
		attack_anim_timer(1000),
		position_update_melee_push_timer(1000)
{
	targeted = 0;
	tar_ndx = 0;
	tar_vector = 0;
	currently_fleeing = false;

	last_major_update_position = m_Position;
	is_distance_roamer = false;

	AI_Init();
	SetMoving(false);
	moved = false;
	m_RewindLocation = glm::vec3();

	_egnode = nullptr;
	name[0] = 0;
	orig_name[0] = 0;
	clean_name[0] = 0;
	lastname[0] = 0;
	if (in_name) {
		strn0cpy(name, in_name, 64);
		strn0cpy(orig_name, in_name, 64);
	}
	if (in_lastname)
		strn0cpy(lastname, in_lastname, 64);
	cur_hp = in_cur_hp;
	max_hp = in_max_hp;
	base_hp = in_max_hp;
	gender = in_gender;
	race = in_race;
	base_gender = in_gender;
	base_race = in_race;
	class_ = in_class;
	bodytype = in_bodytype;
	orig_bodytype = in_bodytype;
	deity = in_deity;
	level = in_level;
	orig_level = in_level;
	npctype_id = in_npctype_id;
	size = in_size;
	base_size = size;
	runspeed = in_runspeed;
	// neotokyo: sanity check
	if (runspeed < 0 || runspeed > 20)
		runspeed = 1.25f;
	base_runspeed = (int)((float)runspeed * 40.0f);
	// clients
	if (runspeed == 0.7f) {
		base_runspeed = 28;
		walkspeed = 0.3f;
		base_walkspeed = 12;
		fearspeed = 0.625f;
		base_fearspeed = 25;
		// npcs
	}
	else {
		base_walkspeed = base_runspeed * 100 / 265;
		walkspeed = ((float)base_walkspeed) * 0.025f;
		base_fearspeed = base_runspeed * 100 / 127;
		fearspeed = ((float)base_fearspeed) * 0.025f;
	}

	last_hp_percent = 0;
	last_hp = 0;

	current_speed = base_runspeed;

	m_PlayerState = 0;


	// sanity check
	if (runspeed < 0 || runspeed > 20)
		runspeed = 1.25f;

	m_Light.Type[EQEmu::lightsource::LightInnate] = in_light;
	m_Light.Level[EQEmu::lightsource::LightInnate] = EQEmu::lightsource::TypeToLevel(m_Light.Type[EQEmu::lightsource::LightInnate]);
	m_Light.Type[EQEmu::lightsource::LightActive] = m_Light.Type[EQEmu::lightsource::LightInnate];
	m_Light.Level[EQEmu::lightsource::LightActive] = m_Light.Level[EQEmu::lightsource::LightInnate];

	texture = in_texture;
	helmtexture = in_helmtexture;
	armtexture = in_armtexture;
	bracertexture = in_bracertexture;
	handtexture = in_handtexture;
	legtexture = in_legtexture;
	feettexture = in_feettexture;
	multitexture = (armtexture || bracertexture || handtexture || legtexture || feettexture);

	haircolor = in_haircolor;
	beardcolor = in_beardcolor;
	eyecolor1 = in_eyecolor1;
	eyecolor2 = in_eyecolor2;
	hairstyle = in_hairstyle;
	luclinface = in_luclinface;
	beard = in_beard;
	drakkin_heritage = in_drakkin_heritage;
	drakkin_tattoo = in_drakkin_tattoo;
	drakkin_details = in_drakkin_details;
	attack_speed = 0;
	attack_delay = 0;
	slow_mitigation = 0;
	findable = false;
	trackable = true;
	has_shieldequiped = false;
	has_twohandbluntequiped = false;
	has_twohanderequipped = false;
	can_facestab = false;
	has_numhits = false;
	has_MGB = false;
	has_ProjectIllusion = false;
	SpellPowerDistanceMod = 0;
	last_los_check = false;

	if (in_aa_title > 0)
		aa_title = in_aa_title;
	else
		aa_title = 0xFF;
	AC = in_ac;
	ATK = in_atk;
	STR = in_str;
	STA = in_sta;
	DEX = in_dex;
	AGI = in_agi;
	INT = in_int;
	WIS = in_wis;
	CHA = in_cha;
	MR = CR = FR = DR = PR = Corrup = 0;

	ExtraHaste = 0;
	bEnraged = false;

	shield_target = nullptr;
	current_mana = 0;
	max_mana = 0;
	hp_regen = in_hp_regen;
	mana_regen = in_mana_regen;
	oocregen = RuleI(NPC, OOCRegen); //default Out of Combat Regen
	maxlevel = in_maxlevel;
	scalerate = in_scalerate;
	invisible = false;
	invisible_undead = false;
	invisible_animals = false;
	sneaking = false;
	hidden = false;
	improved_hidden = false;
	invulnerable = false;
	IsFullHP = (cur_hp == max_hp);
	qglobal = 0;
	spawned = false;

	InitializeBuffSlots();

	// clear the proc arrays
	int i;
	int j;
	for (j = 0; j < MAX_PROCS; j++)
	{
		PermaProcs[j].spellID = SPELL_UNKNOWN;
		PermaProcs[j].chance = 0;
		PermaProcs[j].base_spellID = SPELL_UNKNOWN;
		PermaProcs[j].level_override = -1;
		SpellProcs[j].spellID = SPELL_UNKNOWN;
		SpellProcs[j].chance = 0;
		SpellProcs[j].base_spellID = SPELL_UNKNOWN;
		SpellProcs[j].level_override = -1;
		DefensiveProcs[j].spellID = SPELL_UNKNOWN;
		DefensiveProcs[j].chance = 0;
		DefensiveProcs[j].base_spellID = SPELL_UNKNOWN;
		DefensiveProcs[j].level_override = -1;
		RangedProcs[j].spellID = SPELL_UNKNOWN;
		RangedProcs[j].chance = 0;
		RangedProcs[j].base_spellID = SPELL_UNKNOWN;
		RangedProcs[j].level_override = -1;
	}

	for (i = EQEmu::textures::textureBegin; i < EQEmu::textures::materialCount; i++)
	{
		armor_tint.Slot[i].Color = in_armor_tint.Slot[i].Color;
	}

	std::fill(std::begin(m_spellHitsLeft), std::end(m_spellHitsLeft), 0);

	m_Delta = glm::vec4();
	animation = 0;

	logging_enabled = false;
	isgrouped = false;
	israidgrouped = false;

	IsHorse = false;

	entity_id_being_looted = 0;
	_appearance = eaStanding;
	pRunAnimSpeed = 0;

	spellend_timer.Disable();
	bardsong_timer.Disable();
	bardsong = 0;
	bardsong_target_id = 0;
	casting_spell_id = 0;
	casting_spell_timer = 0;
	casting_spell_timer_duration = 0;
	casting_spell_inventory_slot = 0;
	casting_spell_aa_id = 0;
	target = 0;

	ActiveProjectileATK = false;
	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++)
	{
		ProjectileAtk[i].increment = 0;
		ProjectileAtk[i].hit_increment = 0;
		ProjectileAtk[i].target_id = 0;
		ProjectileAtk[i].wpn_dmg = 0;
		ProjectileAtk[i].origin_x = 0.0f;
		ProjectileAtk[i].origin_y = 0.0f;
		ProjectileAtk[i].origin_z = 0.0f;
		ProjectileAtk[i].tlast_x = 0.0f;
		ProjectileAtk[i].tlast_y = 0.0f;
		ProjectileAtk[i].ranged_id = 0;
		ProjectileAtk[i].ammo_id = 0;
		ProjectileAtk[i].ammo_slot = 0;
		ProjectileAtk[i].skill = 0;
		ProjectileAtk[i].speed_mod = 0.0f;
	}

	memset(&itembonuses, 0, sizeof(StatBonuses));
	memset(&spellbonuses, 0, sizeof(StatBonuses));
	memset(&aabonuses, 0, sizeof(StatBonuses));
	spellbonuses.AggroRange = -1;
	spellbonuses.AssistRange = -1;
	pLastChange = 0;
	SetPetID(0);
	SetOwnerID(0);
	typeofpet = petNone; // default to not a pet
	petpower = 0;
	held = false;
	gheld = false;
	nocast = false;
	focused = false;
	pet_stop = false;
	pet_regroup = false;
	_IsTempPet = false;
	pet_owner_client = false;
	pet_targetlock_id = 0;

	attacked_count = 0;
	mezzed = false;
	stunned = false;
	silenced = false;
	amnesiad = false;
	inWater = false;
	int m;
	for (m = 0; m < MAX_SHIELDERS; m++)
	{
		shielder[m].shielder_id = 0;
		shielder[m].shielder_bonus = 0;
	}

	destructibleobject = false;
	wandertype = 0;
	pausetype = 0;
	cur_wp = 0;
	m_CurrentWayPoint = glm::vec4();
	cur_wp_pause = 0;
	patrol = 0;
	follow = 0;
	follow_dist = 100;	// Default Distance for Follow
	no_target_hotkey = false;
	flee_mode = false;
	currently_fleeing = false;
	flee_timer.Start();

	permarooted = (runspeed > 0) ? false : true;

	movetimercompleted = false;
	roamer = false;
	rooted = false;
	charmed = false;
	has_virus = false;
	for (i = 0; i < MAX_SPELL_TRIGGER * 2; i++) {
		viral_spells[i] = 0;
	}
	pStandingPetOrder = SPO_Follow;
	pseudo_rooted = false;

	see_invis = in_see_invis;
	see_invis_undead = in_see_invis_undead != 0;
	see_hide = in_see_hide != 0;
	see_improved_hide = in_see_improved_hide != 0;
	qglobal = in_qglobal != 0;

	// Bind wound
	bindwound_timer.Disable();
	bindwound_target = 0;

	trade = new Trade(this);
	// hp event
	nexthpevent = -1;
	nextinchpevent = -1;

	hasTempPet = false;
	count_TempPet = 0;

	m_is_running = false;

	nimbus_effect1 = 0;
	nimbus_effect2 = 0;
	nimbus_effect3 = 0;
	m_targetable = true;

	m_TargetRing = glm::vec3();

	flymode = FlyMode3;
	// Pathing
	PathingLOSState = UnknownLOS;
	PathingLoopCount = 0;
	PathingLastNodeVisited = -1;
	PathingLOSCheckTimer = new Timer(RuleI(Pathing, LOSCheckFrequency));
	PathingRouteUpdateTimerShort = new Timer(RuleI(Pathing, RouteUpdateFrequencyShort));
	PathingRouteUpdateTimerLong = new Timer(RuleI(Pathing, RouteUpdateFrequencyLong));
	DistractedFromGrid = false;
	PathingTraversedNodes = 0;
	hate_list.SetHateOwner(this);

	m_AllowBeneficial = false;
	m_DisableMelee = false;
	for (int i = 0; i < EQEmu::skills::HIGHEST_SKILL + 2; i++) { SkillDmgTaken_Mod[i] = 0; }
	for (int i = 0; i < HIGHEST_RESIST + 2; i++) { Vulnerability_Mod[i] = 0; }

	emoteid = 0;
	endur_upkeep = false;
	degenerating_effects = false;
	PrimaryAggro = false;
	AssistAggro = false;
	npc_assist_cap = 0;
}

Mob::~Mob()
{
	AI_Stop();
	if (GetPet()) {
		if (GetPet()->Charmed())
			GetPet()->BuffFadeByEffect(SE_Charm);
		else
			SetPet(0);
	}

	EQApplicationPacket app;
	CreateDespawnPacket(&app, !IsCorpse());
	Corpse* corpse = entity_list.GetCorpseByID(GetID());
	if(!corpse || (corpse && !corpse->IsPlayerCorpse()))
		entity_list.QueueClients(this, &app, true);

	entity_list.RemoveFromTargets(this, true);

	if(trade) {
		Mob *with = trade->With();
		if(with && with->IsClient()) {
			with->CastToClient()->FinishTrade(with);
			with->trade->Reset();
		}
		delete trade;
	}

	if(HasTempPetsActive()){
		entity_list.DestroyTempPets(this);
	}
	entity_list.UnMarkNPC(GetID());
	safe_delete(PathingLOSCheckTimer);
	safe_delete(PathingRouteUpdateTimerShort);
	safe_delete(PathingRouteUpdateTimerLong);
	UninitializeBuffSlots();

#ifdef BOTS
	LeaveHealRotationTargetPool();
#endif
}

uint32 Mob::GetAppearanceValue(EmuAppearance iAppearance) {
	switch (iAppearance) {
		// 0 standing, 1 sitting, 2 ducking, 3 lieing down, 4 looting
		case eaStanding: {
			return ANIM_STAND;
		}
		case eaSitting: {
			return ANIM_SIT;
		}
		case eaCrouching: {
			return ANIM_CROUCH;
		}
		case eaDead: {
			return ANIM_DEATH;
		}
		case eaLooting: {
			return ANIM_LOOT;
		}
		//to shup up compiler:
		case _eaMaxAppearance:
			break;
	}
	return(ANIM_STAND);
}

void Mob::SetInvisible(uint8 state)
{
	invisible = state;
	SendAppearancePacket(AT_Invis, invisible);
	// Invis and hide breaks charms

	auto formerpet = GetPet();
	if (formerpet && formerpet->GetPetType() == petCharmed && (invisible || hidden || improved_hidden))
		formerpet->BuffFadeByEffect(SE_Charm);
}

//check to see if `this` is invisible to `other`
bool Mob::IsInvisible(Mob* other) const
{
	if(!other)
		return(false);

	uint8 SeeInvisBonus = 0;
	if (IsClient())
		SeeInvisBonus = aabonuses.SeeInvis;

	//check regular invisibility
	if (invisible && invisible > (other->SeeInvisible()))
		return true;

	//check invis vs. undead
	if (other->GetBodyType() == BT_Undead || other->GetBodyType() == BT_SummonedUndead) {
		if(invisible_undead && !other->SeeInvisibleUndead())
			return true;
	}

	//check invis vs. animals...
	if (other->GetBodyType() == BT_Animal){
		if(invisible_animals && !other->SeeInvisible())
			return true;
	}

	if(hidden){
		if(!other->see_hide && !other->see_improved_hide){
			return true;
		}
	}

	if(improved_hidden){
		if(!other->see_improved_hide){
			return true;
		}
	}

	//handle sneaking
	if(sneaking) {
		if(BehindMob(other, GetX(), GetY()) )
			return true;
	}

	return(false);
}

int Mob::_GetWalkSpeed() const {

	if (IsRooted() || IsStunned() || IsMezzed())
		return 0;

	else if (IsPseudoRooted())
		return 0;

	int aa_mod = 0;
	int speed_mod = base_walkspeed;
	int base_run = base_runspeed;
	bool has_horse = false;
	int runspeedcap = RuleI(Character,BaseRunSpeedCap);
	runspeedcap += itembonuses.IncreaseRunSpeedCap + spellbonuses.IncreaseRunSpeedCap + aabonuses.IncreaseRunSpeedCap;
	aa_mod += aabonuses.BaseMovementSpeed;

	if (IsClient()) {
		Mob *horse = entity_list.GetMob(CastToClient()->GetHorseId());
		if (horse) {
			speed_mod = horse->GetBaseRunspeed();
			return speed_mod;
		}
	}

	int spell_mod = spellbonuses.movementspeed + itembonuses.movementspeed;
	int movemod = 0;

	if (spell_mod < 0)
		movemod += spell_mod;
	else if (spell_mod > aa_mod)
		movemod = spell_mod;
	else
		movemod = aa_mod;

	// hard cap
	if (runspeedcap > 225)
		runspeedcap = 225;

	if(movemod < -85) //cap it at moving very very slow
		movemod = -85;

	if (!has_horse && movemod != 0)
		speed_mod += (base_run * movemod / 100);

	if(speed_mod < 1)
		return(0);

	//runspeed cap.
#ifdef BOTS
	if (IsClient() || IsBot())
#else
	if(IsClient())
#endif
	{
		if(speed_mod > runspeedcap)
			speed_mod = runspeedcap;
	}
	return speed_mod;
}

int Mob::_GetRunSpeed() const {
	if (IsRooted() || IsStunned() || IsMezzed() || IsPseudoRooted())
		return 0;

	int aa_mod = 0;
	int speed_mod = base_runspeed;
	int base_walk = base_walkspeed;
	bool has_horse = false;
	if (IsClient())
	{
		if(CastToClient()->GetGMSpeed())
		{
			speed_mod = 325;
		}
		else
		{
			Mob* horse = entity_list.GetMob(CastToClient()->GetHorseId());
			if(horse)
			{
				speed_mod = horse->GetBaseRunspeed();
				base_walk = horse->GetBaseWalkspeed();
				has_horse = true;
			}
		}
	}

	int runspeedcap = RuleI(Character,BaseRunSpeedCap);
	runspeedcap += itembonuses.IncreaseRunSpeedCap + spellbonuses.IncreaseRunSpeedCap + aabonuses.IncreaseRunSpeedCap;

	aa_mod = itembonuses.IncreaseRunSpeedCap + spellbonuses.IncreaseRunSpeedCap + aabonuses.IncreaseRunSpeedCap;
	int spell_mod = spellbonuses.movementspeed + itembonuses.movementspeed;
	int movemod = 0;

	if(spell_mod < 0)
	{
		movemod += spell_mod;
	}
	else if(spell_mod > aa_mod)
	{
		movemod = spell_mod;
	}
	else
	{
		movemod = aa_mod;
	}

	if(movemod < -85) //cap it at moving very very slow
		movemod = -85;

	if (!has_horse && movemod != 0)
	{
#ifdef BOTS
		if (IsClient() || IsBot())
#else
		if (IsClient())
#endif
		{
			speed_mod += (speed_mod * movemod / 100);
		} else {
			if (movemod < 0) {
				speed_mod += (50 * movemod / 100);
				// basically stoped
				if(speed_mod < 1)
				{
					return(0);
				}
				// moving slowly
				if (speed_mod < 8)
					return(8);
			} else {
				speed_mod += GetBaseWalkspeed();
				if (movemod > 50)
					speed_mod += 4;
				if (movemod > 40)
					speed_mod += 3;
			}
		}
	}

	if(speed_mod < 1)
	{
		return(0);
	}
	//runspeed cap.
#ifdef BOTS
	if (IsClient() || IsBot())
#else
	if(IsClient())
#endif
	{
		if(speed_mod > runspeedcap)
			speed_mod = runspeedcap;
	}
	return speed_mod;
}

int Mob::_GetFearSpeed() const {

	if (IsRooted() || IsStunned() || IsMezzed())
		return 0;

	//float speed_mod = fearspeed;
	int speed_mod = GetBaseFearSpeed();

	// use a max of 1.75f in calcs.
	int base_run = std::min(GetBaseRunspeed(), 70);

	int spell_mod = spellbonuses.movementspeed + itembonuses.movementspeed;
	int movemod = 0;

	if(spell_mod < 0)
	{
		movemod += spell_mod;
	}

	if(movemod < -85) //cap it at moving very very slow
		movemod = -85;

	if (IsClient()) {
		if (CastToClient()->GetRunMode())
			speed_mod = GetBaseRunspeed();
		else
			speed_mod = GetBaseWalkspeed();
		if (movemod < 0)
			return GetBaseWalkspeed();
		speed_mod += (base_run * movemod / 100);
		return speed_mod;
	} else {
		int hp_ratio = GetIntHPRatio();
		// very large snares 50% or higher
		if (movemod < -49)
		{
			if (hp_ratio < 25)
			{
				return (0);
			}
			if (hp_ratio < 50)
				return (8);
			else
				return (12);
		}
		if (hp_ratio < 5) {
			speed_mod = base_walkspeed / 3;
		} else if (hp_ratio < 15) {
			speed_mod = base_walkspeed / 2;
		} else if (hp_ratio < 25) {
			speed_mod = base_walkspeed + 1; // add the +1 so they do the run animation
		} else if (hp_ratio < 50) {
			speed_mod *= 82;
			speed_mod /= 100;
		}
		if (movemod > 0) {
			speed_mod += GetBaseWalkspeed();
			if (movemod > 50)
				speed_mod += 4;
			if (movemod > 40)
				speed_mod += 3;
			return speed_mod;
		}
		else if (movemod < 0) {
			speed_mod += (base_run * movemod / 100);
		}
	}
	if (speed_mod < 1)
		return (0);
	if (speed_mod < 9)
		return (8);
	if (speed_mod < 13)
		return (12);

	return speed_mod;
}

int32 Mob::CalcMaxMana() {
	switch (GetCasterClass()) {
		case 'I':
			max_mana = (((GetINT()/2)+1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
			break;
		case 'W':
			max_mana = (((GetWIS()/2)+1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
			break;
		case 'N':
		default:
			max_mana = 0;
			break;
	}
	if (max_mana < 0) {
		max_mana = 0;
	}

	return max_mana;
}

int32 Mob::CalcMaxHP() {
	max_hp = (base_hp + itembonuses.HP + spellbonuses.HP);
	max_hp += max_hp * ((aabonuses.MaxHPChange + spellbonuses.MaxHPChange + itembonuses.MaxHPChange) / 10000.0f);
	return max_hp;
}

int32 Mob::GetItemHPBonuses() {
	int32 item_hp = 0;
	item_hp = itembonuses.HP;
	item_hp += item_hp * itembonuses.MaxHPChange / 10000;
	return item_hp;
}

int32 Mob::GetSpellHPBonuses() {
	int32 spell_hp = 0;
	spell_hp = spellbonuses.HP;
	spell_hp += spell_hp * spellbonuses.MaxHPChange / 10000;
	return spell_hp;
}

char Mob::GetCasterClass() const {
	switch(class_)
	{
	case CLERIC:
	case PALADIN:
	case RANGER:
	case DRUID:
	case SHAMAN:
	case BEASTLORD:
	case CLERICGM:
	case PALADINGM:
	case RANGERGM:
	case DRUIDGM:
	case SHAMANGM:
	case BEASTLORDGM:
		return 'W';
		break;

	case SHADOWKNIGHT:
	case BARD:
	case NECROMANCER:
	case WIZARD:
	case MAGICIAN:
	case ENCHANTER:
	case SHADOWKNIGHTGM:
	case BARDGM:
	case NECROMANCERGM:
	case WIZARDGM:
	case MAGICIANGM:
	case ENCHANTERGM:
		return 'I';
		break;

	default:
		return 'N';
		break;
	}
}

uint8 Mob::GetArchetype() const {
	switch(class_)
	{
	case PALADIN:
	case RANGER:
	case SHADOWKNIGHT:
	case BARD:
	case BEASTLORD:
	case PALADINGM:
	case RANGERGM:
	case SHADOWKNIGHTGM:
	case BARDGM:
	case BEASTLORDGM:
		return ARCHETYPE_HYBRID;
		break;
	case CLERIC:
	case DRUID:
	case SHAMAN:
	case NECROMANCER:
	case WIZARD:
	case MAGICIAN:
	case ENCHANTER:
	case CLERICGM:
	case DRUIDGM:
	case SHAMANGM:
	case NECROMANCERGM:
	case WIZARDGM:
	case MAGICIANGM:
	case ENCHANTERGM:
		return ARCHETYPE_CASTER;
		break;
	case WARRIOR:
	case MONK:
	case ROGUE:
	case BERSERKER:
	case WARRIORGM:
	case MONKGM:
	case ROGUEGM:
	case BERSERKERGM:
		return ARCHETYPE_MELEE;
		break;
	default:
		return ARCHETYPE_HYBRID;
		break;
	}
}

void Mob::CreateSpawnPacket(EQApplicationPacket* app, Mob* ForWho) {
	app->SetOpcode(OP_NewSpawn);
	app->size = sizeof(NewSpawn_Struct);
	app->pBuffer = new uchar[app->size];
	memset(app->pBuffer, 0, app->size);
	NewSpawn_Struct* ns = (NewSpawn_Struct*)app->pBuffer;
	FillSpawnStruct(ns, ForWho);

	if(RuleB(NPC, UseClassAsLastName) && strlen(ns->spawn.lastName) == 0)
	{
		switch(ns->spawn.class_)
		{
		case TRIBUTE_MASTER:
			strcpy(ns->spawn.lastName, "Tribute Master");
			break;
		case ADVENTURERECRUITER:
			strcpy(ns->spawn.lastName, "Adventure Recruiter");
			break;
		case BANKER:
			strcpy(ns->spawn.lastName, "Banker");
			break;
		case ADVENTUREMERCHANT:
			strcpy(ns->spawn.lastName,"Adventure Merchant");
			break;
		case WARRIORGM:
			strcpy(ns->spawn.lastName, "GM Warrior");
			break;
		case PALADINGM:
			strcpy(ns->spawn.lastName, "GM Paladin");
			break;
		case RANGERGM:
			strcpy(ns->spawn.lastName, "GM Ranger");
			break;
		case SHADOWKNIGHTGM:
			strcpy(ns->spawn.lastName, "GM Shadowknight");
			break;
		case DRUIDGM:
			strcpy(ns->spawn.lastName, "GM Druid");
			break;
		case BARDGM:
			strcpy(ns->spawn.lastName, "GM Bard");
			break;
		case ROGUEGM:
			strcpy(ns->spawn.lastName, "GM Rogue");
			break;
		case SHAMANGM:
			strcpy(ns->spawn.lastName, "GM Shaman");
			break;
		case NECROMANCERGM:
			strcpy(ns->spawn.lastName, "GM Necromancer");
			break;
		case WIZARDGM:
			strcpy(ns->spawn.lastName, "GM Wizard");
			break;
		case MAGICIANGM:
			strcpy(ns->spawn.lastName, "GM Magician");
			break;
		case ENCHANTERGM:
			strcpy(ns->spawn.lastName, "GM Enchanter");
			break;
		case BEASTLORDGM:
			strcpy(ns->spawn.lastName, "GM Beastlord");
			break;
		case BERSERKERGM:
			strcpy(ns->spawn.lastName, "GM Berserker");
			break;
		case MERCERNARY_MASTER:
			strcpy(ns->spawn.lastName, "Mercenary Recruiter");
			break;
		default:
			break;
		}
	}
}

void Mob::CreateSpawnPacket(EQApplicationPacket* app, NewSpawn_Struct* ns) {
	app->SetOpcode(OP_NewSpawn);
	app->size = sizeof(NewSpawn_Struct);

	app->pBuffer = new uchar[sizeof(NewSpawn_Struct)];

	// Copy ns directly into packet
	memcpy(app->pBuffer, ns, sizeof(NewSpawn_Struct));

	// Custom packet data
	NewSpawn_Struct* ns2 = (NewSpawn_Struct*)app->pBuffer;
	strcpy(ns2->spawn.name, ns->spawn.name);

	// Set default Last Names for certain Classes if not defined
	if (RuleB(NPC, UseClassAsLastName) && strlen(ns->spawn.lastName) == 0)
	{
		switch (ns->spawn.class_)
		{
			case TRIBUTE_MASTER:
				strcpy(ns2->spawn.lastName, "Tribute Master");
				break;
			case ADVENTURERECRUITER:
				strcpy(ns2->spawn.lastName, "Adventure Recruiter");
				break;
			case BANKER:
				strcpy(ns2->spawn.lastName, "Banker");
				break;
			case ADVENTUREMERCHANT:
				strcpy(ns2->spawn.lastName, "Adventure Merchant");
				break;
			case WARRIORGM:
				strcpy(ns2->spawn.lastName, "GM Warrior");
				break;
			case PALADINGM:
				strcpy(ns2->spawn.lastName, "GM Paladin");
				break;
			case RANGERGM:
				strcpy(ns2->spawn.lastName, "GM Ranger");
				break;
			case SHADOWKNIGHTGM:
				strcpy(ns2->spawn.lastName, "GM Shadowknight");
				break;
			case DRUIDGM:
				strcpy(ns2->spawn.lastName, "GM Druid");
				break;
			case BARDGM:
				strcpy(ns2->spawn.lastName, "GM Bard");
				break;
			case ROGUEGM:
				strcpy(ns2->spawn.lastName, "GM Rogue");
				break;
			case SHAMANGM:
				strcpy(ns2->spawn.lastName, "GM Shaman");
				break;
			case NECROMANCERGM:
				strcpy(ns2->spawn.lastName, "GM Necromancer");
				break;
			case WIZARDGM:
				strcpy(ns2->spawn.lastName, "GM Wizard");
				break;
			case MAGICIANGM:
				strcpy(ns2->spawn.lastName, "GM Magician");
				break;
			case ENCHANTERGM:
				strcpy(ns2->spawn.lastName, "GM Enchanter");
				break;
			case BEASTLORDGM:
				strcpy(ns2->spawn.lastName, "GM Beastlord");
				break;
			case BERSERKERGM:
				strcpy(ns2->spawn.lastName, "GM Berserker");
				break;
			case MERCERNARY_MASTER:
				strcpy(ns2->spawn.lastName, "Mercenary liaison");
				break;
			default:
				strcpy(ns2->spawn.lastName, ns->spawn.lastName);
				break;
		}
	}
	else
	{
		strcpy(ns2->spawn.lastName, ns->spawn.lastName);
	}
	memset(&app->pBuffer[sizeof(Spawn_Struct)-7], 0xFF, 7);
}

void Mob::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho)
{
	int i;

	strcpy(ns->spawn.name, name);
	if(IsClient()) {
		strn0cpy(ns->spawn.lastName, lastname, sizeof(ns->spawn.lastName));
	}

	ns->spawn.heading	= FloatToEQ19(m_Position.w);
	ns->spawn.x			= FloatToEQ19(m_Position.x);//((int32)x_pos)<<3;
	ns->spawn.y			= FloatToEQ19(m_Position.y);//((int32)y_pos)<<3;
	ns->spawn.z			= FloatToEQ19(m_Position.z);//((int32)z_pos)<<3;
	ns->spawn.spawnId	= GetID();
	ns->spawn.curHp	= static_cast<uint8>(GetHPRatio());
	ns->spawn.max_hp	= 100;		//this field needs a better name
	ns->spawn.race		= race;
	ns->spawn.runspeed	= runspeed;
	ns->spawn.walkspeed	= walkspeed;
	ns->spawn.class_	= class_;
	ns->spawn.gender	= gender;
	ns->spawn.level		= level;
	ns->spawn.PlayerState	= m_PlayerState;
	ns->spawn.deity		= deity;
	ns->spawn.animation	= 0;
	ns->spawn.findable	= findable?1:0;

	UpdateActiveLight();
	ns->spawn.light		= m_Light.Type[EQEmu::lightsource::LightActive];

	ns->spawn.showhelm = (helmtexture && helmtexture != 0xFF) ? 1 : 0;

	ns->spawn.invis		= (invisible || hidden) ? 1 : 0;	// TODO: load this before spawning players
	ns->spawn.NPC		= IsClient() ? 0 : 1;
	ns->spawn.IsMercenary = IsMerc() ? 1 : 0;
	ns->spawn.targetable_with_hotkey = no_target_hotkey ? 0 : 1; // opposite logic!

	ns->spawn.petOwnerId	= ownerid;

	ns->spawn.haircolor = haircolor;
	ns->spawn.beardcolor = beardcolor;
	ns->spawn.eyecolor1 = eyecolor1;
	ns->spawn.eyecolor2 = eyecolor2;
	ns->spawn.hairstyle = hairstyle;
	ns->spawn.face = luclinface;
	ns->spawn.beard = beard;
	ns->spawn.StandState = GetAppearanceValue(_appearance);
	ns->spawn.drakkin_heritage = drakkin_heritage;
	ns->spawn.drakkin_tattoo = drakkin_tattoo;
	ns->spawn.drakkin_details = drakkin_details;
	ns->spawn.equip_chest2 = GetHerosForgeModel(1) != 0 || multitexture? 0xff : texture;

//	ns->spawn.invis2 = 0xff;//this used to be labeled beard.. if its not FF it will turn mob invis

	if (helmtexture && helmtexture != 0xFF && GetHerosForgeModel(0) == 0)
	{
		ns->spawn.helm=helmtexture;
	} else {
		ns->spawn.helm = 0;
	}

	ns->spawn.guildrank	= 0xFF;
	ns->spawn.size = size;
	ns->spawn.bodytype = bodytype;
	// The 'flymode' settings have the following effect:
	// 0 - Mobs in water sink like a stone to the bottom
	// 1 - Same as #flymode 1
	// 2 - Same as #flymode 2
	// 3 - Mobs in water do not sink. A value of 3 in this field appears to be the default setting for all mobs
	// (in water or not) according to 6.2 era packet collects.
	if(IsClient())
		ns->spawn.flymode = FindType(SE_Levitate) ? 2 : 0;
	else
		ns->spawn.flymode = flymode;

	if(IsBoat()) {
		ns->spawn.flymode = 1;
	}

	ns->spawn.lastName[0] = '\0';

	strn0cpy(ns->spawn.lastName, lastname, sizeof(ns->spawn.lastName));

	//for (i = 0; i < _MaterialCount; i++)
	for (i = 0; i < 9; i++)
	{
		// Only Player Races Wear Armor
		if (Mob::IsPlayerRace(race) || i > 6)
		{
			ns->spawn.equipment.Slot[i].Material = GetEquipmentMaterial(i);
			ns->spawn.equipment.Slot[i].EliteModel = IsEliteMaterialItem(i);
			ns->spawn.equipment.Slot[i].HerosForgeModel = GetHerosForgeModel(i);
			ns->spawn.equipment_tint.Slot[i].Color = GetEquipmentColor(i);
		}
	}

	memset(ns->spawn.set_to_0xFF, 0xFF, sizeof(ns->spawn.set_to_0xFF));
	if(IsNPC() && IsDestructibleObject())
	{
		ns->spawn.DestructibleObject = true;

		// Changing the first string made it vanish, so it has some significance.
		if(lastname)
			sprintf(ns->spawn.DestructibleModel, "%s", lastname);
		// Changing the second string made no visible difference
		sprintf(ns->spawn.DestructibleName2, "%s", ns->spawn.name);
		// Putting a string in the final one that was previously empty had no visible effect.
		sprintf(ns->spawn.DestructibleString, "");

		// Sets damage appearance level of the object.
		ns->spawn.DestructibleAppearance = luclinface; // Was 0x00000000
		//ns->spawn.DestructibleAppearance = static_cast<EmuAppearance>(_appearance);
		// #appearance 44 1 makes it jump but no visible damage
		// #appearance 44 2 makes it look completely broken but still visible
		// #appearance 44 3 makes it jump but not visible difference to 3
		// #appearance 44 4 makes it disappear altogether
		// #appearance 44 5 makes the client crash.

		ns->spawn.DestructibleUnk1 = 0x00000224;	// Was 0x000001f5;
		// These next 4 are mostly always sequential
		// Originally they were 633, 634, 635, 636
		// Changing them all to 633 - no visible effect.
		// Changing them all to 636 - no visible effect.
		// Reversing the order of these four numbers and then using #appearance gain had no visible change.
		// Setting these four ids to zero had no visible effect when the catapult spawned, nor when #appearance was used.
		ns->spawn.DestructibleID1 = 1968;
		ns->spawn.DestructibleID2 = 1969;
		ns->spawn.DestructibleID3 = 1970;
		ns->spawn.DestructibleID4 = 1971;
		// Next one was originally 0x1ce45008, changing it to 0x00000000 made no visible difference
		ns->spawn.DestructibleUnk2 = 0x13f79d00;
		// Next one was originally 0x1a68fe30, changing it to 0x00000000 made no visible difference
		ns->spawn.DestructibleUnk3 = 0x00000000;
		// Next one was already 0x00000000
		ns->spawn.DestructibleUnk4 = 0x13f79d58;
		// Next one was originally 0x005a69ec, changing it to 0x00000000 made no visible difference.
		ns->spawn.DestructibleUnk5 = 0x13c55b00;
		// Next one was originally 0x1a68fe30, changing it to 0x00000000 made no visible difference.
		ns->spawn.DestructibleUnk6 = 0x00128860;
		// Next one was originally 0x0059de6d, changing it to 0x00000000 made no visible difference.
		ns->spawn.DestructibleUnk7 = 0x005a8f66;
		// Next one was originally 0x00000201, changing it to 0x00000000 made no visible difference.
		// For the Minohten tents, 0x00000000 had them up in the air, while 0x201 put them on the ground.
		// Changing it it 0x00000001 makes the tent sink into the ground.
		ns->spawn.DestructibleUnk8 = 0x01;			// Needs to be 1 for tents?
		ns->spawn.DestructibleUnk9 = 0x00000002;	// Needs to be 2 for tents?

		ns->spawn.flymode = 0;
	}
}

void Mob::CreateDespawnPacket(EQApplicationPacket* app, bool Decay)
{
	app->SetOpcode(OP_DeleteSpawn);
	app->size = sizeof(DeleteSpawn_Struct);
	app->pBuffer = new uchar[app->size];
	memset(app->pBuffer, 0, app->size);
	DeleteSpawn_Struct* ds = (DeleteSpawn_Struct*)app->pBuffer;
	ds->spawn_id = GetID();
	// The next field only applies to corpses. If 0, they vanish instantly, otherwise they 'decay'
	ds->Decay = Decay ? 1 : 0;
}

void Mob::CreateHPPacket(EQApplicationPacket* app)
{
	this->IsFullHP=(cur_hp>=max_hp);
	app->SetOpcode(OP_MobHealth);
	app->size = sizeof(SpawnHPUpdate_Struct2);
	app->pBuffer = new uchar[app->size];
	memset(app->pBuffer, 0, sizeof(SpawnHPUpdate_Struct2));
	SpawnHPUpdate_Struct2* ds = (SpawnHPUpdate_Struct2*)app->pBuffer;

	ds->spawn_id = GetID();
	// they don't need to know the real hp
	ds->hp = (int)GetHPRatio();

	// hp event
	if (IsNPC() && (GetNextHPEvent() > 0))
	{
		if (ds->hp < GetNextHPEvent())
		{
			char buf[10];
			snprintf(buf, 9, "%i", GetNextHPEvent());
			buf[9] = '\0';
			SetNextHPEvent(-1);
			parse->EventNPC(EVENT_HP, CastToNPC(), nullptr, buf, 0);
		}
	}

	if (IsNPC() && (GetNextIncHPEvent() > 0))
	{
		if (ds->hp > GetNextIncHPEvent())
		{
			char buf[10];
			snprintf(buf, 9, "%i", GetNextIncHPEvent());
			buf[9] = '\0';
			SetNextIncHPEvent(-1);
			parse->EventNPC(EVENT_HP, CastToNPC(), nullptr, buf, 1);
		}
	}
}

// sends hp update of this mob to people who might care
void Mob::SendHPUpdate(bool skip_self /*= false*/, bool force_update_all /*= false*/)
{
	
	/* If our HP is different from last HP update call - let's update ourself */
	if (IsClient()) {
		if (cur_hp != last_hp || force_update_all) {
			/* This is to prevent excessive packet sending under trains/fast combat */
			if (this->CastToClient()->hp_self_update_throttle_timer.Check() || force_update_all) {
				Log(Logs::General, Logs::HP_Update,
					"Mob::SendHPUpdate :: Update HP of self (%s) HP: %i last: %i skip_self: %s",
					this->GetCleanName(),
					cur_hp,
					last_hp,
					(skip_self ? "true" : "false")
				);

				if (!skip_self || this->CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::SoD) {
					auto client_packet = new EQApplicationPacket(OP_HPUpdate, sizeof(SpawnHPUpdate_Struct));
					SpawnHPUpdate_Struct* hp_packet_client = (SpawnHPUpdate_Struct*)client_packet->pBuffer;

					hp_packet_client->cur_hp = CastToClient()->GetHP() - itembonuses.HP;
					hp_packet_client->spawn_id = GetID();
					hp_packet_client->max_hp = CastToClient()->GetMaxHP() - itembonuses.HP;

					CastToClient()->QueuePacket(client_packet);

					safe_delete(client_packet);

					ResetHPUpdateTimer();
				}

				/* Used to check if HP has changed to update self next round */
				last_hp = cur_hp;
			}
		}
	}

	int8 current_hp_percent = (max_hp == 0 ? 0 : static_cast<int>(cur_hp * 100 / max_hp));

	Log(Logs::General, Logs::HP_Update, "Mob::SendHPUpdate :: SendHPUpdate %s HP is %i last %i", this->GetCleanName(), current_hp_percent, last_hp_percent);

	if (current_hp_percent == last_hp_percent && !force_update_all) {
		Log(Logs::General, Logs::HP_Update, "Mob::SendHPUpdate :: Same HP - skipping update");
		ResetHPUpdateTimer();
		return;
	}
	else {

		if (IsClient() && RuleB(Character, MarqueeHPUpdates))
			this->CastToClient()->SendHPUpdateMarquee();

		Log(Logs::General, Logs::HP_Update, "Mob::SendHPUpdate :: HP Changed - Send update");

		last_hp_percent = current_hp_percent;
	}

	EQApplicationPacket hp_packet;
	Group *group = nullptr;

	CreateHPPacket(&hp_packet);

	/* Update those who have us targeted */
	entity_list.QueueClientsByTarget(this, &hp_packet, false, 0, false, true, EQEmu::versions::bit_AllClients);

	/* Update those who have us on x-target */
	entity_list.QueueClientsByXTarget(this, &hp_packet, false);

	/* Update groups using Group LAA health name tag counter */
	entity_list.QueueToGroupsForNPCHealthAA(this, &hp_packet);

	/* Update group */
	if(IsGrouped()) {
		group = entity_list.GetGroupByMob(this);
		if(group)
			group->SendHPPacketsFrom(this);
	}

	/* Update Raid */
	if(IsClient()){
		Raid *raid = entity_list.GetRaidByClient(CastToClient());
		if (raid)
			raid->SendHPManaEndPacketsFrom(this);
	}

	/* Pet - Update master - group and raid if exists */
	if(GetOwner() && GetOwner()->IsClient()) {
		GetOwner()->CastToClient()->QueuePacket(&hp_packet, false);
		group = entity_list.GetGroupByClient(GetOwner()->CastToClient());

		if(group)
			group->SendHPPacketsFrom(this);

		Raid *raid = entity_list.GetRaidByClient(GetOwner()->CastToClient());
		if(raid)
			raid->SendHPManaEndPacketsFrom(this);
	}

	/* Send to pet */
	if(GetPet() && GetPet()->IsClient()) {
		GetPet()->CastToClient()->QueuePacket(&hp_packet, false);
	}

	/* Destructible objects */
	if (IsNPC() && IsDestructibleObject()) {
		if (GetHPRatio() > 74) {
			if (GetAppearance() != eaStanding) {
				SendAppearancePacket(AT_DamageState, eaStanding);
				_appearance = eaStanding;
			}
		}
		else if (GetHPRatio() > 49) {
			if (GetAppearance() != eaSitting) {
				SendAppearancePacket(AT_DamageState, eaSitting);
				_appearance = eaSitting;
			}
		}
		else if (GetHPRatio() > 24) {
			if (GetAppearance() != eaCrouching) {
				SendAppearancePacket(AT_DamageState, eaCrouching);
				_appearance = eaCrouching;
			}
		}
		else if (GetHPRatio() > 0) {
			if (GetAppearance() != eaDead) {
				SendAppearancePacket(AT_DamageState, eaDead);
				_appearance = eaDead;
			}
		}
		else if (GetAppearance() != eaLooting) {
			SendAppearancePacket(AT_DamageState, eaLooting);
			_appearance = eaLooting;
		}
	}
}

/* Used for mobs standing still - this does not send a delta */
void Mob::SendPosition() {
	auto app = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)app->pBuffer;
	MakeSpawnUpdateNoDelta(spu);

	/* When an NPC has made a large distance change - we should update all clients to prevent "ghosts" */
	if (DistanceSquared(last_major_update_position, m_Position) >= (100 * 100)) {
		entity_list.QueueClients(this, app, true, true);
		last_major_update_position = m_Position;
		is_distance_roamer = true;
	}
	else {
		entity_list.QueueCloseClients(this, app, true, RuleI(Range, MobPositionUpdates), nullptr, false);
	}

	safe_delete(app);
}

void Mob::SendPositionUpdateToClient(Client *client) {
	auto app = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct* spawn_update = (PlayerPositionUpdateServer_Struct*)app->pBuffer;

	if(this->IsMoving())
		MakeSpawnUpdate(spawn_update);
	else
		MakeSpawnUpdateNoDelta(spawn_update);

	client->QueuePacket(app, false);

	safe_delete(app);
}

/* Position updates for mobs on the move */
void Mob::SendPositionUpdate(uint8 iSendToSelf) {
	auto app = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)app->pBuffer;
	MakeSpawnUpdate(spu);

	if (iSendToSelf == 2) {
		if (IsClient()) {
			CastToClient()->FastQueuePacket(&app, false);
		}
	}
	else {
		entity_list.QueueCloseClients(this, app, (iSendToSelf == 0), RuleI(Range, MobPositionUpdates), nullptr, false);
	}
	safe_delete(app);
}

// this is for SendPosition()
void Mob::MakeSpawnUpdateNoDelta(PlayerPositionUpdateServer_Struct *spu) {
	memset(spu, 0xff, sizeof(PlayerPositionUpdateServer_Struct));
	spu->spawn_id = GetID();
	spu->x_pos = FloatToEQ19(m_Position.x);
	spu->y_pos = FloatToEQ19(m_Position.y);
	spu->z_pos = FloatToEQ19(m_Position.z);
	spu->delta_x = NewFloatToEQ13(0);
	spu->delta_y = NewFloatToEQ13(0);
	spu->delta_z = NewFloatToEQ13(0);
	spu->heading = FloatToEQ19(m_Position.w);
	spu->animation = 0;
	spu->delta_heading = NewFloatToEQ13(0);
	spu->padding0002 = 0;
	spu->padding0006 = 7;
	spu->padding0014 = 0x7f;
	spu->padding0018 = 0x5df27;

}

// this is for SendPosUpdate()
void Mob::MakeSpawnUpdate(PlayerPositionUpdateServer_Struct* spu) {
	spu->spawn_id = GetID();
	spu->x_pos = FloatToEQ19(m_Position.x);
	spu->y_pos = FloatToEQ19(m_Position.y);
	spu->z_pos = FloatToEQ19(m_Position.z);
	spu->delta_x = NewFloatToEQ13(m_Delta.x);
	spu->delta_y = NewFloatToEQ13(m_Delta.y);
	spu->delta_z = NewFloatToEQ13(m_Delta.z);
	spu->heading = FloatToEQ19(m_Position.w);
	spu->padding0002 = 0;
	spu->padding0006 = 7;
	spu->padding0014 = 0x7f;
	spu->padding0018 = 0x5df27;
#ifdef BOTS
	if (this->IsClient() || this->IsBot())
#else
	if (this->IsClient())
#endif
		spu->animation = animation;
	else
		spu->animation = pRunAnimSpeed;//animation;

	spu->delta_heading = NewFloatToEQ13(m_Delta.w);
}

void Mob::ShowStats(Client* client)
{
	if (IsClient()) {
		CastToClient()->SendStatsWindow(client, RuleB(Character, UseNewStatsWindow));
	}
	else if (IsCorpse()) {
		if (IsPlayerCorpse()) {
			client->Message(0, "  CharID: %i  PlayerCorpse: %i", CastToCorpse()->GetCharID(), CastToCorpse()->GetCorpseDBID());
		}
		else {
			client->Message(0, "  NPCCorpse", GetID());
		}
	}
	else {
		client->Message(0, "  Level: %i  AC: %i  Class: %i  Size: %1.1f  Haste: %i", GetLevel(), GetAC(), GetClass(), GetSize(), GetHaste());
		client->Message(0, "  HP: %i  Max HP: %i",GetHP(), GetMaxHP());
		client->Message(0, "  Mana: %i  Max Mana: %i", GetMana(), GetMaxMana());
		client->Message(0, "  Total ATK: %i  Worn/Spell ATK (Cap %i): %i", GetATK(), RuleI(Character, ItemATKCap), GetATKBonus());
		client->Message(0, "  STR: %i  STA: %i  DEX: %i  AGI: %i  INT: %i  WIS: %i  CHA: %i", GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA());
		client->Message(0, "  MR: %i  PR: %i  FR: %i  CR: %i  DR: %i Corruption: %i PhR: %i", GetMR(), GetPR(), GetFR(), GetCR(), GetDR(), GetCorrup(), GetPhR());
		client->Message(0, "  Race: %i  BaseRace: %i  Texture: %i  HelmTexture: %i  Gender: %i  BaseGender: %i", GetRace(), GetBaseRace(), GetTexture(), GetHelmTexture(), GetGender(), GetBaseGender());
		if (client->Admin() >= 100)
			client->Message(0, "  EntityID: %i  PetID: %i  OwnerID: %i AIControlled: %i Targetted: %i", GetID(), GetPetID(), GetOwnerID(), IsAIControlled(), targeted);

		if (IsNPC()) {
			NPC *n = CastToNPC();
			uint32 spawngroupid = 0;
			if(n->respawn2 != 0)
				spawngroupid = n->respawn2->SpawnGroupID();
			client->Message(0, "  NPCID: %u  SpawnGroupID: %u Grid: %i LootTable: %u FactionID: %i SpellsID: %u ", GetNPCTypeID(),spawngroupid, n->GetGrid(), n->GetLoottableID(), n->GetNPCFactionID(), n->GetNPCSpellsID());
			client->Message(0, "  Accuracy: %i MerchantID: %i EmoteID: %i Runspeed: %.3f Walkspeed: %.3f", n->GetAccuracyRating(), n->MerchantType, n->GetEmoteID(), static_cast<float>(0.025f * n->GetRunspeed()), static_cast<float>(0.025f * n->GetWalkspeed()));
			n->QueryLoot(client);
		}
		if (IsAIControlled()) {
			client->Message(0, "  AggroRange: %1.0f  AssistRange: %1.0f", GetAggroRange(), GetAssistRange());
		}

		client->Message(0, "  compute_tohit: %i TotalToHit: %i", compute_tohit(EQEmu::skills::SkillHandtoHand), GetTotalToHit(EQEmu::skills::SkillHandtoHand, 0));
		client->Message(0, "  compute_defense: %i TotalDefense: %i", compute_defense(), GetTotalDefense());
		client->Message(0, "  offense: %i mitigation ac: %i", offense(EQEmu::skills::SkillHandtoHand), GetMitigationAC());
	}
}

void Mob::DoAnim(const int animnum, int type, bool ackreq, eqFilterType filter) {
	if (!attack_anim_timer.Check())
		return;

	auto outapp = new EQApplicationPacket(OP_Animation, sizeof(Animation_Struct));
	Animation_Struct* anim = (Animation_Struct*)outapp->pBuffer;
	anim->spawnid = GetID();

	if(type == 0){
		anim->action = animnum;
		anim->speed = 10;
	}
	else {
		anim->action = animnum;
		anim->speed = type;
	}

	entity_list.QueueCloseClients(
		this, /* Sender */
		outapp, /* Packet */
		false, /* Ignore Sender */
		RuleI(Range, Anims),
		0, /* Skip this mob */
		ackreq, /* Packet ACK */
		filter /* eqFilterType filter */
	);

	safe_delete(outapp);
}

void Mob::ShowBuffs(Client* client) {
	if(SPDAT_RECORDS <= 0)
		return;
	client->Message(0, "Buffs on: %s", this->GetName());
	uint32 i;
	uint32 buff_count = GetMaxTotalSlots();
	for (i=0; i < buff_count; i++) {
		if (buffs[i].spellid != SPELL_UNKNOWN) {
			if (spells[buffs[i].spellid].buffdurationformula == DF_Permanent)
				client->Message(0, "  %i: %s: Permanent", i, spells[buffs[i].spellid].name);
			else
				client->Message(0, "  %i: %s: %i tics left", i, spells[buffs[i].spellid].name, buffs[i].ticsremaining);

		}
	}
	if (IsClient()){
		client->Message(0, "itembonuses:");
		client->Message(0, "Atk:%i Ac:%i HP(%i):%i Mana:%i", itembonuses.ATK, itembonuses.AC, itembonuses.HPRegen, itembonuses.HP, itembonuses.Mana);
		client->Message(0, "Str:%i Sta:%i Dex:%i Agi:%i Int:%i Wis:%i Cha:%i",
			itembonuses.STR,itembonuses.STA,itembonuses.DEX,itembonuses.AGI,itembonuses.INT,itembonuses.WIS,itembonuses.CHA);
		client->Message(0, "SvMagic:%i SvFire:%i SvCold:%i SvPoison:%i SvDisease:%i",
				itembonuses.MR,itembonuses.FR,itembonuses.CR,itembonuses.PR,itembonuses.DR);
		client->Message(0, "DmgShield:%i Haste:%i", itembonuses.DamageShield, itembonuses.haste );
		client->Message(0, "spellbonuses:");
		client->Message(0, "Atk:%i Ac:%i HP(%i):%i Mana:%i", spellbonuses.ATK, spellbonuses.AC, spellbonuses.HPRegen, spellbonuses.HP, spellbonuses.Mana);
		client->Message(0, "Str:%i Sta:%i Dex:%i Agi:%i Int:%i Wis:%i Cha:%i",
			spellbonuses.STR,spellbonuses.STA,spellbonuses.DEX,spellbonuses.AGI,spellbonuses.INT,spellbonuses.WIS,spellbonuses.CHA);
		client->Message(0, "SvMagic:%i SvFire:%i SvCold:%i SvPoison:%i SvDisease:%i",
				spellbonuses.MR,spellbonuses.FR,spellbonuses.CR,spellbonuses.PR,spellbonuses.DR);
		client->Message(0, "DmgShield:%i Haste:%i", spellbonuses.DamageShield, spellbonuses.haste );
	}
}

void Mob::ShowBuffList(Client* client) {
	if(SPDAT_RECORDS <= 0)
		return;

	client->Message(0, "Buffs on: %s", this->GetCleanName());
	uint32 i;
	uint32 buff_count = GetMaxTotalSlots();
	for (i = 0; i < buff_count; i++) {
		if (buffs[i].spellid != SPELL_UNKNOWN) {
			if (spells[buffs[i].spellid].buffdurationformula == DF_Permanent)
				client->Message(0, "  %i: %s: Permanent", i, spells[buffs[i].spellid].name);
			else
				client->Message(0, "  %i: %s: %i tics left", i, spells[buffs[i].spellid].name, buffs[i].ticsremaining);
		}
	}
}

void Mob::GMMove(float x, float y, float z, float heading, bool SendUpdate) {

	Route.clear();

	if(IsNPC()) {
		entity_list.ProcessMove(CastToNPC(), x, y, z);
	}

	m_Position.x = x;
	m_Position.y = y;
	m_Position.z = z;
	if (m_Position.w != 0.01)
		this->m_Position.w = heading;
	if(IsNPC())
		CastToNPC()->SaveGuardSpot(true);
	if(SendUpdate)
		SendPosition();
}

void Mob::SendIllusionPacket(uint16 in_race, uint8 in_gender, uint8 in_texture, uint8 in_helmtexture, uint8 in_haircolor, uint8 in_beardcolor, uint8 in_eyecolor1, uint8 in_eyecolor2, uint8 in_hairstyle, uint8 in_luclinface, uint8 in_beard, uint8 in_aa_title, uint32 in_drakkin_heritage, uint32 in_drakkin_tattoo, uint32 in_drakkin_details, float in_size) {

	uint16 BaseRace = GetBaseRace();

	if (in_race == 0)
	{
		race = BaseRace;
		if (in_gender == 0xFF)
			gender = GetBaseGender();
		else
			gender = in_gender;
	}
	else
	{
		race = in_race;
		if (in_gender == 0xFF)
			gender = GetDefaultGender(race, gender);
		else
			gender = in_gender;
	}

	if (in_texture == 0xFF)
	{
		if (IsPlayerRace(in_race))
			texture = 0xFF;
		else
			texture = GetTexture();
	}
	else
	{
		texture = in_texture;
	}

	if (in_helmtexture == 0xFF)
	{
		if (IsPlayerRace(in_race))
			helmtexture = 0xFF;
		else if (in_texture != 0xFF)
			helmtexture = in_texture;
		else
			helmtexture = GetHelmTexture();
	}
	else
	{
		helmtexture = in_helmtexture;
	}

	if (in_haircolor == 0xFF)
		haircolor = GetHairColor();
	else
		haircolor = in_haircolor;

	if (in_beardcolor == 0xFF)
		beardcolor = GetBeardColor();
	else
		beardcolor = in_beardcolor;

	if (in_eyecolor1 == 0xFF)
		eyecolor1 = GetEyeColor1();
	else
		eyecolor1 = in_eyecolor1;

	if (in_eyecolor2 == 0xFF)
		eyecolor2 = GetEyeColor2();
	else
		eyecolor2 = in_eyecolor2;

	if (in_hairstyle == 0xFF)
		hairstyle = GetHairStyle();
	else
		hairstyle = in_hairstyle;

	if (in_luclinface == 0xFF)
		luclinface = GetLuclinFace();
	else
		luclinface = in_luclinface;

	if (in_beard == 0xFF)
		beard = GetBeard();
	else
		beard = in_beard;

	aa_title = in_aa_title;

	if (in_drakkin_heritage == 0xFFFFFFFF)
		drakkin_heritage = GetDrakkinHeritage();
	else
		drakkin_heritage = in_drakkin_heritage;

	if (in_drakkin_tattoo == 0xFFFFFFFF)
		drakkin_tattoo = GetDrakkinTattoo();
	else
		drakkin_tattoo = in_drakkin_tattoo;

	if (in_drakkin_details == 0xFFFFFFFF)
		drakkin_details = GetDrakkinDetails();
	else
		drakkin_details = in_drakkin_details;

	if (in_size <= 0.0f)
		size = GetSize();
	else
		size = in_size;

	// Reset features to Base from the Player Profile
	if (IsClient() && in_race == 0)
	{
		race = CastToClient()->GetBaseRace();
		gender = CastToClient()->GetBaseGender();
		texture = 0xFF;
		helmtexture = 0xFF;
		haircolor = CastToClient()->GetBaseHairColor();
		beardcolor = CastToClient()->GetBaseBeardColor();
		eyecolor1 = CastToClient()->GetBaseEyeColor();
		eyecolor2 = CastToClient()->GetBaseEyeColor();
		hairstyle = CastToClient()->GetBaseHairStyle();
		luclinface = CastToClient()->GetBaseFace();
		beard	= CastToClient()->GetBaseBeard();
		aa_title = 0xFF;
		drakkin_heritage = CastToClient()->GetBaseHeritage();
		drakkin_tattoo = CastToClient()->GetBaseTattoo();
		drakkin_details = CastToClient()->GetBaseDetails();
		switch(race){
			case OGRE:
				size = 9;
				break;
			case TROLL:
				size = 8;
				break;
			case VAHSHIR:
			case BARBARIAN:
				size = 7;
				break;
			case HALF_ELF:
			case WOOD_ELF:
			case DARK_ELF:
			case FROGLOK:
				size = 5;
				break;
			case DWARF:
				size = 4;
				break;
			case HALFLING:
			case GNOME:
				size = 3;
				break;
			default:
				size = 6;
				break;
		}
	}

	auto outapp = new EQApplicationPacket(OP_Illusion, sizeof(Illusion_Struct));
	Illusion_Struct* is = (Illusion_Struct*) outapp->pBuffer;
	is->spawnid = GetID();
	strcpy(is->charname, GetCleanName());
	is->race = race;
	is->gender = gender;
	is->texture = texture;
	is->helmtexture = helmtexture;
	is->haircolor = haircolor;
	is->beardcolor = beardcolor;
	is->beard = beard;
	is->eyecolor1 = eyecolor1;
	is->eyecolor2 = eyecolor2;
	is->hairstyle = hairstyle;
	is->face = luclinface;
	is->drakkin_heritage = drakkin_heritage;
	is->drakkin_tattoo = drakkin_tattoo;
	is->drakkin_details = drakkin_details;
	is->size = size;

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);

	/* Refresh armor and tints after send illusion packet */
	this->SendArmorAppearance();

	Log(Logs::Detail, Logs::Spells, "Illusion: Race = %i, Gender = %i, Texture = %i, HelmTexture = %i, HairColor = %i, BeardColor = %i, EyeColor1 = %i, EyeColor2 = %i, HairStyle = %i, Face = %i, DrakkinHeritage = %i, DrakkinTattoo = %i, DrakkinDetails = %i, Size = %f",
		race, gender, texture, helmtexture, haircolor, beardcolor, eyecolor1, eyecolor2, hairstyle, luclinface, drakkin_heritage, drakkin_tattoo, drakkin_details, size);
}

bool Mob::RandomizeFeatures(bool send_illusion, bool set_variables)
{
	if (IsPlayerRace(GetRace())) {
		uint8 Gender = GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = 0xFF;
		uint8 BeardColor = 0xFF;
		uint8 EyeColor1 = 0xFF;
		uint8 EyeColor2 = 0xFF;
		uint8 HairStyle = 0xFF;
		uint8 LuclinFace = 0xFF;
		uint8 Beard = 0xFF;
		uint32 DrakkinHeritage = 0xFFFFFFFF;
		uint32 DrakkinTattoo = 0xFFFFFFFF;
		uint32 DrakkinDetails = 0xFFFFFFFF;

		// Set some common feature settings
		EyeColor1 = zone->random.Int(0, 9);
		EyeColor2 = zone->random.Int(0, 9);
		LuclinFace = zone->random.Int(0, 7);

		// Adjust all settings based on the min and max for each feature of each race and gender
		switch (GetRace()) {
		case HUMAN:
			HairColor = zone->random.Int(0, 19);
			if (Gender == MALE) {
				BeardColor = HairColor;
				HairStyle = zone->random.Int(0, 3);
				Beard = zone->random.Int(0, 5);
			}
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 2);
			}
			break;
		case BARBARIAN:
			HairColor = zone->random.Int(0, 19);
			LuclinFace = zone->random.Int(0, 87);
			if (Gender == MALE) {
				BeardColor = HairColor;
				HairStyle = zone->random.Int(0, 3);
				Beard = zone->random.Int(0, 5);
			}
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 2);
			}
			break;
		case ERUDITE:
			if (Gender == MALE) {
				BeardColor = zone->random.Int(0, 19);
				Beard = zone->random.Int(0, 5);
				LuclinFace = zone->random.Int(0, 57);
			}
			if (Gender == FEMALE) {
				LuclinFace = zone->random.Int(0, 87);
			}
			break;
		case WOOD_ELF:
			HairColor = zone->random.Int(0, 19);
			if (Gender == MALE) {
				HairStyle = zone->random.Int(0, 3);
			}
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 2);
			}
			break;
		case HIGH_ELF:
			HairColor = zone->random.Int(0, 14);
			if (Gender == MALE) {
				HairStyle = zone->random.Int(0, 3);
				LuclinFace = zone->random.Int(0, 37);
				BeardColor = HairColor;
			}
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 2);
			}
			break;
		case DARK_ELF:
			HairColor = zone->random.Int(13, 18);
			if (Gender == MALE) {
				HairStyle = zone->random.Int(0, 3);
				LuclinFace = zone->random.Int(0, 37);
				BeardColor = HairColor;
			}
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 2);
			}
			break;
		case HALF_ELF:
			HairColor = zone->random.Int(0, 19);
			if (Gender == MALE) {
				HairStyle = zone->random.Int(0, 3);
				LuclinFace = zone->random.Int(0, 37);
				BeardColor = HairColor;
			}
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 2);
			}
			break;
		case DWARF:
			HairColor = zone->random.Int(0, 19);
			BeardColor = HairColor;
			if (Gender == MALE) {
				HairStyle = zone->random.Int(0, 3);
				Beard = zone->random.Int(0, 5);
			}
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 2);
				LuclinFace = zone->random.Int(0, 17);
			}
			break;
		case TROLL:
			EyeColor1 = zone->random.Int(0, 10);
			EyeColor2 = zone->random.Int(0, 10);
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 3);
				HairColor = zone->random.Int(0, 23);
			}
			break;
		case OGRE:
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 3);
				HairColor = zone->random.Int(0, 23);
			}
			break;
		case HALFLING:
			HairColor = zone->random.Int(0, 19);
			if (Gender == MALE) {
				BeardColor = HairColor;
				HairStyle = zone->random.Int(0, 3);
				Beard = zone->random.Int(0, 5);
			}
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 2);
			}
			break;
		case GNOME:
			HairColor = zone->random.Int(0, 24);
			if (Gender == MALE) {
				BeardColor = HairColor;
				HairStyle = zone->random.Int(0, 3);
				Beard = zone->random.Int(0, 5);
			}
			if (Gender == FEMALE) {
				HairStyle = zone->random.Int(0, 2);
			}
			break;
		case IKSAR:
		case VAHSHIR:
			break;
		case FROGLOK:
			LuclinFace = zone->random.Int(0, 9);
		case DRAKKIN:
			HairColor = zone->random.Int(0, 3);
			BeardColor = HairColor;
			EyeColor1 = zone->random.Int(0, 11);
			EyeColor2 = zone->random.Int(0, 11);
			LuclinFace = zone->random.Int(0, 6);
			DrakkinHeritage = zone->random.Int(0, 6);
			DrakkinTattoo = zone->random.Int(0, 7);
			DrakkinDetails = zone->random.Int(0, 7);
			if (Gender == MALE) {
				Beard = zone->random.Int(0, 12);
				HairStyle = zone->random.Int(0, 8);
			}
			if (Gender == FEMALE) {
				Beard = zone->random.Int(0, 3);
				HairStyle = zone->random.Int(0, 7);
			}
			break;
		default:
			break;
		}

		if (set_variables) {
			haircolor = HairColor;
			beardcolor = BeardColor;
			eyecolor1 = EyeColor1;
			eyecolor2 = EyeColor2;
			hairstyle = HairStyle;
			luclinface = LuclinFace;
			beard = Beard;
			drakkin_heritage = DrakkinHeritage;
			drakkin_tattoo = DrakkinTattoo;
			drakkin_details = DrakkinDetails;
		}

		if (send_illusion) {
			SendIllusionPacket(GetRace(), Gender, Texture, HelmTexture, HairColor, BeardColor,
				EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF, DrakkinHeritage,
				DrakkinTattoo, DrakkinDetails);
		}

		return true;
	}
	return false;
}


bool Mob::IsPlayerRace(uint16 in_race) {

	if ((in_race >= HUMAN && in_race <= GNOME) || in_race == IKSAR || in_race == VAHSHIR || in_race == FROGLOK || in_race == DRAKKIN)
	{
		return true;
	}

	return false;
}


uint8 Mob::GetDefaultGender(uint16 in_race, uint8 in_gender) {
	if (Mob::IsPlayerRace(in_race) || in_race == 15 || in_race == 50 || in_race == 57 || in_race == 70 || in_race == 98 || in_race == 118 || in_race == 23) {
		if (in_gender >= 2) {
			// Male default for PC Races
			return 0;
		}
		else
			return in_gender;
	}
	else if (in_race == 44 || in_race == 52 || in_race == 55 || in_race == 65 || in_race == 67 || in_race == 88 || in_race == 117 || in_race == 127 ||
		in_race == 77 || in_race == 78 || in_race == 81 || in_race == 90 || in_race == 92 || in_race == 93 || in_race == 94 || in_race == 106 || in_race == 112 || in_race == 471) {
		// Male only races
		return 0;

	}
	else if (in_race == 25 || in_race == 56) {
		// Female only races
		return 1;
	}
	else {
		// Neutral default for NPC Races
		return 2;
	}
}

void Mob::SendAppearancePacket(uint32 type, uint32 value, bool WholeZone, bool iIgnoreSelf, Client *specific_target) {
	if (!GetID())
		return;
	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* appearance = (SpawnAppearance_Struct*)outapp->pBuffer;
	appearance->spawn_id = this->GetID();
	appearance->type = type;
	appearance->parameter = value;
	if (WholeZone)
		entity_list.QueueClients(this, outapp, iIgnoreSelf);
	else if(specific_target != nullptr)
		specific_target->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
	else if (this->IsClient())
		this->CastToClient()->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
	safe_delete(outapp);
}

void Mob::SendLevelAppearance(){
	auto outapp = new EQApplicationPacket(OP_LevelAppearance, sizeof(LevelAppearance_Struct));
	LevelAppearance_Struct* la = (LevelAppearance_Struct*)outapp->pBuffer;
	la->parm1 = 0x4D;
	la->parm2 = la->parm1 + 1;
	la->parm3 = la->parm2 + 1;
	la->parm4 = la->parm3 + 1;
	la->parm5 = la->parm4 + 1;
	la->spawn_id = GetID();
	la->value1a = 1;
	la->value2a = 2;
	la->value3a = 1;
	la->value3b = 1;
	la->value4a = 1;
	la->value4b = 1;
	la->value5a = 2;
	entity_list.QueueCloseClients(this,outapp);
	safe_delete(outapp);
}

void Mob::SendStunAppearance()
{
	auto outapp = new EQApplicationPacket(OP_LevelAppearance, sizeof(LevelAppearance_Struct));
	LevelAppearance_Struct* la = (LevelAppearance_Struct*)outapp->pBuffer;
	la->parm1 = 58;
	la->parm2 = 60;
	la->spawn_id = GetID();
	la->value1a = 2;
	la->value1b = 0;
	la->value2a = 2;
	la->value2b = 0;
	entity_list.QueueCloseClients(this,outapp);
	safe_delete(outapp);
}

void Mob::SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5, Client *specific_target){
	auto outapp = new EQApplicationPacket(OP_LevelAppearance, sizeof(LevelAppearance_Struct));
	LevelAppearance_Struct* la = (LevelAppearance_Struct*)outapp->pBuffer;
	la->spawn_id = GetID();
	la->parm1 = parm1;
	la->parm2 = parm2;
	la->parm3 = parm3;
	la->parm4 = parm4;
	la->parm5 = parm5;
	// Note that setting the b values to 0 will disable the related effect from the corresponding parameter.
	// Setting the a value appears to have no affect at all.s
	la->value1a = 1;
	la->value1b = 1;
	la->value2a = 1;
	la->value2b = 1;
	la->value3a = 1;
	la->value3b = 1;
	la->value4a = 1;
	la->value4b = 1;
	la->value5a = 1;
	la->value5b = 1;
	if(specific_target == nullptr) {
		entity_list.QueueClients(this,outapp);
	}
	else if (specific_target->IsClient()) {
		specific_target->CastToClient()->QueuePacket(outapp, false);
	}
	safe_delete(outapp);
}

void Mob::SendTargetable(bool on, Client *specific_target) {
	auto outapp = new EQApplicationPacket(OP_Untargetable, sizeof(Untargetable_Struct));
	Untargetable_Struct *ut = (Untargetable_Struct*)outapp->pBuffer;
	ut->id = GetID();
	ut->targetable_flag = on == true ? 1 : 0;

	if(specific_target == nullptr) {
		entity_list.QueueClients(this, outapp);
	}
	else if (specific_target->IsClient()) {
		specific_target->CastToClient()->QueuePacket(outapp);
	}
	safe_delete(outapp);
}

void Mob::CameraEffect(uint32 duration, uint32 intensity, Client *c, bool global) {


	if(global == true)
	{
		auto pack = new ServerPacket(ServerOP_CameraShake, sizeof(ServerCameraShake_Struct));
		ServerCameraShake_Struct* scss = (ServerCameraShake_Struct*) pack->pBuffer;
		scss->duration = duration;
		scss->intensity = intensity;
		worldserver.SendPacket(pack);
		safe_delete(pack);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_CameraEffect, sizeof(Camera_Struct));
	Camera_Struct* cs = (Camera_Struct*) outapp->pBuffer;
	cs->duration = duration;	// Duration in milliseconds
	cs->intensity = ((intensity * 6710886) + 1023410176);	// Intensity ranges from 1023410176 to 1090519040, so simplify it from 0 to 10.

	if(c)
		c->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
	else
		entity_list.QueueClients(this, outapp);

	safe_delete(outapp);
}

void Mob::SendSpellEffect(uint32 effectid, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020, bool perm_effect, Client *c) {

	auto outapp = new EQApplicationPacket(OP_SpellEffect, sizeof(SpellEffect_Struct));
	SpellEffect_Struct* se = (SpellEffect_Struct*) outapp->pBuffer;
	se->EffectID = effectid;	// ID of the Particle Effect
	se->EntityID = GetID();
	se->EntityID2 = GetID();	// EntityID again
	se->Duration = duration;	// In Milliseconds
	se->FinishDelay = finish_delay;	// Seen 0
	se->Unknown020 = unk020;	// Seen 3000
	se->Unknown024 = 1;		// Seen 1 for SoD
	se->Unknown025 = 1;		// Seen 1 for Live
	se->Unknown026 = 0;		// Seen 1157

	if(c)
		c->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
	else if(zone_wide)
		entity_list.QueueClients(this, outapp);
	else
		entity_list.QueueCloseClients(this, outapp);

	safe_delete(outapp);

	if (perm_effect) {
		if(!IsNimbusEffectActive(effectid)) {
			SetNimbusEffect(effectid);
		}
	}

}

void Mob::TempName(const char *newname)
{
	char temp_name[64];
	char old_name[64];
	strn0cpy(old_name, GetName(), 64);

	if(newname)
		strn0cpy(temp_name, newname, 64);

	// Reset the name to the original if left null.
	if(!newname) {
		strn0cpy(temp_name, GetOrigName(), 64);
		SetName(temp_name);
		//CleanMobName(GetName(), temp_name);
		strn0cpy(temp_name, GetCleanName(), 64);
	}

	// Remove Numbers before making name unique
	EntityList::RemoveNumbers(temp_name);
	// Make the new name unique and set it
	entity_list.MakeNameUnique(temp_name);

	// Send the new name to all clients
	auto outapp = new EQApplicationPacket(OP_MobRename, sizeof(MobRename_Struct));
	MobRename_Struct* mr = (MobRename_Struct*) outapp->pBuffer;
	strn0cpy(mr->old_name, old_name, 64);
	strn0cpy(mr->old_name_again, old_name, 64);
	strn0cpy(mr->new_name, temp_name, 64);
	mr->unknown192 = 0;
	mr->unknown196 = 1;
	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);

	SetName(temp_name);
}

void Mob::SetTargetable(bool on) {
	if(m_targetable != on) {
		m_targetable = on;
		SendTargetable(on);
	}
}

const int32& Mob::SetMana(int32 amount)
{
	CalcMaxMana();
	int32 mmana = GetMaxMana();
	current_mana = amount < 0 ? 0 : (amount > mmana ? mmana : amount);
/*
	if(IsClient())
		LogFile->write(EQEMuLog::Debug, "Setting mana for %s to %d (%4.1f%%)", GetName(), amount, GetManaRatio());
*/

	return current_mana;
}


void Mob::SetAppearance(EmuAppearance app, bool iIgnoreSelf) {
	if (_appearance == app)
		return;
	_appearance = app;
	SendAppearancePacket(AT_Anim, GetAppearanceValue(app), true, iIgnoreSelf);
	if (this->IsClient() && this->IsAIControlled())
		SendAppearancePacket(AT_Anim, ANIM_FREEZE, false, false);
}

bool Mob::UpdateActiveLight()
{
	uint8 old_light_level = m_Light.Level[EQEmu::lightsource::LightActive];

	m_Light.Type[EQEmu::lightsource::LightActive] = 0;
	m_Light.Level[EQEmu::lightsource::LightActive] = 0;

	if (EQEmu::lightsource::IsLevelGreater((m_Light.Type[EQEmu::lightsource::LightInnate] & 0x0F), m_Light.Type[EQEmu::lightsource::LightActive])) { m_Light.Type[EQEmu::lightsource::LightActive] = m_Light.Type[EQEmu::lightsource::LightInnate]; }
	if (m_Light.Level[EQEmu::lightsource::LightEquipment] > m_Light.Level[EQEmu::lightsource::LightActive]) { m_Light.Type[EQEmu::lightsource::LightActive] = m_Light.Type[EQEmu::lightsource::LightEquipment]; } // limiter in property handler
	if (m_Light.Level[EQEmu::lightsource::LightSpell] > m_Light.Level[EQEmu::lightsource::LightActive]) { m_Light.Type[EQEmu::lightsource::LightActive] = m_Light.Type[EQEmu::lightsource::LightSpell]; } // limiter in property handler

	m_Light.Level[EQEmu::lightsource::LightActive] = EQEmu::lightsource::TypeToLevel(m_Light.Type[EQEmu::lightsource::LightActive]);

	return (m_Light.Level[EQEmu::lightsource::LightActive] != old_light_level);
}

void Mob::ChangeSize(float in_size = 0, bool bNoRestriction) {
	// Size Code
	if (!bNoRestriction)
	{
		if (this->IsClient() || this->petid != 0)
			if (in_size < 3.0)
				in_size = 3.0;


			if (this->IsClient() || this->petid != 0)
				if (in_size > 15.0)
					in_size = 15.0;
	}


	if (in_size < 1.0)
		in_size = 1.0;

	if (in_size > 255.0)
		in_size = 255.0;
	//End of Size Code
	this->size = in_size;
	SendAppearancePacket(AT_Size, (uint32) in_size);
}

Mob* Mob::GetOwnerOrSelf() {
	if (!GetOwnerID())
		return this;
	Mob* owner = entity_list.GetMob(this->GetOwnerID());
	if (!owner) {
		SetOwnerID(0);
		return(this);
	}
	if (owner->GetPetID() == this->GetID()) {
		return owner;
	}
	if(IsNPC() && CastToNPC()->GetSwarmInfo()){
		return (CastToNPC()->GetSwarmInfo()->GetOwner());
	}
	SetOwnerID(0);
	return this;
}

Mob* Mob::GetOwner() {
	Mob* owner = entity_list.GetMob(this->GetOwnerID());
	if (owner && owner->GetPetID() == this->GetID()) {

		return owner;
	}
	if(IsNPC() && CastToNPC()->GetSwarmInfo()){
		return (CastToNPC()->GetSwarmInfo()->GetOwner());
	}
	SetOwnerID(0);
	return 0;
}

Mob* Mob::GetUltimateOwner()
{
	Mob* Owner = GetOwner();

	if(!Owner)
		return this;

	while(Owner && Owner->HasOwner())
		Owner = Owner->GetOwner();

	return Owner ? Owner : this;
}

void Mob::SetOwnerID(uint16 NewOwnerID) {
	if (NewOwnerID == GetID() && NewOwnerID != 0) // ok, no charming yourself now =p
		return;
	ownerid = NewOwnerID;
	// if we're setting the owner ID to 0 and they're not either charmed or not-a-pet then
	// they're a normal pet and should be despawned
	if (ownerid == 0 && IsNPC() && GetPetType() != petCharmed && GetPetType() != petNone)
		Depop();
}

// used in checking for behind (backstab) and checking in front (melee LoS)
float Mob::MobAngle(Mob *other, float ourx, float oury) const {
	if (!other || other == this)
		return 0.0f;

	float angle, lengthb, vectorx, vectory, dotp;
	float mobx = -(other->GetX());	// mob xloc (inverse because eq)
	float moby = other->GetY();		// mob yloc
	float heading = other->GetHeading();	// mob heading
	heading = (heading * 360.0f) / 256.0f;	// convert to degrees
	if (heading < 270)
		heading += 90;
	else
		heading -= 270;

	heading = heading * 3.1415f / 180.0f;	// convert to radians
	vectorx = mobx + (10.0f * cosf(heading));	// create a vector based on heading
	vectory = moby + (10.0f * sinf(heading));	// of mob length 10

	// length of mob to player vector
	lengthb = (float) sqrtf(((-ourx - mobx) * (-ourx - mobx)) + ((oury - moby) * (oury - moby)));

	// calculate dot product to get angle
	// Handle acos domain errors due to floating point rounding errors
	dotp = ((vectorx - mobx) * (-ourx - mobx) +
			(vectory - moby) * (oury - moby)) / (10 * lengthb);
	// I haven't seen any errors that  cause problems that weren't slightly
	// larger/smaller than 1/-1, so only handle these cases for now
	if (dotp > 1)
		return 0.0f;
	else if (dotp < -1)
		return 180.0f;

	angle = acosf(dotp);
	angle = angle * 180.0f / 3.1415f;

	return angle;
}

void Mob::SetZone(uint32 zone_id, uint32 instance_id)
{
	if(IsClient())
	{
		CastToClient()->GetPP().zone_id = zone_id;
		CastToClient()->GetPP().zoneInstance = instance_id;
	}
	Save();
}

void Mob::Kill() {
	Death(this, 0, SPELL_UNKNOWN, EQEmu::skills::SkillHandtoHand);
}

bool Mob::CanThisClassDualWield(void) const {
	if(!IsClient()) {
		return(GetSkill(EQEmu::skills::SkillDualWield) > 0);
	}
	else if (CastToClient()->HasSkill(EQEmu::skills::SkillDualWield)) {
		const EQEmu::ItemInstance* pinst = CastToClient()->GetInv().GetItem(EQEmu::inventory::slotPrimary);
		const EQEmu::ItemInstance* sinst = CastToClient()->GetInv().GetItem(EQEmu::inventory::slotSecondary);

		// 2HS, 2HB, or 2HP
		if(pinst && pinst->IsWeapon()) {
			const EQEmu::ItemData* item = pinst->GetItem();

			if (item->IsType2HWeapon())
				return false;
		}

		// OffHand Weapon
		if(sinst && !sinst->IsWeapon())
			return false;

		// Dual-Wielding Empty Fists
		if(!pinst && !sinst)
			if(class_ != MONK && class_ != MONKGM && class_ != BEASTLORD && class_ != BEASTLORDGM)
				return false;

		return true;
	}

	return false;
}

bool Mob::CanThisClassDoubleAttack(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQEmu::skills::SkillDoubleAttack) > 0);
	} else {
		if(aabonuses.GiveDoubleAttack || itembonuses.GiveDoubleAttack || spellbonuses.GiveDoubleAttack) {
			return true;
		}
		return(CastToClient()->HasSkill(EQEmu::skills::SkillDoubleAttack));
	}
}

bool Mob::CanThisClassTripleAttack() const
{
	if (!IsClient())
		return false; // When they added the real triple attack skill, mobs lost the ability to triple
	else
		return CastToClient()->HasSkill(EQEmu::skills::SkillTripleAttack);
}

bool Mob::IsWarriorClass(void) const
{
	switch(GetClass())
	{
	case WARRIOR:
	case WARRIORGM:
	case ROGUE:
	case ROGUEGM:
	case MONK:
	case MONKGM:
	case PALADIN:
	case PALADINGM:
	case SHADOWKNIGHT:
	case SHADOWKNIGHTGM:
	case RANGER:
	case RANGERGM:
	case BEASTLORD:
	case BEASTLORDGM:
	case BERSERKER:
	case BERSERKERGM:
	case BARD:
	case BARDGM:
		{
			return true;
		}
	default:
		{
			return false;
		}
	}

}

bool Mob::CanThisClassParry(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQEmu::skills::SkillParry) > 0);
	} else {
		return(CastToClient()->HasSkill(EQEmu::skills::SkillParry));
	}
}

bool Mob::CanThisClassDodge(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQEmu::skills::SkillDodge) > 0);
	} else {
		return(CastToClient()->HasSkill(EQEmu::skills::SkillDodge));
	}
}

bool Mob::CanThisClassRiposte(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQEmu::skills::SkillRiposte) > 0);
	} else {
		return(CastToClient()->HasSkill(EQEmu::skills::SkillRiposte));
	}
}

bool Mob::CanThisClassBlock(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQEmu::skills::SkillBlock) > 0);
	} else {
		return(CastToClient()->HasSkill(EQEmu::skills::SkillBlock));
	}
}
/*
float Mob::GetReciprocalHeading(Mob* target) {
	float Result = 0;

	if(target) {
		// Convert to radians
		float h = (target->GetHeading() / 256.0f) * 6.283184f;

		// Calculate the reciprocal heading in radians
		Result = h + 3.141592f;

		// Convert back to eq heading from radians
		Result = (Result / 6.283184f) * 256.0f;
	}

	return Result;
}
*/
bool Mob::PlotPositionAroundTarget(Mob* target, float &x_dest, float &y_dest, float &z_dest, bool lookForAftArc) {
	bool Result = false;

	if(target) {
		float look_heading = 0;

		if(lookForAftArc)
			look_heading = GetReciprocalHeading(target->GetPosition());
		else
			look_heading = target->GetHeading();

		// Convert to sony heading to radians
		look_heading = (look_heading / 256.0f) * 6.283184f;

		float tempX = 0;
		float tempY = 0;
		float tempZ = 0;
		float tempSize = 0;
		const float rangeCreepMod = 0.25;
		const uint8 maxIterationsAllowed = 4;
		uint8 counter = 0;
		float rangeReduction= 0;

		tempSize = target->GetSize();
		rangeReduction = (tempSize * rangeCreepMod);

		while(tempSize > 0 && counter != maxIterationsAllowed) {
			tempX = GetX() + (tempSize * static_cast<float>(sin(double(look_heading))));
			tempY = GetY() + (tempSize * static_cast<float>(cos(double(look_heading))));
			tempZ = target->GetZ();

			if(!CheckLosFN(tempX, tempY, tempZ, tempSize)) {
				tempSize -= rangeReduction;
			}
			else {
				Result = true;
				break;
			}

			counter++;
		}

		if(!Result) {
			// Try to find an attack arc to position at from the opposite direction.
			look_heading += (3.141592 / 2);

			tempSize = target->GetSize();
			counter = 0;

			while(tempSize > 0 && counter != maxIterationsAllowed) {
				tempX = GetX() + (tempSize * static_cast<float>(sin(double(look_heading))));
				tempY = GetY() + (tempSize * static_cast<float>(cos(double(look_heading))));
				tempZ = target->GetZ();

				if(!CheckLosFN(tempX, tempY, tempZ, tempSize)) {
					tempSize -= rangeReduction;
				}
				else {
					Result = true;
					break;
				}

				counter++;
			}
		}

		if(Result) {
			x_dest = tempX;
			y_dest = tempY;
			z_dest = tempZ;
		}
	}

	return Result;
}

bool Mob::HateSummon() {
	// check if mob has ability to summon
	// 97% is the offical % that summoning starts on live, not 94
	if (IsCharmed())
		return false;

	int summon_level = GetSpecialAbility(SPECATK_SUMMON);
	if(summon_level == 1 || summon_level == 2) {
		if(!GetTarget()) {
			return false;
		}
	} else {
		//unsupported summon level or OFF
		return false;
	}

	// validate hp
	int hp_ratio = GetSpecialAbilityParam(SPECATK_SUMMON, 1);
	hp_ratio = hp_ratio > 0 ? hp_ratio : 97;
	if(GetHPRatio() > static_cast<float>(hp_ratio)) {
		return false;
	}

	// now validate the timer
	int summon_timer_duration = GetSpecialAbilityParam(SPECATK_SUMMON, 0);
	summon_timer_duration = summon_timer_duration > 0 ? summon_timer_duration : 6000;
	Timer *timer = GetSpecialAbilityTimer(SPECATK_SUMMON);
	if (!timer)
	{
		StartSpecialAbilityTimer(SPECATK_SUMMON, summon_timer_duration);
	} else {
		if(!timer->Check())
			return false;

		timer->Start(summon_timer_duration);
	}

	// get summon target
	SetTarget(GetHateTop());
	if(target)
	{
		if(summon_level == 1) {
			entity_list.MessageClose(this, true, 500, MT_Say, "%s says,'You will not evade me, %s!' ", GetCleanName(), target->GetCleanName() );

			if (target->IsClient()) {
				target->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), m_Position.x, m_Position.y, m_Position.z, target->GetHeading(), 0, SummonPC);
			}
			else {
#ifdef BOTS
				if(target && target->IsBot()) {
					// set pre summoning info to return to (to get out of melee range for caster)
					target->CastToBot()->SetHasBeenSummoned(true);
					target->CastToBot()->SetPreSummonLocation(glm::vec3(target->GetPosition()));

				}
#endif //BOTS
				target->GMMove(m_Position.x, m_Position.y, m_Position.z, target->GetHeading());
			}

			return true;
		} else if(summon_level == 2) {
			entity_list.MessageClose(this, true, 500, MT_Say, "%s says,'You will not evade me, %s!'", GetCleanName(), target->GetCleanName());
			GMMove(target->GetX(), target->GetY(), target->GetZ());
		}
	}
	return false;
}

void Mob::FaceTarget(Mob* mob_to_face /*= 0*/) {
	Mob* faced_mob = mob_to_face;
	if(!faced_mob) {
		if(!GetTarget()) {
			return;
		}
		else {
			faced_mob = GetTarget();
		}
	}

	float current_heading = GetHeading();
	float new_heading = CalculateHeadingToTarget(faced_mob->GetX(), faced_mob->GetY());
	if(current_heading != new_heading) {
		SetHeading(new_heading);
		if (moving) {
			SendPositionUpdate();
		}
		else {
			SendPosition();
		}
	}

	if(IsNPC() && !IsEngaged()) {
		CastToNPC()->GetRefaceTimer()->Start(15000);
		CastToNPC()->GetRefaceTimer()->Enable();
	}
}

bool Mob::RemoveFromHateList(Mob* mob)
{
	SetRunAnimSpeed(0);
	bool bFound = false;
	if(IsEngaged())
	{
		bFound = hate_list.RemoveEntFromHateList(mob);
		if(hate_list.IsHateListEmpty())
		{
			AI_Event_NoLongerEngaged();
			zone->DelAggroMob();
			if (IsNPC() && !RuleB(Aggro, AllowTickPulling))
				ResetAssistCap();
		}
	}
	if(GetTarget() == mob)
	{
		SetTarget(hate_list.GetEntWithMostHateOnList(this));
	}

	return bFound;
}

void Mob::WipeHateList()
{
	if(IsEngaged())
	{
		hate_list.WipeHateList();
		AI_Event_NoLongerEngaged();
	}
	else
	{
		hate_list.WipeHateList();
	}
}

uint32 Mob::RandomTimer(int min,int max) {
	int r = 14000;
	if(min != 0 && max != 0 && min < max)
	{
		r = zone->random.Int(min, max);
	}
	return r;
}

uint32 NPC::GetEquipment(uint8 material_slot) const
{
	if(material_slot > 8)
		return 0;
	int16 invslot = EQEmu::InventoryProfile::CalcSlotFromMaterial(material_slot);
	if (invslot == INVALID_INDEX)
		return 0;
	return equipment[invslot];
}

void Mob::SendArmorAppearance(Client *one_client)
{
	// one_client of 0 means sent to all clients
	//
	// Despite the fact that OP_NewSpawn and OP_ZoneSpawns include the
	// armor being worn and its mats, the client doesn't update the display
	// on arrival of these packets reliably.
	//
	// Send Wear changes if mob is a PC race and item is an armor slot.
	// The other packets work for primary/secondary.

	if (IsPlayerRace(race))
	{
		if (!IsClient())
		{
			const EQEmu::ItemData *item = nullptr;
			for (int i = 0; i < 7; ++i)
			{
				item = database.GetItem(GetEquipment(i));
				if (item != 0)
				{
					SendWearChange(i, one_client);
				}
			}
		}
	}
}

void Mob::SendWearChange(uint8 material_slot, Client *one_client)
{
	auto outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	WearChange_Struct* wc = (WearChange_Struct*)outapp->pBuffer;

	wc->spawn_id = GetID();
	wc->material = GetEquipmentMaterial(material_slot);
	wc->elite_material = IsEliteMaterialItem(material_slot);
	wc->hero_forge_model = GetHerosForgeModel(material_slot);

#ifdef BOTS
	if (IsBot()) {
		auto item_inst = CastToBot()->GetBotItem(EQEmu::InventoryProfile::CalcSlotFromMaterial(material_slot));
		if (item_inst)
			wc->color.Color = item_inst->GetColor();
		else
			wc->color.Color = 0;
	}
	else {
		wc->color.Color = GetEquipmentColor(material_slot);
	}
#else
	wc->color.Color = GetEquipmentColor(material_slot);
#endif

	wc->wear_slot_id = material_slot;

	if (!one_client)
	{
		entity_list.QueueClients(this, outapp);
	}
	else
	{
		one_client->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
	}

	safe_delete(outapp);
}

void Mob::SendTextureWC(uint8 slot, uint16 texture, uint32 hero_forge_model, uint32 elite_material, uint32 unknown06, uint32 unknown18)
{
	auto outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	WearChange_Struct* wc = (WearChange_Struct*)outapp->pBuffer;

	wc->spawn_id = this->GetID();
	wc->material = texture;
	if (this->IsClient())
		wc->color.Color = GetEquipmentColor(slot);
	else
		wc->color.Color = this->GetArmorTint(slot);
	wc->wear_slot_id = slot;

	wc->unknown06 = unknown06;
	wc->elite_material = elite_material;
	wc->hero_forge_model = hero_forge_model;
	wc->unknown18 = unknown18;


	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

void Mob::SetSlotTint(uint8 material_slot, uint8 red_tint, uint8 green_tint, uint8 blue_tint)
{
	uint32 color;
	color = (red_tint & 0xFF) << 16;
	color |= (green_tint & 0xFF) << 8;
	color |= (blue_tint & 0xFF);
	color |= (color) ? (0xFF << 24) : 0;
	armor_tint.Slot[material_slot].Color = color;

	auto outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	WearChange_Struct* wc = (WearChange_Struct*)outapp->pBuffer;

	wc->spawn_id = this->GetID();
	wc->material = GetEquipmentMaterial(material_slot);
	wc->hero_forge_model = GetHerosForgeModel(material_slot);
	wc->color.Color = color;
	wc->wear_slot_id = material_slot;

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

void Mob::WearChange(uint8 material_slot, uint16 texture, uint32 color, uint32 hero_forge_model)
{
	armor_tint.Slot[material_slot].Color = color;

	auto outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	WearChange_Struct* wc = (WearChange_Struct*)outapp->pBuffer;

	wc->spawn_id = this->GetID();
	wc->material = texture;
	wc->hero_forge_model = hero_forge_model;
	wc->color.Color = color;
	wc->wear_slot_id = material_slot;

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

int32 Mob::GetEquipmentMaterial(uint8 material_slot) const
{
	uint32 equipmaterial = 0;
	int32 ornamentationAugtype = RuleI(Character, OrnamentationAugmentType);
	const EQEmu::ItemData *item = nullptr;
	item = database.GetItem(GetEquipment(material_slot));

	if (item != 0)
	{
		// For primary and secondary we need the model, not the material
		if (material_slot == EQEmu::textures::weaponPrimary || material_slot == EQEmu::textures::weaponSecondary)
		{
			if (this->IsClient())
			{
				int16 invslot = EQEmu::InventoryProfile::CalcSlotFromMaterial(material_slot);
				if (invslot == INVALID_INDEX)
				{
					return 0;
				}
				const EQEmu::ItemInstance* inst = CastToClient()->m_inv[invslot];
				if (inst)
				{
					if (inst->GetOrnamentationAug(ornamentationAugtype))
					{
						item = inst->GetOrnamentationAug(ornamentationAugtype)->GetItem();
						if (item && strlen(item->IDFile) > 2)
						{
							equipmaterial = atoi(&item->IDFile[2]);
						}
					}
					else if (inst->GetOrnamentationIDFile())
					{
						equipmaterial = inst->GetOrnamentationIDFile();
					}
				}
			}

			if (equipmaterial == 0 && strlen(item->IDFile) > 2)
			{
				equipmaterial = atoi(&item->IDFile[2]);
			}
		}
		else
		{
			equipmaterial = item->Material;
		}
	}

	return equipmaterial;
}

int32 Mob::GetHerosForgeModel(uint8 material_slot) const
{
	uint32 HeroModel = 0;
	if (material_slot >= 0 && material_slot < EQEmu::textures::weaponPrimary)
	{
		uint32 ornamentationAugtype = RuleI(Character, OrnamentationAugmentType);
		const EQEmu::ItemData *item = nullptr;
		item = database.GetItem(GetEquipment(material_slot));
		int16 invslot = EQEmu::InventoryProfile::CalcSlotFromMaterial(material_slot);

		if (item != 0 && invslot != INVALID_INDEX)
		{
			if (IsClient())
			{
				const EQEmu::ItemInstance* inst = CastToClient()->m_inv[invslot];
				if (inst)
				{
					if (inst->GetOrnamentationAug(ornamentationAugtype))
					{
						item = inst->GetOrnamentationAug(ornamentationAugtype)->GetItem();
						HeroModel = item->HerosForgeModel;
					}
					else if (inst->GetOrnamentHeroModel())
					{
						HeroModel = inst->GetOrnamentHeroModel();
					}
				}
			}

			if (HeroModel == 0)
			{
				HeroModel = item->HerosForgeModel;
			}
		}

		if (IsNPC())
		{
			HeroModel = CastToNPC()->GetHeroForgeModel();
			// Robes require full model number, and should only be sent to chest, arms, wrists, and legs slots
			if (HeroModel > 1000 && material_slot != 1 && material_slot != 2 && material_slot != 3 && material_slot != 5)
			{
				HeroModel = 0;
			}
		}
	}

	// Auto-Convert Hero Model to match the slot
	// Otherwise, use the exact Model if model is > 999
	// Robes for example are 11607 to 12107 in RoF
	if (HeroModel > 0 && HeroModel < 1000)
	{
		HeroModel *= 100;
		HeroModel += material_slot;
	}

	return HeroModel;
}

uint32 Mob::GetEquipmentColor(uint8 material_slot) const
{
	const EQEmu::ItemData *item = nullptr;

	if (armor_tint.Slot[material_slot].Color)
	{
		return armor_tint.Slot[material_slot].Color;
	}

	item = database.GetItem(GetEquipment(material_slot));
	if (item != 0)
		return item->Color;

	return 0;
}

uint32 Mob::IsEliteMaterialItem(uint8 material_slot) const
{
	const EQEmu::ItemData *item = nullptr;

	item = database.GetItem(GetEquipment(material_slot));
	if(item != 0)
	{
		return item->EliteMaterial;
	}

	return 0;
}

// works just like a printf
void Mob::Say(const char *format, ...)
{
	char buf[1000];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, 1000, format, ap);
	va_end(ap);

	Mob* talker = this;
	if(spellbonuses.VoiceGraft != 0) {
		if(spellbonuses.VoiceGraft == GetPetID())
			talker = entity_list.GetMob(spellbonuses.VoiceGraft);
		else
			spellbonuses.VoiceGraft = 0;
	}

	if(!talker)
		talker = this;

	entity_list.MessageClose_StringID(talker, false, 200, 10,
		GENERIC_SAY, GetCleanName(), buf);
}

//
// this is like the above, but the first parameter is a string id
//
void Mob::Say_StringID(uint32 string_id, const char *message3, const char *message4, const char *message5, const char *message6, const char *message7, const char *message8, const char *message9)
{
	char string_id_str[10];

	snprintf(string_id_str, 10, "%d", string_id);

	entity_list.MessageClose_StringID(this, false, 200, 10,
		GENERIC_STRINGID_SAY, GetCleanName(), string_id_str, message3, message4, message5,
		message6, message7, message8, message9
	);
}

void Mob::Say_StringID(uint32 type, uint32 string_id, const char *message3, const char *message4, const char *message5, const char *message6, const char *message7, const char *message8, const char *message9)
{
	char string_id_str[10];

	snprintf(string_id_str, 10, "%d", string_id);

	entity_list.MessageClose_StringID(this, false, 200, type,
		GENERIC_STRINGID_SAY, GetCleanName(), string_id_str, message3, message4, message5,
		message6, message7, message8, message9
	);
}

void Mob::SayTo_StringID(Client *to, uint32 string_id, const char *message3, const char *message4, const char *message5, const char *message6, const char *message7, const char *message8, const char *message9)
{
	if (!to)
		return;

	auto string_id_str = std::to_string(string_id);

	to->Message_StringID(10, GENERIC_STRINGID_SAY, GetCleanName(), string_id_str.c_str(), message3, message4, message5, message6, message7, message8, message9);
}

void Mob::SayTo_StringID(Client *to, uint32 type, uint32 string_id, const char *message3, const char *message4, const char *message5, const char *message6, const char *message7, const char *message8, const char *message9)
{
	if (!to)
		return;

	auto string_id_str = std::to_string(string_id);

	to->Message_StringID(type, GENERIC_STRINGID_SAY, GetCleanName(), string_id_str.c_str(), message3, message4, message5, message6, message7, message8, message9);
}

void Mob::Shout(const char *format, ...)
{
	char buf[1000];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, 1000, format, ap);
	va_end(ap);

	entity_list.Message_StringID(this, false, MT_Shout,
		GENERIC_SHOUT, GetCleanName(), buf);
}

void Mob::Emote(const char *format, ...)
{
	char buf[1000];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, 1000, format, ap);
	va_end(ap);

	entity_list.MessageClose_StringID(this, false, 200, 10,
		GENERIC_EMOTE, GetCleanName(), buf);
}

void Mob::QuestJournalledSay(Client *QuestInitiator, const char *str)
{
		entity_list.QuestJournalledSayClose(this, QuestInitiator, 200, GetCleanName(), str);
}

const char *Mob::GetCleanName()
{
	if(!strlen(clean_name))
	{
		CleanMobName(GetName(), clean_name);
	}

	return clean_name;
}

// hp event
void Mob::SetNextHPEvent( int hpevent )
{
	nexthpevent = hpevent;
}

void Mob::SetNextIncHPEvent( int inchpevent )
{
	nextinchpevent = inchpevent;
}
//warp for quest function,from sandy
void Mob::Warp(const glm::vec3& location)
{
	if(IsNPC())
		entity_list.ProcessMove(CastToNPC(), location.x, location.y, location.z);

	m_Position = glm::vec4(location, m_Position.w);

	Mob* target = GetTarget();
	if (target)
		FaceTarget( target );

	SendPosition();
}

int16 Mob::GetResist(uint8 type) const
{
	if (IsNPC())
	{
		if (type == 1)
			return MR + spellbonuses.MR + itembonuses.MR;
		else if (type == 2)
			return FR + spellbonuses.FR + itembonuses.FR;
		else if (type == 3)
			return CR + spellbonuses.CR + itembonuses.CR;
		else if (type == 4)
			return PR + spellbonuses.PR + itembonuses.PR;
		else if (type == 5)
			return DR + spellbonuses.DR + itembonuses.DR;
	}
	else if (IsClient())
	{
		if (type == 1)
			return CastToClient()->GetMR();
		else if (type == 2)
			return CastToClient()->GetFR();
		else if (type == 3)
			return CastToClient()->GetCR();
		else if (type == 4)
			return CastToClient()->GetPR();
		else if (type == 5)
			return CastToClient()->GetDR();
	}
	return 25;
}

uint32 Mob::GetLevelHP(uint8 tlevel)
{
	int multiplier = 0;
	if (tlevel < 10)
	{
		multiplier = tlevel*20;
	}
	else if (tlevel < 20)
	{
		multiplier = tlevel*25;
	}
	else if (tlevel < 40)
	{
		multiplier = tlevel*tlevel*12*((tlevel*2+60)/100)/10;
	}
	else if (tlevel < 45)
	{
		multiplier = tlevel*tlevel*15*((tlevel*2+60)/100)/10;
	}
	else if (tlevel < 50)
	{
		multiplier = tlevel*tlevel*175*((tlevel*2+60)/100)/100;
	}
	else
	{
		multiplier = tlevel*tlevel*2*((tlevel*2+60)/100)*(1+((tlevel-50)*20/10));
	}
	return multiplier;
}

int32 Mob::GetActSpellCasttime(uint16 spell_id, int32 casttime) {

	int32 cast_reducer = 0;
	cast_reducer += GetFocusEffect(focusSpellHaste, spell_id);

	if (level >= 60 && casttime > 1000)
	{
		casttime = casttime / 2;
		if (casttime < 1000)
			casttime = 1000;
	} else if (level >= 50 && casttime > 1000) {
		int32 cast_deduction = (casttime*(level - 49))/5;
		if (cast_deduction > casttime/2)
			casttime /= 2;
		else
			casttime -= cast_deduction;
	}

	casttime = (casttime*(100 - cast_reducer)/100);
	return casttime;
}

void Mob::ExecWeaponProc(const EQEmu::ItemInstance *inst, uint16 spell_id, Mob *on, int level_override) {
	// Changed proc targets to look up based on the spells goodEffect flag.
	// This should work for the majority of weapons.
	if(spell_id == SPELL_UNKNOWN || on->GetSpecialAbility(NO_HARM_FROM_CLIENT)) {
		//This is so 65535 doesn't get passed to the client message and to logs because it is not relavant information for debugging.
		return;
	}

	if (IsNoCast())
		return;

	if(!IsValidSpell(spell_id)) { // Check for a valid spell otherwise it will crash through the function
		if(IsClient()){
			Message(0, "Invalid spell proc %u", spell_id);
			Log(Logs::Detail, Logs::Spells, "Player %s, Weapon Procced invalid spell %u", this->GetName(), spell_id);
		}
		return;
	}

	if(inst && IsClient()) {
		//const cast is dirty but it would require redoing a ton of interfaces at this point
		//It should be safe as we don't have any truly const EQEmu::ItemInstance floating around anywhere.
		//So we'll live with it for now
		int i = parse->EventItem(EVENT_WEAPON_PROC, CastToClient(), const_cast<EQEmu::ItemInstance*>(inst), on, "", spell_id);
		if(i != 0) {
			return;
		}
	}

	bool twinproc = false;
	int32 twinproc_chance = 0;

	if(IsClient())
		twinproc_chance = CastToClient()->GetFocusEffect(focusTwincast, spell_id);

	if(twinproc_chance && zone->random.Roll(twinproc_chance))
		twinproc = true;

	if (IsBeneficialSpell(spell_id) && (!IsNPC() || (IsNPC() && CastToNPC()->GetInnateProcSpellID() != spell_id))) { // NPC innate procs don't take this path ever
		SpellFinished(spell_id, this, EQEmu::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff, true, level_override);
		if(twinproc)
			SpellOnTarget(spell_id, this, false, false, 0, true, level_override);
	}
	else if(!(on->IsClient() && on->CastToClient()->dead)) { //dont proc on dead clients
		SpellFinished(spell_id, on, EQEmu::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff, true, level_override);
		if(twinproc)
			SpellOnTarget(spell_id, on, false, false, 0, true, level_override);
	}
	return;
}

uint32 Mob::GetZoneID() const {
	return(zone->GetZoneID());
}

int Mob::GetHaste()
{
	// See notes in Client::CalcHaste
	// Need to check if the effect of inhibit melee differs for NPCs
	if (spellbonuses.haste < 0) {
		if (-spellbonuses.haste <= spellbonuses.inhibitmelee)
			return 100 - spellbonuses.inhibitmelee;
		else
			return 100 + spellbonuses.haste;
	}

	if (spellbonuses.haste == 0 && spellbonuses.inhibitmelee)
		return 100 - spellbonuses.inhibitmelee;

	int h = 0;
	int cap = 0;
	int level = GetLevel();

	if (spellbonuses.haste)
		h += spellbonuses.haste - spellbonuses.inhibitmelee;
	if (spellbonuses.hastetype2 && level > 49)
		h += spellbonuses.hastetype2 > 10 ? 10 : spellbonuses.hastetype2;

	// 26+ no cap, 1-25 10
	if (level > 25) // 26+
		h += itembonuses.haste;
	else // 1-25
		h += itembonuses.haste > 10 ? 10 : itembonuses.haste;

	// mobs are different!
	Mob *owner = nullptr;
	if (IsPet())
		owner = GetOwner();
	else if (IsNPC() && CastToNPC()->GetSwarmOwner())
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());
	if (owner) {
		cap = 10 + level;
		cap += std::max(0, owner->GetLevel() - 39) + std::max(0, owner->GetLevel() - 60);
	} else {
		cap = 150;
	}

	if(h > cap)
		h = cap;

	// 51+ 25 (despite there being higher spells...), 1-50 10
	if (level > 50) // 51+
		h += spellbonuses.hastetype3 > 25 ? 25 : spellbonuses.hastetype3;
	else // 1-50
		h += spellbonuses.hastetype3 > 10 ? 10 : spellbonuses.hastetype3;

	h += ExtraHaste;	//GM granted haste.

	return 100 + h;
}

void Mob::SetTarget(Mob* mob) {

	if (target == mob) 
		return;

	target = mob;
	entity_list.UpdateHoTT(this);
	
	if(IsNPC())
		parse->EventNPC(EVENT_TARGET_CHANGE, CastToNPC(), mob, "", 0);
	else if (IsClient())
		parse->EventPlayer(EVENT_TARGET_CHANGE, CastToClient(), "", 0);

	if(IsPet() && GetOwner() && GetOwner()->IsClient())
		GetOwner()->CastToClient()->UpdateXTargetType(MyPetTarget, mob);

	if (this->IsClient() && this->GetTarget() && this->CastToClient()->hp_other_update_throttle_timer.Check())
		this->GetTarget()->SendHPUpdate(false, true);
}

// For when we want a Ground Z at a location we are not at yet
// Like MoveTo.
float Mob::FindDestGroundZ(glm::vec3 dest, float z_offset)
{
	float best_z = BEST_Z_INVALID;
	if (zone->zonemap != nullptr)
	{
		dest.z += z_offset;
		best_z = zone->zonemap->FindBestZ(dest, nullptr);
	}
	return best_z;
}

float Mob::FindGroundZ(float new_x, float new_y, float z_offset)
{
	float ret = BEST_Z_INVALID;
	if (zone->zonemap != nullptr)
	{
		glm::vec3 me;
		me.x = new_x;
		me.y = new_y;
		me.z = m_Position.z + z_offset;
		glm::vec3 hit;
		float best_z = zone->zonemap->FindBestZ(me, &hit);
		if (best_z != BEST_Z_INVALID)
		{
			ret = best_z;
		}
	}
	return ret;
}

// Copy of above function that isn't protected to be exported to Perl::Mob
float Mob::GetGroundZ(float new_x, float new_y, float z_offset)
{
	float ret = BEST_Z_INVALID;
	if (zone->zonemap != 0)
	{
		glm::vec3 me;
		me.x = new_x;
		me.y = new_y;
		me.z = m_Position.z+z_offset;
		glm::vec3 hit;
		float best_z = zone->zonemap->FindBestZ(me, &hit);
		if (best_z != BEST_Z_INVALID)
		{
			ret = best_z;
		}
	}
	return ret;
}

//helper function for npc AI; needs to be mob:: cause we need to be able to count buffs on other clients and npcs
int Mob::CountDispellableBuffs()
{
	int val = 0;
	int buff_count = GetMaxTotalSlots();
	for(int x = 0; x < buff_count; x++)
	{
		if(!IsValidSpell(buffs[x].spellid))
			continue;

		if(buffs[x].counters)
			continue;

		if(spells[buffs[x].spellid].goodEffect == 0)
			continue;

		if(buffs[x].spellid != SPELL_UNKNOWN &&	spells[buffs[x].spellid].buffdurationformula != DF_Permanent)
			val++;
	}
	return val;
}

// Returns the % that a mob is snared (as a positive value). -1 means not snared
int Mob::GetSnaredAmount()
{
	int worst_snare = -1;

	int buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; i++)
	{
		if (!IsValidSpell(buffs[i].spellid))
			continue;

		for(int j = 0; j < EFFECT_COUNT; j++)
		{
			if (spells[buffs[i].spellid].effectid[j] == SE_MovementSpeed)
			{
				int val = CalcSpellEffectValue_formula(spells[buffs[i].spellid].formula[j], spells[buffs[i].spellid].base[j], spells[buffs[i].spellid].max[j], buffs[i].casterlevel, buffs[i].spellid);
				//int effect = CalcSpellEffectValue(buffs[i].spellid, spells[buffs[i].spellid].effectid[j], buffs[i].casterlevel);
				if (val < 0 && std::abs(val) > worst_snare)
					worst_snare = std::abs(val);
			}
		}
	}

	return worst_snare;
}

void Mob::TriggerDefensiveProcs(Mob *on, uint16 hand, bool FromSkillProc, int damage)
{
	if (!on)
		return;

	if (!FromSkillProc)
		on->TryDefensiveProc(this, hand);

	//Defensive Skill Procs
	if (damage < 0 && damage >= -4) {
		uint16 skillinuse = 0;
		switch (damage) {
			case (-1):
				skillinuse = EQEmu::skills::SkillBlock;
			break;

			case (-2):
				skillinuse = EQEmu::skills::SkillParry;
			break;

			case (-3):
				skillinuse = EQEmu::skills::SkillRiposte;
			break;

			case (-4):
				skillinuse = EQEmu::skills::SkillDodge;
			break;
		}

		if (on->HasSkillProcs())
			on->TrySkillProc(this, skillinuse, 0, false, hand, true);

		if (on->HasSkillProcSuccess())
			on->TrySkillProc(this, skillinuse, 0, true, hand, true);
	}
}

void Mob::SetDelta(const glm::vec4& delta) {
	m_Delta = delta;
}

void Mob::SetEntityVariable(const char *id, const char *m_var)
{
	std::string n_m_var = m_var;
	m_EntityVariables[id] = n_m_var;
}

const char* Mob::GetEntityVariable(const char *id)
{
	auto iter = m_EntityVariables.find(id);
	if(iter != m_EntityVariables.end())
	{
		return iter->second.c_str();
	}
	return nullptr;
}

bool Mob::EntityVariableExists(const char *id)
{
	auto iter = m_EntityVariables.find(id);
	if(iter != m_EntityVariables.end())
	{
		return true;
	}
	return false;
}

void Mob::SetFlyMode(uint8 flymode)
{
	if(IsClient() && flymode >= 0 && flymode < 3)
	{
		this->SendAppearancePacket(AT_Levitate, flymode);
	}
	else if(IsNPC() && flymode >= 0 && flymode <= 3)
	{
		this->SendAppearancePacket(AT_Levitate, flymode);
		this->CastToNPC()->SetFlyMode(flymode);
	}
}

bool Mob::IsNimbusEffectActive(uint32 nimbus_effect)
{
	if(nimbus_effect1 == nimbus_effect || nimbus_effect2 == nimbus_effect || nimbus_effect3 == nimbus_effect)
	{
		return true;
	}
	return false;
}

void Mob::SetNimbusEffect(uint32 nimbus_effect)
{
	if(nimbus_effect1 == 0)
	{
		nimbus_effect1 = nimbus_effect;
	}
	else if(nimbus_effect2 == 0)
	{
		nimbus_effect2 = nimbus_effect;
	}
	else
	{
		nimbus_effect3 = nimbus_effect;
	}
}

void Mob::TryTriggerOnCast(uint32 spell_id, bool aa_trigger)
{
	if(!IsValidSpell(spell_id))
			return;

	if (aabonuses.SpellTriggers[0] || spellbonuses.SpellTriggers[0] || itembonuses.SpellTriggers[0]){

		for(int i = 0; i < MAX_SPELL_TRIGGER; i++){

			if(aabonuses.SpellTriggers[i] && IsClient())
				TriggerOnCast(aabonuses.SpellTriggers[i], spell_id,1);

			if(spellbonuses.SpellTriggers[i])
				TriggerOnCast(spellbonuses.SpellTriggers[i], spell_id,0);

			if(itembonuses.SpellTriggers[i])
				TriggerOnCast(spellbonuses.SpellTriggers[i], spell_id,0);
		}
	}
}

void Mob::TriggerOnCast(uint32 focus_spell, uint32 spell_id, bool aa_trigger)
{
	if (!IsValidSpell(focus_spell) || !IsValidSpell(spell_id))
		return;

	uint32 trigger_spell_id = 0;

	if (aa_trigger && IsClient()) {
		// focus_spell = aaid
		auto rank = zone->GetAlternateAdvancementRank(focus_spell);
		if (rank)
			trigger_spell_id = CastToClient()->CalcAAFocus(focusTriggerOnCast, *rank, spell_id);

		if (IsValidSpell(trigger_spell_id) && GetTarget())
			SpellFinished(trigger_spell_id, GetTarget(), EQEmu::CastingSlot::Item, 0, -1,
				      spells[trigger_spell_id].ResistDiff);
	}

	else {
		trigger_spell_id = CalcFocusEffect(focusTriggerOnCast, focus_spell, spell_id);

		if (IsValidSpell(trigger_spell_id) && GetTarget()) {
			SpellFinished(trigger_spell_id, GetTarget(), EQEmu::CastingSlot::Item, 0, -1,
				      spells[trigger_spell_id].ResistDiff);
			CheckNumHitsRemaining(NumHit::MatchingSpells, -1, focus_spell);
		}
	}
}

bool Mob::TrySpellTrigger(Mob *target, uint32 spell_id, int effect)
{
	if(!target || !IsValidSpell(spell_id))
		return false;

	int spell_trig = 0;
	// Count all the percentage chances to trigger for all effects
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if (spells[spell_id].effectid[i] == SE_SpellTrigger)
			spell_trig += spells[spell_id].base[i];
	}
	// If all the % add to 100, then only one of the effects can fire but one has to fire.
	if (spell_trig == 100)
	{
		int trig_chance = 100;
		for(int i = 0; i < EFFECT_COUNT; i++)
		{
			if (spells[spell_id].effectid[i] == SE_SpellTrigger)
			{
				if(zone->random.Int(0, trig_chance) <= spells[spell_id].base[i])
				{
					// If we trigger an effect then its over.
					if (IsValidSpell(spells[spell_id].base2[i])){
						SpellFinished(spells[spell_id].base2[i], target, EQEmu::CastingSlot::Item, 0, -1, spells[spells[spell_id].base2[i]].ResistDiff);
						return true;
					}
				}
				else
				{
					// Increase the chance to fire for the next effect, if all effects fail, the final effect will fire.
					trig_chance -= spells[spell_id].base[i];
				}
			}

		}
	}
	// if the chances don't add to 100, then each effect gets a chance to fire, chance for no trigger as well.
	else
	{
		if(zone->random.Int(0, 100) <= spells[spell_id].base[effect])
		{
			if (IsValidSpell(spells[spell_id].base2[effect])){
				SpellFinished(spells[spell_id].base2[effect], target, EQEmu::CastingSlot::Item, 0, -1, spells[spells[spell_id].base2[effect]].ResistDiff);
				return true; //Only trigger once of these per spell effect.
			}
		}
	}
	return false;
}

void Mob::TryTriggerOnValueAmount(bool IsHP, bool IsMana, bool IsEndur, bool IsPet)
{
	/*
	At present time there is no obvious difference between ReqTarget and ReqCaster
	ReqTarget is typically used in spells cast on a target where the trigger occurs on that target.
	ReqCaster is typically self only spells where the triggers on self.
	Regardless both trigger on the owner of the buff.
	*/

	/*
	Base2 Range: 1004	 = Below < 80% HP
	Base2 Range: 500-520 = Below (base2 - 500)*5 HP
	Base2 Range: 521	 = Below (?) Mana UKNOWN - Will assume its 20% unless proven otherwise
	Base2 Range: 522	 = Below (40%) Endurance
	Base2 Range: 523	 = Below (40%) Mana
	Base2 Range: 220-?	 = Number of pets on hatelist to trigger (base2 - 220) (Set at 30 pets max for now)
	38311 = < 10% mana;
	*/

	if (!spellbonuses.TriggerOnValueAmount)
		return;

	if (spellbonuses.TriggerOnValueAmount){

		int buff_count = GetMaxTotalSlots();

		for(int e = 0; e < buff_count; e++){

			uint32 spell_id = buffs[e].spellid;

			if (IsValidSpell(spell_id)){

				for(int i = 0; i < EFFECT_COUNT; i++){

					if ((spells[spell_id].effectid[i] == SE_TriggerOnReqTarget) || (spells[spell_id].effectid[i] == SE_TriggerOnReqCaster)) {

						int base2 = spells[spell_id].base2[i];
						bool use_spell = false;

						if (IsHP){
							if ((base2 >= 500 && base2 <= 520) && GetHPRatio() < (base2 - 500)*5)
								use_spell = true;

							else if (base2 == 1004 && GetHPRatio() < 80)
								use_spell = true;
						}

						else if (IsMana){
							if ( (base2 = 521 && GetManaRatio() < 20) || (base2 = 523 && GetManaRatio() < 40))
								use_spell = true;

							else if (base2 == 38311 && GetManaRatio() < 10)
								use_spell = true;
						}

						else if (IsEndur){
							if (base2 == 522 && GetEndurancePercent() < 40){
								use_spell = true;
							}
						}

						else if (IsPet){
							int count = hate_list.GetSummonedPetCountOnHateList(this);
							if ((base2 >= 220 && base2 <= 250) && count >= (base2 - 220)){
								use_spell = true;
							}
						}

						if (use_spell){
							SpellFinished(spells[spell_id].base[i], this, EQEmu::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff);

							if(!TryFadeEffect(e))
								BuffFadeBySlot(e);
						}
					}
				}
			}
		}
	}
}


//Twincast Focus effects should stack across different types (Spell, AA - when implemented ect)
void Mob::TryTwincast(Mob *caster, Mob *target, uint32 spell_id)
{
	if(!IsValidSpell(spell_id))
		return;

	if(IsClient())
	{
		int32 focus = CastToClient()->GetFocusEffect(focusTwincast, spell_id);

		if (focus > 0)
		{
			if(zone->random.Roll(focus))
			{
				Message(MT_Spells,"You twincast %s!",spells[spell_id].name);
				SpellFinished(spell_id, target, EQEmu::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff);
			}
		}
	}

	//Retains function for non clients
	else if (spellbonuses.FocusEffects[focusTwincast] || itembonuses.FocusEffects[focusTwincast])
	{
		int buff_count = GetMaxTotalSlots();
		for(int i = 0; i < buff_count; i++)
		{
			if(IsEffectInSpell(buffs[i].spellid, SE_FcTwincast))
			{
				int32 focus = CalcFocusEffect(focusTwincast, buffs[i].spellid, spell_id);
				if(focus > 0)
				{
					if(zone->random.Roll(focus))
					{
						SpellFinished(spell_id, target, EQEmu::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff);
					}
				}
			}
		}
	}
}

int32 Mob::GetVulnerability(Mob* caster, uint32 spell_id, uint32 ticsremaining)
{
	if (!IsValidSpell(spell_id))
		return 0;

	if (!caster)
		return 0;

	int32 value = 0;

	//Apply innate vulnerabilities
	if (Vulnerability_Mod[GetSpellResistType(spell_id)] != 0)
		value = Vulnerability_Mod[GetSpellResistType(spell_id)];


	else if (Vulnerability_Mod[HIGHEST_RESIST+1] != 0)
		value = Vulnerability_Mod[HIGHEST_RESIST+1];

	//Apply spell derived vulnerabilities
	if (spellbonuses.FocusEffects[focusSpellVulnerability]){

		int32 tmp_focus = 0;
		int tmp_buffslot = -1;

		int buff_count = GetMaxTotalSlots();
		for(int i = 0; i < buff_count; i++) {

			if((IsValidSpell(buffs[i].spellid) && IsEffectInSpell(buffs[i].spellid, SE_FcSpellVulnerability))){

				int32 focus = caster->CalcFocusEffect(focusSpellVulnerability, buffs[i].spellid, spell_id, true);

				if (!focus)
					continue;

				if (tmp_focus && focus > tmp_focus){
					tmp_focus = focus;
					tmp_buffslot = i;
				}

				else if (!tmp_focus){
					tmp_focus = focus;
					tmp_buffslot = i;
				}

			}
		}

		tmp_focus = caster->CalcFocusEffect(focusSpellVulnerability, buffs[tmp_buffslot].spellid, spell_id);

		if (tmp_focus < -99)
			tmp_focus = -99;

		value += tmp_focus;

		if (tmp_buffslot >= 0)
			CheckNumHitsRemaining(NumHit::MatchingSpells, tmp_buffslot);
	}
	return value;
}

int16 Mob::GetSkillDmgTaken(const EQEmu::skills::SkillType skill_used, ExtraAttackOptions *opts)
{
	int skilldmg_mod = 0;

	// All skill dmg mod + Skill specific
	skilldmg_mod += itembonuses.SkillDmgTaken[EQEmu::skills::HIGHEST_SKILL + 1] + spellbonuses.SkillDmgTaken[EQEmu::skills::HIGHEST_SKILL + 1] +
					itembonuses.SkillDmgTaken[skill_used] + spellbonuses.SkillDmgTaken[skill_used];

	skilldmg_mod += SkillDmgTaken_Mod[skill_used] + SkillDmgTaken_Mod[EQEmu::skills::HIGHEST_SKILL + 1];

	if (opts)
		skilldmg_mod += opts->skilldmgtaken_bonus_flat;

	if(skilldmg_mod < -100)
		skilldmg_mod = -100;

	return skilldmg_mod;
}

int16 Mob::GetHealRate(uint16 spell_id, Mob* caster) {

	int16 heal_rate = 0;

	heal_rate += itembonuses.HealRate + spellbonuses.HealRate + aabonuses.HealRate;
	heal_rate += GetFocusIncoming(focusFcHealPctIncoming, SE_FcHealPctIncoming, caster, spell_id);

	if(heal_rate < -99)
		heal_rate = -99;

	return heal_rate;
}

bool Mob::TryFadeEffect(int slot)
{
	if (!buffs[slot].spellid)
		return false;

	if(IsValidSpell(buffs[slot].spellid))
	{
		for(int i = 0; i < EFFECT_COUNT; i++)
		{

			if (!spells[buffs[slot].spellid].effectid[i])
				continue;

			if (spells[buffs[slot].spellid].effectid[i] == SE_CastOnFadeEffectAlways ||
				spells[buffs[slot].spellid].effectid[i] == SE_CastOnRuneFadeEffect)
			{
				uint16 spell_id = spells[buffs[slot].spellid].base[i];
				BuffFadeBySlot(slot);

				if(spell_id)
				{

					if(spell_id == SPELL_UNKNOWN)
						return false;

					if(IsValidSpell(spell_id))
					{
						if (IsBeneficialSpell(spell_id)) {
							SpellFinished(spell_id, this, EQEmu::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff);
						}
						else if(!(IsClient() && CastToClient()->dead)) {
							SpellFinished(spell_id, this, EQEmu::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff);
						}
						return true;
					}
				}
			}
		}
	}
	return false;
}

void Mob::TrySympatheticProc(Mob *target, uint32 spell_id)
{
	if(target == nullptr || !IsValidSpell(spell_id) || !IsClient())
		return;

	uint16 focus_spell = CastToClient()->GetSympatheticFocusEffect(focusSympatheticProc,spell_id);

	if(!IsValidSpell(focus_spell))
		return;

	uint16 focus_trigger = GetSympatheticSpellProcID(focus_spell);

	if(!IsValidSpell(focus_trigger))
		return;

	// For beneficial spells, if the triggered spell is also beneficial then proc it on the target
	// if the triggered spell is detrimental, then it will trigger on the caster(ie cursed items)
	if(IsBeneficialSpell(spell_id))
	{
		if(IsBeneficialSpell(focus_trigger))
			SpellFinished(focus_trigger, target);

		else
			SpellFinished(focus_trigger, this, EQEmu::CastingSlot::Item, 0, -1, spells[focus_trigger].ResistDiff);
	}
	// For detrimental spells, if the triggered spell is beneficial, then it will land on the caster
	// if the triggered spell is also detrimental, then it will land on the target
	else
	{
		if(IsBeneficialSpell(focus_trigger))
			SpellFinished(focus_trigger, this);

		else
			SpellFinished(focus_trigger, target, EQEmu::CastingSlot::Item, 0, -1, spells[focus_trigger].ResistDiff);
	}

	CheckNumHitsRemaining(NumHit::MatchingSpells, -1, focus_spell);
}

int32 Mob::GetItemStat(uint32 itemid, const char *identifier)
{
	const EQEmu::ItemInstance* inst = database.CreateItem(itemid);
	if (!inst)
		return 0;

	const EQEmu::ItemData* item = inst->GetItem();
	if (!item)
		return 0;

	if (!identifier)
		return 0;

	int32 stat = 0;

	std::string id = identifier;
	for(uint32 i = 0; i < id.length(); ++i)
	{
		id[i] = tolower(id[i]);
	}

	if (id == "itemclass")
		stat = int32(item->ItemClass);
	if (id == "id")
		stat = int32(item->ID);
	if (id == "idfile")
		stat = atoi(&item->IDFile[2]);
	if (id == "weight")
		stat = int32(item->Weight);
	if (id == "norent")
		stat = int32(item->NoRent);
	if (id == "nodrop")
		stat = int32(item->NoDrop);
	if (id == "size")
		stat = int32(item->Size);
	if (id == "slots")
		stat = int32(item->Slots);
	if (id == "price")
		stat = int32(item->Price);
	if (id == "icon")
		stat = int32(item->Icon);
	if (id == "loregroup")
		stat = int32(item->LoreGroup);
	if (id == "loreflag")
		stat = int32(item->LoreFlag);
	if (id == "pendingloreflag")
		stat = int32(item->PendingLoreFlag);
	if (id == "artifactflag")
		stat = int32(item->ArtifactFlag);
	if (id == "summonedflag")
		stat = int32(item->SummonedFlag);
	if (id == "fvnodrop")
		stat = int32(item->FVNoDrop);
	if (id == "favor")
		stat = int32(item->Favor);
	if (id == "guildfavor")
		stat = int32(item->GuildFavor);
	if (id == "pointtype")
		stat = int32(item->PointType);
	if (id == "bagtype")
		stat = int32(item->BagType);
	if (id == "bagslots")
		stat = int32(item->BagSlots);
	if (id == "bagsize")
		stat = int32(item->BagSize);
	if (id == "bagwr")
		stat = int32(item->BagWR);
	if (id == "benefitflag")
		stat = int32(item->BenefitFlag);
	if (id == "tradeskills")
		stat = int32(item->Tradeskills);
	if (id == "cr")
		stat = int32(item->CR);
	if (id == "dr")
		stat = int32(item->DR);
	if (id == "pr")
		stat = int32(item->PR);
	if (id == "mr")
		stat = int32(item->MR);
	if (id == "fr")
		stat = int32(item->FR);
	if (id == "astr")
		stat = int32(item->AStr);
	if (id == "asta")
		stat = int32(item->ASta);
	if (id == "aagi")
		stat = int32(item->AAgi);
	if (id == "adex")
		stat = int32(item->ADex);
	if (id == "acha")
		stat = int32(item->ACha);
	if (id == "aint")
		stat = int32(item->AInt);
	if (id == "awis")
		stat = int32(item->AWis);
	if (id == "hp")
		stat = int32(item->HP);
	if (id == "mana")
		stat = int32(item->Mana);
	if (id == "ac")
		stat = int32(item->AC);
	if (id == "deity")
		stat = int32(item->Deity);
	if (id == "skillmodvalue")
		stat = int32(item->SkillModValue);
	if (id == "skillmodtype")
		stat = int32(item->SkillModType);
	if (id == "banedmgrace")
		stat = int32(item->BaneDmgRace);
	if (id == "banedmgamt")
		stat = int32(item->BaneDmgAmt);
	if (id == "banedmgbody")
		stat = int32(item->BaneDmgBody);
	if (id == "magic")
		stat = int32(item->Magic);
	if (id == "casttime_")
		stat = int32(item->CastTime_);
	if (id == "reqlevel")
		stat = int32(item->ReqLevel);
	if (id == "bardtype")
		stat = int32(item->BardType);
	if (id == "bardvalue")
		stat = int32(item->BardValue);
	if (id == "light")
		stat = int32(item->Light);
	if (id == "delay")
		stat = int32(item->Delay);
	if (id == "reclevel")
		stat = int32(item->RecLevel);
	if (id == "recskill")
		stat = int32(item->RecSkill);
	if (id == "elemdmgtype")
		stat = int32(item->ElemDmgType);
	if (id == "elemdmgamt")
		stat = int32(item->ElemDmgAmt);
	if (id == "range")
		stat = int32(item->Range);
	if (id == "damage")
		stat = int32(item->Damage);
	if (id == "color")
		stat = int32(item->Color);
	if (id == "classes")
		stat = int32(item->Classes);
	if (id == "races")
		stat = int32(item->Races);
	if (id == "maxcharges")
		stat = int32(item->MaxCharges);
	if (id == "itemtype")
		stat = int32(item->ItemType);
	if (id == "material")
		stat = int32(item->Material);
	if (id == "casttime")
		stat = int32(item->CastTime);
	if (id == "elitematerial")
		stat = int32(item->EliteMaterial);
	if (id == "herosforgemodel")
		stat = int32(item->HerosForgeModel);
	if (id == "procrate")
		stat = int32(item->ProcRate);
	if (id == "combateffects")
		stat = int32(item->CombatEffects);
	if (id == "shielding")
		stat = int32(item->Shielding);
	if (id == "stunresist")
		stat = int32(item->StunResist);
	if (id == "strikethrough")
		stat = int32(item->StrikeThrough);
	if (id == "extradmgskill")
		stat = int32(item->ExtraDmgSkill);
	if (id == "extradmgamt")
		stat = int32(item->ExtraDmgAmt);
	if (id == "spellshield")
		stat = int32(item->SpellShield);
	if (id == "avoidance")
		stat = int32(item->Avoidance);
	if (id == "accuracy")
		stat = int32(item->Accuracy);
	if (id == "charmfileid")
		stat = int32(item->CharmFileID);
	if (id == "factionmod1")
		stat = int32(item->FactionMod1);
	if (id == "factionmod2")
		stat = int32(item->FactionMod2);
	if (id == "factionmod3")
		stat = int32(item->FactionMod3);
	if (id == "factionmod4")
		stat = int32(item->FactionMod4);
	if (id == "factionamt1")
		stat = int32(item->FactionAmt1);
	if (id == "factionamt2")
		stat = int32(item->FactionAmt2);
	if (id == "factionamt3")
		stat = int32(item->FactionAmt3);
	if (id == "factionamt4")
		stat = int32(item->FactionAmt4);
	if (id == "augtype")
		stat = int32(item->AugType);
	if (id == "ldontheme")
		stat = int32(item->LDoNTheme);
	if (id == "ldonprice")
		stat = int32(item->LDoNPrice);
	if (id == "ldonsold")
		stat = int32(item->LDoNSold);
	if (id == "banedmgraceamt")
		stat = int32(item->BaneDmgRaceAmt);
	if (id == "augrestrict")
		stat = int32(item->AugRestrict);
	if (id == "endur")
		stat = int32(item->Endur);
	if (id == "dotshielding")
		stat = int32(item->DotShielding);
	if (id == "attack")
		stat = int32(item->Attack);
	if (id == "regen")
		stat = int32(item->Regen);
	if (id == "manaregen")
		stat = int32(item->ManaRegen);
	if (id == "enduranceregen")
		stat = int32(item->EnduranceRegen);
	if (id == "haste")
		stat = int32(item->Haste);
	if (id == "damageshield")
		stat = int32(item->DamageShield);
	if (id == "recastdelay")
		stat = int32(item->RecastDelay);
	if (id == "recasttype")
		stat = int32(item->RecastType);
	if (id == "augdistiller")
		stat = int32(item->AugDistiller);
	if (id == "attuneable")
		stat = int32(item->Attuneable);
	if (id == "nopet")
		stat = int32(item->NoPet);
	if (id == "potionbelt")
		stat = int32(item->PotionBelt);
	if (id == "stackable")
		stat = int32(item->Stackable);
	if (id == "notransfer")
		stat = int32(item->NoTransfer);
	if (id == "questitemflag")
		stat = int32(item->QuestItemFlag);
	if (id == "stacksize")
		stat = int32(item->StackSize);
	if (id == "potionbeltslots")
		stat = int32(item->PotionBeltSlots);
	if (id == "book")
		stat = int32(item->Book);
	if (id == "booktype")
		stat = int32(item->BookType);
	if (id == "svcorruption")
		stat = int32(item->SVCorruption);
	if (id == "purity")
		stat = int32(item->Purity);
	if (id == "backstabdmg")
		stat = int32(item->BackstabDmg);
	if (id == "dsmitigation")
		stat = int32(item->DSMitigation);
	if (id == "heroicstr")
		stat = int32(item->HeroicStr);
	if (id == "heroicint")
		stat = int32(item->HeroicInt);
	if (id == "heroicwis")
		stat = int32(item->HeroicWis);
	if (id == "heroicagi")
		stat = int32(item->HeroicAgi);
	if (id == "heroicdex")
		stat = int32(item->HeroicDex);
	if (id == "heroicsta")
		stat = int32(item->HeroicSta);
	if (id == "heroiccha")
		stat = int32(item->HeroicCha);
	if (id == "heroicmr")
		stat = int32(item->HeroicMR);
	if (id == "heroicfr")
		stat = int32(item->HeroicFR);
	if (id == "heroiccr")
		stat = int32(item->HeroicCR);
	if (id == "heroicdr")
		stat = int32(item->HeroicDR);
	if (id == "heroicpr")
		stat = int32(item->HeroicPR);
	if (id == "heroicsvcorrup")
		stat = int32(item->HeroicSVCorrup);
	if (id == "healamt")
		stat = int32(item->HealAmt);
	if (id == "spelldmg")
		stat = int32(item->SpellDmg);
	if (id == "ldonsellbackrate")
		stat = int32(item->LDoNSellBackRate);
	if (id == "scriptfileid")
		stat = int32(item->ScriptFileID);
	if (id == "expendablearrow")
		stat = int32(item->ExpendableArrow);
	if (id == "clairvoyance")
		stat = int32(item->Clairvoyance);
	// Begin Effects
	if (id == "clickeffect")
		stat = int32(item->Click.Effect);
	if (id == "clicktype")
		stat = int32(item->Click.Type);
	if (id == "clicklevel")
		stat = int32(item->Click.Level);
	if (id == "clicklevel2")
		stat = int32(item->Click.Level2);
	if (id == "proceffect")
		stat = int32(item->Proc.Effect);
	if (id == "proctype")
		stat = int32(item->Proc.Type);
	if (id == "proclevel")
		stat = int32(item->Proc.Level);
	if (id == "proclevel2")
		stat = int32(item->Proc.Level2);
	if (id == "worneffect")
		stat = int32(item->Worn.Effect);
	if (id == "worntype")
		stat = int32(item->Worn.Type);
	if (id == "wornlevel")
		stat = int32(item->Worn.Level);
	if (id == "wornlevel2")
		stat = int32(item->Worn.Level2);
	if (id == "focuseffect")
		stat = int32(item->Focus.Effect);
	if (id == "focustype")
		stat = int32(item->Focus.Type);
	if (id == "focuslevel")
		stat = int32(item->Focus.Level);
	if (id == "focuslevel2")
		stat = int32(item->Focus.Level2);
	if (id == "scrolleffect")
		stat = int32(item->Scroll.Effect);
	if (id == "scrolltype")
		stat = int32(item->Scroll.Type);
	if (id == "scrolllevel")
		stat = int32(item->Scroll.Level);
	if (id == "scrolllevel2")
		stat = int32(item->Scroll.Level2);

	safe_delete(inst);
	return stat;
}

std::string Mob::GetGlobal(const char *varname) {
	int qgCharid = 0;
	int qgNpcid = 0;

	if (this->IsNPC())
		qgNpcid = this->GetNPCTypeID();

	if (this->IsClient())
		qgCharid = this->CastToClient()->CharacterID();

	QGlobalCache *qglobals = nullptr;
	std::list<QGlobal> globalMap;

	if (this->IsClient())
		qglobals = this->CastToClient()->GetQGlobals();

	if (this->IsNPC())
		qglobals = this->CastToNPC()->GetQGlobals();

	if(qglobals)
		QGlobalCache::Combine(globalMap, qglobals->GetBucket(), qgNpcid, qgCharid, zone->GetZoneID());

	auto iter = globalMap.begin();
	while(iter != globalMap.end()) {
		if ((*iter).name.compare(varname) == 0)
			return (*iter).value;

		++iter;
	}

	return "Undefined";
}

void Mob::SetGlobal(const char *varname, const char *newvalue, int options, const char *duration, Mob *other) {

	int qgZoneid = zone->GetZoneID();
	int qgCharid = 0;
	int qgNpcid = 0;

	if (this->IsNPC())
	{
		qgNpcid = this->GetNPCTypeID();
	}
	else if (other && other->IsNPC())
	{
		qgNpcid = other->GetNPCTypeID();
	}

	if (this->IsClient())
	{
		qgCharid = this->CastToClient()->CharacterID();
	}
	else if (other && other->IsClient())
	{
		qgCharid = other->CastToClient()->CharacterID();
	}
	else
	{
		qgCharid = -qgNpcid;		// make char id negative npc id as a fudge
	}

	if (options < 0 || options > 7)
	{
		//cerr << "Invalid options for global var " << varname << " using defaults" << endl;
		options = 0;	// default = 0 (only this npcid,player and zone)
	}
	else
	{
		if (options & 1)
			qgNpcid=0;
		if (options & 2)
			qgCharid=0;
		if (options & 4)
			qgZoneid=0;
	}

	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, newvalue, QGVarDuration(duration));
}

void Mob::TarGlobal(const char *varname, const char *value, const char *duration, int qgNpcid, int qgCharid, int qgZoneid)
{
	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, value, QGVarDuration(duration));
}

void Mob::DelGlobal(const char *varname) {

	int qgZoneid=zone->GetZoneID();
	int qgCharid=0;
	int qgNpcid=0;

	if (this->IsNPC())
		qgNpcid = this->GetNPCTypeID();

	if (this->IsClient())
		qgCharid = this->CastToClient()->CharacterID();
	else
		qgCharid = -qgNpcid;		// make char id negative npc id as a fudge

    std::string query = StringFormat("DELETE FROM quest_globals "
                                    "WHERE name='%s' && (npcid=0 || npcid=%i) "
                                    "&& (charid=0 || charid=%i) "
                                    "&& (zoneid=%i || zoneid=0)",
                                    varname, qgNpcid, qgCharid, qgZoneid);

	database.QueryDatabase(query);

	if(zone)
	{
		auto pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
		ServerQGlobalDelete_Struct *qgu = (ServerQGlobalDelete_Struct*)pack->pBuffer;

		qgu->npc_id = qgNpcid;
		qgu->char_id = qgCharid;
		qgu->zone_id = qgZoneid;
		strcpy(qgu->name, varname);

		entity_list.DeleteQGlobal(std::string((char*)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);
		zone->DeleteQGlobal(std::string((char*)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

// Inserts global variable into quest_globals table
void Mob::InsertQuestGlobal(int charid, int npcid, int zoneid, const char *varname, const char *varvalue, int duration) {

	// Make duration string either "unix_timestamp(now()) + xxx" or "NULL"
	std::stringstream duration_ss;

	if (duration == INT_MAX)
		duration_ss << "NULL";
	else
		duration_ss << "unix_timestamp(now()) + " << duration;

	//NOTE: this should be escaping the contents of arglist
	//npcwise a malicious script can arbitrarily alter the DB
	uint32 last_id = 0;
	std::string query = StringFormat("REPLACE INTO quest_globals "
                                    "(charid, npcid, zoneid, name, value, expdate)"
                                    "VALUES (%i, %i, %i, '%s', '%s', %s)",
                                    charid, npcid, zoneid, varname, varvalue, duration_ss.str().c_str());
	database.QueryDatabase(query);

	if(zone)
	{
		//first delete our global
		auto pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
		ServerQGlobalDelete_Struct *qgd = (ServerQGlobalDelete_Struct*)pack->pBuffer;
		qgd->npc_id = npcid;
		qgd->char_id = charid;
		qgd->zone_id = zoneid;
		qgd->from_zone_id = zone->GetZoneID();
		qgd->from_instance_id = zone->GetInstanceID();
		strcpy(qgd->name, varname);

		entity_list.DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);
		zone->DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		//then create a new one with the new id
		pack = new ServerPacket(ServerOP_QGlobalUpdate, sizeof(ServerQGlobalUpdate_Struct));
		ServerQGlobalUpdate_Struct *qgu = (ServerQGlobalUpdate_Struct*)pack->pBuffer;
		qgu->npc_id = npcid;
		qgu->char_id = charid;
		qgu->zone_id = zoneid;

		if(duration == INT_MAX)
			qgu->expdate = 0xFFFFFFFF;
		else
			qgu->expdate = Timer::GetTimeSeconds() + duration;

		strcpy((char*)qgu->name, varname);
		strcpy((char*)qgu->value, varvalue);
		qgu->id = last_id;
		qgu->from_zone_id = zone->GetZoneID();
		qgu->from_instance_id = zone->GetInstanceID();

		QGlobal temp;
		temp.npc_id = npcid;
		temp.char_id = charid;
		temp.zone_id = zoneid;
		temp.expdate = qgu->expdate;
		temp.name.assign(qgu->name);
		temp.value.assign(qgu->value);
		entity_list.UpdateQGlobal(qgu->id, temp);
		zone->UpdateQGlobal(qgu->id, temp);

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}

}

// Converts duration string to duration value (in seconds)
// Return of INT_MAX indicates infinite duration
int Mob::QGVarDuration(const char *fmt)
{
	int duration = 0;

	// format:	Y#### or D## or H## or M## or S## or T###### or C#######

	int len = static_cast<int>(strlen(fmt));

	// Default to no duration
	if (len < 1)
		return 0;

	// Set val to value after type character
	// e.g., for "M3924", set to 3924
	int val = atoi(&fmt[0] + 1);

	switch (fmt[0])
	{
		// Forever
		case 'F':
		case 'f':
			duration = INT_MAX;
			break;
		// Years
		case 'Y':
		case 'y':
			duration = val * 31556926;
			break;
		case 'D':
		case 'd':
			duration = val * 86400;
			break;
		// Hours
		case 'H':
		case 'h':
			duration = val * 3600;
			break;
		// Minutes
		case 'M':
		case 'm':
			duration = val * 60;
			break;
		// Seconds
		case 'S':
		case 's':
			duration = val;
			break;
		// Invalid
		default:
			duration = 0;
			break;
	}

	return duration;
}

void Mob::DoKnockback(Mob *caster, uint32 pushback, uint32 pushup)
{
	if(IsClient())
	{
		CastToClient()->SetKnockBackExemption(true);

		auto outapp_push = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
		PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp_push->pBuffer;

		double look_heading = caster->CalculateHeadingToTarget(GetX(), GetY());
		look_heading /= 256;
		look_heading *= 360;
		if(look_heading > 360)
			look_heading -= 360;

		//x and y are crossed mkay
		double new_x = pushback * sin(double(look_heading * 3.141592 / 180.0));
		double new_y = pushback * cos(double(look_heading * 3.141592 / 180.0));

		spu->spawn_id	= GetID();
		spu->x_pos		= FloatToEQ19(GetX());
		spu->y_pos		= FloatToEQ19(GetY());
		spu->z_pos		= FloatToEQ19(GetZ());
		spu->delta_x	= NewFloatToEQ13(static_cast<float>(new_x));
		spu->delta_y	= NewFloatToEQ13(static_cast<float>(new_y));
		spu->delta_z	= NewFloatToEQ13(static_cast<float>(pushup));
		spu->heading	= FloatToEQ19(GetHeading());
		spu->padding0002	=0;
		spu->padding0006	=7;
		spu->padding0014	=0x7f;
		spu->padding0018	=0x5df27;
		spu->animation = 0;
		spu->delta_heading = NewFloatToEQ13(0);
		outapp_push->priority = 6;
		entity_list.QueueClients(this, outapp_push, true);
		CastToClient()->FastQueuePacket(&outapp_push);
	}
}

void Mob::TrySpellOnKill(uint8 level, uint16 spell_id)
{
	if (spell_id != SPELL_UNKNOWN)
	{
		if(IsEffectInSpell(spell_id, SE_ProcOnSpellKillShot)) {
			for (int i = 0; i < EFFECT_COUNT; i++) {
				if (spells[spell_id].effectid[i] == SE_ProcOnSpellKillShot)
				{
					if (IsValidSpell(spells[spell_id].base2[i]) && spells[spell_id].max[i] <= level)
					{
						if(zone->random.Roll(spells[spell_id].base[i]))
							SpellFinished(spells[spell_id].base2[i], this, EQEmu::CastingSlot::Item, 0, -1, spells[spells[spell_id].base2[i]].ResistDiff);
					}
				}
			}
		}
	}

	if (!aabonuses.SpellOnKill[0] && !itembonuses.SpellOnKill[0] && !spellbonuses.SpellOnKill[0])
		return;

	// Allow to check AA, items and buffs in all cases. Base2 = Spell to fire | Base1 = % chance | Base3 = min level
	for(int i = 0; i < MAX_SPELL_TRIGGER*3; i+=3) {

		if(aabonuses.SpellOnKill[i] && IsValidSpell(aabonuses.SpellOnKill[i]) && (level >= aabonuses.SpellOnKill[i + 2])) {
			if(zone->random.Roll(static_cast<int>(aabonuses.SpellOnKill[i + 1])))
				SpellFinished(aabonuses.SpellOnKill[i], this, EQEmu::CastingSlot::Item, 0, -1, spells[aabonuses.SpellOnKill[i]].ResistDiff);
		}

		if(itembonuses.SpellOnKill[i] && IsValidSpell(itembonuses.SpellOnKill[i]) && (level >= itembonuses.SpellOnKill[i + 2])){
			if(zone->random.Roll(static_cast<int>(itembonuses.SpellOnKill[i + 1])))
				SpellFinished(itembonuses.SpellOnKill[i], this, EQEmu::CastingSlot::Item, 0, -1, spells[aabonuses.SpellOnKill[i]].ResistDiff);
		}

		if(spellbonuses.SpellOnKill[i] && IsValidSpell(spellbonuses.SpellOnKill[i]) && (level >= spellbonuses.SpellOnKill[i + 2])) {
			if(zone->random.Roll(static_cast<int>(spellbonuses.SpellOnKill[i + 1])))
				SpellFinished(spellbonuses.SpellOnKill[i], this, EQEmu::CastingSlot::Item, 0, -1, spells[aabonuses.SpellOnKill[i]].ResistDiff);
		}

	}
}

bool Mob::TrySpellOnDeath()
{
	if (IsNPC() && !spellbonuses.SpellOnDeath[0] && !itembonuses.SpellOnDeath[0])
		return false;

	if (IsClient() && !aabonuses.SpellOnDeath[0] && !spellbonuses.SpellOnDeath[0] && !itembonuses.SpellOnDeath[0])
		return false;

	for(int i = 0; i < MAX_SPELL_TRIGGER*2; i+=2) {
		if(IsClient() && aabonuses.SpellOnDeath[i] && IsValidSpell(aabonuses.SpellOnDeath[i])) {
			if(zone->random.Roll(static_cast<int>(aabonuses.SpellOnDeath[i + 1]))) {
				SpellFinished(aabonuses.SpellOnDeath[i], this, EQEmu::CastingSlot::Item, 0, -1, spells[aabonuses.SpellOnDeath[i]].ResistDiff);
			}
		}

		if(itembonuses.SpellOnDeath[i] && IsValidSpell(itembonuses.SpellOnDeath[i])) {
			if(zone->random.Roll(static_cast<int>(itembonuses.SpellOnDeath[i + 1]))) {
				SpellFinished(itembonuses.SpellOnDeath[i], this, EQEmu::CastingSlot::Item, 0, -1, spells[itembonuses.SpellOnDeath[i]].ResistDiff);
			}
		}

		if(spellbonuses.SpellOnDeath[i] && IsValidSpell(spellbonuses.SpellOnDeath[i])) {
			if(zone->random.Roll(static_cast<int>(spellbonuses.SpellOnDeath[i + 1]))) {
				SpellFinished(spellbonuses.SpellOnDeath[i], this, EQEmu::CastingSlot::Item, 0, -1, spells[spellbonuses.SpellOnDeath[i]].ResistDiff);
				}
			}
		}

	BuffFadeAll();
	return false;
	//You should not be able to use this effect and survive (ALWAYS return false),
	//attempting to place a heal in these effects will still result
	//in death because the heal will not register before the script kills you.
}

int16 Mob::GetCritDmgMod(uint16 skill)
{
	int critDmg_mod = 0;

	// All skill dmg mod + Skill specific
	critDmg_mod += itembonuses.CritDmgMod[EQEmu::skills::HIGHEST_SKILL + 1] + spellbonuses.CritDmgMod[EQEmu::skills::HIGHEST_SKILL + 1] + aabonuses.CritDmgMod[EQEmu::skills::HIGHEST_SKILL + 1] +
					itembonuses.CritDmgMod[skill] + spellbonuses.CritDmgMod[skill] + aabonuses.CritDmgMod[skill];

	return critDmg_mod;
}

void Mob::SetGrouped(bool v)
{
	if(v)
	{
		israidgrouped = false;
	}
	isgrouped = v;

	if(IsClient())
	{
			parse->EventPlayer(EVENT_GROUP_CHANGE, CastToClient(), "", 0);

		if(!v)
			CastToClient()->RemoveGroupXTargets();
	}
}

void Mob::SetRaidGrouped(bool v)
{
	if(v)
	{
		isgrouped = false;
	}
	israidgrouped = v;

	if(IsClient())
	{
		parse->EventPlayer(EVENT_GROUP_CHANGE, CastToClient(), "", 0);
	}
}

int Mob::GetCriticalChanceBonus(uint16 skill)
{
	int critical_chance = 0;

	// All skills + Skill specific
	critical_chance += itembonuses.CriticalHitChance[EQEmu::skills::HIGHEST_SKILL + 1] + spellbonuses.CriticalHitChance[EQEmu::skills::HIGHEST_SKILL + 1] + aabonuses.CriticalHitChance[EQEmu::skills::HIGHEST_SKILL + 1] +
						itembonuses.CriticalHitChance[skill] + spellbonuses.CriticalHitChance[skill] + aabonuses.CriticalHitChance[skill];

	if(critical_chance < -100)
		critical_chance = -100;

	return critical_chance;
}

int16 Mob::GetMeleeDamageMod_SE(uint16 skill)
{
	int dmg_mod = 0;

	// All skill dmg mod + Skill specific
	dmg_mod += itembonuses.DamageModifier[EQEmu::skills::HIGHEST_SKILL + 1] + spellbonuses.DamageModifier[EQEmu::skills::HIGHEST_SKILL + 1] + aabonuses.DamageModifier[EQEmu::skills::HIGHEST_SKILL + 1] +
				itembonuses.DamageModifier[skill] + spellbonuses.DamageModifier[skill] + aabonuses.DamageModifier[skill];

	dmg_mod += itembonuses.DamageModifier2[EQEmu::skills::HIGHEST_SKILL + 1] + spellbonuses.DamageModifier2[EQEmu::skills::HIGHEST_SKILL + 1] + aabonuses.DamageModifier2[EQEmu::skills::HIGHEST_SKILL + 1] +
				itembonuses.DamageModifier2[skill] + spellbonuses.DamageModifier2[skill] + aabonuses.DamageModifier2[skill];

	if(dmg_mod < -100)
		dmg_mod = -100;

	return dmg_mod;
}

int16 Mob::GetMeleeMinDamageMod_SE(uint16 skill)
{
	int dmg_mod = 0;

	dmg_mod = itembonuses.MinDamageModifier[skill] + spellbonuses.MinDamageModifier[skill] +
		itembonuses.MinDamageModifier[EQEmu::skills::HIGHEST_SKILL + 1] + spellbonuses.MinDamageModifier[EQEmu::skills::HIGHEST_SKILL + 1];

	if(dmg_mod < -100)
		dmg_mod = -100;

	return dmg_mod;
}

int16 Mob::GetCrippBlowChance()
{
	int16 crip_chance = 0;

	crip_chance += itembonuses.CrippBlowChance + spellbonuses.CrippBlowChance + aabonuses.CrippBlowChance;

	if(crip_chance < 0)
		crip_chance = 0;

	return crip_chance;
}

int16 Mob::GetSkillReuseTime(uint16 skill)
{
	int skill_reduction = this->itembonuses.SkillReuseTime[skill] + this->spellbonuses.SkillReuseTime[skill] + this->aabonuses.SkillReuseTime[skill];

	return skill_reduction;
}

int16 Mob::GetSkillDmgAmt(uint16 skill)
{
	int skill_dmg = 0;

	// All skill dmg(only spells do this) + Skill specific
	skill_dmg += spellbonuses.SkillDamageAmount[EQEmu::skills::HIGHEST_SKILL + 1] + itembonuses.SkillDamageAmount[EQEmu::skills::HIGHEST_SKILL + 1] + aabonuses.SkillDamageAmount[EQEmu::skills::HIGHEST_SKILL + 1]
				+ itembonuses.SkillDamageAmount[skill] + spellbonuses.SkillDamageAmount[skill] + aabonuses.SkillDamageAmount[skill];

	skill_dmg += spellbonuses.SkillDamageAmount2[EQEmu::skills::HIGHEST_SKILL + 1] + itembonuses.SkillDamageAmount2[EQEmu::skills::HIGHEST_SKILL + 1]
				+ itembonuses.SkillDamageAmount2[skill] + spellbonuses.SkillDamageAmount2[skill];

	return skill_dmg;
}

void Mob::MeleeLifeTap(int32 damage) {

	int32 lifetap_amt = 0;
	lifetap_amt = spellbonuses.MeleeLifetap + itembonuses.MeleeLifetap + aabonuses.MeleeLifetap
				+ spellbonuses.Vampirism + itembonuses.Vampirism + aabonuses.Vampirism;

	if(lifetap_amt && damage > 0){

		lifetap_amt = damage * lifetap_amt / 100;
		Log(Logs::Detail, Logs::Combat, "Melee lifetap healing for %d damage.", damage);

		if (lifetap_amt > 0)
			HealDamage(lifetap_amt); //Heal self for modified damage amount.
		else
			Damage(this, -lifetap_amt, 0, EQEmu::skills::SkillEvocation, false); //Dmg self for modified damage amount.
	}
}

bool Mob::TryReflectSpell(uint32 spell_id)
{
	if (!spells[spell_id].reflectable)
 		return false;

	int chance = itembonuses.reflect_chance + spellbonuses.reflect_chance + aabonuses.reflect_chance;

	if(chance && zone->random.Roll(chance))
		return true;

	return false;
}

void Mob::DoGravityEffect()
{
	Mob *caster = nullptr;
	int away = -1;
	float caster_x, caster_y, amount, value, cur_x, my_x, cur_y, my_y, x_vector, y_vector, hypot;

	// Set values so we can run through all gravity effects and then apply the culmative move at the end
	// instead of many small moves if the mob/client had more than 1 gravity effect on them
	cur_x = my_x = GetX();
	cur_y = my_y = GetY();

	int buff_count = GetMaxTotalSlots();
	for (int slot = 0; slot < buff_count; slot++)
	{
		if (buffs[slot].spellid != SPELL_UNKNOWN && IsEffectInSpell(buffs[slot].spellid, SE_GravityEffect))
		{
			for (int i = 0; i < EFFECT_COUNT; i++)
			{
				if(spells[buffs[slot].spellid].effectid[i] == SE_GravityEffect) {

					int casterId = buffs[slot].casterid;
					if(casterId)
						caster = entity_list.GetMob(casterId);

					if(!caster || casterId == this->GetID())
						continue;

					caster_x = caster->GetX();
					caster_y = caster->GetY();

					value = static_cast<float>(spells[buffs[slot].spellid].base[i]);
					if(value == 0)
						continue;

					if(value > 0)
						away = 1;

					amount = std::abs(value) /
						 (100.0f); // to bring the values in line, arbitarily picked

					x_vector = cur_x - caster_x;
					y_vector = cur_y - caster_y;
					hypot = sqrt(x_vector*x_vector + y_vector*y_vector);

					if(hypot <= 5) // dont want to be inside the mob, even though we can, it looks bad
						continue;

					x_vector /= hypot;
					y_vector /= hypot;

					cur_x = cur_x + (x_vector * amount * away);
					cur_y = cur_y + (y_vector * amount * away);
				}
			}
		}
	}

	if ((std::abs(my_x - cur_x) > 0.01) || (std::abs(my_y - cur_y) > 0.01)) {
		float new_ground = GetGroundZ(cur_x, cur_y);
		// If we cant get LoS on our new spot then keep checking up to 5 units up.
		if(!CheckLosFN(cur_x, cur_y, new_ground, GetSize())) {
			for(float z_adjust = 0.1f; z_adjust < 5; z_adjust += 0.1f) {
				if(CheckLosFN(cur_x, cur_y, new_ground+z_adjust, GetSize())) {
					new_ground += z_adjust;
					break;
				}
			}
			// If we still fail, then lets only use the x portion(ie sliding around a wall)
			if(!CheckLosFN(cur_x, my_y, new_ground, GetSize())) {
				// If that doesnt work, try the y
				if(!CheckLosFN(my_x, cur_y, new_ground, GetSize())) {
					// If everything fails, then lets do nothing
					return;
				}
				else {
					cur_x = my_x;
				}
			}
			else {
				cur_y = my_y;
			}
		}

		if(IsClient())
			this->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), cur_x, cur_y, new_ground, GetHeading()*2); // I know the heading thing is weird(chance of movepc to halve the heading value, too lazy to figure out why atm)
		else
			this->GMMove(cur_x, cur_y, new_ground, GetHeading());
	}
}

void Mob::SpreadVirus(uint16 spell_id, uint16 casterID)
{
	int num_targs = spells[spell_id].viral_targets;

	Mob* caster = entity_list.GetMob(casterID);
	Mob* target = nullptr;
	// Only spread in zones without perm buffs
	if(!zone->BuffTimersSuspended()) {
		for(int i = 0; i < num_targs; i++) {
			target = entity_list.GetTargetForVirus(this, spells[spell_id].viral_range);
			if(target) {
				// Only spreads to the uninfected
				if(!target->FindBuff(spell_id)) {
					if(caster)
						caster->SpellOnTarget(spell_id, target);

				}
			}
		}
	}
}

void Mob::AddNimbusEffect(int effectid)
{
	SetNimbusEffect(effectid);

	auto outapp = new EQApplicationPacket(OP_AddNimbusEffect, sizeof(RemoveNimbusEffect_Struct));
	auto ane = (RemoveNimbusEffect_Struct *)outapp->pBuffer;
	ane->spawnid = GetID();
	ane->nimbus_effect = effectid;
	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

void Mob::RemoveNimbusEffect(int effectid)
{
	if (effectid == nimbus_effect1)
		nimbus_effect1 = 0;

	else if (effectid == nimbus_effect2)
		nimbus_effect2 = 0;

	else if (effectid == nimbus_effect3)
		nimbus_effect3 = 0;

	auto outapp = new EQApplicationPacket(OP_RemoveNimbusEffect, sizeof(RemoveNimbusEffect_Struct));
	RemoveNimbusEffect_Struct* rne = (RemoveNimbusEffect_Struct*)outapp->pBuffer;
	rne->spawnid = GetID();
	rne->nimbus_effect = effectid;
	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

bool Mob::IsBoat() const {
	return (race == 72 || race == 73 || race == 114 || race == 404 || race == 550 || race == 551 || race == 552);
}

void Mob::SetBodyType(bodyType new_body, bool overwrite_orig) {
	bool needs_spawn_packet = false;
	if(bodytype == 11 || bodytype >= 65 || new_body == 11 || new_body >= 65) {
		needs_spawn_packet = true;
	}

	if(overwrite_orig) {
		orig_bodytype = new_body;
	}
	bodytype = new_body;

	if(needs_spawn_packet) {
		auto app = new EQApplicationPacket;
		CreateDespawnPacket(app, true);
		entity_list.QueueClients(this, app);
		CreateSpawnPacket(app, this);
		entity_list.QueueClients(this, app);
		safe_delete(app);
	}
}


void Mob::ModSkillDmgTaken(EQEmu::skills::SkillType skill_num, int value)
{
	if (skill_num == ALL_SKILLS)
		SkillDmgTaken_Mod[EQEmu::skills::HIGHEST_SKILL + 1] = value;

	else if (skill_num >= 0 && skill_num <= EQEmu::skills::HIGHEST_SKILL)
		SkillDmgTaken_Mod[skill_num] = value;
}

int16 Mob::GetModSkillDmgTaken(const EQEmu::skills::SkillType skill_num)
{
	if (skill_num == ALL_SKILLS)
		return SkillDmgTaken_Mod[EQEmu::skills::HIGHEST_SKILL + 1];

	else if (skill_num >= 0 && skill_num <= EQEmu::skills::HIGHEST_SKILL)
		return SkillDmgTaken_Mod[skill_num];

	return 0;
}

void Mob::ModVulnerability(uint8 resist, int16 value)
{
	if (resist < HIGHEST_RESIST+1)
		Vulnerability_Mod[resist] = value;

	else if (resist == 255)
		Vulnerability_Mod[HIGHEST_RESIST+1] = value;
}

int16 Mob::GetModVulnerability(const uint8 resist)
{
	if (resist < HIGHEST_RESIST+1)
		return Vulnerability_Mod[resist];

	else if (resist == 255)
		return Vulnerability_Mod[HIGHEST_RESIST+1];

	return 0;
}

void Mob::CastOnCurer(uint32 spell_id)
{
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if (spells[spell_id].effectid[i] == SE_CastOnCurer)
		{
			if(IsValidSpell(spells[spell_id].base[i]))
			{
				SpellFinished(spells[spell_id].base[i], this);
			}
		}
	}
}

void Mob::CastOnCure(uint32 spell_id)
{
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if (spells[spell_id].effectid[i] == SE_CastOnCure)
		{
			if(IsValidSpell(spells[spell_id].base[i]))
			{
				SpellFinished(spells[spell_id].base[i], this);
			}
		}
	}
}

void Mob::CastOnNumHitFade(uint32 spell_id)
{
	if(!IsValidSpell(spell_id))
		return;

	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if (spells[spell_id].effectid[i] == SE_CastonNumHitFade)
		{
			if(IsValidSpell(spells[spell_id].base[i]))
			{
				SpellFinished(spells[spell_id].base[i], this);
			}
		}
	}
}

void Mob::SlowMitigation(Mob* caster)
{
	if (GetSlowMitigation() && caster && caster->IsClient())
	{
		if ((GetSlowMitigation() > 0) && (GetSlowMitigation() < 26))
			caster->Message_StringID(MT_SpellFailure, SLOW_MOSTLY_SUCCESSFUL);

		else if ((GetSlowMitigation() >= 26) && (GetSlowMitigation() < 74))
			caster->Message_StringID(MT_SpellFailure, SLOW_PARTIALLY_SUCCESSFUL);

		else if ((GetSlowMitigation() >= 74) && (GetSlowMitigation() < 101))
			caster->Message_StringID(MT_SpellFailure, SLOW_SLIGHTLY_SUCCESSFUL);

		else if (GetSlowMitigation() > 100)
			caster->Message_StringID(MT_SpellFailure, SPELL_OPPOSITE_EFFECT);
	}
}

uint16 Mob::GetSkillByItemType(int ItemType)
{
	switch (ItemType) {
	case EQEmu::item::ItemType1HSlash:
		return EQEmu::skills::Skill1HSlashing;
	case EQEmu::item::ItemType2HSlash:
		return EQEmu::skills::Skill2HSlashing;
	case EQEmu::item::ItemType1HPiercing:
		return EQEmu::skills::Skill1HPiercing;
	case EQEmu::item::ItemType1HBlunt:
		return EQEmu::skills::Skill1HBlunt;
	case EQEmu::item::ItemType2HBlunt:
		return EQEmu::skills::Skill2HBlunt;
	case EQEmu::item::ItemType2HPiercing:
		if (IsClient() && CastToClient()->ClientVersion() < EQEmu::versions::ClientVersion::RoF2)
			return EQEmu::skills::Skill1HPiercing;
		else
			return EQEmu::skills::Skill2HPiercing;
	case EQEmu::item::ItemTypeBow:
		return EQEmu::skills::SkillArchery;
	case EQEmu::item::ItemTypeLargeThrowing:
	case EQEmu::item::ItemTypeSmallThrowing:
		return EQEmu::skills::SkillThrowing;
	case EQEmu::item::ItemTypeMartial:
		return EQEmu::skills::SkillHandtoHand;
	default:
		return EQEmu::skills::SkillHandtoHand;
	}
 }

uint8 Mob::GetItemTypeBySkill(EQEmu::skills::SkillType skill)
{
	switch (skill) {
	case EQEmu::skills::SkillThrowing:
		return EQEmu::item::ItemTypeSmallThrowing;
	case EQEmu::skills::SkillArchery:
		return EQEmu::item::ItemTypeArrow;
	case EQEmu::skills::Skill1HSlashing:
		return EQEmu::item::ItemType1HSlash;
	case EQEmu::skills::Skill2HSlashing:
		return EQEmu::item::ItemType2HSlash;
	case EQEmu::skills::Skill1HPiercing:
		return EQEmu::item::ItemType1HPiercing;
	case EQEmu::skills::Skill2HPiercing: // watch for undesired client behavior
		return EQEmu::item::ItemType2HPiercing;
	case EQEmu::skills::Skill1HBlunt:
		return EQEmu::item::ItemType1HBlunt;
	case EQEmu::skills::Skill2HBlunt:
		return EQEmu::item::ItemType2HBlunt;
	case EQEmu::skills::SkillHandtoHand:
		return EQEmu::item::ItemTypeMartial;
	default:
		return EQEmu::item::ItemTypeMartial;
	}
 }


bool Mob::PassLimitToSkill(uint16 spell_id, uint16 skill) {

	if (!IsValidSpell(spell_id))
		return false;

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (spells[spell_id].effectid[i] == SE_LimitToSkill){
			if (spells[spell_id].base[i] == skill){
				return true;
			}
		}
	}
	return false;
}

uint16 Mob::GetWeaponSpeedbyHand(uint16 hand) {

	uint16 weapon_speed = 0;
	switch (hand) {

		case 13:
			weapon_speed = attack_timer.GetDuration();
			break;
		case 14:
			weapon_speed = attack_dw_timer.GetDuration();
			break;
		case 11:
			weapon_speed = ranged_timer.GetDuration();
			break;
	}

	if (weapon_speed < RuleI(Combat, MinHastedDelay))
		weapon_speed = RuleI(Combat, MinHastedDelay);

	return weapon_speed;
}

int8 Mob::GetDecayEffectValue(uint16 spell_id, uint16 spelleffect) {

	if (!IsValidSpell(spell_id))
		return false;

	int spell_level = spells[spell_id].classes[(GetClass()%17) - 1];
	int effect_value = 0;
	int lvlModifier = 100;

	int buff_count = GetMaxTotalSlots();
	for (int slot = 0; slot < buff_count; slot++){
		if (IsValidSpell(buffs[slot].spellid)){
			for (int i = 0; i < EFFECT_COUNT; i++){
				if(spells[buffs[slot].spellid].effectid[i] == spelleffect) {

					int critchance = spells[buffs[slot].spellid].base[i];
					int decay = spells[buffs[slot].spellid].base2[i];
					int lvldiff = spell_level - spells[buffs[slot].spellid].max[i];

					if(lvldiff > 0 && decay > 0)
					{
						lvlModifier -= decay*lvldiff;
						if (lvlModifier > 0){
							critchance = (critchance*lvlModifier)/100;
							effect_value += critchance;
						}
					}

					else
						effect_value += critchance;
				}
			}
		}
	}

	return effect_value;
}

// Faction Mods for Alliance type spells
void Mob::AddFactionBonus(uint32 pFactionID,int32 bonus) {
	std::map <uint32, int32> :: const_iterator faction_bonus;
	typedef std::pair <uint32, int32> NewFactionBonus;

	faction_bonus = faction_bonuses.find(pFactionID);
	if(faction_bonus == faction_bonuses.end())
	{
		faction_bonuses.insert(NewFactionBonus(pFactionID,bonus));
	}
	else
	{
		if(faction_bonus->second<bonus)
		{
			faction_bonuses.erase(pFactionID);
			faction_bonuses.insert(NewFactionBonus(pFactionID,bonus));
		}
	}
}

// Faction Mods from items
void Mob::AddItemFactionBonus(uint32 pFactionID,int32 bonus) {
	std::map <uint32, int32> :: const_iterator faction_bonus;
	typedef std::pair <uint32, int32> NewFactionBonus;

	faction_bonus = item_faction_bonuses.find(pFactionID);
	if(faction_bonus == item_faction_bonuses.end())
	{
		item_faction_bonuses.insert(NewFactionBonus(pFactionID,bonus));
	}
	else
	{
		if((bonus > 0 && faction_bonus->second < bonus) || (bonus < 0 && faction_bonus->second > bonus))
		{
			item_faction_bonuses.erase(pFactionID);
			item_faction_bonuses.insert(NewFactionBonus(pFactionID,bonus));
		}
	}
}

int32 Mob::GetFactionBonus(uint32 pFactionID) {
	std::map <uint32, int32> :: const_iterator faction_bonus;
	faction_bonus = faction_bonuses.find(pFactionID);
	if(faction_bonus != faction_bonuses.end())
	{
		return (*faction_bonus).second;
	}
	return 0;
}

int32 Mob::GetItemFactionBonus(uint32 pFactionID) {
	std::map <uint32, int32> :: const_iterator faction_bonus;
	faction_bonus = item_faction_bonuses.find(pFactionID);
	if(faction_bonus != item_faction_bonuses.end())
	{
		return (*faction_bonus).second;
	}
	return 0;
}

void Mob::ClearItemFactionBonuses() {
	item_faction_bonuses.clear();
}

FACTION_VALUE Mob::GetSpecialFactionCon(Mob* iOther) {
	if (!iOther)
		return FACTION_INDIFFERENT;

	iOther = iOther->GetOwnerOrSelf();
	Mob* self = this->GetOwnerOrSelf();

	bool selfAIcontrolled = self->IsAIControlled();
	bool iOtherAIControlled = iOther->IsAIControlled();
	int selfPrimaryFaction = self->GetPrimaryFaction();
	int iOtherPrimaryFaction = iOther->GetPrimaryFaction();

	if (selfPrimaryFaction >= 0 && selfAIcontrolled)
		return FACTION_INDIFFERENT;
	if (iOther->GetPrimaryFaction() >= 0)
		return FACTION_INDIFFERENT;
/* special values:
	-2 = indiff to player, ally to AI on special values, indiff to AI
	-3 = dub to player, ally to AI on special values, indiff to AI
	-4 = atk to player, ally to AI on special values, indiff to AI
	-5 = indiff to player, indiff to AI
	-6 = dub to player, indiff to AI
	-7 = atk to player, indiff to AI
	-8 = indiff to players, ally to AI on same value, indiff to AI
	-9 = dub to players, ally to AI on same value, indiff to AI
	-10 = atk to players, ally to AI on same value, indiff to AI
	-11 = indiff to players, ally to AI on same value, atk to AI
	-12 = dub to players, ally to AI on same value, atk to AI
	-13 = atk to players, ally to AI on same value, atk to AI
*/
	switch (iOtherPrimaryFaction) {
		case -2: // -2 = indiff to player, ally to AI on special values, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_ALLY;
			else
				return FACTION_INDIFFERENT;
		case -3: // -3 = dub to player, ally to AI on special values, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_ALLY;
			else
				return FACTION_DUBIOUS;
		case -4: // -4 = atk to player, ally to AI on special values, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_ALLY;
			else
				return FACTION_SCOWLS;
		case -5: // -5 = indiff to player, indiff to AI
			return FACTION_INDIFFERENT;
		case -6: // -6 = dub to player, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_INDIFFERENT;
			else
				return FACTION_DUBIOUS;
		case -7: // -7 = atk to player, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_INDIFFERENT;
			else
				return FACTION_SCOWLS;
		case -8: // -8 = indiff to players, ally to AI on same value, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_INDIFFERENT;
			}
			else
				return FACTION_INDIFFERENT;
		case -9: // -9 = dub to players, ally to AI on same value, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_INDIFFERENT;
			}
			else
				return FACTION_DUBIOUS;
		case -10: // -10 = atk to players, ally to AI on same value, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_INDIFFERENT;
			}
			else
				return FACTION_SCOWLS;
		case -11: // -11 = indiff to players, ally to AI on same value, atk to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_SCOWLS;
			}
			else
				return FACTION_INDIFFERENT;
		case -12: // -12 = dub to players, ally to AI on same value, atk to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_SCOWLS;


			}
			else
				return FACTION_DUBIOUS;
		case -13: // -13 = atk to players, ally to AI on same value, atk to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_SCOWLS;
			}
			else
				return FACTION_SCOWLS;
		default:
			return FACTION_INDIFFERENT;
	}
}

bool Mob::HasSpellEffect(int effectid)
{
    int i;

    int buff_count = GetMaxTotalSlots();
    for(i = 0; i < buff_count; i++)
    {
        if(buffs[i].spellid == SPELL_UNKNOWN) { continue; }

        if(IsEffectInSpell(buffs[i].spellid, effectid))
        {
            return(1);
        }
    }
    return(0);
}

int Mob::GetSpecialAbility(int ability) {
	if(ability >= MAX_SPECIAL_ATTACK || ability < 0) {
		return 0;
	}

	return SpecialAbilities[ability].level;
}

int Mob::GetSpecialAbilityParam(int ability, int param) {
	if(param >= MAX_SPECIAL_ATTACK_PARAMS || param < 0 || ability >= MAX_SPECIAL_ATTACK || ability < 0) {
		return 0;
	}

	return SpecialAbilities[ability].params[param];
}

void Mob::SetSpecialAbility(int ability, int level) {
	if(ability >= MAX_SPECIAL_ATTACK || ability < 0) {
		return;
	}

	SpecialAbilities[ability].level = level;
}

void Mob::SetSpecialAbilityParam(int ability, int param, int value) {
	if(param >= MAX_SPECIAL_ATTACK_PARAMS || param < 0 || ability >= MAX_SPECIAL_ATTACK || ability < 0) {
		return;
	}

	SpecialAbilities[ability].params[param] = value;
}

void Mob::StartSpecialAbilityTimer(int ability, uint32 time) {
	if (ability >= MAX_SPECIAL_ATTACK || ability < 0) {
		return;
	}

	if(SpecialAbilities[ability].timer) {
		SpecialAbilities[ability].timer->Start(time);
	} else {
		SpecialAbilities[ability].timer = new Timer(time);
		SpecialAbilities[ability].timer->Start();
	}
}

void Mob::StopSpecialAbilityTimer(int ability) {
	if (ability >= MAX_SPECIAL_ATTACK || ability < 0) {
		return;
	}

	safe_delete(SpecialAbilities[ability].timer);
}

Timer *Mob::GetSpecialAbilityTimer(int ability) {
	if (ability >= MAX_SPECIAL_ATTACK || ability < 0) {
		return nullptr;
	}

	return SpecialAbilities[ability].timer;
}

void Mob::ClearSpecialAbilities() {
	for(int a = 0; a < MAX_SPECIAL_ATTACK; ++a) {
		SpecialAbilities[a].level = 0;
		safe_delete(SpecialAbilities[a].timer);
		for(int p = 0; p < MAX_SPECIAL_ATTACK_PARAMS; ++p) {
			SpecialAbilities[a].params[p] = 0;
		}
	}
}

void Mob::ProcessSpecialAbilities(const std::string &str) {
	ClearSpecialAbilities();

	std::vector<std::string> sp = SplitString(str, '^');
	for(auto iter = sp.begin(); iter != sp.end(); ++iter) {
		std::vector<std::string> sub_sp = SplitString((*iter), ',');
		if(sub_sp.size() >= 2) {
			int ability = std::stoi(sub_sp[0]);
			int value = std::stoi(sub_sp[1]);

			SetSpecialAbility(ability, value);
			switch(ability) {
			case SPECATK_QUAD:
				if(value > 0) {
					SetSpecialAbility(SPECATK_TRIPLE, 1);
				}
				break;
			case DESTRUCTIBLE_OBJECT:
				if(value == 0) {
					SetDestructibleObject(false);
				} else {
					SetDestructibleObject(true);
				}
				break;
			default:
				break;
			}

			for(size_t i = 2, p = 0; i < sub_sp.size(); ++i, ++p) {
				if(p >= MAX_SPECIAL_ATTACK_PARAMS) {
					break;
				}

				SetSpecialAbilityParam(ability, p, std::stoi(sub_sp[i]));
			}
		}
	}
}

// derived from client to keep these functions more consistent
// if anything seems weird, blame SoE
bool Mob::IsFacingMob(Mob *other)
{
	if (!other)
		return false;
	float angle = HeadingAngleToMob(other);
	// what the client uses appears to be 2x our internal heading
	float heading = GetHeading() * 2.0f;

	if (angle > 472.0 && heading < 40.0)
		angle = heading;
	if (angle < 40.0 && heading > 472.0)
		angle = heading;

	if (std::abs(angle - heading) <= 80.0)
		return true;

	return false;
}

// All numbers derived from the client
float Mob::HeadingAngleToMob(Mob *other)
{
	float mob_x = other->GetX();
	float mob_y = other->GetY();
	float this_x = GetX();
	float this_y = GetY();

	float y_diff = std::abs(this_y - mob_y);
	float x_diff = std::abs(this_x - mob_x);
	if (y_diff < 0.0000009999999974752427)
		y_diff = 0.0000009999999974752427;

	float angle = atan2(x_diff, y_diff) * 180.0f * 0.3183099014828645f; // angle, nice "pi"

	// return the right thing based on relative quadrant
	// I'm sure this could be improved for readability, but whatever
	if (this_y >= mob_y) {
		if (mob_x >= this_x)
			return (90.0f - angle + 90.0f) * 511.5f * 0.0027777778f;
		if (mob_x <= this_x)
			return (angle + 180.0f) * 511.5f * 0.0027777778f;
	}
	if (this_y > mob_y || mob_x > this_x)
		return angle * 511.5f * 0.0027777778f;
	else
		return (90.0f - angle + 270.0f) * 511.5f * 0.0027777778f;
}

int32 Mob::GetSpellStat(uint32 spell_id, const char *identifier, uint8 slot)
{
	if (!IsValidSpell(spell_id))
		return 0;

	if (!identifier)
		return 0;

	int32 stat = 0;

	if (slot > 0)
		slot = slot - 1;

	std::string id = identifier;
	for(uint32 i = 0; i < id.length(); ++i)
	{
		id[i] = tolower(id[i]);
	}

	if (slot < 16){
		if (id == "classes") {return spells[spell_id].classes[slot]; }
		else if (id == "dieties") {return spells[spell_id].deities[slot];}
	}

	if (slot < 12){
		if (id == "base") {return spells[spell_id].base[slot];}
		else if (id == "base2") {return spells[spell_id].base2[slot];}
		else if (id == "max") {return spells[spell_id].max[slot];}
		else if (id == "formula") {return spells[spell_id].formula[slot];}
		else if (id == "effectid") {return spells[spell_id].effectid[slot];}
	}

	if (slot < 4){
		if (id == "components") { return spells[spell_id].components[slot];}
		else if (id == "component_counts") { return spells[spell_id].component_counts[slot];}
		else if (id == "NoexpendReagent") {return spells[spell_id].NoexpendReagent[slot];}
	}

	if (id == "range") {return static_cast<int32>(spells[spell_id].range); }
	else if (id == "aoerange") {return static_cast<int32>(spells[spell_id].aoerange);}
	else if (id == "pushback") {return static_cast<int32>(spells[spell_id].pushback);}
	else if (id == "pushup") {return static_cast<int32>(spells[spell_id].pushup);}
	else if (id == "cast_time") {return spells[spell_id].cast_time;}
	else if (id == "recovery_time") {return spells[spell_id].recovery_time;}
	else if (id == "recast_time") {return spells[spell_id].recast_time;}
	else if (id == "buffdurationformula") {return spells[spell_id].buffdurationformula;}
	else if (id == "buffduration") {return spells[spell_id].buffduration;}
	else if (id == "AEDuration") {return spells[spell_id].AEDuration;}
	else if (id == "mana") {return spells[spell_id].mana;}
	//else if (id == "LightType") {stat = spells[spell_id].LightType;} - Not implemented
	else if (id == "goodEffect") {return spells[spell_id].goodEffect;}
	else if (id == "Activated") {return spells[spell_id].Activated;}
	else if (id == "resisttype") {return spells[spell_id].resisttype;}
	else if (id == "targettype") {return spells[spell_id].targettype;}
	else if (id == "basedeiff") {return spells[spell_id].basediff;}
	else if (id == "skill") {return spells[spell_id].skill;}
	else if (id == "zonetype") {return spells[spell_id].zonetype;}
	else if (id == "EnvironmentType") {return spells[spell_id].EnvironmentType;}
	else if (id == "TimeOfDay") {return spells[spell_id].TimeOfDay;}
	else if (id == "CastingAnim") {return spells[spell_id].CastingAnim;}
	else if (id == "SpellAffectIndex") {return spells[spell_id].SpellAffectIndex; }
	else if (id == "disallow_sit") {return spells[spell_id].disallow_sit; }
	//else if (id == "spellanim") {stat = spells[spell_id].spellanim; } - Not implemented
	else if (id == "uninterruptable") {return spells[spell_id].uninterruptable; }
	else if (id == "ResistDiff") {return spells[spell_id].ResistDiff; }
	else if (id == "dot_stacking_exemp") {return spells[spell_id].dot_stacking_exempt; }
	else if (id == "RecourseLink") {return spells[spell_id].RecourseLink; }
	else if (id == "no_partial_resist") {return spells[spell_id].no_partial_resist; }
	else if (id == "short_buff_box") {return spells[spell_id].short_buff_box; }
	else if (id == "descnum") {return spells[spell_id].descnum; }
	else if (id == "effectdescnum") {return spells[spell_id].effectdescnum; }
	else if (id == "npc_no_los") {return spells[spell_id].npc_no_los; }
	else if (id == "reflectable") {return spells[spell_id].reflectable; }
	else if (id == "bonushate") {return spells[spell_id].bonushate; }
	else if (id == "EndurCost") {return spells[spell_id].EndurCost; }
	else if (id == "EndurTimerIndex") {return spells[spell_id].EndurTimerIndex; }
	else if (id == "IsDisciplineBuf") {return spells[spell_id].IsDisciplineBuff; }
	else if (id == "HateAdded") {return spells[spell_id].HateAdded; }
	else if (id == "EndurUpkeep") {return spells[spell_id].EndurUpkeep; }
	else if (id == "numhitstype") {return spells[spell_id].numhitstype; }
	else if (id == "numhits") {return spells[spell_id].numhits; }
	else if (id == "pvpresistbase") {return spells[spell_id].pvpresistbase; }
	else if (id == "pvpresistcalc") {return spells[spell_id].pvpresistcalc; }
	else if (id == "pvpresistcap") {return spells[spell_id].pvpresistcap; }
	else if (id == "spell_category") {return spells[spell_id].spell_category; }
	else if (id == "can_mgb") {return spells[spell_id].can_mgb; }
	else if (id == "dispel_flag") {return spells[spell_id].dispel_flag; }
	else if (id == "MinResist") {return spells[spell_id].MinResist; }
	else if (id == "MaxResist") {return spells[spell_id].MaxResist; }
	else if (id == "viral_targets") {return spells[spell_id].viral_targets; }
	else if (id == "viral_timer") {return spells[spell_id].viral_timer; }
	else if (id == "NimbusEffect") {return spells[spell_id].NimbusEffect; }
	else if (id == "directional_start") {return static_cast<int32>(spells[spell_id].directional_start); }
	else if (id == "directional_end") {return static_cast<int32>(spells[spell_id].directional_end); }
	else if (id == "not_focusable") {return spells[spell_id].not_focusable; }
	else if (id == "suspendable") {return spells[spell_id].suspendable; }
	else if (id == "viral_range") {return spells[spell_id].viral_range; }
	else if (id == "spellgroup") {return spells[spell_id].spellgroup; }
	else if (id == "rank") {return spells[spell_id].rank; }
	else if (id == "no_resist") {return spells[spell_id].no_resist; }
	else if (id == "CastRestriction") {return spells[spell_id].CastRestriction; }
	else if (id == "AllowRest") {return spells[spell_id].AllowRest; }
	else if (id == "InCombat") {return spells[spell_id].InCombat; }
	else if (id == "OutofCombat") {return spells[spell_id].OutofCombat; }
	else if (id == "aemaxtargets") {return spells[spell_id].aemaxtargets; }
	else if (id == "no_heal_damage_item_mod") {return spells[spell_id].no_heal_damage_item_mod; }
	else if (id == "persistdeath") {return spells[spell_id].persistdeath; }
	else if (id == "min_dist") {return static_cast<int32>(spells[spell_id].min_dist); }
	else if (id == "min_dist_mod") {return static_cast<int32>(spells[spell_id].min_dist_mod); }
	else if (id == "max_dist") {return static_cast<int32>(spells[spell_id].max_dist); }
	else if (id == "min_range") {return static_cast<int32>(spells[spell_id].min_range); }
	else if (id == "DamageShieldType") {return spells[spell_id].DamageShieldType; }

	return stat;
}

bool Mob::CanClassEquipItem(uint32 item_id)
{
	const EQEmu::ItemData* itm = nullptr;
	itm = database.GetItem(item_id);

	if (!itm)
		return false;

	if(itm->Classes == 65535 )
		return true;

	if (GetClass() > 16)
		return false;

	int bitmask = 1;
	bitmask = bitmask << (GetClass() - 1);

	if(!(itm->Classes & bitmask))
		return false;
	else
		return true;
}

void Mob::SendAddPlayerState(PlayerState new_state)
{
	auto app = new EQApplicationPacket(OP_PlayerStateAdd, sizeof(PlayerState_Struct));
	auto ps = (PlayerState_Struct *)app->pBuffer;

	ps->spawn_id = GetID();
	ps->state = static_cast<uint32>(new_state);

	AddPlayerState(ps->state);
	entity_list.QueueClients(nullptr, app);
	safe_delete(app);
}

void Mob::SendRemovePlayerState(PlayerState old_state)
{
	auto app = new EQApplicationPacket(OP_PlayerStateRemove, sizeof(PlayerState_Struct));
	auto ps = (PlayerState_Struct *)app->pBuffer;

	ps->spawn_id = GetID();
	ps->state = static_cast<uint32>(old_state);

	RemovePlayerState(ps->state);
	entity_list.QueueClients(nullptr, app);
	safe_delete(app);
}

void Mob::SetCurrentSpeed(int in){
	if (current_speed != in)
	{
		current_speed = in;
		tar_ndx = 20;
		if (in == 0) {
			SetRunAnimSpeed(0);
			SetMoving(false);
			SendPosition();
		}
	}
}

int32 Mob::GetMeleeMitigation() {
	int32 mitigation = 0;
	mitigation += spellbonuses.MeleeMitigationEffect;
	mitigation += itembonuses.MeleeMitigationEffect;
	mitigation += aabonuses.MeleeMitigationEffect;
	return mitigation;
}

/* this is the mob being attacked.
 * Pass in the weapon's EQEmu::ItemInstance
 */
int Mob::ResistElementalWeaponDmg(const EQEmu::ItemInstance *item)
{
	if (!item)
		return 0;
	int magic = 0, fire = 0, cold = 0, poison = 0, disease = 0, chromatic = 0, prismatic = 0, physical = 0,
	    corruption = 0;
	int resist = 0;
	int roll = 0;
	/*  this is how the client does the resist rolls for these.
	 *  Given the difficulty of parsing out these resists, I'll trust the client
	 */
	if (item->GetItemElementalDamage(magic, fire, cold, poison, disease, chromatic, prismatic, physical, corruption, true)) {
		if (magic) {
			resist = GetMR();
			if (resist >= 201) {
				magic = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					magic = 0;
				else if (roll < 100)
					magic = magic * roll / 100;
			}
		}

		if (fire) {
			resist = GetFR();
			if (resist >= 201) {
				fire = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					fire = 0;
				else if (roll < 100)
					fire = fire * roll / 100;
			}
		}

		if (cold) {
			resist = GetCR();
			if (resist >= 201) {
				cold = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					cold = 0;
				else if (roll < 100)
					cold = cold * roll / 100;
			}
		}

		if (poison) {
			resist = GetPR();
			if (resist >= 201) {
				poison = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					poison = 0;
				else if (roll < 100)
					poison = poison * roll / 100;
			}
		}

		if (disease) {
			resist = GetDR();
			if (resist >= 201) {
				disease = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					disease = 0;
				else if (roll < 100)
					disease = disease * roll / 100;
			}
		}

		if (corruption) {
			resist = GetCorrup();
			if (resist >= 201) {
				corruption = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					corruption = 0;
				else if (roll < 100)
					corruption = corruption * roll / 100;
			}
		}

		if (chromatic) {
			resist = GetFR();
			int temp = GetCR();
			if (temp < resist)
				resist = temp;

			temp = GetMR();
			if (temp < resist)
				resist = temp;

			temp = GetDR();
			if (temp < resist)
				resist = temp;

			temp = GetPR();
			if (temp < resist)
				resist = temp;

			if (resist >= 201) {
				chromatic = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					chromatic = 0;
				else if (roll < 100)
					chromatic = chromatic * roll / 100;
			}
		}

		if (prismatic) {
			resist = (GetFR() + GetCR() + GetMR() + GetDR() + GetPR()) / 5;
			if (resist >= 201) {
				prismatic = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					prismatic = 0;
				else if (roll < 100)
					prismatic = prismatic * roll / 100;
			}
		}

		if (physical) {
			resist = GetPhR();
			if (resist >= 201) {
				physical = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					physical = 0;
				else if (roll < 100)
					physical = physical * roll / 100;
			}
		}
	}

	return magic + fire + cold + poison + disease + chromatic + prismatic + physical + corruption;
}

/* this is the mob being attacked.
 * Pass in the weapon's EQEmu::ItemInstance
 */
int Mob::CheckBaneDamage(const EQEmu::ItemInstance *item)
{
	if (!item)
		return 0;

	int damage = item->GetItemBaneDamageBody(GetBodyType(), true);
	damage += item->GetItemBaneDamageRace(GetRace(), true);

	return damage;
}

void Mob::CancelSneakHide()
{
	if (hidden || improved_hidden) {
		hidden = false;
		improved_hidden = false;
		auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}
}

void Mob::CommonBreakInvisible()
{
	BreakInvisibleSpells();
	CancelSneakHide();
}

#ifdef BOTS
bool Mob::JoinHealRotationTargetPool(std::shared_ptr<HealRotation>* heal_rotation)
{
	if (IsHealRotationTarget())
		return false;
	if (!heal_rotation->use_count())
		return false;
	if (!(*heal_rotation))
		return false;
	if (!IsHealRotationTargetMobType(this))
		return false;

	if (!(*heal_rotation)->AddTargetToPool(this))
		return false;

	m_target_of_heal_rotation = *heal_rotation;

	return IsHealRotationTarget();
}

bool Mob::LeaveHealRotationTargetPool()
{
	if (!IsHealRotationTarget()) {
		m_target_of_heal_rotation.reset();
		return true;
	}

	m_target_of_heal_rotation->RemoveTargetFromPool(this);
	m_target_of_heal_rotation.reset();
	
	return !IsHealRotationTarget();
}

uint32 Mob::HealRotationHealCount()
{
	if (!IsHealRotationTarget())
		return 0;

	return m_target_of_heal_rotation->HealCount(this);
}

uint32 Mob::HealRotationExtendedHealCount()
{
	if (!IsHealRotationTarget())
		return 0;

	return m_target_of_heal_rotation->ExtendedHealCount(this);
}

float Mob::HealRotationHealFrequency()
{
	if (!IsHealRotationTarget())
		return 0.0f;

	return m_target_of_heal_rotation->HealFrequency(this);
}

float Mob::HealRotationExtendedHealFrequency()
{
	if (!IsHealRotationTarget())
		return 0.0f;

	return m_target_of_heal_rotation->ExtendedHealFrequency(this);
}
#endif
