class KOTH_PlayerAuditSystem
{
    const int MAX_TIME = 20;

    // map<playerId, timeJoined>
    private ref map<int, int> m_playersToCheck = new map<int, int>();

    void KOTH_PlayerAuditSystem()
    {
        #ifdef WORKBENCH
        #else
            GetGame().GetCallqueue().CallLater(CheckPlayers, 5000);
        #endif
    }

    void CheckPlayers()
    {
        foreach (int playerID, int time : m_playersToCheck)
        {
            if (System.GetUnixTime() - time >= MAX_TIME)
            {
				string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);
				string playerUID = KOTH_Helper.GetPlayerUID(playerID);
				
                if (playerUID)
                {
                    RemovePlayer(playerID);
                    Log("KOTH_PlayerAuditSystem remove playerName: "+playerName+" playerUID: "+playerUID);
                }
                else 
				{
                    Log("KOTH_PlayerAuditSystem banned playerName: "+playerName+" playerUID: "+playerUID);
                    GetGame().GetPlayerManager().KickPlayer(playerID, PlayerManagerKickReason.BAN);
                }
            }
        }
    }

    void AddPlayer(int playerID)
    {
        m_playersToCheck.Insert(playerID, System.GetUnixTime());
    }

    void RemovePlayer(int playerID)
    {
        m_playersToCheck.Remove(playerID);
    }
}