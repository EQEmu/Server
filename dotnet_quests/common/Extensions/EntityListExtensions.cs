public static class EntityListExtensions
{
    public static NPC? GetNPCByName(this EntityList entityList, string name)
    {
        foreach (var npc in entityList.GetNPCList())
        {
            if (npc.Value.GetOrigName() == name)
            {
                return npc.Value;
            }
        }
        return null;
    }
}
