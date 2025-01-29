[EntityEditorProps(category: "GameScripted/Koth", description: "Handles Map Markers.")]
class KOTH_SCR_PlayerMapMarkerHandlerComponentClass : ScriptComponentClass{};
class KOTH_SCR_PlayerMapMarkerHandlerComponent : ScriptComponent
{
	protected SCR_PlayerController m_playerController;
	protected SCR_MapEntity m_mapEntity;
	protected RplComponent m_rplComponent;
	protected SCR_BaseGameMode m_gameMode;
	protected KOTH_ZoneManagerComponent m_kothZoneManager;
	
	protected float m_zoomValue;


	void ~KOTH_SCR_PlayerMapMarkerHandlerComponent()
	{
		m_mapEntity.GetOnMapOpen().Remove(OnPlayerMapOpen);
		m_mapEntity.GetOnMapZoom().Remove(OnPlayerMapZoom);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (SCR_Global.IsEditMode(owner))
			return;

		m_playerController = SCR_PlayerController.Cast(PlayerController.Cast(owner));
		if (!m_playerController)
		{
			return;
		}

		m_rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);

		m_mapEntity = SCR_MapEntity.GetMapInstance();

		m_mapEntity.GetOnMapOpen().Insert(OnPlayerMapOpen);
		m_mapEntity.GetOnMapZoom().Insert(OnPlayerMapZoom);
	}

	protected void OnPlayerMapOpen(MapConfiguration config)
	{
		m_zoomValue = m_mapEntity.GetCurrentZoom();
		m_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_kothZoneManager = KOTH_ZoneManagerComponent.Cast(m_gameMode.FindComponent(KOTH_ZoneManagerComponent));
		
		UpdatePrioIcon();
	}

	void UpdateTowersIcon()
	{
		// Implement logic for tower icons if necessary
	}

	void UpdatePrioIcon()
	{
	    if (!m_gameMode || !m_kothZoneManager)
	        return;

		  UpdateTriggerProperties();
		
	    float currentZoom = m_mapEntity.GetCurrentZoom(); // 0.0-20.0 ZOOM
	    float priorityZoneRadius = m_kothZoneManager.GetPriorityZoneRadius();
	    vector priorityZoneCenter = m_kothZoneManager.GetPriorityZoneCenter();
	
	    KOTH_PriorityAreaPresenceTriggerEntity kothPAZone = KOTH_PriorityAreaPresenceTriggerEntity.Cast(m_gameMode.m_kothPriorityTrigger);
	    KOTH_SCR_PriorityAreaMapDescriptorComponent zonePA = KOTH_SCR_PriorityAreaMapDescriptorComponent.Cast(kothPAZone.FindComponent(KOTH_SCR_PriorityAreaMapDescriptorComponent));
	    if (!zonePA)
	        return;
		
	    MapItem itemPA = zonePA.Item();
	    if (itemPA)
	    {
	        float zoomMultiplier = (priorityZoneRadius * 0.80) / 100;
	
	        float posX = priorityZoneCenter[0];
	        float posY = priorityZoneCenter[2];
	
	        // Update position in case it changed
	        itemPA.SetPos(posX, posY);
			
	        // Update size
	        MapDescriptorProps paProps = itemPA.GetProps();
	        paProps.SetIconSize(256, -currentZoom * zoomMultiplier, -currentZoom * zoomMultiplier);
	        itemPA.SetProps(paProps);
	    }
	}

	protected void OnPlayerMapZoom(float targetPPU)
	{
	    if (!m_gameMode || !m_gameMode.m_kothTrigger || !m_gameMode.m_kothPriorityTrigger || !m_kothZoneManager)
	    {
	        return;
	    }
		
	    float currentZoom = m_mapEntity.GetCurrentZoom();
	    float zoneRadius = m_kothZoneManager.GetZoneRadius();
		
		KOTH_PresenceTriggerEntity kothZone = KOTH_PresenceTriggerEntity.Cast(m_gameMode.m_kothTrigger);
	    KOTH_PriorityAreaPresenceTriggerEntity kothPAZone = KOTH_PriorityAreaPresenceTriggerEntity.Cast(m_gameMode.m_kothPriorityTrigger);
	
		if(!kothZone || !kothPAZone)
			return;
		
	    // Main Zone
	    KOTH_SCR_MapDescriptorComponent mapDescriptor = KOTH_SCR_MapDescriptorComponent.Cast(kothZone.FindComponent(KOTH_SCR_MapDescriptorComponent));
	    if (mapDescriptor)
	    {
	        MapItem item = mapDescriptor.Item();
	        if (item)
	        {
	            float zoomMultiplier = (zoneRadius * 0.80) / 100;
	            MapDescriptorProps props = item.GetProps();
	            props.SetIconSize(256, -currentZoom * zoomMultiplier, -currentZoom * zoomMultiplier);
	            item.SetProps(props);
	        }
	    }
	
	    // Priority Zone
	    UpdatePrioIcon();
	}

	void UpdateTriggerProperties()
	{
	    KOTH_PriorityAreaPresenceTriggerEntity priorityTrigger = KOTH_PriorityAreaPresenceTriggerEntity.Cast(m_gameMode.m_kothPriorityTrigger);
	    if (!priorityTrigger)
	    {
	        return;
	    }
	
	    vector priorityZoneCenter = m_kothZoneManager.GetPriorityZoneCenter();
	    float priorityZoneRadius = m_kothZoneManager.GetPriorityZoneRadius();
	
	    priorityTrigger.SetOrigin(priorityZoneCenter);
	    priorityTrigger.SetSphereRadius(priorityZoneRadius);
	}
}