modded class SCR_PlayerRestrictionZoneWarningComponent
{
	private SCR_BaseGameMode m_gameMode;
	private Widget m_zoneWarningWdg;
	
	void ~SCR_PlayerRestrictionZoneWarningComponent(IEntity owner)
	{
		if (m_zoneWarningWdg)
			m_zoneWarningWdg.RemoveFromHierarchy();
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		#ifdef WORKBENCH
			GetGame().GetCallqueue().CallLater(CheckShouldShowSpawnCampWarning, 100, true);
			m_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		#else
			if (Replication.IsClient())
			{
				GetGame().GetCallqueue().CallLater(CheckShouldShowSpawnCampWarning, 100, true);
				m_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			}
		#endif
	}

	void CheckShouldShowSpawnCampWarning()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity)
		{
			HideWarning(controlledEntity);
			return;
		}

		Faction playerFaction = SCR_FactionManager.Cast(GetGame().GetFactionManager()).GetPlayerFaction(playerController.GetPlayerId());
		if (!playerFaction)
		{
			HideWarning(controlledEntity);
			return;
		}
		
		if (!SCR_AIDamageHandling.IsAlive(controlledEntity))
		{
			HideWarning(controlledEntity);
			return;
		}

		const int DISTANCE_FOR_KILL = 150; 
		int DISTANCE_FOR_WARNING = 400;

		float dist1 = vector.Distance(m_gameMode.m_firstSpawnPoint.GetOrigin(), m_gameMode.m_kothTrigger.GetOrigin());
		if (dist1 < 600)
   			DISTANCE_FOR_WARNING = 250;

		float dist2 = vector.Distance(m_gameMode.m_secondSpawnPoint.GetOrigin(), m_gameMode.m_kothTrigger.GetOrigin());
		if (dist2 < 600)
   			DISTANCE_FOR_WARNING = 250;
			
		float dist3 = vector.Distance(m_gameMode.m_thirdSpawnPoint.GetOrigin(), m_gameMode.m_kothTrigger.GetOrigin());
		if (dist3 < 600)
   			DISTANCE_FOR_WARNING = 250;

		if (m_gameMode.m_firstSpawnPoint.GetFactionKey() != playerFaction.GetFactionKey())
		{
			float distanceFromZone = vector.Distance(controlledEntity.GetOrigin(), m_gameMode.m_firstSpawn.GetOrigin());
			if (distanceFromZone < DISTANCE_FOR_WARNING)
			{
				if (distanceFromZone < DISTANCE_FOR_KILL)
					KillPlayerTooClose(controlledEntity);

				ShowWarning(controlledEntity);
				return;
			}
		}
		if (m_gameMode.m_secondSpawnPoint.GetFactionKey() != playerFaction.GetFactionKey())
		{
			float distanceFromZone = vector.Distance(controlledEntity.GetOrigin(), m_gameMode.m_secondSpawn.GetOrigin());
			if (distanceFromZone < DISTANCE_FOR_WARNING)
			{
				if (distanceFromZone < DISTANCE_FOR_KILL)
					KillPlayerTooClose(controlledEntity);

				ShowWarning(controlledEntity);
				return;
			}
		}
		if (m_gameMode.m_thirdSpawnPoint.GetFactionKey() != playerFaction.GetFactionKey())
		{
			float distanceFromZone = vector.Distance(controlledEntity.GetOrigin(), m_gameMode.m_thirdSpawn.GetOrigin());
			if (distanceFromZone < DISTANCE_FOR_WARNING)
			{
				if (distanceFromZone < DISTANCE_FOR_KILL)
					KillPlayerTooClose(controlledEntity);
				
				ShowWarning(controlledEntity);
				return;
			}
		}
		
		HideWarning(controlledEntity);
	}
	
	private void KillPlayerTooClose(IEntity controlledEntity)
	{
		SCR_CharacterControllerComponent characterController = GetCharacterController(controlledEntity);
		if (!characterController)
			return;

		characterController.ForceDeath();
	}

	private void ShowWarning(IEntity controlledEntity)
	{
		if (!m_zoneWarningWdg)
		{
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			m_zoneWarningWdg = workspace.CreateWidgets("{309707579229F6A8}UI/layouts/HUD/RestrictionZoneWarning/RestrictionZoneWarning.layout");
			TextWidget explainTxtWdg = TextWidget.Cast(m_zoneWarningWdg.FindAnyWidget("Explanation"));
			explainTxtWdg.SetText("Too close from enemy spawn");
			TextWidget warningTxtWdg = TextWidget.Cast(m_zoneWarningWdg.FindAnyWidget("Warning"));
			warningTxtWdg.SetText("Spawn camping is not allowed go further back");
		}
		else
		{
			m_zoneWarningWdg.SetVisible(true);
		}
		
		SCR_CharacterControllerComponent characterController = GetCharacterController(controlledEntity);
		if (!characterController)
			return;

		// Waiting for fix -> https://feedback.bistudio.com/T181953
		characterController.SetDisableWeaponControls(true);
		
		//characterController.SetFireWeaponWanted(false);
		//characterController.SetWeaponNoFireTime(1000);
		characterController.SetSafety(true, false);
		characterController.SetDisableViewControls(true);
	}
	
	private void HideWarning(IEntity controlledEntity)
	{
		if (!m_zoneWarningWdg)
			return;

		m_zoneWarningWdg.SetVisible(false);
		
		if (!controlledEntity)
			return;

		SCR_CharacterControllerComponent characterController = GetCharacterController(controlledEntity);
		if (!characterController)
			return;

		// Waiting for fix -> https://feedback.bistudio.com/T181953
		characterController.SetDisableWeaponControls(true);

//		characterController.SetFireWeaponWanted(true);
		characterController.SetSafety(false, false);
		characterController.SetDisableViewControls(false);
	}

	protected SCR_CharacterControllerComponent GetCharacterController(IEntity from)
	{
		if (!from)
			return null;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(from);
		if (!character)
			return SCR_CharacterControllerComponent.Cast(from.FindComponent(SCR_CharacterControllerComponent));
		
		return SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
	}
}