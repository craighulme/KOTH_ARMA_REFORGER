modded class SCR_FlushToilet
{
	// properties and methods here
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		if (!Replication.IsServer())
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		if (playerId == 0)
			return;

		KOTH_PlayerEventsGameModeComponent playerEventComp = KOTH_PlayerEventsGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_PlayerEventsGameModeComponent)); 
		playerEventComp.HandleFlushToilet(playerId);
	}
}
