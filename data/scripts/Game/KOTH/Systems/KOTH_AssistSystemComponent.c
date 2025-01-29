class KOTH_AssistSystemComponentClass : SCR_BaseGameModeComponentClass {}

class KOTH_AssistSystemComponent : SCR_BaseGameModeComponent
{
    // Map to store assist relationships: Key = Assisted Player UID, Value = Array of Assisting Player UIDs
    protected ref map<string, ref array<string>> m_assistRelationships;
    
    // Duration of assist relationship in seconds (5 minutes)
    const float ASSIST_DURATION = 300;
    
    // Store assist expiry times: Key = Assisted Player UID + Assistant UID, Value = Expiry Time
    protected ref map<string, float> m_assistExpiryTimes;
    
    protected PlayerManager m_playerManager;
    protected KOTH_BackendApiGameModeComponent m_kothBackendApi;
    
    override void OnPostInit(IEntity owner)
    {
        if (SCR_Global.IsEditMode(owner))
            return;
            
        m_assistRelationships = new map<string, ref array<string>>();
        m_assistExpiryTimes = new map<string, float>();
        
        m_playerManager = GetGame().GetPlayerManager();
        m_kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
        
        // Start periodic cleanup of expired assists
        GetGame().GetCallqueue().CallLater(CleanupExpiredAssists, 10000, true);
    }
    
    // Add an assist relationship
    void AddAssistRelationship(string assistedUID, string assistantUID)
    {
        if (assistedUID == assistantUID)
            return;
            
        // Get or create array of assistants for this player
        if (!m_assistRelationships.Contains(assistedUID))
        {
            m_assistRelationships.Set(assistedUID, new array<string>());
        }
        
        array<string> assistants = m_assistRelationships.Get(assistedUID);
        
        // Check if relationship already exists
        if (assistants.Find(assistantUID) == -1)
        {
            assistants.Insert(assistantUID);
            
            // Set expiry time
            string relationshipKey = GetRelationshipKey(assistedUID, assistantUID);
            float expiryTime = GetGame().GetWorld().GetWorldTime() + ASSIST_DURATION;
            m_assistExpiryTimes.Set(relationshipKey, expiryTime);
            
            LogWorkbench(string.Format("Added assist relationship: %1 assisting %2", assistantUID, assistedUID));
        }
    }
    
    // Remove a specific assist relationship
    void RemoveAssistRelationship(string assistedUID, string assistantUID)
    {
        if (m_assistRelationships.Contains(assistedUID))
        {
            array<string> assistants = m_assistRelationships.Get(assistedUID);
            int index = assistants.Find(assistantUID);
            if (index != -1)
            {
                assistants.Remove(index);
                string relationshipKey = GetRelationshipKey(assistedUID, assistantUID);
                m_assistExpiryTimes.Remove(relationshipKey);
                
                LogWorkbench(string.Format("Removed assist relationship: %1 no longer assisting %2", assistantUID, assistedUID));
            }
        }
    }
    
    // Award XP/Money to all assisting players
    void HandleAssistRewards(string assistedUID, int xpAmount, int moneyAmount)
    {
        if (!m_assistRelationships.Contains(assistedUID))
            return;
            
        array<string> assistants = m_assistRelationships.Get(assistedUID);
        float currentTime = GetGame().GetWorld().GetWorldTime();
        
        foreach (string assistantUID : assistants)
        {
            string relationshipKey = GetRelationshipKey(assistedUID, assistantUID);
            
            // Check if relationship hasn't expired
            if (currentTime <= m_assistExpiryTimes.Get(relationshipKey))
            {
                // Get assistant's profile and award rewards
                KOTH_PlayerProfileJson assistantProfile = m_kothBackendApi.m_CurrentProfileList.Get(assistantUID);
                if (assistantProfile)
                {
                    assistantProfile.AddXp(xpAmount);
                    assistantProfile.AddMoney(moneyAmount);
                    m_kothBackendApi.DoRpcSyncProfileToPlayer(assistantProfile);
                    
                    // Notify the assistant
                    int playerId = KOTH_Helper.GetPlayerID(assistantUID);
                    PlayerController playerController = m_playerManager.GetPlayerController(playerId);
                    if (playerController)
                    {
                        KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
                        if (profileComp)
                        {
                            profileComp.DoRpc_Notif_AssistReward(xpAmount, moneyAmount);
                        }
                    }
                }
            }
        }
    }
    
    // Cleanup expired assist relationships
    protected void CleanupExpiredAssists()
    {
        float currentTime = GetGame().GetWorld().GetWorldTime();
        
        array<string> expiredKeys = {};
        
        // Find all expired relationships
        foreach (string relationshipKey, float expiryTime : m_assistExpiryTimes)
        {
            if (currentTime > expiryTime)
            {
                expiredKeys.Insert(relationshipKey);
            }
        }
        
        // Remove expired relationships
        foreach (string key : expiredKeys)
        {
            array<string> parts = SplitRelationshipKey(key);
            if (parts.Count() == 2)
            {
                RemoveAssistRelationship(parts[0], parts[1]);
            }
        }
    }
    
    // Helper method to create a unique key for storing expiry times
    protected string GetRelationshipKey(string assistedUID, string assistantUID)
    {
        return string.Format("%1_%2", assistedUID, assistantUID);
    }
    
    // Helper method to split relationship key back into UIDs
    protected array<string> SplitRelationshipKey(string key)
    {
        return key.Split("_");
    }
}