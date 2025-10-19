class KitManager
{
    private ref map<string, ref Kit> m_Kits;
    private ref KitPersistence m_Persistence;
    private ref array<string> m_AdminUsers;

    void KitManager()
    {
        m_Kits = new map<string, ref Kit>;
        m_Persistence = new KitPersistence();
        m_AdminUsers = new array<string>;
    }

    bool LoadConfig()
    {
        string configPath = "$profile:KitConfig.json";
        if (!FileExist(configPath))
        {
            Print("[KitSystem] Missing config, creating default.");
            CreateDefaultConfig(configPath);
        }
        JsonFileLoader<map<string, ref Kit>>.JsonLoadFile(configPath, m_Kits);
        return true;
    }

    void CreateDefaultConfig(string path)
    {
        Kit starter = new Kit("starter");
        starter.Items.Insert(new KitItem("Canteen", 1));
        starter.Items.Insert(new KitItem("TacticalBaconCan", 2));
        starter.Items.Insert(new KitItem("BandageDressing", 5));
        starter.OneTimeOnly = true;
        m_Kits.Set("starter", starter);

        Kit build = new Kit("build");
        build.Items.Insert(new KitItem("Hammer", 1));
        build.Items.Insert(new KitItem("Nail", 50));
        build.Items.Insert(new KitItem("WoodenPlank", 10));
        build.OneTimeOnly = true;
        m_Kits.Set("build", build);

        Kit vip = new Kit("vip");
        vip.Items.Insert(new KitItem("M4A1", 1));
        vip.Items.Insert(new KitItem("Mag_STANAG_30Rnd", 3));
        vip.OneTimeOnly = false;
        vip.RequiredPermission = "vip";
        m_Kits.Set("vip", vip);

        Kit daily = new Kit("daily");
        daily.Items.Insert(new KitItem("Apple", 2));
        daily.Items.Insert(new KitItem("SodaCan_Cola", 1));
        daily.Items.Insert(new KitItem("BandageDressing", 1));
        daily.CooldownHours = 24;
        m_Kits.Set("daily", daily);

        JsonFileLoader<map<string, ref Kit>>.JsonSaveFile(path, m_Kits);
    }

    void GiveKit(PlayerBase player, string kitName)
    {
        if (!m_Kits.Contains(kitName))
        {
            SendMessageToPlayer(player, "[KitSystem] Kit not found: " + kitName);
            return;
        }

        Kit kit = m_Kits.Get(kitName);
        string playerId = player.GetIdentity().GetId();
        KitClaimData data = m_Persistence.GetPlayerData(playerId);
        float currentTime = GetGame().GetTime() / 1000 / 60 / 60; // hours since server start

        // One-time kit check
        if (kit.OneTimeOnly && data.OneTimeKits.Contains(kitName))
        {
            SendMessageToPlayer(player, "[KitSystem] You already claimed this kit.");
            return;
        }

        // Cooldown kit check
        if (kit.CooldownHours > 0)
        {
            if (data.CooldownKits.Contains(kitName))
            {
                float lastClaim = data.CooldownKits.Get(kitName);
                float hoursSince = currentTime - lastClaim;
                if (hoursSince < kit.CooldownHours)
                {
                    float remaining = kit.CooldownHours - hoursSince;
                    SendMessageToPlayer(player, "[KitSystem] You must wait " + Math.Round(remaining, 2).ToString() + " more hours to claim this kit again.");
                    return;
                }
            }
            data.CooldownKits.Set(kitName, currentTime);
        }

        foreach (KitItem item : kit.Items)
        {
            for (int i = 0; i < item.Quantity; i++)
                player.GetInventory().CreateInInventory(item.ClassName);
        }

        if (kit.OneTimeOnly)
            data.OneTimeKits.Set(kitName, true);

        m_Persistence.SaveData();
        SendMessageToPlayer(player, "[KitSystem] You received the " + kitName + " kit!");
    }

    void ShowAvailableKits(PlayerBase player)
    {
        SendMessageToPlayer(player, "[KitSystem] Available kits:");
        foreach (string name, Kit kit : m_Kits)
            SendMessageToPlayer(player, " - " + name);
    }

    void ShowKitHelp(PlayerBase player)
    {
        SendMessageToPlayer(player, "Use /kit, /buildkit, /vip, /daily to claim kits.");
    }

    void ResetKitClaim(PlayerBase admin, string targetId, string kitName)
    {
        m_Persistence.ResetKitClaim(targetId, kitName);
        SendMessageToPlayer(admin, "[KitSystem] Reset kit '" + kitName + "' for player " + targetId);
    }

    bool HasPermission(PlayerIdentity sender, string permission)
    {
        if (permission == "admin")
            return m_AdminUsers.Find(sender.GetPlainId()) > -1;
        return true;
    }

    void SendMessageToPlayer(PlayerBase player, string msg)
    {
        Param1<string> data = new Param1<string>(msg);
        GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, data, true, player.GetIdentity());
    }
}
