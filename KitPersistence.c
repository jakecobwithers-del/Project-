class KitClaimData
{
    ref map<string, bool> OneTimeKits;
    ref map<string, float> CooldownKits;

    void KitClaimData()
    {
        OneTimeKits = new map<string, bool>;
        CooldownKits = new map<string, float>;
    }
}

class KitPersistence
{
    private ref map<string, ref KitClaimData> m_PlayerKitData;
    private const string STORAGE_FOLDER = "$profile:KitSystem";
    private const string PLAYER_DATA_FILE = "PlayerKitData.json";

    void KitPersistence()
    {
        m_PlayerKitData = new map<string, ref KitClaimData>;
        LoadData();
    }

    private void LoadData()
    {
        if (!FileExist(STORAGE_FOLDER))
            MakeDirectory(STORAGE_FOLDER);

        string filePath = STORAGE_FOLDER + "/" + PLAYER_DATA_FILE;
        if (FileExist(filePath))
            JsonFileLoader<map<string, ref KitClaimData>>.JsonLoadFile(filePath, m_PlayerKitData);
        else
            SaveData();
    }

    void SaveData()
    {
        string filePath = STORAGE_FOLDER + "/" + PLAYER_DATA_FILE;
        JsonFileLoader<map<string, ref KitClaimData>>.JsonSaveFile(filePath, m_PlayerKitData);
    }

    KitClaimData GetPlayerData(string playerId)
    {
        if (!m_PlayerKitData.Contains(playerId))
            m_PlayerKitData.Set(playerId, new KitClaimData());
        return m_PlayerKitData.Get(playerId);
    }

    void ResetKitClaim(string playerId, string kitName)
    {
        KitClaimData data = GetPlayerData(playerId);
        data.OneTimeKits.Remove(kitName);
        data.CooldownKits.Remove(kitName);
        SaveData();
    }
}
