class KOTH_PriorityAreaPresenceTriggerEntityClass : SCR_BaseTriggerEntityClass{}
class KOTH_PriorityAreaPresenceTriggerEntity : SCR_BaseTriggerEntity
{
	SCR_BaseGameMode m_gameMode;
	
	override protected void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(owner);
		if (!trigger)
			return;

		m_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_gameMode.m_kothPriorityTrigger = this;
	}
}
