#ifndef LB_DISABLE_CHAT
modded class LBChatHandler
{
    override bool ChatCommandExists(string cmd)
    {
        if (cmd == "buildkit" || cmd == "starterkit" || cmd == "survivalkit" || cmd == "vipkit" || cmd == "resetkits" || cmd == "reloadkits")
            return true;

        return super.ChatCommandExists(cmd);
    }

    override void ExecuteChatCommand(PlayerBase player, string cmd, string param)
    {
        if (cmd == "buildkit" || cmd == "starterkit" || cmd == "survivalkit" || cmd == "vipkit" || cmd == "resetkits" || cmd == "reloadkits")
        {
            BuildKitCommand.Execute(player, cmd);
            return;
        }

        super.ExecuteChatCommand(player, cmd, param);
    }
}
#endif

class BuildKitItem
{
    string classname;
    int quantity;
}

class BuildKitEntry
{
    string permission;
    float cooldown_hours;
    ref array<ref BuildKitItem> items;
}

class BuildKitConfig
{
    ref map<string, ref BuildKitEntry> kits;
    ref array<string> vip_ids;
    ref map<string, string> messages;
}

class BuildKitCommand
{
    static ref map<string, ref array<string>> usedKits = new map<string, ref array<string>>;
    static ref map<string, ref map<string, float>> kitCooldowns = new map<string, ref map<string, float>>;
    static BuildKitConfig cfg;

    static void Init()
    {
        cfg = LoadConfig();
        LoadUsage();
    }

    static BuildKitConfig LoadConfig()
    {
        string path = "$profile:\\BuildKitConfig.json";
        if (!FileExist(path))
        {
            FileHandle file = OpenFile(path, FileMode.WRITE);
            if (file)
            {
                FPrintln(file, "{");
                FPrintln(file, "  \"kits\": {");
                FPrintln(file, "    \"buildkit\": { \"permission\": \"all\", \"cooldown_hours\": 0, \"items\": [ { \"classname\": \"WoodenPlank\", \"quantity\": 50 }, { \"classname\": \"Nail\", \"quantity\": 150 }, { \"classname\": \"Hammer\", \"quantity\": 1 } ] },");
                FPrintln(file, "    \"starterkit\": { \"permission\": \"all\", \"cooldown_hours\": 0, \"items\": [ { \"classname\": \"Rag\", \"quantity\": 6 }, { \"classname\": \"Canteen\", \"quantity\": 1 }, { \"classname\": \"Knife\", \"quantity\": 1 } ] },");
                FPrintln(file, "    \"vipkit\": { \"permission\": \"vip\", \"cooldown_hours\": 24, \"items\": [ { \"classname\": \"M4A1\", \"quantity\": 1 }, { \"classname\": \"Mag_STANAG_30Rnd\", \"quantity\": 3 }, { \"classname\": \"TacticalHelmet_Black\", \"quantity\": 1 } ] },");
                FPrintln(file, "    \"builderkit\": { \"permission\": \"group:Builders\", \"cooldown_hours\": 12, \"items\": [ { \"classname\": \"WoodenLog\", \"quantity\": 10 }, { \"classname\": \"Hatchet\", \"quantity\": 1 } ] }");
                FPrintln(file, "  },");
                FPrintln(file, "  \"vip_ids\": [\"76561198000000000\"],");
                FPrintln(file, "  \"messages\": {");
                FPrintln(file, "    \"on_receive\": \"You received your kit!\",");
                FPrintln(file, "    \"on_invalid\": \"Invalid item skipped: \",");
                FPrintln(file, "    \"on_reset\": \"All kit usage has been reset!\",");
                FPrintln(file, "    \"on_no_permission\": \"You do not have permission to claim this kit.\",");
                FPrintln(file, "    \"on_already_claimed\": \"You have already claimed this kit.\",");
                FPrintln(file, "    \"on_cooldown\": \"You must wait before claiming this kit again.\"");
                FPrintln(file, "  }");
                FPrintln(file, "}");
                CloseFile(file);
            }
        }

        BuildKitConfig cfg;
        JsonFileLoader<BuildKitConfig>.JsonLoadFile(path, cfg);
        return cfg;
    }

    static void SaveUsage()
    {
        JsonFileLoader<map<string, ref array<string>>>.JsonSaveFile("$profile:\\BuildKitUsage.json", usedKits);
        JsonFileLoader<map<string, ref map<string, float>>>.JsonSaveFile("$profile:\\BuildKitCooldowns.json", kitCooldowns);
    }

    static void LoadUsage()
    {
        if (FileExist("$profile:\\BuildKitUsage.json"))
            JsonFileLoader<map<string, ref array<string>>>.JsonLoadFile("$profile:\\BuildKitUsage.json", usedKits);

        if (FileExist("$profile:\\BuildKitCooldowns.json"))
            JsonFileLoader<map<string, ref map<string, float>>>.JsonLoadFile("$profile:\\BuildKitCooldowns.json", kitCooldowns);
    }

    static bool HasPermission(PlayerBase player, string permission)
    {
        if (permission == "all") return true;

        string uid = player.GetIdentity().GetPlainId();
        if (permission == "vip" && cfg.vip_ids.Find(uid) != -1)
            return true;

        if (permission == "admin" && player.IsAdmin())
            return true;

        if (permission.StartsWith("group:"))
        {
            string groupName = permission.Substring(6, permission.Length() - 6);
            #ifdef ADVANCED_GROUPS
            AGroupsManager groups = AGroupsManager.GetInstance();
            if (groups)
            {
                AGroup group = groups.GetGroupByMember(player);
                if (group && group.GetName() == groupName)
                    return true;
            }
            #endif
        }

        #ifdef ADVANCED_GROUPS
        AGroupsManager groups = AGroupsManager.GetInstance();
        if (groups)
        {
            AGroup group = groups.GetGroupByMember(player);
            if (group && group.GetName().Contains("VIP"))
                return true;
        }
        #endif

        return false;
    }

    static bool IsOnCooldown(PlayerBase player, string kitName, float hours)
    {
        if (hours <= 0) return false;

        string uid = player.GetIdentity().GetPlainId();
        if (!kitCooldowns.Contains(uid)) return false;

        float lastUse = kitCooldowns.Get(uid).Get(kitName);
        return (GetGame().GetTime() - lastUse) < (hours * 3600000);
    }

    static void Execute(PlayerBase player, string cmd)
    {
        if (!cfg) Init();

        if (cmd == "reloadkits")
        {
            cfg = LoadConfig();
            player.MessageStatus("BuildKit config reloaded.");
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

        if (!cfg.kits.Contains(cmd))
        {
            player.MessageStatus("[BuildKit] No kit found for command: " + cmd);
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
            usedKits.Set(uid, new array<string>);

        array<string> used = usedKits.Get(uid);
        if (used.Find(cmd) != -1)
        {
            player.MessageStatus(cfg.messages.Get("on_already_claimed"));
            return;
        }

        foreach (BuildKitItem i : entry.items)
        {
            if (!GetGame().IsKindOf(i.classname, "InventoryItem"))
            {
                player.MessageStatus(cfg.messages.Get("on_invalid") + i.classname);
                continue;
            }

            for (int x = 0; x < i.quantity; x++)
            {
                EntityAI item = player.GetInventory().CreateInInventory(i.classname);
                if (!item)
                {
                    vector pos = player.GetPosition();
                    GetGame().CreateObject(i.classname, pos, false, true);
                }
            }
        }

        used.Insert(cmd);

        if (!kitCooldowns.Contains(uid))
            kitCooldowns.Set(uid, new map<string, float>);

        kitCooldowns.Get(uid).Set(cmd, GetGame().GetTime());
        SaveUsage();

        player.MessageStatus(cfg.messages.Get("on_receive"));
    }
}
