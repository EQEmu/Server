
public class ExpeditionInfo
{
    public string ExpeditionName { get; set; }
    public uint MinPlayers { get; set; }
    public uint MaxPlayers { get; set; }
    public InstanceInfo Instance { get; set; }
    public DynamicZoneLocation Compass { get; set; }
    public DynamicZoneLocation SafeReturn { get; set; }
    public DynamicZoneLocation ZoneIn { get; set; }

    public class InstanceInfo
    {
        public uint ZoneID { get; set; }
        public uint Version { get; set; }
        public uint Duration { get; set; }
    }
}
