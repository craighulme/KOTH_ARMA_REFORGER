sealed class KOTH_PostFrameSystem : GameSystem
{
	protected ref map<int, vector> m_previousPosition = new map<int, vector>();
	protected PlayerManager m_playerManager;
	
	protected override void OnInit()
	{
		m_playerManager = GetGame().GetPlayerManager();
	}
	
	override void OnUpdate(ESystemPoint point)
	{
		array<int> playerIds = {};
		m_playerManager.GetPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{
			IEntity entity = m_playerManager.GetPlayerControlledEntity(playerId);
			if (!entity)
			{
				m_previousPosition.Remove(playerId);
				continue;
			}
			
			ChimeraCharacter chimera = ChimeraCharacter.Cast(entity);
			if (!chimera)
			{
				m_previousPosition.Remove(playerId);
				continue;
			}
			
			CharacterControllerComponent charControl = chimera.GetCharacterController();
			if (!charControl || charControl.IsDead() || chimera.IsInVehicle())
			{
				m_previousPosition.Remove(playerId);
				continue;
			}
			
			vector currentPos = entity.GetOrigin();
			vector previousPos = m_previousPosition.Get(playerId);
			
			if (previousPos && vector.Distance(previousPos, currentPos) > 5)
				Log("player name: "+m_playerManager.GetPlayerName(playerId)+" playerUID: "+KOTH_Helper.GetPlayerUID(playerId)+" teleported from "+previousPos+" to "+currentPos);
			
			m_previousPosition.Set(playerId, currentPos);
		}
	}
}