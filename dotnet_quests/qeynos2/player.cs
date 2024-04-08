class Player : IPlayerEvent {
    public void Say(PlayerEvent e) {
       // e.globals.logSys.QuestDebug($"{e.player.GetName()} said: {e.data}");
    }
}