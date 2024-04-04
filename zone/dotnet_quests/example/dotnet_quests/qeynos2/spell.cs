class Spell : ISpellEvent {
    public void SpellEffectClient (SpellEvent e) {
      //  e.globals.logSys.QuestDebug($"Spell ID fired: {e.spellID}");
    }

    public void SpellEffectBuffTicClient(SpellEvent e) {
      //  e.globals.logSys.QuestDebug($"Spell ID tic fired: {e.spellID}");
    }
}