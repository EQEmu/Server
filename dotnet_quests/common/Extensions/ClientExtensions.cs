
public static class ClientExtensions
{
    public static Expedition CreateExpedition(this Client client, ExpeditionInfo ei)
    {
        DynamicZone dz = new DynamicZone(ei.Instance.ZoneID, ei.Instance.Version, ei.Instance.Duration, DynamicZoneType.Expedition);
        dz.SetName(ei.ExpeditionName);
        dz.SetMinPlayers(ei.MinPlayers);
        dz.SetMinPlayers(ei.MaxPlayers);
        dz.SetCompass(ei.Compass);
        dz.SetSafeReturn(ei.SafeReturn);
        dz.SetZoneInLocation(ei.ZoneIn);

        return client.CreateExpedition(dz);
    }
}

