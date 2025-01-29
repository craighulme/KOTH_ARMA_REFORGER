class KOTH_SpawnProtectionTriggerEntityClass : SCR_BaseTriggerEntityClass{}
class KOTH_SpawnProtectionTriggerEntity : SCR_BaseTriggerEntity
{
	// check restriction zones / players
	override protected void EOnInit(IEntity owner)
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(owner);
		if (!trigger)
			return;

		trigger.AddClassType(ChimeraCharacter);
		trigger.EnablePeriodicQueries(false);
		trigger.SetSphereRadius(200);
		trigger.SetUpdateRate(0);
	}
}
