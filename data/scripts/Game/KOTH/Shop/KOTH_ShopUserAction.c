class KOTH_ShopWeaponUserAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_PlayerController scrPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!scrPlayerController || pUserEntity != scrPlayerController.GetMainEntity())
			return;

		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTH_ShopWeapon);
	}
}
class KOTH_ShopWeaponRearmUserAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_PlayerController scrPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!scrPlayerController || pUserEntity != scrPlayerController.GetMainEntity())
			return;

		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTH_ShopWeaponRearm);
	}
}

class KOTH_ShopVehicleUserAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_PlayerController scrPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!scrPlayerController || pUserEntity != scrPlayerController.GetMainEntity())
			return;

		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTH_ShopVehicle);
	}
}