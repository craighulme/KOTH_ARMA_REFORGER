[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class KOTH_SpectateAction : SCR_SelectedEntitiesContextAction 
{
	// Determine if the spectate action can be shown
	override bool CanBeShown(
		SCR_EditableEntityComponent hoveredEntity, 
		notnull set<SCR_EditableEntityComponent> selectedEntities, 
		vector cursorWorldPosition, 
		int flags
	) {
		ChimeraWorld gameWorld = GetGame().GetWorld();
		
		if (!hoveredEntity)
			return false;
		
		if (gameWorld.IsGameTimePaused())
			return false;
		
		if (!hoveredEntity.GetOwner())
			return false;
		
		// Check if the hovered entity is a Chimera character
		return SCR_ChimeraCharacter.Cast(hoveredEntity.GetOwner());
	}
	
	// Perform the spectate action on the hovered entity
	override void Perform(
		SCR_EditableEntityComponent hoveredEntity, 
		notnull set<SCR_EditableEntityComponent> selectedEntities, 
		vector cursorWorldPosition, 
		int flags, 
		int param = -1
	) {
		IEntity targetEntity = hoveredEntity.GetOwner();
		if (!targetEntity) {
			return;
		}
		
		// Retrieve the spectator component from the controlled entity
		IEntity gmPlayer = GetGame().GetPlayerController().GetControlledEntity();
		if(!gmPlayer)
			return;
		
		KOTH_1stPersonSpectatorComponent spectatorComp = KOTH_1stPersonSpectatorComponent.Cast(gmPlayer.FindComponent(KOTH_1stPersonSpectatorComponent));
		if (!spectatorComp) {
			return;
		}
		
		// Start spectating the target entity
		spectatorComp.StartSpectating(targetEntity);
	}
}