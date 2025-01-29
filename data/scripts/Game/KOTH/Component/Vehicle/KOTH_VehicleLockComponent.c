class KOTH_SCR_VehicleLockComponentClass : ScriptComponentClass {}
class KOTH_SCR_VehicleLockComponent : ScriptComponent
{
	[RplProp()]
	private bool isLocked = false;
	
	bool IsLocked() { return isLocked; }
	
	void ToggleLock() { isLocked = !isLocked; Replication.BumpMe(); }
}