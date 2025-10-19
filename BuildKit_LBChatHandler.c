#ifndef LB_DISABLE_CHAT

modded class LBChatHandler
{
    private ref KitManager m_KitManager;
    
    override void LBChatHandler()
    {
        if (!m_KitManager && GetGame().IsServer())
        {
            m_KitManager = new KitManager();
            m_KitManager.LoadConfig();
        }
    }
    
    override bool ChatCommandExists(string cmd)
    {
        if (super.ChatCommandExists(cmd))
            return true;

        // Our kit commands
        return (cmd == "kit" || cmd == "buildkit" || cmd == "vip" || cmd == "daily" || cmd == "kits" || cmd == "kithelp" || cmd == "resetkit");
    }

    override bool HasPermission(PlayerIdentity sender, string cmd)
    {
        if (super.HasPermission(sender, cmd))
            return true;

        // Basic kit commands available to all players
        if (cmd == "kit" || cmd == "buildkit" || cmd == "daily" || cmd == "kits" || cmd == "kithelp")
            return true;

        // VIP kit requires permission check
        if (cmd == "vip")
            return m_KitManager.HasPermission(sender, "vip");

        // Reset kit requires admin permission
        if (cmd == "resetkit")
            return m_KitManager.HasPermission(sender, "admin");

        return false;
    }

    override void OnChatCommand(PlayerIdentity sender, string cmd, TStringArray args)
    {
        super.OnChatCommand(sender, cmd, args);

        if (!GetGame().IsServer())
            return;

        PlayerBase player = MissionServer.GetPlayerFromId(sender.GetId());
        if (!player)
        {
            Print("[KitSystem] Failed to find PlayerBase for identity: " + sender.GetName());
            return;
        }

        if (cmd == "kit")
        {
            m_KitManager.GiveKit(player, "starter");
        }
        else if (cmd == "buildkit")
        {
            m_KitManager.GiveKit(player, "build");
        }
        else if (cmd == "vip")
        {
            m_KitManager.GiveKit(player, "vip");
        }
        else if (cmd == "daily")
        {
            m_KitManager.GiveKit(player, "daily");
        }
        else if (cmd == "kits")
        {
            m_KitManager.ShowAvailableKits(player);
        }
        else if (cmd == "kithelp")
        {
            m_KitManager.ShowKitHelp(player);
        }
        else if (cmd == "resetkit")
        {
            if (args.Count() >= 2)
            {
                string targetPlayerId = args[0];
                string kitName = args[1];
                m_KitManager.ResetKitClaim(player, targetPlayerId, kitName);
            }
            else
            {
                SendMessageToPlayer(player, "Usage: /resetkit <playerId> <kitName>");
            }
        }
    }
}

#endif