class KOTH_AssistSystemComponent
{
    protected ref map<int, ref array<int>> m_assistRelationships;
    const float ASSIST_DURATION = 300;
    protected ref map<string, float> m_assistExpiryTimes;

    protected PlayerManager m_playerManager;
    protected KOTH_BackendApiGameModeComponent m_kothBackendApi;
    protected KOTH_SessionDataGameModeComponent m_sessionDataGameComp;

    void OnPostInit(IEntity owner)
    {
        if (SCR_Global.IsEditMode(owner))
            return;

        m_assistRelationships = new map<int, ref array<int>>();
        m_assistExpiryTimes = new map<string, float>();

        m_playerManager = GetGame().GetPlayerManager();
        m_kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
        m_sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_SessionDataGameModeComponent));

        GetGame().GetCallqueue().CallLater(CleanupExpiredAssists, 10000, true);
    }

    void RewardAssistant(int assistantID, int xpAmount, int moneyAmount) 
    {
        string assistantUID = KOTH_Helper.GetPlayerUID(assistantID);
        if (!m_kothBackendApi || !m_kothBackendApi.m_CurrentProfileList.Contains(assistantUID))
            return;

        KOTH_PlayerProfileJson profile = m_kothBackendApi.m_CurrentProfileList.Get(assistantUID);
        if (!profile)
            return;

        profile.AddXp(xpAmount);
        profile.AddMoney(moneyAmount);
        profile.AddAssist();

        m_kothBackendApi.DoRpcSyncProfileToPlayer(profile);
        m_sessionDataGameComp.AddSessionXpAndMoney(xpAmount, moneyAmount, assistantUID);
    }

    void AddAssistRelationship(int assistedID, int assistantID)
    {
        if (assistedID == assistantID)
            return;

        // Use int keys for m_assistRelationships
        if (!m_assistRelationships.Contains(assistantID))
            m_assistRelationships.Set(assistantID, new array<int>());

        array<int> assistedList = m_assistRelationships.Get(assistantID);

        if (assistedList.Find(assistedID) == -1)
        {
            assistedList.Insert(assistedID);
            m_assistRelationships.Set(assistantID, assistedList);
        }

        // Convert assistantID to string for m_assistExpiryTimes
        string assistantKey = assistantID.ToString();
        float expiryTime = GetGame().GetWorld().GetWorldTime() + ASSIST_DURATION;
        m_assistExpiryTimes.Set(assistantKey, expiryTime);  
    }

    void CleanupExpiredAssists()
    {
        float currentTime = GetGame().GetWorld().GetWorldTime();
        array<string> expiredAssistants = {}; // Store string keys

        // Collect expired assistants
        foreach (string assistantKey, float expiryTime : m_assistExpiryTimes)
        {
            if (currentTime > expiryTime)
                expiredAssistants.Insert(assistantKey);
        }

        // Remove expired relationships
        foreach (string assistantKey : expiredAssistants)
        {
            int assistantID = assistantKey.ToInt();

            if (m_assistRelationships.Contains(assistantID))
                m_assistRelationships.Remove(assistantID);

            m_assistExpiryTimes.Remove(assistantKey);
        }
    }

    void RemoveAssistRelationship(int assistedID, int assistantID)
    {
        if (m_assistRelationships.Contains(assistantID))
        {
            array<int> assistedList = m_assistRelationships.Get(assistantID);
            int index = assistedList.Find(assistedID);
            if (index != -1)
            {
                assistedList.Remove(index);
                m_assistRelationships.Set(assistantID, assistedList);
            }
        }
        
        // Convert assistantID to string for removal
        string assistantKey = assistantID.ToString();
        if (m_assistExpiryTimes.Contains(assistantKey))
            m_assistExpiryTimes.Remove(assistantKey);
    }

    array<int> GetAssistants(int playerID)
    {
        if (!m_assistRelationships.Contains(playerID))
            return new array<int>();

        return m_assistRelationships.Get(playerID);
    }

    void ClearAllAssists(int playerID)
    {
        if (m_assistRelationships.Contains(playerID))
            m_assistRelationships.Remove(playerID);

        // Convert playerID to string before checking in m_assistExpiryTimes
        string playerKey = playerID.ToString();
        if (m_assistExpiryTimes.Contains(playerKey))
            m_assistExpiryTimes.Remove(playerKey);
    }


}

