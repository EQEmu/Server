MonkACBonusWeight = RuleI.Get(Rule.MonkACBonusWeight);
NPCACFactor = RuleR.Get(Rule.NPCACFactor);
OldACSoftcapRules = RuleB.Get(Rule.OldACSoftcapRules);
ClothACSoftcap = RuleI.Get(Rule.ClothACSoftcap);
LeatherACSoftcap = RuleI.Get(Rule.LeatherACSoftcap);
MonkACSoftcap = RuleI.Get(Rule.MonkACSoftcap);
ChainACSoftcap = RuleI.Get(Rule.ChainACSoftcap);
PlateACSoftcap = RuleI.Get(Rule.PlateACSoftcap);

AAMitigationACFactor = RuleR.Get(Rule.AAMitigationACFactor);
WarriorACSoftcapReturn = RuleR.Get(Rule.WarriorACSoftcapReturn);
KnightACSoftcapReturn = RuleR.Get(Rule.KnightACSoftcapReturn);
LowPlateChainACSoftcapReturn = RuleR.Get(Rule.LowPlateChainACSoftcapReturn);
LowChainLeatherACSoftcapReturn = RuleR.Get(Rule.LowChainLeatherACSoftcapReturn);
CasterACSoftcapReturn = RuleR.Get(Rule.CasterACSoftcapReturn);
MiscACSoftcapReturn = RuleR.Get(Rule.MiscACSoftcapReturn);
WarACSoftcapReturn = RuleR.Get(Rule.WarACSoftcapReturn);
ClrRngMnkBrdACSoftcapReturn = RuleR.Get(Rule.ClrRngMnkBrdACSoftcapReturn);
PalShdACSoftcapReturn = RuleR.Get(Rule.PalShdACSoftcapReturn);
DruNecWizEncMagACSoftcapReturn = RuleR.Get(Rule.DruNecWizEncMagACSoftcapReturn);
RogShmBstBerACSoftcapReturn = RuleR.Get(Rule.RogShmBstBerACSoftcapReturn);
SoftcapFactor = RuleR.Get(Rule.SoftcapFactor);
ACthac0Factor = RuleR.Get(Rule.ACthac0Factor);
ACthac20Factor = RuleR.Get(Rule.ACthac20Factor);

MeleeBaseCritChance = 0.0;
ClientBaseCritChance = 0.0;
BerserkBaseCritChance = 6.0;
WarBerBaseCritChance = 3.0;
RogueCritThrowingChance = 25;
RogueDeadlyStrikeChance = 80;
RogueDeadlyStrikeMod = 2;

BaseHitChance = RuleR.Get(Rule.BaseHitChance);
NPCBonusHitChance = RuleR.Get(Rule.NPCBonusHitChance);
HitFalloffMinor = RuleR.Get(Rule.HitFalloffMinor);
HitFalloffModerate = RuleR.Get(Rule.HitFalloffModerate);
HitFalloffMajor = RuleR.Get(Rule.HitFalloffMajor);
HitBonusPerLevel = RuleR.Get(Rule.HitBonusPerLevel);
AgiHitFactor = RuleR.Get(Rule.AgiHitFactor);
WeaponSkillFalloff = RuleR.Get(Rule.WeaponSkillFalloff);
ArcheryHitPenalty = RuleR.Get(Rule.ArcheryHitPenalty);
UseOldDamageIntervalRules = RuleB.Get(Rule.UseOldDamageIntervalRules);

CriticalMessageRange = RuleI.Get(Rule.CriticalDamage);

function MeleeMitigation(e)
	e.IgnoreDefault = true;
	
	if e.hit.damage_done < 0 or e.hit.base_damage == 0 then
		return e;
	end
	
	e.hit.damage_done = 2 * e.hit.base_damage * GetDamageTable(e.other, e.hit.skill) / 100;
	e.hit = DoMeleeMitigation(e.self, e.other, e.hit, e.opts);	
	return e;
end

function CheckHitChance(e)
	e.IgnoreDefault = true;
	
	local other = e.other;
	local attacker = other;
	local self = e.self;
	local defender = self;
	local chancetohit = BaseHitChance;
    local chance_mod = 0;	
	
	if(e.opts ~= nil) then
		chance_mod = e.opts.hit_chance;
	end
	
	if(attacker:IsNPC() and not attacker:IsPet()) then
		chancetohit = chancetohit + NPCBonusHitChance;
	end

	local pvpmode = false;
	if(self:IsClient() and other:IsClient()) then
		pvpmode = true;
	end
    
	if (chance_mod >= 10000) then
		e.ReturnValue = true;
		return e;
	end
	  
	local avoidanceBonus = 0;
	local hitBonus = 0;
    
	local attacker_level = attacker:GetLevel();
	if(attacker_level < 1) then
		attacker_level = 1;
	end
	
	local defender_level = defender:GetLevel();
    if(defender_level < 1) then
		defender_level = 1;
	end

	local level_difference = attacker_level - defender_level;
	local range = defender_level;
	range = ((range / 4) + 3);
    
	if(level_difference < 0) then
		if(level_difference >= -range) then
			chancetohit = chancetohit + ((level_difference / range) * HitFalloffMinor);
		elseif (level_difference >= -(range+3.0)) then
			chancetohit = chancetohit - HitFalloffMinor;
			chancetohit = chancetohit + (((level_difference + range) / 3.0) * HitFalloffModerate);
		else
			chancetohit = chancetohit - (HitFalloffMinor + HitFalloffModerate);
			chancetohit = chancetohit + (((level_difference + range + 3.0) / 12.0) * HitFalloffMajor);
		end
	else
		chancetohit = chancetohit + (HitBonusPerLevel * level_difference);
	end

	chancetohit = chancetohit - (defender:GetAGI() * AgiHitFactor);
	
	if(attacker:IsClient()) then
		chancetohit = chancetohit - (WeaponSkillFalloff * (attacker:CastToClient():MaxSkill(e.hit.skill) - attacker:GetSkill(e.hit.skill)));
	end
    
	if(defender:IsClient()) then
		chancetohit = chancetohit + (WeaponSkillFalloff * (defender:CastToClient():MaxSkill(Skill.Defense) - defender:GetSkill(Skill.Defense)));
	end
	
	local attacker_spellbonuses = attacker:GetSpellBonuses();
	local attacker_itembonuses = attacker:GetItemBonuses();
	local attacker_aabonuses = attacker:GetAABonuses();
	local defender_spellbonuses = defender:GetSpellBonuses();
	local defender_itembonuses = defender:GetItemBonuses();
	local defender_aabonuses = defender:GetAABonuses();
	
	if(attacker_spellbonuses:MeleeSkillCheckSkill() == e.hit.skill or attacker_spellbonuses:MeleeSkillCheckSkill() == 255) then
		chancetohit = chancetohit + attacker_spellbonuses:MeleeSkillCheck();
	end
	
	if(attacker_itembonuses:MeleeSkillCheckSkill() == e.hit.skill or attacker_itembonuses:MeleeSkillCheckSkill() == 255) then
		chancetohit = chancetohit +  attacker_itembonuses:MeleeSkillCheck();
	end

	avoidanceBonus = defender_spellbonuses:AvoidMeleeChanceEffect() +
				defender_itembonuses:AvoidMeleeChanceEffect() +
				defender_aabonuses:AvoidMeleeChanceEffect() +
				(defender_itembonuses:AvoidMeleeChance() / 10.0);
    
	local owner = Mob();
	if (defender:IsPet()) then
		owner = defender:GetOwner();
	elseif (defender:IsNPC() and defender:CastToNPC():GetSwarmOwner()) then
		local entity_list = eq.get_entity_list();
		owner = entity_list:GetMobID(defender:CastToNPC():GetSwarmOwner());
	end
    
	if (owner.valid) then
		avoidanceBonus = avoidanceBonus + owner:GetAABonuses():PetAvoidance() + owner:GetSpellBonuses():PetAvoidance() + owner:GetItemBonuses():PetAvoidance();
	end

	if(defender:IsNPC()) then
		avoidanceBonus = avoidanceBonus + (defender:CastToNPC():GetAvoidanceRating() / 10.0);
	end

	hitBonus = hitBonus + attacker_itembonuses:HitChanceEffect(e.hit.skill) +
				attacker_spellbonuses:HitChanceEffect(e.hit.skill) +
				attacker_aabonuses:HitChanceEffect(e.hit.skill) +
				attacker_itembonuses:HitChanceEffect(Skill.HIGHEST_SKILL + 1) +
				attacker_spellbonuses:HitChanceEffect(Skill.HIGHEST_SKILL + 1) +
				attacker_aabonuses:HitChanceEffect(Skill.HIGHEST_SKILL + 1);
    
	hitBonus = hitBonus + (attacker_itembonuses:Accuracy(Skill.HIGHEST_SKILL + 1) +
				attacker_spellbonuses:Accuracy(Skill.HIGHEST_SKILL + 1) +
				attacker_aabonuses:Accuracy(Skill.HIGHEST_SKILL + 1) +
				attacker_aabonuses:Accuracy(e.hit.skill) +
				attacker_itembonuses:HitChance()) / 15.0;
    
	hitBonus = hitBonus + chance_mod;
    
	if(attacker:IsNPC()) then
		hitBonus = hitBonus + (attacker:CastToNPC():GetAccuracyRating() / 10.0);
	end
    
	if (e.hit.skill == Skill.Archery) then
		hitBonus = hitBonus - (hitBonus * ArcheryHitPenalty);
	end

	chancetohit = chancetohit + ((chancetohit * (hitBonus - avoidanceBonus)) / 100.0);

	if(chancetohit > 1000 or chancetohit < -1000) then
	elseif(chancetohit > 95) then
		chancetohit = 95;
	elseif(chancetohit < 5) then
		chancetohit = 5;
	end

	local tohit_roll = Random.Real(0, 100);
	if(tohit_roll <= chancetohit) then
		e.ReturnValue = true;
	else
		e.ReturnValue = false;
	end

	return e;
end

function TryCriticalHit(e)
	e.IgnoreDefault = true;
	
	local self = e.self;
	local defender = e.other;
	
	if(e.hit.damage_done < 1 or defender.null) then
		return e;
	end
	
	if ((self:IsPet() and self:GetOwner():IsClient()) or (self:IsNPC() and self:CastToNPC():GetSwarmOwner() ~= 0)) then
		e.hit = TryPetCriticalHit(self, defender, e.hit);
		return e;
	end
	
	if (self:IsPet() and self:GetOwner().valid and self:GetOwner():IsBot()) then
		e.hit = TryPetCriticalHit(self, defender, e.hit);
		return e;
	end

	local critChance = 0.0;
	local IsBerskerSPA = false;
	local aabonuses = self:GetAABonuses();
	local itembonuses = self:GetItemBonuses();
	local spellbonuses = self:GetSpellBonuses();
	local entity_list = eq.get_entity_list();

	if (defender:GetBodyType() == BT.Undead or defender:GetBodyType() == BT.SummonedUndead or defender:GetBodyType() == BT.Vampire) then
		local SlayRateBonus = aabonuses:SlayUndead(0) + itembonuses:SlayUndead(0) + spellbonuses:SlayUndead(0);
		if (SlayRateBonus > 0) then
			local slayChance = SlayRateBonus / 10000.0;
			if (Random.RollReal(slayChance)) then
				local SlayDmgBonus = aabonuses:SlayUndead(1) + itembonuses:SlayUndead(1) + spellbonuses:SlayUndead(1);
				e.hit.damage_done = (e.hit.damage_done * SlayDmgBonus * 2.25) / 100;
				
				if (self:GetGender() == 1) then
					entity_list:FilteredMessageClose(self, false, CriticalMessageRange, MT.CritMelee, Filter.MeleeCrits, string.format('%s\'s holy blade cleanses her target! (%d)', self:GetCleanName(), e.hit.damage_done));
				else
					entity_list:FilteredMessageClose(self, false, CriticalMessageRange, MT.CritMelee, Filter.MeleeCrits, string.format('%s\'s holy blade cleanses his target! (%d)', self:GetCleanName(), e.hit.damage_done));
				end
				
				return e;
			end
		end
	end
    
	critChance = critChance + MeleeBaseCritChance;
    
	if (self:IsClient()) then
		critChance  = critChance + ClientBaseCritChance;
    
		if (spellbonuses:BerserkSPA() or itembonuses:BerserkSPA() or aabonuses:BerserkSPA()) then
			IsBerskerSPA = true;
		end
    
		if (((self:GetClass() == Class.WARRIOR or self:GetClass() == Class.BERSERKER) and self:GetLevel() >= 12) or IsBerskerSPA) then
			if (self:IsBerserk() or IsBerskerSPA) then
				critChance = critChance + BerserkBaseCritChance;
			else
				critChance = critChance + WarBerBaseCritChance;
			end
		end
	end
    
	local deadlyChance = 0;
	local deadlyMod = 0;
	if (e.hit.skill == Skill.Archery and self:GetClass() == Class.RANGER and self:GetSkill(Skill.Archery) >= 65) then
		critChance = critChance + 6;
	end

	if (e.hit.skill == Skill.Throwing and self:GetClass() == Class.ROGUE and self:GetSkill(Skill.Throwing) >= 65) then
		critChance = critChance + RogueCritThrowingChance;
		deadlyChance = RogueDeadlyStrikeChance;
		deadlyMod = RogueDeadlyStrikeMod;
	end
    
	local CritChanceBonus = GetCriticalChanceBonus(self, e.hit.skill);
    
	if (CritChanceBonus > 0 or critChance > 0) then
		if (self:GetDEX() <= 255) then
			critChance = critChance + (self:GetDEX() / 125.0);
		elseif (self:GetDEX() > 255) then
			critChance = critChance + ((self:GetDEX() - 255) / 500.0) + 2.0;
		end
		critChance = critChance + (critChance * CritChanceBonus / 100.0);
	end
    
	if(opts ~= nil) then
		critChance = critChance * opts.crit_percent;
		critChance = critChance + opts.crit_flat;
	end
    
	if(critChance > 0) then
    
		critChance = critChance / 100;
    
		if(Random.RollReal(critChance)) then
			local critMod = 200;
			local crip_success = false;
			local CripplingBlowChance = GetCrippBlowChance(self);
    
			if (CripplingBlowChance > 0 or (self:IsBerserk() or IsBerskerSPA)) then
				if (not self:IsBerserk() and not IsBerskerSPA) then
					critChance = critChance * (CripplingBlowChance / 100.0);
				end
    
				if ((self:IsBerserk() or IsBerskerSPA) or Random.RollReal(critChance)) then
					critMod = 400;
					crip_success = true;
				end
			end
    
			critMod = critMod + GetCritDmgMod(self, e.hit.skill) * 2;
			e.hit.damage_done = e.hit.damage_done * critMod / 100;
    
			local deadlySuccess = false;
			if (deadlyChance > 0 and Random.RollReal(deadlyChance / 100.0)) then
				if (self:BehindMob(defender, self:GetX(), self:GetY())) then
					e.hit.damage_done = e.hit.damage_done * deadlyMod;
					deadlySuccess = true;
				end
			end
    
			if (crip_success) then
				entity_list:FilteredMessageClose(self, false, CriticalMessageRange, MT.CritMelee, Filter.MeleeCrits, string.format('%s lands a Crippling Blow! (%d)', self:GetCleanName(), e.hit.damage_done));
				if (defender:GetLevel() <= 55 and not defender:GetSpecialAbility(SpecialAbility.unstunable)) then
					defender:Emote("staggers.");
					defender:Stun(0);
				end
			elseif (deadlySuccess) then
				entity_list:FilteredMessageClose(self, false, CriticalMessageRange, MT.CritMelee, Filter.MeleeCrits, string.format('%s scores a Deadly Strike! (%d)', self:GetCleanName(), e.hit.damage_done));
			else
				entity_list:FilteredMessageClose(self, false, CriticalMessageRange, MT.CritMelee, Filter.MeleeCrits, string.format('%s scores a critical hit! (%d)', self:GetCleanName(), e.hit.damage_done));
			end
		end
	end
	
	return e;
end

function TryPetCriticalHit(self, defender, hit)
	if(hit.damage_done < 1) then
		return hit;
	end

	local owner = Mob();
	local critChance = MeleeBaseCritChance;
	local critMod = 163;

	if (self:IsPet()) then
		owner = self:GetOwner();
	elseif (self:IsNPC() and self:CastToNPC():GetSwarmOwner()) then
		local entity_list = eq.get_entity_list();
		owner = entity_list:GetMobID(self:CastToNPC():GetSwarmOwner());
	else
		return hit;
	end

	if (owner.null) then
		return hit;
	end
	
	local CritPetChance = owner:GetAABonuses():PetCriticalHit() + owner:GetItemBonuses():PetCriticalHit() + owner:GetSpellBonuses():PetCriticalHit();
	local CritChanceBonus = GetCriticalChanceBonus(self, hit.skill);

	if (CritPetChance or critChance) then
		critChance = critChance + CritPetChance;
		critChance = critChance + (critChance * CritChanceBonus / 100.0);
	end

	if(critChance > 0) then
		critChance = critChance / 100;

		if(Random.RollReal(critChance)) then
			local entity_list = eq.get_entity_list();
			critMod = critMod + GetCritDmgMod(self, hit.skill) * 2;
			hit.damage_done = (hit.damage_done * critMod) / 100;
			entity_list:FilteredMessageClose(this, false, CriticalMessageRange,
					MT.CritMelee, Filter.MeleeCrits, string.format('%s scores a critical hit! (%d)',
					self:GetCleanName(), e.hit.damage_done));
		end
	end
	
	return hit;
end

function GetCriticalChanceBonus(self, skill)
	
	local critical_chance = 0;
	
	local aabonuses = self:GetAABonuses();
	local itembonuses = self:GetItemBonuses();
	local spellbonuses = self:GetSpellBonuses();
	
	critical_chance = critical_chance + itembonuses:CriticalHitChance(Skill.HIGHEST_SKILL + 1);
	critical_chance = critical_chance + spellbonuses:CriticalHitChance(Skill.HIGHEST_SKILL + 1);
	critical_chance = critical_chance + aabonuses:CriticalHitChance(Skill.HIGHEST_SKILL + 1);
	critical_chance = critical_chance + itembonuses:CriticalHitChance(skill);
	critical_chance = critical_chance + spellbonuses:CriticalHitChance(skill);
	critical_chance = critical_chance + aabonuses:CriticalHitChance(skill);
		
	return critical_chance;
end

function GetCritDmgMod(self, skill)
	local critDmg_mod = 0;
	
	local aabonuses = self:GetAABonuses();
	local itembonuses = self:GetItemBonuses();
	local spellbonuses = self:GetSpellBonuses();
	
	critDmg_mod = critDmg_mod + itembonuses:CritDmgMod(Skill.HIGHEST_SKILL + 1);
	critDmg_mod = critDmg_mod + spellbonuses:CritDmgMod(Skill.HIGHEST_SKILL + 1);
	critDmg_mod = critDmg_mod + aabonuses:CritDmgMod(Skill.HIGHEST_SKILL + 1);
	critDmg_mod = critDmg_mod + itembonuses:CritDmgMod(skill);
	critDmg_mod = critDmg_mod + spellbonuses:CritDmgMod(skill);
	critDmg_mod = critDmg_mod + aabonuses:CritDmgMod(skill);
		
	return critDmg_mod;
end

function GetCrippBlowChance(self)
	local aabonuses = self:GetAABonuses();
	local itembonuses = self:GetItemBonuses();
	local spellbonuses = self:GetSpellBonuses();
	local crip_chance = itembonuses:CrippBlowChance() + spellbonuses:CrippBlowChance() + aabonuses:CrippBlowChance();

	if(crip_chance < 0) then
		crip_chance = 0;
	end

	return crip_chance;
end

function DoMeleeMitigation(defender, attacker, hit, opts)
	if hit.damage_done <= 0 then
		return hit;
	end
	
	local aabonuses = defender:GetAABonuses();
	local itembonuses = defender:GetItemBonuses();
	local spellbonuses = defender:GetSpellBonuses();
	
	local aa_mit = (aabonuses:CombatStability() + itembonuses:CombatStability() + spellbonuses:CombatStability()) / 100.0;
	local softcap = (defender:GetSkill(15) + defender:GetLevel()) * SoftcapFactor * (1.0 + aa_mit);
	local mitigation_rating = 0.0;
	local attack_rating = 0.0;
	local shield_ac = 0;
	local armor = 0;
	local weight = 0.0;
	local monkweight = MonkACBonusWeight;
	
	if defender:IsClient() then
		armor, shield_ac = GetRawACNoShield(defender);
		weight = defender:CastToClient():CalcCurrentWeight() / 10;
	elseif defender:IsNPC() then
		armor = defender:CastToNPC():GetRawAC();
		local PetACBonus = 0;
		
		if not defender:IsPet() then
			armor = armor / NPCACFactor;
		end
		
		local owner = Mob();
		if defender:IsPet() then
			owner = defender:GetOwner();
		elseif defender:CastToNPC():GetSwarmOwner() ~= 0 then
			local entity_list = eq.get_entity_list();
			owner = entity_list:GetMobID(defender:CastToNPC():GetSwarmOwner());
		end
		
		if owner.valid then
			PetACBonus = owner:GetAABonuses():PetMeleeMitigation() + owner:GetItemBonuses():PetMeleeMitigation() + owner:GetSpellBonuses():PetMeleeMitigation();
		end
		
		armor = armor + defender:GetSpellBonuses():AC() + defender:GetItemBonuses():AC() + PetACBonus + 1;
	end
	
	if (opts ~= nil) then
		armor = armor * (1.0 - opts.armor_pen_percent);
		armor = armor - opts.armor_pen_flat;
	end
	
	local defender_class = defender:GetClass();
	if OldACSoftcapRules then
		if defender_class == Class.WIZARD or defender_class == Class.MAGICIAN or defender_class == Class.NECROMANCER or defender_class == Class.ENCHANTER then
			softcap = ClothACSoftcap;
		elseif defender_class == Class.MONK and weight <= monkweight then
			softcap = MonkACSoftcap;
		elseif defender_class == Class.DRUID or defender_class == Class.BEASTLORD or defender_class == Class.MONK then
			softcap = LeatherACSoftcap;
		elseif defender_class == Class.SHAMAN or defender_class == Class.ROGUE or defender_class == Class.BERSERKER or defender_class == Class.RANGER then
			softcap = ChainACSoftcap;
		else
			softcap = PlateACSoftcap;
		end
	end

	softcap = softcap + shield_ac;
	armor = armor + shield_ac;
	
	if OldACSoftcapRules then
		softcap = softcap + (softcap * (aa_mit * AAMitigationACFactor));
	end
	
	if armor > softcap then
		local softcap_armor = armor - softcap;
		if OldACSoftcapRules then
			if defender_class == Class.WARRIOR then
				softcap_armor = softcap_armor * WarriorACSoftcapReturn;
			elseif defender_class == Class.SHADOWKNIGHT or defender_class == Class.PALADIN or (defender_class == Class.MONK and weight <= monkweight) then
				softcap_armor = softcap_armor * KnightACSoftcapReturn;
			elseif defender_class == Class.CLERIC or defender_class == Class.BARD or defender_class == Class.BERSERKER or defender_class == Class.ROGUE or defender_class == Class.SHAMAN or defender_class == Class.MONK then
				softcap_armor = softcap_armor * LowPlateChainACSoftcapReturn;
			elseif defender_class == Class.RANGER or defender_class == Class.BEASTLORD then
				softcap_armor = softcap_armor * LowChainLeatherACSoftcapReturn;
			elseif defender_class == Class.WIZARD or defender_class == Class.MAGICIAN or defender_class == Class.NECROMANCER or defender_class == Class.ENCHANTER or defender_class == Class.DRUID then
				softcap_armor = softcap_armor * CasterACSoftcapReturn;
			else
				softcap_armor = softcap_armor * MiscACSoftcapReturn;
			end
		else
			if defender_class == Class.WARRIOR then
				softcap_armor = softcap_armor * WarACSoftcapReturn;
			elseif defender_class == Class.PALADIN or defender_class == Class.SHADOWKNIGHT then
				softcap_armor = softcap_armor * PalShdACSoftcapReturn;
			elseif defender_class == Class.CLERIC or defender_class == Class.RANGER or	defender_class == Class.MONK or defender_class == Class.BARD then
				softcap_armor = softcap_armor * ClrRngMnkBrdACSoftcapReturn;
			elseif defender_class == Class.DRUID or defender_class == Class.NECROMANCER or defender_class == Class.WIZARD or defender_class == Class.ENCHANTER or defender_class == Class.MAGICIAN then
				softcap_armor = softcap_armor * DruNecWizEncMagACSoftcapReturn;
			elseif defender_class == Class.ROGUE or defender_class == Class.SHAMAN or defender_class == Class.BEASTLORD or defender_class == Class.BERSERKER then
				softcap_armor = softcap_armor * RogShmBstBerACSoftcapReturn;
			else
				softcap_armor = softcap_armor * MiscACSoftcapReturn;
			end
		end
		
		armor = softcap + softcap_armor;
	end
		
	local mitigation_rating;
	if defender_class == Class.WIZARD or defender_class == Class.MAGICIAN or defender_class == Class.NECROMANCER or defender_class == Class.ENCHANTER then
		mitigation_rating = ((defender:GetSkill(Skill.Defense) + defender:GetItemBonuses():HeroicAGI() / 10) / 4.0) + armor + 1;
	else
		mitigation_rating = ((defender:GetSkill(Skill.Defense) + defender:GetItemBonuses():HeroicAGI() / 10) / 3.0) + (armor * 1.333333) + 1;
	end
	
	mitigation_rating = mitigation_rating * 0.847;
	
	local attack_rating;
	if attacker:IsClient() then
		attack_rating = (attacker:CastToClient():CalcATK() + ((attacker:GetSTR() - 66) * 0.9) + (attacker:GetSkill(Skill.Offense)*1.345));
	else
		attack_rating = (attacker:GetATK() + (attacker:GetSkill(Skill.Offense)*1.345) + ((attacker:GetSTR() - 66) * 0.9));
	end
	
	hit.damage_done = GetMeleeMitDmg(defender, attacker, hit.damage_done, hit.min_damage, mitigation_rating, attack_rating);

	if hit.damage_done < 0 then 
		hit.damage_done = 0;
	end
	
	return hit;
end

function GetMeleeMitDmg(defender, attacker, damage, min_damage, mitigation_rating, attack_rating)
	if defender:IsClient() then
		return ClientGetMeleeMitDmg(defender, attacker, damage, min_damage, mitigation_rating, attack_rating);
	else
		return MobGetMeleeMitDmg(defender, attacker, damage, min_damage, mitigation_rating, attack_rating);
	end
end

function ClientGetMeleeMitDmg(defender, attacker, damage, min_damage, mitigation_rating, attack_rating)
	if (not attacker:IsNPC() or UseOldDamageIntervalRules) then
		return MobGetMeleeMitDmg(defender, attacker, damage, min_damage, mitigation_rating, attack_rating);
	end
	
	local d = 10;
	local dmg_interval = (damage - min_damage) / 19.0;
	local dmg_bonus = min_damage - dmg_interval;
	local spellMeleeMit = (defender:GetSpellBonuses():MeleeMitigationEffect() + defender:GetItemBonuses():MeleeMitigationEffect() + defender:GetAABonuses():MeleeMitigationEffect()) / 100.0;
	if (defender:GetClass() == Class.WARRIOR) then
		spellMeleeMit = spellMeleeMit - 0.05;
	end
		
	dmg_bonus = dmg_bonus - (dmg_bonus * (defender:GetItemBonuses():MeleeMitigation() / 100.0));
	dmg_interval = dmg_interval + (dmg_interval * spellMeleeMit);

	local mit_roll = Random.Real(0, mitigation_rating);
	local atk_roll = Random.Real(0, attack_rating);

	if (atk_roll > mit_roll) then
		local a_diff = atk_roll - mit_roll;
		local thac0 = attack_rating * ACthac0Factor;
		local thac0cap = attacker:GetLevel() * 9 + 20;
		if (thac0 > thac0cap) then
			thac0 = thac0cap;
		end

		d = d + (10 * (a_diff / thac0));
	elseif (mit_roll > atk_roll) then
		local m_diff = mit_roll - atk_roll;
		local thac20 = mitigation_rating * ACthac20Factor;
		local thac20cap = defender:GetLevel() * 9 + 20;
		if (thac20 > thac20cap) then
			thac20 = thac20cap;
		end

		d = d - (10 * (m_diff / thac20));
	end

	if (d < 1) then
		d = 1;
	elseif (d > 20) then
		d = 20;
	end

	return math.floor(dmg_bonus + dmg_interval * d);
end

function MobGetMeleeMitDmg(defender, attacker, damage, min_damage, mitigation_rating, attack_rating)	
	local d = 10.0;
	local mit_roll = Random.Real(0, mitigation_rating);
	local atk_roll = Random.Real(0, attack_rating);

	if (atk_roll > mit_roll) then
		local a_diff = atk_roll - mit_roll;
		local thac0 = attack_rating * ACthac0Factor;
		local thac0cap = attacker:GetLevel() * 9 + 20;
		if (thac0 > thac0cap) then
			thac0 = thac0cap;
		end

		d = d - (10.0 * (a_diff / thac0));
	elseif (mit_roll > atk_roll) then
		local m_diff = mit_roll - atk_roll;
		local thac20 = mitigation_rating * ACthac20Factor;
		local thac20cap = defender:GetLevel() * 9 + 20;
		if (thac20 > thac20cap) then
			thac20 = thac20cap;
		end

		d = d + (10.0 * (m_diff / thac20));
	end

	if (d < 0.0) then
		d = 0.0;
	elseif (d > 20.0) then
		d = 20.0;
	end
	
	local interval = (damage - min_damage) / 20.0;
	damage = damage - (math.floor(d) * interval);	
	damage = damage - (min_damage * defender:GetItemBonuses():MeleeMitigation() / 100);	
	damage = damage + (damage * (defender:GetSpellBonuses():MeleeMitigationEffect() + defender:GetItemBonuses():MeleeMitigationEffect() + defender:GetAABonuses():MeleeMitigationEffect()) / 100);
	
	return damage;
end

function GetRawACNoShield(self)
	self = self:CastToClient();
	
	local ac = self:GetItemBonuses():AC() + self:GetSpellBonuses():AC() + self:GetAABonuses():AC();
	local shield_ac = 0;
	local inst = self:GetInventory():GetItem(Slot.Secondary);
	
	if inst.valid then
		if inst:GetItem():ItemType() == 8 then
			shield_ac = inst:GetItem():AC();
			
			for i = 1, 6 do
				local augment = inst:GetAugment(i - 1);
				if augment.valid then
					shield_ac = shield_ac + augment:GetItem():AC();
				end
			end
		end
	end
	
	ac = ac - shield_ac;
	return ac, shield_ac;
end

function GetDamageTable(attacker, skill)
	if not attacker:IsClient() then
		return 100;
	end
	
	if attacker:GetLevel() <= 51 then
		local ret_table = 0;
		local str_over_75 = 0;
		if attacker:GetSTR() > 75 then
			str_over_75 = attacker:GetSTR() - 75;
		end
		
		if str_over_75 > 255 then
			ret_table = (attacker:GetSkill(skill) + 255) / 2;
		else
			ret_table = (attacker:GetSkill(skill) + str_over_75) / 2;
		end
		
		if ret_table < 100 then
			return 100;
		end
		
		return ret_table;
	elseif attacker:GetLevel() >= 90 then
		if attacker:GetClass() == 7 then
			return 379;
		else
			return 345;
		end
	else
		local dmg_table = { 275, 275, 275, 275, 275, 280, 280, 280, 280, 285, 285, 285, 290, 290, 295, 295, 300, 300, 300, 305, 305, 305, 310, 310, 315, 315, 320, 320, 320, 325, 325, 325, 330, 330, 335, 335, 340, 340, 340 };
		
		if attacker:GetClass() == 7 then
			local monkDamageTableBonus = 20;
			return (dmg_table[attacker:GetLevel() - 50] * (100 + monkDamageTableBonus) / 100);
		else
			return dmg_table[attacker:GetLevel() - 50];
		end
	end
	return 100;
end

function ApplyDamageTable(e)
	e.IgnoreDefault = true;
	return e;
end

function CommonOutgoingHitSuccess(e)
	e = ApplyMeleeDamageBonus(e);
	e.hit.damage_done = e.hit.damage_done + (e.hit.damage_done * e.other:GetSkillDmgTaken(e.hit.skill) / 100) + (e.self:GetSkillDmgAmt(e.hit.skill) + e.other:GetFcDamageAmtIncoming(e.self, 0, true, e.hit.skill));
	e = TryCriticalHit(e);
	e.self:CheckNumHitsRemaining(5, -1, 65535);	
	e.IgnoreDefault = true;
	return e;
end

function ApplyMeleeDamageBonus(e)
	local dmgbonusmod = e.self:GetMeleeDamageMod_SE(e.hit.skill);
	if (opts) then
		dmgbonusmod = dmgbonusmod + e.opts.melee_damage_bonus_flat;
	end

	e.hit.damage_done = e.hit.damage_done + (e.hit.damage_done * dmgbonusmod / 100);
	return e;
end
