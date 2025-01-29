class KOTH_CheckBanRestCallback : RestCallback 
{
	override void OnError(int errorCode)
	{
		Log("KOTH_CheckBanRestCallback Error with code "+errorCode, LogLevel.ERROR);
	};

	override void OnTimeout()
	{
		Log("KOTH_CheckBanRestCallback timeout", LogLevel.ERROR);
	};

	override void OnSuccess(string data, int dataSize)
	{
		Log("KOTH_CheckBanRestCallback success size= " + dataSize.ToString());
		
		KOTH_ListPlayerBanJson bans = new KOTH_ListPlayerBanJson();
		bans.ExpandFromRAW(data);
		
		array<int> allPlayers = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		playerManager.GetPlayers(allPlayers);

		foreach (int playerId : allPlayers)
		{
			// its local info so can stupid spam 
			// we dont use helper since we might ask for identity of players who are not audited yet
			string uid = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
			#ifdef WORKBENCH
			uid = KOTH_Helper.GetPlayerUID(playerId);
			#endif
			
			if (uid == string.Empty)
				continue;

			if (bans.m_list.Contains(uid))
			{
				Log("banned "+uid);
				playerManager.KickPlayer(playerId, PlayerManagerKickReason.BAN);
			}
		}
	}
}
