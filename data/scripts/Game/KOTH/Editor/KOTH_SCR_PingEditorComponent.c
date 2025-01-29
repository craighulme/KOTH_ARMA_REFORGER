modded class SCR_PingEditorComponent : SCR_BaseEditorComponent
{
	//disable ability for players to ping GM to see if they are online. Also is annoying
	override void SendPing(bool unlimitedOnly = false, vector position = vector.Zero, SCR_EditableEntityComponent target = null)
	{
		// Always send out that GM is not online
		SCR_PlayerDelegateEditorComponent playerDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
		if (playerDelegateManager)
		{
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PING_NO_GM_TO_PING);
			return;
		}
	}
	

}