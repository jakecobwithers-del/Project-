class BuildKitCommand
{
    // Data structures
    class BuildKitItem
    {
        string classname;
        int    quantity;
        float  health = 100.0;       // Item health percentage (0-100)
        bool   attachMagazine = false; // For weapons, auto-attach magazine
    }

    class BuildKitEntry
    {
        string                          permission;
        float                           cooldown_hours;
        bool                            daily_reset = false;  // Reset usage on new day
        ref array<ref BuildKitItem>     items;
    }

    class BuildKitConfig
    {
        ref map<string, ref BuildKitEntry> kits;
        ref array<string>                  vip_ids;
        ref map<string, string>            messages;
        bool                               log_usage = true;  // Log kit usage to server logs
        string                             version = "1.0";
    }

    // Persistence data
    class KitUsageData
    {
        ref map<string, ref array<string>> usedKits;
        ref map<string, ref map<string, float>> cooldowns;
        int lastResetDay = -1;
    }

    // File paths
    static string CONFIG_DIR = "$profile:\\LBmaster";
    static string CONFIG_PATH = CONFIG_DIR + "\\BuildKitConfig.json";
    static string USAGE_PATH = CONFIG_DIR + "\\BuildKitUsage.json";
    
    // Module state
    private ref BuildKitConfig m_Config;
    private ref KitUsageData m_UsageData;
    private int m_CurrentDay = -1;
    void Init()
    {
        if (!m_Config)
        {
            m_Config = LoadConfig();
            m_UsageData = LoadUsageData();
            
            // Setup daily reset check
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CheckDailyReset, 60000, true);
            
            // Get current day for reset tracking
            int year, month, day;
            GetGame().GetWorld().GetDate(year, month, day);
            m_CurrentDay = day;
        }
    }

    BuildKitConfig LoadConfig()
    {
        // Ensure directory exists
        if (!FileExist(CONFIG_DIR))
        {
            MakeDirectory(CONFIG_DIR);
            Print("[Sloppy BuildKits] Created directory: " + CONFIG_DIR);
        }
        
        // Create default config if it doesn't exist
        if (!FileExist(CONFIG_PATH))
            CreateDefaultConfig();
        
        // Load config
        BuildKitConfig config = new BuildKitConfig();
        JsonFileLoader<BuildKitConfig>.JsonLoadFile(CONFIG_PATH, config);
        
        // Validate config
        if (!config || !config.kits || !config.messages)
        {
            Print("[Sloppy BuildKits] Config corrupt or missing required fields, regenerating");
            CreateDefaultConfig();
            JsonFileLoader<BuildKitConfig>.JsonLoadFile(CONFIG_PATH, config);
        }
        
        return config;
    }

    KitUsageData LoadUsageData()
    {
        KitUsageData data = new KitUsageData();
        data.usedKits = new map<string, ref array<string>>();
        data.cooldowns = new map<string, ref map<string, float>>();
        
        if (FileExist(USAGE_PATH))
        {
            JsonFileLoader<KitUsageData>.JsonLoadFile(USAGE_PATH, data);
        }
        
        return data;
    }

    void SaveUsageData()
    {
        JsonFileLoader<KitUsageData>.JsonSaveFile(USAGE_PATH, m_UsageData);
    }
    void CreateDefaultConfig()
    {
        Print("[Sloppy BuildKits] Writing default config to: " + CONFIG_PATH);
        
        FileHandle file = OpenFile(CONFIG_PATH, FileMode.WRITE);
        if (!file) return;
        
        FPrintln(file, "{");
        FPrintln(file, "  \"kits\": {");
        FPrintln(file, "    \"buildkit\": {");
        FPrintln(file, "      \"permission\": \"all\",");
        FPrintln(file, "      \"cooldown_hours\": 0,");
        FPrintln(file, "      \"daily_reset\": true,");
        FPrintln(file, "      \"items\": [");
        FPrintln(file, "        { \"classname\": \"WoodenPlank\", \"quantity\": 50 },");
        FPrintln(file, "        { \"classname\": \"Nail\", \"quantity\": 150 },");
        FPrintln(file, "        { \"classname\": \"Hammer\", \"quantity\": 1 }");
        FPrintln(file, "      ]");
        FPrintln(file, "    },");
        FPrintln(file, "    \"starterkit\": {");
        FPrintln(file, "      \"permission\": \"all\",");
        FPrintln(file, "      \"cooldown_hours\": 0,");
        FPrintln(file, "      \"daily_reset\": false,");
        FPrintln(file, "      \"items\": [");
        FPrintln(file, "        { \"classname\": \"Rag\", \"quantity\": 6 },");
        FPrintln(file, "        { \"classname\": \"Canteen\", \"quantity\": 1, \"health\": 100 },");
        FPrintln(file, "        { \"classname\": \"Knife\", \"quantity\": 1 }");
        FPrintln(file, "      ]");
        FPrintln(file, "    },");
        FPrintln(file, "    \"vipkit\": {");
        FPrintln(file, "      \"permission\": \"vip\",");
        FPrintln(file, "      \"cooldown_hours\": 24,");
        FPrintln(file, "      \"daily_reset\": true,");
        FPrintln(file, "      \"items\": [");
        FPrintln(file, "        { \"classname\": \"M4A1\", \"quantity\": 1 },");
        FPrintln(file, "        { \"classname\": \"Mag_STANAG_30Rnd\", \"quantity\": 3 },");
        FPrintln(file, "        { \"classname\": \"TacticalHelmet_Black\", \"quantity\": 1 }");
        FPrintln(file, "      ]");
        FPrintln(file, "    }");
        FPrintln(file, "  },");
        FPrintln(file, "  \"vip_ids\": [\"76561198000000000\"],");
        FPrintln(file, "  \"messages\": {");
        FPrintln(file, "    \"on_receive\": \"You received your kit!\",");
        FPrintln(file, "    \"on_invalid\": \"Invalid item skipped: \",");
        FPrintln(file, "    \"on_reset\": \"All kit usage has been reset!\",");
        FPrintln(file, "    \"on_no_permission\": \"You do not have permission to claim this kit.\",");
                FPrintln(file, "    \"on_already_claimed\": \"You have already claimed this kit.\",");
        FPrintln(file, "    \"on_cooldown\": \"Kit is on cooldown. Try again in: \",");
        FPrintln(file, "    \"help_message\": \"Available kits: \"");
        FPrintln(file, "  },");
        FPrintln(file, "  \"log_usage\": true,");
        FPrintln(file, "  \"version\": \"1.0\"");
        FPrintln(file, "}");
        
        CloseFile(file);
    }

    void CheckDailyReset()
    {
        int year, month, day;
        GetGame().GetWorld().GetDate(year, month, day);
        
        if (m_CurrentDay != day)
        {
            m_CurrentDay = day;
            
            // Reset any daily kits
            ResetDailyKits();
            
            if (m_Config.log_usage)
                Print("[Sloppy BuildKits] Daily kit reset performed");
        }
    }
    
    void ResetDailyKits()
    {
        // Check all kit definitions for daily_reset flag
        foreach (string kitName, BuildKitEntry entry : m_Config.kits)
        {
            if (entry.daily_reset)
            {
                // Clear this kit's usage data
                m_UsageData.usedKits.Remove(kitName);
                m_UsageData.cooldowns.Remove(kitName);
            }
        }
        
        // Save updated usage data
        SaveUsageData();
    }
    // Command handler - called when player uses a kit command
    bool HandleCommand(PlayerBase player, TStringArray args)
    {
        if (!player || !args || args.Count() < 1)
            return false;
            
        Init(); // Ensure we're initialized
        
        string command = args.Trim().ToLower();
        
        // Handle sub-commands
        if (command == "help" || command == "")
        {
            SendKitHelpMessage(player);
            return true;
        }
        else if (command == "reset" && IsAdminPlayer(player))
        {
            ResetAllKitUsage(player);
            return true;
        }
        else if (command == "reload" && IsAdminPlayer(player))
        {
            ReloadConfig(player);
            return true;
        }
        
        // Handle kit claim
        return ClaimKit(player, command);
    }
    
    private bool ClaimKit(PlayerBase player, string kitName)
    {
        string playerId = player.GetIdentity().GetPlainId();
        
        // Check if kit exists
        if (!m_Config.kits.Contains(kitName))
        {
            NotifyPlayer(player, "Kit not found. Use !kit help to see available kits.");
            return false;
        }
        
        BuildKitEntry kitEntry = m_Config.kits.Get(kitName);
        
        // Check permissions
        if (kitEntry.permission == "vip" && !HasVIPPermission(playerId))
        {
            NotifyPlayer(player, m_Config.messages.Get("on_no_permission"));
            return false;
        }
        
        // Check if already claimed (for non-cooldown kits)
        if (kitEntry.cooldown_hours <= 0 && HasPlayerClaimedKit(playerId, kitName))
        {
            NotifyPlayer(player, m_Config.messages.Get("on_already_claimed"));
            return false;
        }
        
        // Check cooldown
        float remainingCooldown = GetRemainingCooldown(playerId, kitName);
        if (remainingCooldown > 0)
        {
            int hours = Math.Floor(remainingCooldown);
            int minutes = Math.Floor((remainingCooldown - hours) * 60);
            
            string cooldownMsg = m_Config.messages.Get("on_cooldown") + hours.ToString() + "h " + minutes.ToString() + "m";
            NotifyPlayer(player, cooldownMsg);
            return false;
        }
        
        // All checks passed, give kit
        if (GiveKitToPlayer(player, kitEntry))
        {
            // Update usage tracking
            TrackKitUsage(playerId, kitName, kitEntry.cooldown_hours);
            
            NotifyPlayer(player, m_Config.messages.Get("on_receive"));
            
            if (m_Config.log_usage)
                Print("[Sloppy BuildKits] Player " + playerId + " claimed kit: " + kitName);
                
            return true;
        }
        
        return false;
    }
    private bool GiveKitToPlayer(PlayerBase player, BuildKitEntry kitEntry)
    {
        if (!player || !kitEntry || !kitEntry.items)
            return false;
            
        bool anyItemGiven = false;
        
        foreach (BuildKitItem kitItem : kitEntry.items)
        {
            if (!kitItem.classname || kitItem.classname == "")
                continue;
                
            int quantity = Math.Max(1, kitItem.quantity);
            
            // Create the item in player inventory or at their feet
            EntityAI itemEnt;
            ItemBase itemBase;
            
            for (int i = 0; i < quantity; i++)
            {
                // Try to create in inventory
                itemEnt = player.GetInventory().CreateInInventory(kitItem.classname);
                
                // If couldn't create in inventory, spawn at feet
                if (!itemEnt)
                {
                    vector position = player.GetPosition();
                    itemEnt = EntityAI.Cast(GetGame().CreateObject(kitItem.classname, position, false, true));
                }
                
                // Skip if failed to create
                if (!itemEnt)
                {
                    NotifyPlayer(player, m_Config.messages.Get("on_invalid") + kitItem.classname);
                    continue;
                }
                
                // Set health percentage if specified
                itemBase = ItemBase.Cast(itemEnt);
                if (itemBase && kitItem.health > 0 && kitItem.health <= 100)
                {
                    float maxHealth = itemBase.GetMaxHealth();
                    float targetHealth = (maxHealth * kitItem.health) / 100;
                    itemBase.SetHealth(targetHealth);
                }
                
                anyItemGiven = true;
            }
        }
        
        return anyItemGiven;
    }
    
    private void TrackKitUsage(string playerId, string kitName, float cooldownHours)
    {
        // Track that player has used this kit
        if (!m_UsageData.usedKits.Contains(kitName))
        {
            m_UsageData.usedKits.Insert(kitName, new array<string>);
        }
        
        array<string> usedPlayers = m_UsageData.usedKits.Get(kitName);
        if (usedPlayers.Find(playerId) == -1)
        {
            usedPlayers.Insert(playerId);
        }
        
        // Set cooldown if applicable
        if (cooldownHours > 0)
        {
            if (!m_UsageData.cooldowns.Contains(kitName))
            {
                m_UsageData.cooldowns.Insert(kitName, new map<string, float>);
            }
            
            map<string, float> cooldowns = m_UsageData.cooldowns.Get(kitName);
            cooldowns.Set(playerId, GetGame().GetTime() + (cooldownHours * 3600 * 1000));
        }
        
        SaveUsageData();
    }
    
    private bool HasPlayerClaimedKit(string playerId, string kitName)
    {
        if (!m_UsageData.usedKits.Contains(kitName))
            return false;
            
        array<string> usedPlayers = m_UsageData.usedKits.Get(kitName);
        return (usedPlayers.Find(playerId) != -1);
    }
    
    private float GetRemainingCooldown(string playerId, string kitName)
    {
        if (!m_UsageData.cooldowns.Contains(kitName))
            return 0;
            
        map<string, float> cooldowns = m_UsageData.cooldowns.Get(kitName);
        if (!cooldowns.Contains(playerId))
            return 0;
            
            float cooldownEnd = cooldowns.Get(playerId);
        float currentTime = GetGame().GetTime();
        
        if (currentTime >= cooldownEnd)
            return 0;
            
        // Return remaining hours
        return (cooldownEnd - currentTime) / (3600 * 1000);
    }
    
    private void ResetAllKitUsage(PlayerBase player)
    {
        m_UsageData = new BuildKitUsageData();
        SaveUsageData();
        
        if (player)
        {
            NotifyPlayer(player, m_Config.messages.Get("on_reset"));
        }
        
        if (m_Config.log_usage)
            Print("[Sloppy BuildKits] All kit usage data has been reset");
    }
    
    private void SendKitHelpMessage(PlayerBase player)
    {
        if (!player)
            return;
            
        string helpMsg = m_Config.messages.Get("help_message");
        string playerId = player.GetIdentity().GetPlainId();
        
        TStringArray availableKits = new TStringArray;
        
        // List only kits the player has permission for
        foreach (string kitName, BuildKitEntry entry : m_Config.kits)
        {
            if (entry.permission == "all" || (entry.permission == "vip" && HasVIPPermission(playerId)))
            {
                string kitInfo = kitName;
                
                // Add cooldown info if applicable
                if (entry.cooldown_hours > 0)
                {
                    float remainingTime = GetRemainingCooldown(playerId, kitName);
                    if (remainingTime > 0)
                    {
                        int hours = Math.Floor(remainingTime);
                        int minutes = Math.Floor((remainingTime - hours) * 60);
                        kitInfo += " (Cooldown: " + hours.ToString() + "h " + minutes.ToString() + "m)";
                    }
                    else
                    {
                        kitInfo += " (Cooldown: " + entry.cooldown_hours.ToString() + "h)";
                    }
                }
                
                availableKits.Insert(kitInfo);
            }
        }
        
        // Add admin commands if player is admin
        if (IsAdminPlayer(player))
        {
            availableKits.Insert("Admin Commands: !kit reset, !kit reload");
        }
        
        // Send the message
        helpMsg += " " + string.Join(availableKits, ", ");
        NotifyPlayer(player, helpMsg);
    }
    
    private bool HasVIPPermission(string playerId)
    {
        return m_Config.vip_ids.Find(playerId) != -1;
    }
    
    private bool IsAdminPlayer(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return false;
            
        string playerId = player.GetIdentity().GetPlainId();
        
        // Consider server admins and those in the admin list as admins
        return GetGame().IsAdmin(playerId) || m_Config.vip_ids.Find(playerId) != -1;
    }
