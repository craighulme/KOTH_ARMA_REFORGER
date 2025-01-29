class KOTH_BonusCodeRestCallback : RestCallback 
{
	override void OnError(int errorCode)
	{
		Log("KOTH_BonusCodeRestCallback Error with code "+errorCode+" = "+typename.EnumToString(ERestResult, errorCode), LogLevel.ERROR);
	}

	override void OnTimeout()
	{
		Log("KOTH_BonusCodeRestCallback timeout", LogLevel.ERROR);
	}

	override void OnSuccess(string data, int dataSize)
	{
		Log("KOTH_BonusCodeRestCallback success data= "+data);
		KOTH_BonusCodeResponseJson profile = new KOTH_BonusCodeResponseJson();
		profile.ExpandFromRAW(data);
		
		int playerId = KOTH_BackendApiGameModeComponent.FindPlayerIdFromBohemiaUID(profile.playerUID);
		if (!playerId)
			return;

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return;
		
		KOTH_ExperienceManager m_expManager = KOTH_ExperienceManager.GetInstance();
		KOTH_PlayerBonusCode bonusCode = new KOTH_PlayerBonusCode();
		bonusCode.name = profile.name;
		bonusCode.dateEnd = profile.dateEnd;
		if (profile.multiplier)
			bonusCode.multiplier = profile.multiplier.ToFloat();

		bonusCode.playerUID = profile.playerUID;

		KOTH_SCR_PlayerProfileComponent playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
		if (profile.error) {
			playerProfileComp.DoRpc_ShowErrorBonusCode(profile.errorReason);
		} else {
			m_expManager.AddKothBonus(profile.playerUID, bonusCode);
			playerProfileComp.DoRpc_ShowSuccessBonusCode(profile);
		}
	}
}
