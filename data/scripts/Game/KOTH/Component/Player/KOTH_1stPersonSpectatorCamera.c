[ComponentEditorProps(category: "GameScripted/Misc", description: "Spectator component for managing player spectating.")]
class KOTH_1stPersonSpectatorComponentClass : ScriptComponentClass { }

class KOTH_1stPersonSpectatorComponent : ScriptComponent 
{
    // Core Components
    private KOTH_SpectatorCamera OGCamera;
    private KOTH_SpectatorCamera SpectatorCamera;
    private CharacterIdentityComponent IdentityComponent;
    private EquipedLoadoutStorageComponent ELSComponent;
    private IEntity HeadCoverEntity;
    private SCR_CharacterCameraHandlerComponent CharCameraHandlerComponent;
    private EventHandlerManagerComponent EventHandler;
    private InputManager Input;

    // Invokers
    private ref ScriptInvoker OnDamageStateChangedSpec;
    private ref ScriptInvoker OnZoomChangedScript;
    
    // States
    protected bool IsSpectating; // True on the observer side
    protected bool IsBeingSpectated; // True on the observed player side
    private bool InADS;
    
    // Camera and FOV
    private vector CurrentCameraTransform[4];
    private vector TargetCameraTransform[4];
	private vector PIPCameraTransform[4];
    private float TargetVerticalFOV;
    private float CurrentVerticalFOV;
    private float currentZoomLevel = 1.0;
    private float currentZoomFOV = 38.0;
    private float cameraLerpFactor = 0.9;

	//Spectated Cached Vars
	private ChimeraCharacter cachedOwnerCharacter;
	private BaseWeaponManagerComponent cachedWeaponManager;
	private SCR_2DPIPSightsComponent cachedPIPSightsComponent;
	private IEntity cachedSightsParentEntity;
	private int cachedPIPCameraSightsIndex = -1;
	
    // Spectating data
    private RplId SpectatorId;
    private SCR_CameraManager CamManager;
    private BaseWeaponManagerComponent weaponManager;
    private IEntity targetEntity;
    private ChimeraCharacter targetCharacter;
    private SCR_2DPIPSightsComponent ObservedPIPSightsComponent;
    private bool PIPCameraActive;
    private float PIPCameraFOV;
	private int currentSightsIndex;
	
	private BaseSightsComponent currentSights;
	private BaseSightsComponent previousSights;
	private int pipCameraSightsIndex = -1;
	private int previousSightsIndex = -1;
    
    // Cleanup on destruction
    void ~KOTH_1stPersonSpectatorComponent() 
    {
        CleanupSpectatingState();
    }

    // RPC Methods ---------------------------------------------------------------------------------------------------------------------------------
	
    private void ResetZoomVariables() 
    {
        if (ObservedPIPSightsComponent) 
        {
            SCR_SightsZoomFOVInfo zoomFOVInfo = SCR_SightsZoomFOVInfo.Cast(ObservedPIPSightsComponent.GetFOVInfo());
            if (zoomFOVInfo) 
            {
                currentZoomFOV = 38.0;
                currentZoomLevel = 1.0;
            }
        }
    }

    // Zoom Logic triggered from event handler on observed player.
    void OnZoomChanged(int zoomLevel, bool increased) 
    {
		if (!IsBeingSpectated) return;
		
        Rpc(RpcSend_OnZoomChanged, zoomLevel, increased);
    }

	// Send to server
    [RplRpc(RplChannel.Unreliable, RplRcver.Server)]
    void RpcSend_OnZoomChanged(int zoomLevel, bool increased) 
    {
		RplId spectatorRplPlayerId = Replication.FindId(GetOwner());
		int spectatorPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromEntityRplId(spectatorRplPlayerId);
	    if (spectatorPlayerId == -1) return; // Invalid player ID
	
	    if (!SCR_Global.IsAdmin(spectatorPlayerId)) 
	    {
	        StopSpectating(true);
	    }
		
        Rpc(RpcSend_ToSpectator_OnZoomChanged, spectatorRplPlayerId, zoomLevel, increased);
    }

	// Send to spectator
    [RplRpc(RplChannel.Unreliable, RplRcver.Owner)]
    void RpcSend_ToSpectator_OnZoomChanged(RplId spectatorId, int zoomLevel, bool increased) 
    {
        IEntity spectator = IEntity.Cast(Replication.FindItem(spectatorId));
        if (!spectator) return;
		
        OnZoomChangedOwner(zoomLevel, increased);
        
    }

	// Sync zoom state on spectator side
    void OnZoomChangedOwner(int zoomLevel, bool increased) 
    {
        if (!IsSpectating) return;

        if (ObservedPIPSightsComponent) 
        {
            SCR_SightsZoomFOVInfo zoomFOVInfo = SCR_SightsZoomFOVInfo.Cast(ObservedPIPSightsComponent.GetFOVInfo());
            if (zoomFOVInfo) 
            {
                currentZoomLevel = zoomLevel;
                currentZoomFOV = zoomFOVInfo.GetCurrentFOVPublic();
                SpectatorCamera.SetVerticalFOV(currentZoomFOV);
            }
        }
    }

    // Spectator Camera Updates ---------------------------------------------------------------------------------------------------------------------
    
	// Triggered on the spectator's end in EOnFixedFrame
	private void SendSpectatorCameraDataToServer() 
	{
	    if (!IsBeingSpectated) return;
	
	    CameraBase currentCamera = GetGame().GetCameraManager().CurrentCamera();
	    if (currentCamera) 
	    {
	        vector cameraTransform[4];
	        currentCamera.GetWorldTransform(cameraTransform);
	        Rpc(RpcSendCameraDataToSpectatorFromServer, SpectatorId, cameraTransform, currentCamera.GetVerticalFOV());
	    }
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	private void RpcSendCameraDataToSpectatorFromServer(RplId spectatorRplId, vector cameraTransform[4], float cameraFOV) 
	{
	    IEntity spectator = IEntity.Cast(Replication.FindItem(spectatorRplId));
	    if (!spectator) return;
	
	    int spectatorPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromEntityRplId(spectatorRplId);
	    if (spectatorPlayerId == -1) return; // Invalid player ID
	
//	    if (!SCR_Global.IsAdmin(spectatorPlayerId)) 
//	    {
//	        StopSpectating(true);
//	    }
	
	    KOTH_1stPersonSpectatorComponent spectatorComponent = KOTH_1stPersonSpectatorComponent.Cast(spectator.FindComponent(KOTH_1stPersonSpectatorComponent));
	    if (spectatorComponent) 
	    {
	        spectatorComponent.UpdateSpectatorCameraData(cameraTransform, cameraFOV);
	    }
	}
	
	private void UpdateSpectatorCameraData(vector cameraTransform[4], float cameraFOV) 
	{
	    Rpc(RpcReceiveSpectatorCameraData, cameraTransform, cameraFOV);
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Owner)]
	private void RpcReceiveSpectatorCameraData(vector cameraTransform[4], float cameraFOV) 
	{
	    TargetCameraTransform = cameraTransform;
	    TargetVerticalFOV = cameraFOV;
	}
	
	// PIP Camera Updates ---------------------------------------------------------------------------------------------
	
	// Triggered on the observed player's end in EOnFixedFrame
	private void SendPIPCameraDataToServer() 
	{
	    if (!IsBeingSpectated) return;
	
	    if (!cachedOwnerCharacter) 
	    {
	        cachedOwnerCharacter = ChimeraCharacter.Cast(GetOwner());
	    }
	    if (!cachedOwnerCharacter) return;
	
	    if (!cachedWeaponManager) 
	    {
	        cachedWeaponManager = BaseWeaponManagerComponent.Cast(cachedOwnerCharacter.FindComponent(BaseWeaponManagerComponent));
	    }
	    if (!cachedWeaponManager) return;
	
	    BaseWeaponComponent currentWeapon = cachedWeaponManager.GetCurrentWeapon();
	    if (!currentWeapon) return;
	
	    // Get current sights and check if their parent entity has changed
	    BaseSightsComponent localCurrentSights = cachedWeaponManager.GetCurrentSights();
	    IEntity currentSightsParent = null;
	
	    if (localCurrentSights)
	    {
	        currentSightsParent = localCurrentSights.GetOwner();  // Get the parent entity
	    }
	
	    if (!currentSightsParent || currentSightsParent != cachedSightsParentEntity)
	    {
	        // The optic or sight has changed, update the cache
	        cachedPIPSightsComponent = SCR_2DPIPSightsComponent.Cast(localCurrentSights);
	        cachedSightsParentEntity = currentSightsParent;
	        cachedPIPCameraSightsIndex = -1;
	    }
	
	    // Cache PIP Camera Sight Index only if it hasn’t been calculated yet
	    if (cachedPIPCameraSightsIndex == -1)
	    {
	        int availableSightsCount = currentWeapon.FindAvailableSights();
	        for (int i = 0; i <= availableSightsCount; i++) 
	        {
	            BaseSightsComponent sightsComponent = currentWeapon.GetSightsAt(i);
	            SCR_2DPIPSightsComponent pipSightsComponent = SCR_2DPIPSightsComponent.Cast(sightsComponent);
	            if (pipSightsComponent && pipSightsComponent.GetPIPCamera()) 
	            {
	                cachedPIPCameraSightsIndex = i;
	                break;
	            }
	        }
	    }
	
	    bool pipCamActive = (cachedWeaponManager.GetCurrentSightsIndex() == cachedPIPCameraSightsIndex) && cachedPIPSightsComponent && cachedPIPSightsComponent.GetPIPCamera();
	
	    if (pipCamActive)
	    {
	        float pipCameraFOV = cachedPIPSightsComponent.GetPIPCamera().GetVerticalFOV();
	        vector pipCameraTransform[4];
	        cachedPIPSightsComponent.GetPIPCamera().GetWorldTransform(pipCameraTransform);
	
	        Rpc(RpcSendPIPCameraDataToSpectatorFromServer, SpectatorId, pipCamActive, pipCameraFOV, pipCameraTransform);
	    }
	    else
	    {
	        Rpc(RpcSendPIPCameraDataToSpectatorFromServer, SpectatorId, false, 0.0, vector.Zero);
	    }
	}
		
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	private void RpcSendPIPCameraDataToSpectatorFromServer(RplId spectatorRplId, bool pipCamActive, float pipCameraFOV, vector pipCameraTransform[4]) 
	{
	    IEntity spectator = IEntity.Cast(Replication.FindItem(spectatorRplId));
	    if (!spectator) return;
	
	    int spectatorPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromEntityRplId(spectatorRplId);
	    if (spectatorPlayerId == -1) return; // Invalid player ID
	
//	    if (!SCR_Global.IsAdmin(spectatorPlayerId)) 
//	    {
//	        StopSpectating(true);
//	    }
	
	    KOTH_1stPersonSpectatorComponent spectatorComponent = KOTH_1stPersonSpectatorComponent.Cast(spectator.FindComponent(KOTH_1stPersonSpectatorComponent));
	    if (spectatorComponent) 
	    {
	        spectatorComponent.UpdateSpectatorPIPCameraData(pipCamActive, pipCameraFOV, pipCameraTransform);
	    }
	}
	
	private void UpdateSpectatorPIPCameraData(bool pipCamActive, float pipCameraFOV, vector pipCameraTransform[4]) 
	{
	    Rpc(RpcReceiveSpectatorPIPCameraData, pipCamActive, pipCameraFOV, pipCameraTransform);
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Owner)]
	private void RpcReceiveSpectatorPIPCameraData(bool pipCamActive, float pipCameraFOV, vector pipCameraTransform[4]) 
	{
	    // Set the PIP camera data on the spectator's side
	    PIPCameraActive = pipCamActive;
	    PIPCameraFOV = pipCameraFOV;
		Math3D.MatrixCopy(pipCameraTransform, PIPCameraTransform); //already world transform
	}
	
	//Cam Adjustments ----------------------------------------------------------------------------------------
    // Camera Transform Application.. already filtered to IsSpectating from parent method EOnPostFrame
    private void ApplyCameraTransform(float timeSlice) 
	{
	    if (SpectatorCamera) 
	    {
	        vector currentTransform[4];
	        SpectatorCamera.GetTransform(currentTransform);
			CharacterControllerComponent targetController = CharacterControllerComponent.Cast(targetEntity.FindComponent(CharacterControllerComponent));
			
			//Set ADS false if sprinting so camera isn't fked in apply transfrom
			if(targetController.IsSprinting())
			{
				InADS = false;
			}

	        if (InADS) 
	        {
	            ApplyADSAlignmentWithStabilization(currentTransform);
	        } 
	        else 
	        {
	            ApplyLerpedTransform(currentTransform);
	        }
	    } 
	}

    private void ApplyLerpedTransform(vector currentTransform[4]) 
    {
        vector lerpedPosition = vector.Lerp(currentTransform[3], TargetCameraTransform[3], cameraLerpFactor);

        float currentQuat[4], targetQuat[4], lerpedQuat[4];
        Math3D.MatrixToQuat(currentTransform, currentQuat);
        Math3D.MatrixToQuat(TargetCameraTransform, targetQuat);
        Math3D.QuatLerp(lerpedQuat, currentQuat, targetQuat, cameraLerpFactor);

        vector lerpedRotation[3];
        Math3D.QuatToMatrix(lerpedQuat, lerpedRotation);

        vector lerpedTransform[4];
        lerpedTransform[0] = lerpedRotation[0];
        lerpedTransform[1] = lerpedRotation[1];
        lerpedTransform[2] = lerpedRotation[2];
        lerpedTransform[3] = lerpedPosition;

        SpectatorCamera.SetTransform(lerpedTransform);
        SpectatorCamera.SetVerticalFOV(TargetVerticalFOV);
    }

    private void ApplyADSAlignmentWithStabilization(vector currentTransform[4]) 
	{
	    if (!targetCharacter || !weaponManager) return;
	
	    BaseWeaponComponent currentWeapon = weaponManager.GetCurrentWeapon();
	    if (!currentWeapon) return;
	
	    // Get the current sights
	    currentSights = weaponManager.GetCurrentSights();
	
	    // Check if the sight has changed
	    if (currentSights != previousSights)
	    {
	        // Reset zoom FOV or other parameters here if the sight has changed
	        ResetZoomFOV();
	        previousSights = currentSights;  // Update the previous sight to the current one
	    }
	
	    vector localMatrix[4];
	    vector worldMatrix[4];
	    float sightsFOV;
	
	    if (PIPCameraActive) 
	    {
	        // Use the PIP camera transform directly for alignment
	        ApplyLerpedTransformWithADS(currentTransform, PIPCameraTransform);
	        HandleSightsFOV(PIPCameraFOV); // Use the PIP camera FOV
	    }
	    else if (weaponManager.GetCurrentSightsCameraTransform(localMatrix, sightsFOV)) 
	    {
	        targetCharacter.GetTransform(worldMatrix);
	        Math3D.MatrixMultiply4(worldMatrix, localMatrix, worldMatrix);
	        ApplyLerpedTransformWithADS(currentTransform, worldMatrix);
	        HandleSightsFOV(TargetVerticalFOV);
	    } 
	    else 
	    {
	        ApplyLerpedTransform(currentTransform);
	    }
	}

    private void ApplyLerpedTransformWithADS(vector currentTransform[4], vector worldMatrix[4]) 
    {
        vector lerpedPosition = vector.Lerp(currentTransform[3], worldMatrix[3], cameraLerpFactor);

        float currentQuat[4], targetQuat[4], lerpedQuat[4];
        Math3D.MatrixToQuat(currentTransform, currentQuat);
        Math3D.MatrixToQuat(worldMatrix, targetQuat);
        Math3D.QuatLerp(lerpedQuat, currentQuat, targetQuat, cameraLerpFactor);

        vector lerpedRotation[3];
        Math3D.QuatToMatrix(lerpedQuat, lerpedRotation);

        vector lerpedTransform[4];
        lerpedTransform[0] = lerpedRotation[0];
        lerpedTransform[1] = lerpedRotation[1];
        lerpedTransform[2] = lerpedRotation[2];
        lerpedTransform[3] = lerpedPosition;

        SpectatorCamera.SetTransform(lerpedTransform);
    }

    private void HandleSightsFOV(float sightsFOV) 
	{
	    if (InADS) 
	    {
	        if (PIPCameraActive) 
	        {
	            // If PIP camera is active, use its FOV
	            SpectatorCamera.SetVerticalFOV(PIPCameraFOV);
	            return;
	        }
	        else if (ObservedPIPSightsComponent) 
	        {
	            // Use the magnification logic if PIP is not active but the sight has zoom
	            float zoomFOV = ObservedPIPSightsComponent.GetFovZoomed();
	            //float magnification = ObservedPIPSightsComponent.GetMagnification();
	            //float magFOV = CalculateMagnificationFOV(magnification, zoomFOV);
	            SpectatorCamera.SetVerticalFOV(zoomFOV);
	            return;
	        }
	    }
	    
	    // Default case
	    SpectatorCamera.SetVerticalFOV(sightsFOV);
	}
	
    // Calculate Magnification FOV
    private float CalculateMagnificationFOV(float magnification, float referenceFOV = 38.0) 
    {
        return Math.RAD2DEG * 2 * Math.Atan2(Math.Tan(Math.DEG2RAD * (referenceFOV / 2)), magnification);
    }
	
	private void ResetZoomFOV()
	{
	    if (ObservedPIPSightsComponent)
	    {
	        PIPCameraActive = false;
	        PIPCameraFOV = 0.0;
	    }
	    SpectatorCamera.SetVerticalFOV(74);
	}

    // Hide and Show Scope
    private void HideScope(BaseWeaponComponent currentWeapon) 
	{
	    IEntity scopeEntity = GetScopeEntity(currentWeapon);
	    if (scopeEntity) 
	    {
	        scopeEntity.ClearFlags(EntityFlags.VISIBLE, true);  // Hide the scope
	    }
	}
	
	private void ShowScope(BaseWeaponComponent currentWeapon) 
	{
	    IEntity scopeEntity = GetScopeEntity(currentWeapon);
	    if (scopeEntity) 
	    {
	        scopeEntity.SetFlags(EntityFlags.VISIBLE, true);  // Show the scope
	    }
	}

    private IEntity GetScopeEntity(BaseWeaponComponent currentWeapon) 
	{
        BaseSightsComponent sightsComponent = currentWeapon.GetSights();
        if (sightsComponent != null) 
        {
            return sightsComponent.GetOwner();
        }
        else
        {
            return null;
        }
	}
//#endregion

//#region Event Handlers
    // Event Methods -----------------------------------------------------------------------------------------------------------------------------
    override void EOnPostFrame(IEntity owner, float timeSlice) 
    {
		if(IsSpectating)
		{
			SyncSpectatorView();
	        ApplyCameraTransform(timeSlice);
	        UpdateHeadAndCameraShake(timeSlice);
		}
    }

    private void UpdateHeadAndCameraShake(float timeSlice) 
    {
        if (SpectatorCamera && targetCharacter && CharCameraHandlerComponent) 
        {
            SCR_RecoilCameraShakeProgress recoilShake = CharCameraHandlerComponent.GetRecoilShake();
            if (recoilShake && !recoilShake.IsFinished()) 
            {
                recoilShake.Update(targetCharacter, timeSlice);
            }
        }
    }
	
	private void SyncSpectatorView() 
	{
	    if (!targetCharacter || !weaponManager) 
	    {
	        return;
	    } 

	    BaseWeaponComponent currentWeapon = weaponManager.GetCurrentWeapon();
	    if (!currentWeapon) 
	    {
	        return;
	    } 
	
		InADS = targetCharacter.GetCharacterController().IsWeaponADS();
		
		InitializePIPSightsComponent();
		
		//add PIPCameraActive later to this if statement when trying to work on reticles and multiple sights. Position will be way off on scope instead of small reticle.
	    if (InADS && ObservedPIPSightsComponent) 
	    {
	        HideScope(currentWeapon);  // Hide the scope when in ADS
	    } 
	    else 
	    {
	        ResetZoomVariables();  // Reset zoom when not in ADS
	        ShowScope(currentWeapon);  // Show the scope when not in ADS
	    }
	}


	// Used only on the spectated player's end 
    override void EOnPostFixedFrame(IEntity owner, float timeSlice) 
    {
		if(IsBeingSpectated)
		{
			SendSpectatorCameraDataToServer();
	       	SendPIPCameraDataToServer();
		}
    }
//#endregion

//#region Spectating Methods
    // Start and Stop Spectating ----------------------------------------------------------------------------------------------------------------
    void StartSpectating(IEntity ent) 
    {
        IEntity owner = GetOwner();
        if (!owner) 
        {
            Log("GM has no existing player body. Stopping spectating");
            return;
        }
		
        if (!Input) 
        {
            Input = GetGame().GetInputManager();
        }
			
		// Need this so editor can be opened or closed and spectator cam displays
		SCR_EditorManagerEntity.GetInstance().GetOnClosed().Insert(OnEditorClosed);
	    SCR_EditorManagerEntity.GetInstance().GetOnOpened().Insert(OnEditorOpened);
	    SCR_EditorManagerEntity.ToggleInstance();
		
        targetEntity = ent;
        targetCharacter = ChimeraCharacter.Cast(targetEntity);
        weaponManager = targetCharacter.GetWeaponManager();

        LockPlayerControls();
        SetupSpectatorComponents(targetEntity);

        SetBeingSpectated(targetEntity, true);
        HideHeadAndAccessories(true);
    }

    void StopSpectating(bool earlyExit = false) 
	{
	    // Check if earlyexit is true to stop stack overflow of SetBeingSpectated
		if(!earlyExit)
		{
			SetBeingSpectated(targetEntity, false);
		}

	    UnlockPlayerControls();
	    ClearEventHandlers();      // Additional cleanup
	    HideHeadAndAccessories(false);
		
		if(!CamManager)
		{
			CamManager = SCR_CameraManager.Cast(GetGame().GetCameraManager());
		}
		
	    if(OGCamera)
		{
			CamManager.SetCamera(OGCamera);
			OGCamera = null;
		}
		
		SpectatorCamera.Destroy();
		SpectatorCamera = null;
		
	    IsSpectating = false;
        weaponManager = null;
		
		// Reset transforms and cached variables
	    CurrentCameraTransform = {};
	    TargetCameraTransform = {};
	    PIPCameraTransform = {};
	    targetEntity = null;
	    targetCharacter = null;
	}

    private void LockPlayerControls() 
    {
        CharacterControllerComponent ownerController = CharacterControllerComponent.Cast(GetOwner().FindComponent(CharacterControllerComponent));
		ownerController.SetDisableMovementControls(true);
		ownerController.SetDisableViewControls(true);
		ownerController.SetDisableWeaponControls(true);
    }

    private void UnlockPlayerControls() 
    {
        CharacterControllerComponent ownerController = CharacterControllerComponent.Cast(GetOwner().FindComponent(CharacterControllerComponent));
		ownerController.SetDisableMovementControls(false);
		ownerController.SetDisableViewControls(false);
		ownerController.SetDisableWeaponControls(false);
    }

    private void ClearEventHandlers() 
    {
		if (!Input) 
        {
            Input = GetGame().GetInputManager();
        }
		
        Input.RemoveActionListener("MenuOpen", EActionTrigger.DOWN, OnEscapeDown);
        if (OnDamageStateChangedSpec) 
        {
            OnDamageStateChangedSpec.Remove(OnSpectatedEntityDamageStateChanged);
        }
		
		SCR_EditorManagerEntity.GetInstance().GetOnClosed().Remove(OnEditorClosed);
	    SCR_EditorManagerEntity.GetInstance().GetOnOpened().Remove(OnEditorOpened);
		
        ClearEventMask(GetOwner(), EntityEvent.POSTFRAME);
    }

    private void SetupSpectatorComponents(IEntity ent) 
    {
        SCR_DamageManagerComponent dmgMngr = SCR_DamageManagerComponent.GetDamageManager(ent);
        if (dmgMngr) 
        {
            SCR_HitZone hitzone = SCR_HitZone.Cast(dmgMngr.GetDefaultHitZone());
            if (hitzone) 
            {
                OnDamageStateChangedSpec = hitzone.GetOnDamageStateChanged();
                OnDamageStateChangedSpec.Insert(OnSpectatedEntityDamageStateChanged);
            }
        }
		CharacterControllerComponent entController = CharacterControllerComponent.Cast(ent.FindComponent(CharacterControllerComponent));
		CharCameraHandlerComponent = SCR_CharacterCameraHandlerComponent.Cast(entController.GetCameraHandlerComponent());
        IdentityComponent = CharacterIdentityComponent.Cast(ent.FindComponent(CharacterIdentityComponent));
        ELSComponent = EquipedLoadoutStorageComponent.Cast(ent.FindComponent(EquipedLoadoutStorageComponent));
    }

    private void InitializePIPSightsComponent() 
	{
	    if (!weaponManager) return;
		
		BaseSightsComponent currentSightsLocal = weaponManager.GetCurrentSights();
		if (!currentSightsLocal) return;
		
	    ObservedPIPSightsComponent = SCR_2DPIPSightsComponent.Cast(currentSightsLocal);

	    if (ObservedPIPSightsComponent) 
	    {
	        SCR_SightsZoomFOVInfo zoomFOVInfo = SCR_SightsZoomFOVInfo.Cast(ObservedPIPSightsComponent.GetFOVInfo());
	        if (zoomFOVInfo) 
	        {
	            currentZoomLevel = zoomFOVInfo.GetCurrentIndex();
	            currentZoomFOV = zoomFOVInfo.GetCurrentFOVPublic();
	        }
	    }
	}
	
//#endregion

//#region Spectating RPC Methods
    // RPC Methods for Spectating
    private void SetBeingSpectated(IEntity ent, bool state) 
    {
        Rpc(RpcAsk_SetBeingSpectated, Replication.FindId(ent), state);
    }

	// Request from server to observed player
    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    private void RpcAsk_SetBeingSpectated(RplId entId, bool state) 
    {
        IEntity ent = IEntity.Cast(Replication.FindItem(entId));
        if (!ent) return;
		
	    int spectatorPlayerId = Replication.FindId(GetOwner());
	    if (spectatorPlayerId == -1) return; // Invalid player ID
	
	    if (!SCR_Global.IsAdmin(spectatorPlayerId)) 
	    {
			//earlyexit bool specified since we haven't setbeingspectated yet
			StopSpectating(true);
	    }

        KOTH_1stPersonSpectatorComponent comp = KOTH_1stPersonSpectatorComponent.Cast(ent.FindComponent(KOTH_1stPersonSpectatorComponent));
        if (comp) 
        {
            comp.SetBeingSpectatedOwner(spectatorPlayerId, state);
        }
    }

    private void SetBeingSpectatedOwner(RplId spectatorId, bool state) 
    {
        Rpc(RpcDo_SetBeingSpectated, spectatorId, state);
    }
	
	// Set the spectated state on the observed player's side
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	private void RpcDo_SetBeingSpectated(RplId spectatorId, bool state) 
	{
	    if (!GetOwner()) return;
	
	    if (state) 
	    {
	        IsBeingSpectated = true;
	        SpectatorId = spectatorId;
	        RegisterEventHandlers();
			SetEventMask(GetOwner(), EntityEvent.POSTFIXEDFRAME);
	    } 
	    else 
	    {
	        UnregisterEventHandlers();
	        CleanupSpectatingState();
			ClearEventMask(GetOwner(), EntityEvent.POSTFIXEDFRAME);
	    }
	}
	
	private void CleanupSpectatingState() 
    {
        IsBeingSpectated = false;
        SpectatorId = -1;
    }
	
	private void UnregisterEventHandlers() 
	{
		EventHandler = EventHandlerManagerComponent.Cast(GetOwner().FindComponent(EventHandlerManagerComponent));
	    if (EventHandler) 
	    {
	        EventHandler.RemoveScriptHandler("OnZoomChanged", this, OnZoomChanged);
	    }
	}

    private void RegisterEventHandlers() 
    {
        EventHandler = EventHandlerManagerComponent.Cast(GetOwner().FindComponent(EventHandlerManagerComponent));
        if (EventHandler) 
        {
            EventHandler.RegisterScriptHandler("OnZoomChanged", this, OnZoomChanged);
        }
    }
//#endregion

    // Hide/Show Head and Accessories
    private void HideHeadAndAccessories(bool hide) 
    {
        if (!ELSComponent && !IsBeingSpectated) return;

        HeadCoverEntity = ELSComponent.GetClothFromArea(LoadoutHeadCoverArea);

        if (hide) 
        {
            if (HeadCoverEntity) 
            {
                HeadCoverEntity.ClearFlags(EntityFlags.VISIBLE, true);
            }

            if (IdentityComponent) 
            {
                IdentityComponent.SetHeadAlpha(255);
            }
        } 
        else 
        {
            if (HeadCoverEntity) 
            {
                HeadCoverEntity.SetFlags(EntityFlags.VISIBLE, true);
            }

            if (IdentityComponent) 
            {
                IdentityComponent.SetHeadAlpha(0);
            }
        }
    }

    // Damage State Change Handling
    private void OnSpectatedEntityDamageStateChanged(HitZone hz) 
    {
        if (hz.GetDamageState() == EDamageState.DESTROYED) 
        {
            StopSpectating();
        }
    }

    // Editor Methods
    void OnEditorClosed() 
    {
        CamManager = SCR_CameraManager.Cast(GetGame().GetCameraManager());
		
		if(CamManager.CurrentCamera() != null)
		{
			CamManager.CurrentCamera().GetTransform(CurrentCameraTransform);
			OGCamera = KOTH_SpectatorCamera.Cast(CamManager.CurrentCamera());
		}

		SpectatorCamera = KOTH_SpectatorCamera.Cast(GetGame().SpawnEntity(KOTH_SpectatorCamera));
		CamManager.SetCamera(SpectatorCamera);
        SetEventMask(GetOwner(), EntityEvent.POSTFRAME);

        IsSpectating = true;
		Input.AddActionListener("MenuOpen", EActionTrigger.DOWN, OnEscapeDown);
    }

    void OnEditorOpened() 
    {
        StopSpectating();
    }
	
	void OnEscapeDown() 
	{
		LogCameraState("OnEscapeDown");
		SCR_EditorManagerEntity.ToggleInstance();
    }
	
	void LogCameraState(string context) 
	{
	    LogWorkbench("[Spectator] {" + context + "} - OGCamera: {"+OGCamera+"}, SpectatorCamera: {"+SpectatorCamera+"}, IsSpectating: {"+IsSpectating+"}");
	}
}
