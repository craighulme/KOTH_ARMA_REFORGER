class KOTH_WaypointInfoDisplay : SCR_InfoDisplayExtended
{
    WorkspaceWidget m_Workspace;
    BaseWorld m_World;
    
    VerticalLayoutWidget m_wWaypointInfo;
    ImageWidget m_wIcon;
    TextWidget m_wDistance;
    
    vector m_vWorldPosWaypoint = "0 0 0";
    vector m_vScreenPosWaypoint;
    
    float m_minDistanceFromCenter;
    
    int m_iFrameCounter = 0;
    int m_iUpdateFrequency = 5;

    // new bool to fix flickering
    bool m_isWaypointVisible = false;
	
	private ArmaReforgerScripted m_game = GetGame();
    
    //------------------------------------------------------------------------------------------------
    override void DisplayStartDraw(IEntity owner)
    {
        if (!m_wRoot) return;

        InitWaypoint();
        RemoveWaypoint();

        // initial waypoint
        CreateWaypointAtPosition();
		
		SCR_BaseGameMode m_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		KOTH_PresenceTriggerEntity trigger = KOTH_PresenceTriggerEntity.Cast(m_gameMode.m_kothTrigger);
		m_minDistanceFromCenter = trigger.GetSphereRadius() * 2 + 100;
    }
    
    //------------------------------------------------------------------------------------------------
    override void DisplayUpdate(IEntity owner, float timeSlice)
    {        
        if (!m_Workspace) return;
		
		if (m_game.ShowWaypointAO)
		{
			AssignWaypoint();
			m_isWaypointVisible = true;
		}
		else
		{
			RemoveWaypoint();
            m_isWaypointVisible = false;
			return;
		}

        UpdateWaypointPositionFromTrigger();

        float distanceFloat = vector.Distance(owner.GetOrigin(), m_vWorldPosWaypoint);

        if (m_vWorldPosWaypoint == "0 0 0" || distanceFloat < m_minDistanceFromCenter)
        {
            if (m_isWaypointVisible)  // remove waypoint only if visible
            {
                RemoveWaypoint();
                m_isWaypointVisible = false;  // refresh status
            }
            return;
        }
        
        if (!m_isWaypointVisible)
        {
            AssignWaypoint();
            m_isWaypointVisible = true;  // refresh status
        }

        vector newScreenPos = m_Workspace.ProjWorldToScreen(m_vWorldPosWaypoint, m_World);
        if (newScreenPos != m_vScreenPosWaypoint)
        {
            m_vScreenPosWaypoint = newScreenPos;
            UpdateWaypointInfo(owner);
        }
    }

    //------------------------------------------------------------------------------------------------
    void InitWaypoint()
    {
        m_wWaypointInfo = VerticalLayoutWidget.Cast(m_wRoot.FindAnyWidget("m_wWaypointInfo"));
        m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("m_wIcon"));
        m_wDistance = TextWidget.Cast(m_wRoot.FindAnyWidget("m_wDistance"));
        
        if (!m_Workspace) m_Workspace = GetGame().GetWorkspace();
        if (!m_World) m_World = GetGame().GetWorld();
    }

    //------------------------------------------------------------------------------------------------
    void UpdateWaypointPositionFromTrigger()
    {
        SCR_BaseGameMode m_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
        
        if (!m_gameMode)
        {
            Log("Game Mode not found!", LogLevel.ERROR);
            return;
        }
    
        KOTH_PresenceTriggerEntity trigger = KOTH_PresenceTriggerEntity.Cast(m_gameMode.m_kothTrigger);
        if (!trigger)
        {
            Log("KOTH Priority Trigger not found!", LogLevel.ERROR);
            return;
        }
    
        vector triggerPosition = trigger.GetOrigin();
        SetWaypointPosition(triggerPosition);
    }

    //------------------------------------------------------------------------------------------------
    void UpdateWaypointInfo(IEntity owner)
    {
        float x = m_vScreenPosWaypoint[0] - 50;
        float y = m_vScreenPosWaypoint[1] - 50;
        float z = m_vScreenPosWaypoint[2];
        
        FrameSlot.SetPos(m_wWaypointInfo, x, y);
        m_wWaypointInfo.SetVisible(z > 0);
        
        m_iFrameCounter++;
        if (m_iFrameCounter < m_iUpdateFrequency) return;
        m_iFrameCounter = 0;
		
        float distanceFloat = vector.Distance(owner.GetOrigin(), m_vWorldPosWaypoint);
        m_wDistance.SetText(Math.Round(distanceFloat).ToString() + "m");
    }

    //------------------------------------------------------------------------------------------------
    void AssignWaypoint()
    {
        Show(true);
        m_wWaypointInfo.SetEnabled(true);
        m_wWaypointInfo.SetVisible(true);
    }

    //------------------------------------------------------------------------------------------------
    void RemoveWaypoint()
    {
        m_vWorldPosWaypoint = "0 0 0";
        Show(false);
        m_wWaypointInfo.SetEnabled(false);
        m_wWaypointInfo.SetVisible(false);
    }

    //------------------------------------------------------------------------------------------------
    void SetWaypointPosition(vector waypointPos)
    {
        m_vWorldPosWaypoint = waypointPos;
    }

    //------------------------------------------------------------------------------------------------
    void CreateWaypointAtPosition()
    {
        SCR_BaseGameMode m_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
        if (!m_gameMode)
        {
            Log("Game Mode not found!", LogLevel.ERROR);
            return;
        }

        KOTH_PriorityAreaPresenceTriggerEntity trigger = KOTH_PriorityAreaPresenceTriggerEntity.Cast(m_gameMode.m_kothPriorityTrigger);
        if (!trigger)
        {
            Log("KOTH Priority Trigger not found!", LogLevel.ERROR);
            return;
        }

        vector centerAO = trigger.GetOrigin();
        
        Log("Creating waypoint at coordinates: " + centerAO.ToString(), LogLevel.NORMAL);
        SetWaypointPosition(centerAO);
        AssignWaypoint();
        m_isWaypointVisible = true;  // initial true
    }
}