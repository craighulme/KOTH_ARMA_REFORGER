class KOTH_SpawnPrefabClass : SCR_BaseTriggerEntityClass{}
class KOTH_SpawnPrefab : SCR_BaseTriggerEntity
{
	private string m_sFaction;
	string GetFactionKey(){ return m_sFaction; }

	protected KOTH_ScoringGameModeComponent m_scoreComp;
	protected ref array<Vehicle> m_armedVehicles = {};
	
	override protected event void OnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode() || !Replication.IsServer())
			return;

		m_scoreComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));
		GetGame().GetCallqueue().CallLater(CheckDeletedArmedVehicles, 30000, true);
	}
	
	bool isSpawnEmpty()
	{
		QueryEntitiesInside();
		array<IEntity> outEntities = {};
		GetEntitiesInside(outEntities);
		if (outEntities.Count() > 0) {
			return false;
		}
		
		return true;
	}
	
	bool Spawn(ResourceName m_prefabName, bool isArmed, int playerId)
	{
		vector pos;
		IEntity entity = this;
		bool isSpawnEmpty = isSpawnEmpty();
		if (!isSpawnEmpty)
		{
			IEntity child = GetChildren();
			KOTH_SpawnPrefab kothSP = KOTH_SpawnPrefab.Cast(child);
			isSpawnEmpty = kothSP.isSpawnEmpty();
			if (!isSpawnEmpty)
			{
				for (int i = 0; i < 100; i++)
				{
					child = child.GetSibling();
					kothSP = KOTH_SpawnPrefab.Cast(child);
					if (!kothSP)
						return false;
					
					isSpawnEmpty = kothSP.isSpawnEmpty();
					if (isSpawnEmpty)
						break;
				}
			}
			
			entity = child;
			if (!isSpawnEmpty)
				return false;
		}
		
		SCR_WorldTools.FindEmptyTerrainPosition(pos, entity.GetOrigin(), 5);
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		entity.GetTransform(params.Transform);
		
		IEntity newEnt = GetGame().SpawnEntityPrefab(Resource.Load(m_prefabName), null, params);
		
		if (!newEnt)
			return false;
		
		CarControllerComponent carController = CarControllerComponent.Cast(newEnt.FindComponent(CarControllerComponent));
		
		// Activate handbrake so the vehicles don't go downhill on their own when spawned
		if (carController)
			carController.SetPersistentHandBrake(true);
		
		Physics physicsComponent = newEnt.GetPhysics();
		
		// Snap to terrain
		if (physicsComponent)
			physicsComponent.SetVelocity("0 -1 0");

		newEnt.SetFlags(EntityFlags.VISIBLE, true);
		
		Vehicle spawnedVehicle = Vehicle.Cast(newEnt);
		spawnedVehicle.SetOwnerUID(KOTH_Helper.GetPlayerUID(playerId));
		Log("attach vehicle for playerId "+playerId+ " uid "+KOTH_Helper.GetPlayerUID(playerId));
		
		
		if (isArmed)
			m_armedVehicles.Insert(spawnedVehicle);

		// lock bought vehicle to buyer
		SCR_VehicleSpawnProtectionComponent protectionComp = SCR_VehicleSpawnProtectionComponent.Cast(newEnt.FindComponent(SCR_VehicleSpawnProtectionComponent));
        if (protectionComp)
        {
            protectionComp.SetVehicleOwner(playerId);
            protectionComp.SetProtectionTime(60);
        }
		
		return true;
	}
	
	void CheckDeletedArmedVehicles()
	{
		foreach(int index, Vehicle vec : m_armedVehicles)
		{
			if (!vec)
				m_armedVehicles.Remove(index);
		}
		
		UpdateCountFromList();
	}
	
	void UpdateCountFromList()
	{
		switch (m_sFaction)
		{
			case KOTH_Faction.BLUFOR:
				m_scoreComp.m_bluforArmedVehiclesCount = m_armedVehicles.Count();
			break;
			case KOTH_Faction.OPFOR:
				m_scoreComp.m_opforArmedVehiclesCount = m_armedVehicles.Count();
			break;
			case KOTH_Faction.INDFOR:
				m_scoreComp.m_indforArmedVehiclesCount = m_armedVehicles.Count();
			break;
		}
	}
	
	void CheckAddedArmedVehicles(Vehicle addedVec)
	{
		bool alreadyInList = false;
		foreach(int index, Vehicle vec : m_armedVehicles)
		{
			if (vec == addedVec)
			{
				alreadyInList = true;
			}
		}
		
		if (alreadyInList)
			return;
		
		m_armedVehicles.Insert(addedVec);
		UpdateCountFromList();
	}
	
	void SetFactionKey(string factionKey)
	{
		if (factionKey == m_sFaction)
			return;

		m_sFaction = factionKey;
	}
};
