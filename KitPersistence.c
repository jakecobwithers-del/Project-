class KitPersistence
{
    private const string STORAGE_PATH = "$profile:KitSystem/PlayerKitData.json";
    private ref map<string, ref map<string, float>> m_KitData;

    void KitPersistence()
    {
        m_KitData = new map<string, ref map<string, float>>;
        LoadData();
        EnsureDefaultSections();
    }

    private void LoadData()
    {
        if (!FileExist("$profile:KitSystem"))
            MakeDirectory("$profile:KitSystem");

        if (FileExist(STORAGE_PATH))
        {
            JsonFileLoader<map<string, ref map<string, float>>>.JsonLoadFile(STORAGE_PATH, m_KitData);
        }
        else
        {
            SaveData();
        }
    }

    private void SaveData()
    {
        JsonFileLoader<map<string, ref map<string, float>>>.JsonSaveFile(STORAGE_PATH, m_KitData);
    }

    private void EnsureDefaultSections()
    {
        array<string> defaultKits = {"kit", "builder", "daily", "vip"};
        foreach (string kitName : defaultKits)
        {
            if (!m_KitData.Contains(kitName))
                m_KitData.Set(kitName, new map<string, float>);
        }
        SaveData();
    }

    private void EnsureKit(string kitName)
    {
        if (!m_KitData.Contains(kitName))
            m_KitData.Set(kitName, new map<string, float>);
    }

    void MarkClaimed(string playerId, string kitName)
    {
        EnsureKit(kitName);
        m_KitData.Get(kitName).Set(playerId, 1);
        SaveData();
    }

    void MarkCooldown(string playerId, string kitName, float nextAvailableTime)
    {
        EnsureKit(kitName);
        m_KitData.Get(kitName).Set(playerId, nextAvailableTime);
        SaveData();
    }

    bool HasClaimed(string playerId, string kitName)
    {
        if (!m_KitData.Contains(kitName)) return false;
        if (!m_KitData.Get(kitName).Contains(playerId)) return false;
        return m_KitData.Get(kitName).Get(playerId) == 1;
    }

    bool CanClaimCooldown(string playerId, string kitName)
    {
        if (!m_KitData.Contains(kitName)) return true;
        if (!m_KitData.Get(kitName).Contains(playerId)) return true;

        float storedTime = m_KitData.Get(kitName).Get(playerId);
        return GetGame().GetTime() >= storedTime;
    }

    void ResetPlayerKit(string playerId, string kitName)
    {
        EnsureKit(kitName);
        m_KitData.Get(kitName).Remove(playerId);
        SaveData();
    }

    void ResetKit(string kitName)
    {
        EnsureKit(kitName);
        m_KitData.Get(kitName).Clear();
        SaveData();
    }

    void ResetAll()
    {
        m_KitData.Clear();
        EnsureDefaultSections();
        SaveData();
    }
}
