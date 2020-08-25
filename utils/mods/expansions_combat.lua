-- Source Function: Client::CheckFizzle()
function CheckFizzle(e)
  if eq.is_lost_dungeons_of_norrath_enabled() then
    e.IgnoreDefault = false;
    return e;

  else -- Classic Fizzle -- Based on p2002eq/wfh's Fizzle code
    e.IgnoreDefault = true;
    local client = e.self;
    local spell_id = e.spell_id;
    local spell = e.spell;

    if client:GetGM() == true then -- GMs never fizzle
      e.ReturnValue = (true);
      return e;
    end
    
    local no_fizzle_level = 0;
    -- TODO: Calculate no fizzle level
    if (spell:Classes(client:GetClass() - 1) < no_fizzle_level) then
      e.ReturnValue = true;
      return e;
    end

    -- Calculate Spell Difficulty - This is Capped at 255
    -- Calculates minimum level this spell is available - ensures similar casting difficulty for all classes
    local minimum_level = 255;
    for i=0, 16 do
      local class_spell_level = spell:Classes(i);
      if class_spell_level < minimum_level then
        minimum_level = class_spell_level;
      end
    end

    local spell_difficulty = minimum_level * 5;
    if spell_difficulty > 255 then
      spell_difficulty = 255;
    end

    -- Calculate Effective Casting Skill with Bonuses
    local bonus_casting_level = 0 -- TODO: Get bonus casting level
    local caster_skill = client:GetRawSkill(spell:Skill()) + bonus_casting_level * 5;
    if caster_skill > 255 then
      caster_skill = 255;
    end

    -- Calculates Effective Specialization Skill Value
    -- TODO

    -- Calculates Effective Casting Stat Value
    -- TODO

    -- Get Spell-Specific Fizzle Chance
    -- TODO

    -- Calculate Final Fizzle Chance
    local fizzle_chance = spell_difficulty - caster_skill -- implement spell_fizzle_adjustment and prime_stat_reduction

    if fizzle_chance > 95.0 then
      fizzle_chance = 95.0;
    elseif fizzle_chance < 2.0 then
      fizzle_chance = 2.0;
    end

    local fizzle_roll = Random.Real(0, 100);

    if fizzle_roll > fizzle_chance then
      e.ReturnValue = true;
      return e;
    else
      e.ReturnValue = false;
      return e;
    end
  end
end
