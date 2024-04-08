class Item : IItemEvent {
    public void ItemClick(ItemEvent e) {
       // e.globals.logSys.QuestDebug($"Item clicked: {e.item.GetItem().Name}");
    }
}