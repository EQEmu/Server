-- Source Function: Client::CheckFizzle()
function CheckFizzle(e)
  if eq.is_lost_dungeons_of_norrath_enabled() then
    e.IgnoreDefault = false;
    return e;

  else -- Classic Fizzle -- Based on TAKP's fizzle formula
    e.IgnoreDefault = true;
    local client = e.self;
    local spell = e.spell;
    if client:GetGM() == true then -- GMs never fizzle
      e.ReturnValue = true;
      return e;
    end

    local caster_class = client:GetClass();
    local spell_level = spell:Classes(caster_class - 1);
    local aa_bonuses = client:GetAABonuses();
    local item_bonuses = client:GetItemBonuses();
    local spell_bonuses = client:GetSpellBonuses();

    local no_fizzle_level = math.max(unpack({
        aa_bonuses:MasteryofPast(),
        item_bonuses:MasteryofPast(),
        spell_bonuses:MasteryofPast()
      }));
    local no_fizzle_level = 0;

    if spell_level < no_fizzle_level then
      e.ReturnValue = true;
      return e;
    end

    local fizzle_adjustment = spell:BaseDiff();
    local spell_casting_skill_effects = aa_bonuses:adjusted_casting_skill() +
        item_bonuses:adjusted_casting_skill() +
        spell_bonuses:adjusted_casting_skill();

    local capped_chance = 95;
    local random_penalty = 0;
    local effective_spell_casting_skill = 0;
    local spell_level_adjustment = 0;
    local prime_stat_bonus = 0;

    if fizzle_adjustment ~= 0 or spell_casting_skill_effects < 0 then
      -- If Superior Healing not cast by cleric
      if spell:ID() == 9 and caster_class ~= Class.CLERIC then
        fizzle_adjustment = 0
      end

      if spell_level > 55 then
        fizzle_adjustment = 0;
      end

      if (caster_class == Class.PALADIN or
          caster_class == Class.RANGER or
          caster_class == Class.SHADOWKNIGHT) and
          spell_level > 40 then
        fizzle_adjustment = 0;
      end

      if caster_class == Class.BARD and fizzle_adjustment > 15 then
        fizzle_adjustment = 15;
      end

      local prime_stat = 0;
      if caster_class == Class.BARD then
        prime_stat = (client:GetCHA() + client:GetDEX()) / 2;
      elseif caster_class == Class.CLERIC or
          caster_class == Class.PALADIN or
          caster_class == Class.RANGER or
          caster_class == Class.DRUID or
          caster_class == Class.SHAMAN or
          caster_class == Class.BEASTLORD then
        prime_stat = client:GetWIS();
      elseif caster_class == Class.SHADOWKNIGHT or
          caster_class == Class.NECROMANCER or
          caster_class == Class.WIZARD or
          caster_class == Class.MAGICIAN or
          caster_class == Class.ENCHANTER then
        prime_stat = client:GetINT();
      end

      prime_stat_bonus = math.floor(prime_stat / 10);

      local effective_spell_level = spell_level - 1;
      if effective_spell_level > 50 then
        effective_spell_level = 50
      end;

      local spell_casting_skill = client:GetSkill(spell:Skill());

      effective_spell_casting_skill = spell_casting_skill + spell_casting_skill_effects;
      if effective_spell_casting_skill < 0 then
        effective_spell_casting_skill = 0;
      end

      random_penalty = Random.Int(0, 10);

      spell_level_adjustment = 5 * (18 - effective_spell_level);
      local chance = 0 +
          effective_spell_casting_skill +
          (spell_level_adjustment + prime_stat_bonus) -
          random_penalty -
          fizzle_adjustment;
      
      capped_chance = chance;
      if caster_class == Class.BARD then
        if capped_chance < 1 then
          capped_chance = 1;
        elseif capped_chance > 95 then
          capped_chance = 95;
        end
      elseif caster_class <= 16 then
        if capped_chance < 5 then
          capped_chance = 5;
        elseif capped_chance > 95 then
          capped_chance = 95;
        end
      else -- Unknown Class
        capped_chance = 0;
      end
    end

    local specialize_skill = client:GetSpecializeSkillValue(spell:ID());
    local specialize_adjustment = 0;
    local spell_casting_mastery_adjustment = 0;

    if specialize_skill > 0 then
      specialize_adjustment = math.floor(specialize_skill / 10) + 1;

      local spell_casting_mastery_level = client:GetAA(83); -- aaSpellCastingMastery
      if spell_casting_mastery_level == 1 then
        spell_casting_mastery_adjustment = 2;
      elseif spell_casting_mastery_level == 2 then
        spell_casting_mastery_adjustment = 5;
      elseif spell_casting_mastery_level == 3 then
        spell_casting_mastery_adjustment = 10;
      end

      capped_chance = capped_chance + specialize_adjustment + spell_casting_mastery_adjustment;
      if capped_chance > 98 then
        capped_chance = 98;
      end
    end

    local roll_100 = Random.Int(1, 100);

    if client:IsSilenced() then
      roll_100 = capped_chance + 1;
    end

    --client:Message(15, "CheckFizzle(LUA:expansions_combat): " ..
    --    "spell_id = " .. spell:ID() ..
    --    ", roll_100 = " .. roll_100 ..
    --    ", capped_chance (" .. capped_chance .. ") = " ..
    --      "effective_spell_casting_skill (" .. effective_spell_casting_skill .. ") + " ..
    --      "spell_level_adjustment (" .. spell_level_adjustment .. ") + " ..
    --      "prime_stat_bonus (" .. prime_stat_bonus .. ") - " ..
    --      "random_penalty (" .. random_penalty .. ") - " ..
    --      "fizzle_adjustment (" .. fizzle_adjustment .. ") + " ..
    --      "specialize_adjustment (" .. specialize_adjustment .. ") + " ..
    --      "spell_casting_mastery_adjustment (" .. spell_casting_mastery_adjustment .. ")");
    
    if capped_chance >= roll_100 then
      e.ReturnValue = true;
      return e;
    end

    e.ReturnValue = false;
    return e;
  end
end
