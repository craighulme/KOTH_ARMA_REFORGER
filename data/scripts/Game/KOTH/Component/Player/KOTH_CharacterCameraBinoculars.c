modded class CharacterCameraBinoculars : CharacterCamera1stPerson
{
    protected float m_fBinocFOV;
    protected SCR_BinocularsComponent m_binoculars;
    protected SCR_2DOpticsComponent m_Optics;
    protected IEntity m_ePlayer;

    protected bool m_bIsBinocularActive;
    protected float m_fRangeInMeters;
    protected float m_fRaycastInterval = 1.0; 
    protected float m_fRaycastTimer = 0;
    protected CanvasWidget m_wCanvasWidget;
    protected ref array<ref CanvasWidgetCommand> m_aCanvasCommands;
    protected vector m_vRaycastHitPosition;

    protected const ResourceName RETICLE_LAYOUT = "{03AC5E7B061DA9EA}UI/Layouts/HUD/Binocs/ReticleLayout.layout";

    private const float RAY_DISTANCE = 2000.0; 
    private const int TRACE_LAYER_CAM = EPhysicsLayerDefs.Default;

    void CharacterCameraBinoculars(CameraHandlerComponent pCameraHandler)
    {
        m_aCanvasCommands = new array<ref CanvasWidgetCommand>();
    }

    override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
    {
        super.OnActivate(pPrevCamera, pPrevCameraResult);
		
		// Get binoComponent as it has not been set yet
		SetBinocularComponent(m_ePlayer);
		
		// Get MeshObject from binoComponent
		VObject binoMeshObject = m_binoculars.GetOwner().GetVObject();
		
		// Check whether resourceName contains "Rangefinder".
		if (!binoMeshObject.GetResourceName().Contains("Rangefinder")) {
			return;
		}
		
        m_bIsBinocularActive = true;
        m_fRaycastTimer = 0;

        m_Optics.s_OnSightsADSChanged.Insert(OnSightsADSChanged);

        CreateCanvasWidget();
    }
	
	protected void SetBinocularComponent(IEntity owner)
	{
		// Finding gadget manager 
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast( m_ePlayer.FindComponent(SCR_GadgetManagerComponent) );
		if (!gadgetManager)
			return;
		
		// Getting binoculars componenet 
		SCR_BinocularsComponent binoculars = SCR_BinocularsComponent.Cast(gadgetManager.GetHeldGadgetComponent());
		if (!binoculars)
			return;
		
		m_binoculars = binoculars;
	}

    protected void OnSightsADSChanged(bool inADS, float fov)
    {
        if (!inADS)
        {
	        if (m_Optics)
	            m_Optics.s_OnSightsADSChanged.Remove(OnSightsADSChanged);
			
            DeleteWidgets();
        }
    }

    override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
    {
        super.OnUpdate(pDt, pOutResult);

        if (m_bIsBinocularActive)
        {
            m_fRaycastTimer += pDt;
            if (m_fRaycastTimer >= m_fRaycastInterval)
            {
                m_fRaycastTimer = 0;
                PerformRaycast();
            }
			
			m_aCanvasCommands.Clear(); 

			// Not needed anymore, but we can leave it in just in case.
            // DrawReticle();
            DrawRangeText();
        }

        pOutResult.m_fFOV = m_fBinocFOV;
        pOutResult.m_bBlendFOV = false;

        if (m_pCompartmentAccess.IsInCompartment())
            pOutResult.m_fUseHeading = 0;

        if (m_Optics)
            pOutResult.m_fNearPlane = m_Optics.GetNearPlane();
        else
        {
            pOutResult.m_fNearPlane = 0.05;
        }
    }

    protected void CreateCanvasWidget()
    {
        WorkspaceWidget workspace = GetGame().GetWorkspace();
        if (!workspace)
            return;

        Widget reticleWidget = workspace.CreateWidgets(RETICLE_LAYOUT);
        if (!reticleWidget)
            return;

        m_wCanvasWidget = CanvasWidget.Cast(reticleWidget.FindAnyWidget("ReticleCanvas"));
        if (!m_wCanvasWidget)
        {
            Print("Reticle CanvasWidget not found!", LogLevel.ERROR);
            return;
        }
    }

    protected void DeleteWidgets()
    {
        if (m_wCanvasWidget)
        {
            m_wCanvasWidget.RemoveFromHierarchy();
            m_wCanvasWidget = null;
        }
    }

	// TODO: This doesn't seem to work well when looking through wooden fences
    protected void PerformRaycast()
    {
        vector transform[4];
        GetGame().GetWorld().GetCurrentCamera(transform);

        vector startPos = transform[3];
        vector forwardVector = transform[2];

        TraceParam traceParam = new TraceParam();
        traceParam.Start = startPos;
        traceParam.End = startPos + forwardVector * RAY_DISTANCE;
        traceParam.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
        traceParam.LayerMask = TRACE_LAYER_CAM;

        if (startPos[1] > GetGame().GetWorld().GetOceanBaseHeight())
            traceParam.Flags |= TraceFlags.OCEAN;

        float rayDistance = GetGame().GetWorld().TraceMove(traceParam, null);

        if (rayDistance < 1)
        {
            m_vRaycastHitPosition = startPos + forwardVector * (rayDistance * RAY_DISTANCE);
            m_fRangeInMeters = vector.Distance(startPos, m_vRaycastHitPosition);
        }
        else
        {
            m_fRangeInMeters = -1; // No object hit
        }
    }

    protected void DrawReticle()
    {
        m_aCanvasCommands.Clear(); 

        if (!m_wCanvasWidget)
            return;

        float width, height;
        m_wCanvasWidget.GetScreenSize(width, height);

        float reticleSize = 20.0; 
        float reticleThickness = 2.0;

        // Center of the screen
        float centerX = width * 0.5;
        float centerY = height * 0.5;

        LineDrawCommand horizontalLine = new LineDrawCommand();
        horizontalLine.m_iColor = 0xFFFFFFFF; // White color
        horizontalLine.m_fWidth = reticleThickness;
        horizontalLine.m_Vertices = { centerX - reticleSize, centerY, centerX + reticleSize, centerY };

        LineDrawCommand verticalLine = new LineDrawCommand();
        verticalLine.m_iColor = 0xFFFFFFFF; // White color
        verticalLine.m_fWidth = reticleThickness;
        verticalLine.m_Vertices = { centerX, centerY - reticleSize, centerX, centerY + reticleSize };

        m_aCanvasCommands.Insert(horizontalLine);
        m_aCanvasCommands.Insert(verticalLine);

        m_wCanvasWidget.SetDrawCommands(m_aCanvasCommands);
    }

    protected void DrawRangeText()
    {
        if (!m_wCanvasWidget || m_fRangeInMeters < 0)
            return;

        float width, height;
        m_wCanvasWidget.GetScreenSize(width, height);

        float offsetX = 60.0;
        float offsetY = 60.0;

        // Center of the screen
        float centerX = width * 0.5;
        float centerY = height * 0.5;

        float textX = centerX + offsetX;
        float textY = centerY + offsetY;

        TextDrawCommand textCommand = new TextDrawCommand();
        textCommand.m_sText = "Range: " + m_fRangeInMeters.ToString(-1, 1) + " meters"; // Display range with one decimal
        textCommand.m_iColor = 0xFFFFFFFF; // White color
        textCommand.m_Position = Vector(textX, textY, 0);
        textCommand.m_fSize = 20.0; // Font size

        m_aCanvasCommands.Insert(textCommand);

        m_wCanvasWidget.SetDrawCommands(m_aCanvasCommands);
    }
}
