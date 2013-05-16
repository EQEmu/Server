//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
//
//

using System;
using System.Collections.Generic;

namespace EQExtractor2.InternalTypes
{
    class Constants
    {
        public const UInt32 MATERIAL_HEAD = 0;
        public const UInt32 MATERIAL_CHEST = 1;
        public const UInt32 MATERIAL_PRIMARY = 7;
        public const UInt32 MATERIAL_SECONDARY = 8;
    }

    class GroundSpawnStruct
    {
        public UInt32 DropID;
        public string Name;
        public UInt16 ZoneID;
        public UInt16 InstanceID;
        public float Heading;
        public float y;
        public float x;
        public float z;
        public UInt32 ObjectType;
    }

    public class NewZoneStruct
    {
        public NewZoneStruct()
        {
            FogRed = new byte[4];
            FogGreen = new byte[4];
            FogBlue = new byte[4];
            FogMinClip = new float[4];
            FogMaxClip = new float[4];
        }

        public string ShortName;
        public string LongName;
        public byte Type;
        public byte[] FogRed;
        public byte[] FogGreen;
        public byte[] FogBlue;
        public float[] FogMinClip;
        public float[] FogMaxClip;
        public float Gravity;
        public byte TimeType;
        public byte Sky;
        public float ZEM;
        public float SafeY;
        public float SafeX;
        public float SafeZ;
        public float MinZ;
        public float MaxZ;
        public float UnderWorld;
        public float MinClip;
        public float MaxClip;
        public string ShortName2;
        public UInt16 ZoneID;
        public UInt16 InstanceID;
        public byte FallDamage;
        public float FogDensity;
    }

    public class ZoneEntryStruct
    {
        public ZoneEntryStruct()
        {
            SlotColour = new UInt32[9];
            Equipment = new UInt32[9];
        }

        public string SpawnName;
        public UInt32 SpawnID;
        public bool Findable;
        public Byte Level;
        public Byte IsNPC;
        public uint Showname;
        public uint TargetableWithHotkey;
        public uint Targetable;
        public uint ShowHelm;
        public uint Gender;
        public byte OtherData;
        public string DestructableString1;
        public string DestructableString2;
        public string DestructableString3;
        public UInt32 DestructableUnk1;
        public UInt32 DestructableUnk2;
        public UInt32 DestructableID1;
        public UInt32 DestructableID2;
        public UInt32 DestructableID3;
        public UInt32 DestructableID4;
        public UInt32 DestructableUnk3;
        public UInt32 DestructableUnk4;
        public UInt32 DestructableUnk5;
        public UInt32 DestructableUnk6;
        public UInt32 DestructableUnk7;
        public UInt32 DestructableUnk8;
        public UInt32 DestructableUnk9;
        public byte DestructableByte;
        public float Size;
        public byte Face;
        public float WalkSpeed;
        public float RunSpeed;
        public UInt32 Race;
        public byte PropCount;
        public UInt32 BodyType;
        public byte HairColor;
        public byte BeardColor;
        public byte EyeColor1;
        public byte EyeColor2;
        public byte HairStyle;
        public byte Beard;
        public UInt32 DrakkinHeritage;
        public UInt32 DrakkinTattoo;
        public UInt32 DrakkinDetails;
        public UInt32 Deity;
        public byte Class;
        public byte EquipChest2;
        public byte Helm;
        public string LastName;
        public UInt32 PetOwnerID;
        public float YPos;
        public float Heading;
        public float XPos;
        public float ZPos;
        public UInt32[] SlotColour;
        public byte ArmorTintRed;
        public byte ArmorTintGreen;
        public byte ArmorTintBlue;
        public UInt32 MeleeTexture1;
        public UInt32 MeleeTexture2;
        public UInt32[] Equipment;
        public string Title;
        public string Suffix;
        public byte IsMercenary;
        public UInt32 Padding5;
        public UInt32 Padding7;
        public UInt32 Padding26;

    }

    public class NPCSpawn
    {
        public NPCSpawn(UInt32 SpawnID, UInt32 Spawn2DBID, UInt32 NPCTypeID, string Name)
        {
            this.SpawnID = SpawnID;
            this.Spawn2DBID = Spawn2DBID;
            this.NPCTypeID = NPCTypeID;
            this.Name = Name;
        }

        public bool DoesHaveHighResWaypoints()
        {
            return HasHighResWaypoints;
        }

        public void SetHasHighResWaypoints()
        {
            HasHighResWaypoints = true;
        }

        public void AddWaypoint(Position p, bool HighRes)
        {
            Waypoints.Add(p);
            if (HighRes)
                HasHighResWaypoints = true;
        }

        public UInt32 SpawnID;
        public UInt32 Spawn2DBID;
        public UInt32 NPCTypeID;
        public string Name;
        bool HasHighResWaypoints = false;
        public List<Position> Waypoints = new List<Position>();
    }

    public class NPCTypeList
    {
        List<NPCType> _NPCTypeList = new List<NPCType>();

        public void AddNPCType(NPCType NewNPC)
        {
            _NPCTypeList.Add(NewNPC);
        }

        public List<NPCType> GetUniqueSpawns()
        {
            foreach (NPCType n in _NPCTypeList)
            {
                if (NPCType.IsPlayableRace(n.Race) || NPCType.IsGuardRace(n.Race))
                    n.Unique = true;
                else
                    n.Unique = false;
            }

            for (int i = 0; i < _NPCTypeList.Count; ++i)
            {
                for (int j = 0; j < _NPCTypeList.Count; ++j)
                {
                    if ((i != j) && (_NPCTypeList[i].Name == _NPCTypeList[j].Name))
                    {
                        _NPCTypeList[i].Unique = false;
                    }
                }
            }

            List<NPCType> Results = new List<NPCType>();

            foreach (NPCType n in _NPCTypeList)
                if (n.Unique)
                    Results.Add(n);

            return Results;
        }


        public UInt32 FindNPCType(string Name, Byte Level, uint Gender, float Size, Byte Face, float WalkSpeed, float RunSpeed, UInt32 Race,
                       UInt32 BodyType, Byte HairColor, Byte BeardColor, Byte EyeColor1, Byte EyeColor2, Byte HairStyle, Byte Beard,
                       UInt32 DrakkinHeritage, UInt32 DrakkinTattoo, UInt32 DrakkinDetails, UInt32 Deity, Byte Class, Byte EquipChest2,
                       Byte Helm, string LastName)
        {
            foreach (NPCType n in _NPCTypeList)
            {
                if (n.Name != Name)
                    continue;

                if (n.Level != Level)
                    continue;

                if (n.Gender != Gender)
                    continue;

                if (n.Size != Size)
                    continue;

                if (n.Face != Face)
                    continue;

                if (n.WalkSpeed != WalkSpeed)
                    continue;

                if (n.RunSpeed != RunSpeed)
                    continue;

                if (n.Race != Race)
                    continue;

                if (n.BodyType != BodyType)
                    continue;

                if (n.HairColor != HairColor)
                    continue;

                if (n.BeardColor != BeardColor)
                    continue;

                if (n.EyeColor1 != EyeColor1)
                    continue;

                if (n.EyeColor2 != EyeColor2)
                    continue;

                if (n.HairStyle != HairStyle)
                    continue;

                if (n.Beard != Beard)
                    continue;

                if (n.DrakkinHeritage != DrakkinHeritage)
                    continue;

                if (n.DrakkinTattoo != DrakkinTattoo)
                    continue;

                if (n.DrakkinDetails != DrakkinDetails)
                    continue;

                if (n.Deity != Deity)
                    continue;

                if (n.Class != Class)
                    continue;

                if (n.EquipChest2 != EquipChest2)
                    continue;

                if (n.Helm != Helm)
                    continue;

                if (n.LastName != LastName)
                    continue;

                //Console.WriteLine("Found NPCType already in list, returning {0}", n.DBID);

                return n.DBID;

            }

            return 0;
        }
    }

    public class NPCSpawnList
    {
        public List<NPCSpawn> _NPCSpawnList = new List<NPCSpawn>();

        public NPCSpawn GetNPC(UInt32 SpawnID)
        {
            foreach (NPCSpawn s in _NPCSpawnList)
            {
                if (s.SpawnID == SpawnID)
                    return s;

            }

            return null;
        }

        public bool DoesHaveHighResWaypoints(UInt32 SpawnID)
        {
            foreach (NPCSpawn s in _NPCSpawnList)
            {
                if (s.SpawnID == SpawnID)
                {
                    if (s.DoesHaveHighResWaypoints())
                        return true;
                    else
                        return false;
                }
            }
            return false;
        }

        public void AddNPCSpawn(UInt32 DBID, UInt32 SpawnID, UInt32 NPCTypeID, string Name)
        {
            NPCSpawn NewSpawn = new NPCSpawn(DBID, SpawnID, NPCTypeID, Name);

            _NPCSpawnList.Add(NewSpawn);
        }


        public void AddWaypoint(UInt32 SpawnID, Position p, bool HighRes)
        {
            foreach (NPCSpawn n in _NPCSpawnList)
            {
                if (n.SpawnID == SpawnID)
                {
                    n.AddWaypoint(p, HighRes);
                    return;
                }
            }
        }
    }

    public struct ZonePoint
    {

        public ZonePoint(UInt32 Number, UInt16 ZoneID, UInt16 Instance, float x, float y, float z,
                         float TargetX, float TargetY, float TargetZ, float Heading, UInt32 TargetZoneID)
        {
            this.Number = Number;
            this.ZoneID = ZoneID;
            this.Instance = Instance;
            this.x = x;
            this.y = y;
            this.z = z;
            this.TargetX = TargetX;
            this.TargetY = TargetY;
            this.TargetZ = TargetZ;
            this.Heading = Heading;
            this.TargetZoneID = TargetZoneID;

        }

        public UInt32 Number;
        public UInt16 ZoneID;
        public UInt16 Instance;
        public float x;
        public float y;
        public float z;
        public float TargetX;
        public float TargetY;
        public float TargetZ;
        public float Heading;
        public UInt32 TargetZoneID;
    };

    struct Item
    {
        public UInt32 StackSize;
        public UInt32 Slot;
        public UInt32 MerchantSlot;
        public UInt32 Price;
        public Int32 Quantity;
        public string Name;
        public string Lore;
        public string IDFile;
        public UInt32 ID;
    };

    struct MerchantItem
    {
        public MerchantItem(UInt32 ItemID, string Name, UInt32 Slot, int Quantity)
        {
            this.ItemID = ItemID;
            this.Name = Name;
            this.Slot = Slot;
            this.Quantity = Quantity;
        }

        public UInt32 ItemID;
        public string Name;
        public UInt32 Slot;
        public int Quantity;
    }

    struct Merchant
    {
        public Merchant(UInt32 SpawnID)
        {
            this.SpawnID = SpawnID;
            this.Items = new List<MerchantItem>();

        }

        public UInt32 SpawnID;
        public List<MerchantItem> Items;
    }

    public class NPCType
    {
        public NPCType(UInt32 DBID, string Name, Byte Level, uint Gender, float Size, Byte Face, float WalkSpeed, float RunSpeed, UInt32 Race,
                       UInt32 BodyType, Byte HairColor, Byte BeardColor, Byte EyeColor1, Byte EyeColor2, Byte HairStyle, Byte Beard,
                       UInt32 DrakkinHeritage, UInt32 DrakkinTattoo, UInt32 DrakkinDetails, UInt32 Deity, Byte Class, Byte EquipChest2,
                       Byte Helm, string LastName, bool Findable, UInt32 MeleeTexture1, UInt32 MeleeTexture2,
                       UInt32 ArmorTintRed, UInt32 ArmorTintGreen, UInt32 ArmorTintBlue, UInt32[] SlotColour)
        {
            this.DBID = DBID;
            this.Name = Name;
            this.Level = Level;
            this.Gender = Gender;
            this.Size = Size;
            this.Face = Face;
            this.WalkSpeed = WalkSpeed;
            this.RunSpeed = RunSpeed;
            this.Race = Race;
            this.BodyType = BodyType;
            this.HairColor = HairColor;
            this.BeardColor = BeardColor;
            this.EyeColor1 = EyeColor1;
            this.EyeColor2 = EyeColor2;
            this.HairStyle = HairStyle;
            this.Beard = Beard;
            this.DrakkinHeritage = DrakkinHeritage;
            this.DrakkinTattoo = DrakkinTattoo;
            this.DrakkinDetails = DrakkinDetails;
            this.Deity = Deity;
            this.Class = Class;
            this.EquipChest2 = EquipChest2;
            this.Helm = Helm;
            this.LastName = LastName;
            this.Findable = Findable;
            this.MeleeTexture1 = MeleeTexture1;
            this.MeleeTexture2 = MeleeTexture2;
            this.ArmorTintRed = ArmorTintRed;
            this.ArmorTintGreen = ArmorTintGreen;
            this.ArmorTintBlue = ArmorTintBlue;
            this.SlotColour = SlotColour;
            this.Unique = true;


        }
        public static bool IsPlayableRace(UInt32 Race)
        {
            if (Race <= 12 || Race == 128 || Race == 130 || Race == 330 || Race == 522)
                return true;

            return false;
        }

        static UInt32[] GuardRaces = { 44, 67, 71, 77, 78, 81, 90, 92, 93, 94, 106, 112, 139, 183, 239 };

        public static bool IsGuardRace(UInt32 Race)
        {
            if (Array.IndexOf(GuardRaces, Race) >= 0)
                return true;

            return false;
        }

        public static bool IsMount(string Name)
        {
            if (Name.IndexOf("`s_Mount") >= 0)
                return true;

            return false;
        }

        public UInt32 DBID;
        public string Name = "";
        public Byte Level = 0;
        public uint Gender = 0;
        public float Size = 0;
        public Byte Face = 0;
        public float WalkSpeed = 0;
        public float RunSpeed = 0;
        public UInt32 Race = 0;
        public UInt32 BodyType = 0;
        public Byte HairColor = 0;
        public Byte BeardColor = 0;
        public Byte EyeColor1 = 0;
        public Byte EyeColor2 = 0;
        public Byte HairStyle = 0;
        public Byte Beard = 0;
        public UInt32 DrakkinHeritage = 0;
        public UInt32 DrakkinTattoo = 0;
        public UInt32 DrakkinDetails = 0;
        public UInt32 Deity = 0;
        public Byte Class = 0;
        public Byte EquipChest2 = 0;
        public Byte Helm = 0;
        public string LastName = "";
        public bool Findable = false;
        public UInt32 MeleeTexture1 = 0;
        public UInt32 MeleeTexture2 = 0;
        public UInt32 ArmorTintRed = 0;
        public UInt32 ArmorTintGreen = 0;
        public UInt32 ArmorTintBlue = 0;
        public UInt32[] SlotColour;

        public bool Unique = true;
    }

    public class Position
    {
        public Position()
        {
        }

        public Position(float x, float y, float z, float heading, DateTime TimeStamp)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.heading = heading;
            this.TimeStamp = TimeStamp;
        }

        public double Distance(Position p)
        {
            float XDiff = this.x - p.x;
            float YDiff = this.y - p.y;

            return Math.Sqrt((XDiff * XDiff) + (YDiff * YDiff));
        }

        public float x, y, z, heading;
        public DateTime TimeStamp;
    }

    public class PositionUpdate
    {
        public PositionUpdate()
        {
            p = new Position();
        }

        public UInt32 SpawnID;

        public Position p;

        public bool HighRes;
    }

    public class ExplorerSpawnRecord
    {
        public ExplorerSpawnRecord(UInt32 ID, string Name)
        {
            this.ID = ID;
            this.Name = Name;
        }

        public UInt32 ID;
        public string Name;
    }

    public class Door
    {
        public Door(string Name, float YPos, float XPos, float ZPos, float Heading, UInt32 Incline, Int32 Size, Byte ID, Byte OpenType,
                         Byte StateAtSpawn, Byte InvertState, Int32 DoorParam, string DestZone, float DestX, float DestY, float DestZ,
                         float DestHeading)
        {
            this.Name = Name;
            this.YPos = YPos;
            this.XPos = XPos;
            this.ZPos = ZPos;
            this.Heading = Heading;
            this.Incline = Incline;
            this.Size = Size;
            this.ID = ID;
            this.OpenType = OpenType;
            this.StateAtSpawn = StateAtSpawn;
            this.InvertState = InvertState;
            this.DoorParam = DoorParam;
            this.DestZone = DestZone;
            this.DestX = DestX;
            this.DestY = DestY;
            this.DestZ = DestZ;
            this.DestHeading = DestHeading;
        }

        public string Name;
        public float YPos;
        public float XPos;
        public float ZPos;
        public float Heading;
        public UInt32 Incline;
        public Int32 Size;
        public Byte ID;
        public Byte OpenType;
        public Byte StateAtSpawn;
        public Byte InvertState;
        public Int32 DoorParam;
        public string DestZone;
        public float DestX;
        public float DestY;
        public float DestZ;
        public float DestHeading;

    }
    class MerchantManager
    {
        public bool AddMerchant(UInt32 SpawnID)
        {
            foreach (Merchant m in MerchantList)
            {
                if (m.SpawnID == SpawnID)
                    return false;
            }

            Merchant nm = new Merchant(SpawnID);

            MerchantList.Add(nm);

            return true;
        }

        public int FindMerchant(UInt32 SpawnID)
        {
            for (int i = 0; i < MerchantList.Count; ++i)
            {
                if (MerchantList[i].SpawnID == SpawnID)
                    return i;
            }

            return -1;
        }

        public bool AddMerchantItem(UInt32 SpawnID, UInt32 ItemID, string Name, UInt32 Slot, int Quantity)
        {

            int i = FindMerchant(SpawnID);


            if (i < 0)
                return false;

            for (int j = 0; j < MerchantList[i].Items.Count; ++j)
                if (MerchantList[i].Items[j].ItemID == ItemID)
                    return false;

            MerchantItem NewItem = new MerchantItem(ItemID, Name, Slot, Quantity);

            MerchantList[i].Items.Add(NewItem);

            return true;

        }


        public List<Merchant> MerchantList = new List<Merchant>();

    }

}
