modded class SCR_LootDeadBodyAction
{
	// disalow looting when uncon
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return false;
		
		// Disallow looting when alive
		CharacterControllerComponent contr = char.GetCharacterController();
		if (!contr)
			return false;

		if (contr.GetLifeState() == ECharacterLifeState.INCAPACITATED)
			return false;
		
		return super.CanBePerformedScript(user);
	}
}
