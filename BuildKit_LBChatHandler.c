#ifdef LBChatHandler

modded class LBChatHandler
{
    ref BuildKitCommand m_BuildKit;

    override void OnInit()
    {
        super.OnInit();

        // DEBUG: confirm this runs in your server log
        Print(">>>> Sloppy BuildKits LBChatHandler.OnInit() called");

        m_BuildKit = new BuildKitCommand();
        m_BuildKit.Init();

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM)
            .CallLater(RegisterKitCommands, 1000, false);
    }

    void RegisterKitCommands()
    {
        Print(">>>> Sloppy BuildKits: registering commands");
        ChatCommandManager.RegisterCommand(new ChatCommand("/kit",        "Claim build kit",       m_BuildKit.Execute));
        ChatCommandManager.RegisterCommand(new ChatCommand("/starterkit", "Claim starter kit",     m_BuildKit.Execute));
        ChatCommandManager.RegisterCommand(new ChatCommand("/vipkit",     "Claim VIP kit",         m_BuildKit.Execute));
        ChatCommandManager.RegisterCommand(new ChatCommand("/reloadkits", "Reload Sloppy config",  m_BuildKit.Execute));
        ChatCommandManager.RegisterCommand(new ChatCommand("/resetkits",  "Reset all kit usage",   m_BuildKit.Execute));
    }

    override bool OnIncomingServerChat(PlayerIdentity sender, string text)
    {
        if (text.StartsWith("/"))
            return true;
        return super.OnIncomingServerChat(sender, text);
    }
}


class BuildKitCommand
{
    class BuildKitItem
    {
        string classname;
        int    quantity;
    }

    class BuildKitEntry
    {
        string                          permission;
        float                           cooldown_hours;
        ref array<ref BuildKitItem>     items;
    }

    class BuildKitConfig
    {
        ref map<string, ref BuildKitEntry> kits;
        ref array<string>                  vip_ids;
        ref map<string, string>            messages;
    }

    static ref BuildKitConfig                      cfg;
    static ref map<string, ref array<string>>      usedKits     = new map<string, ref array<string>>();
    static ref map<string, ref map<string, float>> kitCooldowns = new map<string, ref map<string, float>>();

    // Now writing into serverprofile/LBmaster
    static string configPath = "$profile:\\LBmaster\\BuildKitConfig.json";
    static string usagePath  = "$profile:\\LBmaster\\BuildKitUsage.json";
    static string cdPath     = "$profile:\\LBmaster\\BuildKitCooldowns.json";

    void Init()
    {
        if (!cfg)
        {
            cfg = LoadConfig();
            LoadUsage();
        }
    }

    BuildKitConfig LoadConfig()
    {
        string dir = "$profile:\\LBmaster";
        if (!FileExist(dir))
        {
            MakeDirectory(dir);
            Print(">>>> Sloppy BuildKits: created directory " + dir);
        }

        if (!FileExist(configPath))
            CreateDefaultConfig();

        BuildKitConfig newCfg = new BuildKitConfig();
        JsonFileLoader<BuildKitConfig>.JsonLoadFile(configPath, newCfg);

        if (!newCfg || !newCfg.kits)
        {
            Print(">>>> Sloppy BuildKits: config corrupt, regenerating");
            CreateDefaultConfig();
            JsonFileLoader<BuildKitConfig>.JsonLoadFile(configPath, newCfg);
        }

        return newCfg;
    }

    void CreateDefaultConfig()
    {
        Print(">>>> Sloppy BuildKits: writing default config to " + configPath);
        FileHandle file = OpenFile(configPath, FileMode.WRITE);
        if (!file) return;

        FPrintln(file, "{");
        FPrintln(file, "  \"kits\": {");
        FPrintln(file, "    \"buildkit\": { \"permission\": \"all\", \"cooldown_hours\": 0, \"items\": [");
        FPrintln(file, "      { \"classname\": \"WoodenPlank\",\"quantity\": 50 },");
        FPrintln(file, "      { \"classname\": \"Nail\",\"quantity\": 150 },");
        FPrintln(file, "      { \"classname\": \"Hammer\",\"quantity\": 1 }");
        FPrintln(file, "    ] },");
        FPrintln(file, "    \"starterkit\": { \"permission\": \"all\",\"cooldown_hours\": 0,\"items\": [");
        FPrintln(file, "      { \"classname\": \"Rag\",\"quantity\": 6 },");
        FPrintln(file, "      { \"classname\": \"Canteen\",\"quantity\": 1 },");
        FPrintln(file, "      { \"classname\": \"Knife\",\"quantity\": 1 }");
        FPrintln(file, "    ] },");
        FPrintln(file, "    \"vipkit\": { \"permission\": \"vip\",\"cooldown_hours\": 24,\"items\": [");
        FPrintln(file, "      { \"classname\": \"M4A1\",\"quantity\": 1 },");
        FPrintln(file, "      { \"classname\": \"Mag_STANAG_30Rnd\",\"quantity\": 3 },");
        FPrintln(file, "      { \"classname\": \"TacticalHelmet_Black\",\"quantity\": 1 }");
        FPrintln(file, "    ] }");
        FPrintln(file, "  },");
        FPrintln(file, "  \"vip_ids\": [\"76561198000000000\"],");
        FPrintln(file, "  \"messages\": {");
        FPrintln(file, "    \"on_receive\":         \"You received your kit!\",");
        FPrintln(file, "    \"on_invalid\":         \"Invalid item skipped: \",");
        FPrintln(file, "    \"on_reset\":           \"All kit usage has been reset!\",");
        FPrintln(file, "    \"on_no_permission\":   \"You do not have permission to claim this kit.\",");
        FPrintln(file, "    \"on_already_claimed\": \"You have already claimed this kit.\",");
        FPrintln(file, "    \"on_cooldown\":        \"You must wait before claiming this kit again.\"");
        FPrintln(file, "  }");
        FPrintln(file, "}");
        CloseFile(file);
    }

    void SaveUsage()
    {
        JsonFileLoader<map<string, ref array<string>>>.JsonSaveFile(usagePath, usedKits);
        JsonFileLoader<map<string, ref map<string, float>>>.JsonSaveFile(cdPath, kitCooldowns);
    }

    void LoadUsage()
    {
        if (FileExist(usagePath))
            JsonFileLoader<map<string, ref array<string>>>.JsonLoadFile(usagePath, usedKits);

        if (FileExist(cdPath))
            JsonFileLoader<map<string, ref map<string, float>>>.JsonLoadFile(cdPath, kitCooldowns);
    }

    bool HasPermission(PlayerBase player, string perm)
    {
        if (perm == "all") return true;

        string uid = player.GetIdentity().GetPlainId();
        if (perm == "vip"   && cfg.vip_ids && cfg.vip_ids.Find(uid) != -1) return true;
        if (perm == "admin" && player.IsAdmin())                     return true;

        return false;
    }

    bool IsOnCooldown(PlayerBase player, string kitName, float hours)
    {
        if (hours <= 0) return false;

        string uid = player.GetIdentity().GetPlainId();
        if (!kitCooldowns.Contains(uid)) return false;

        float lastUse = kitCooldowns.Get(uid).Get(kitName);
        return (GetGame().GetTime() - lastUse) < (hours * 3600000);
    }

    void Execute(PlayerBase player, string cmd)
    {
        Init();

        if (cmd == "reloadkits")
        {
            cfg = LoadConfig();
            player.MessageStatus("Sloppy config reloaded.");
            return;
        }

        if (cmd == "resetkits")
        {
            usedKits.Clear();
            kitCooldowns.Clear();
            SaveUsage();
            player.MessageStatus(cfg.messages.Get("on_reset"));
            return;
        }

        if (!cfg.kits || !cfg.kits.Contains(cmd))
        {
            player.MessageStatus("[Sloppy] Unknown kit: " + cmd);
            return;
        }

        BuildKitEntry entry = cfg.kits.Get(cmd);

        if (!HasPermission(player, entry.permission))
        {
            player.MessageStatus(cfg.messages.Get("on_no_permission"));
            return;
        }

        string uid = player.GetIdentity().GetPlainId();

        if (IsOnCooldown(player, cmd, entry.cooldown_hours))
        {
            player.MessageStatus(cfg.messages.Get("on_cooldown"));
            return;
        }

        if (!usedKits.Contains(uid))
            usedKits.Set(uid, new array<string>());

        array<string> used = usedKits.Get(uid);
        if (used.Find(cmd) != -1)
        {
            player.MessageStatus(cfg.messages.Get("on_already_claimed"));
            return;
        }

        foreach (BuildKitItem it : entry.items)
        {
            if (!GetGame().ConfigIsExisting("CfgVehicles " + it.classname))
            {
                player.MessageStatus(cfg.messages.Get("on_invalid") + it.classname);
                continue;
            }

            for (int i = 0; i < it.quantity; i++)
            {
                InventoryLocation loc = new InventoryLocation();
                if (player.GetInventory().FindFreeLocationFor(it.classname, FindInventoryLocationType.ANY, loc))
                    player.GetInventory().CreateInInventory(it.classname);
                else
                    GetGame().CreateObjectEx(it.classname, player.GetPosition(), ECE_PLACE_ON_SURFACE);
            }
        }

        used.Insert(cmd);
        if (!kitCooldowns.Contains(uid))
            kitCooldowns.Set(uid, new map<string, float>());

        kitCooldowns.Get(uid).Set(cmd, GetGame().GetTime());
        SaveUsage();

        player.MessageStatus(cfg.messages.Get("on_receive"));
    }
}

#endif
