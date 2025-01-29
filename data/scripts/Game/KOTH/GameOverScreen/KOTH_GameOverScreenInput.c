modded class GameOverScreenInput: ChimeraMenuBase
{
	override void OnMenuOpen() 
	{
		super.OnMenuOpen();
		
		KOTH_ScoringGameModeComponent scoreComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));

		Widget newRow = GetGame().GetWorkspace().CreateWidgets("{0AB65604E4B00513}UI/Layouts/HUD/GameOver/EndScreen/EndScreenBonusLine.layout", GetRootWidget());
		GetGame().GetWorkspace().CreateWidgets("{D7B478A957368083}UI/Layouts/GameOverEndScreen/EndScreenNextGameLine.layout", GetRootWidget());
		
		TextWidget text = TextWidget.Cast(newRow.FindAnyWidget("TextNotif"));
		TextWidget money = TextWidget.Cast(newRow.FindAnyWidget("MoneyNotif"));
		TextWidget xp = TextWidget.Cast(newRow.FindAnyWidget("XpNotif"));
		text.SetText("END GAME BONUS");
		
 		int playerId = GetGame().GetPlayerController().GetPlayerId();
		KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId).FindComponent(KOTH_SCR_PlayerProfileComponent));

		money.SetText(""+profileComp.GetSessionEndGameBonus()+" $");
		xp.SetText(""+profileComp.GetSessionEndGameBonus()+" xp");
	}
	
	void ShowRefund(string bonus)
	{
		Widget newRow = GetGame().GetWorkspace().CreateWidgets("{A5BBB3B32AB5DA04}UI/Layouts/GameOverEndScreen/EndScreenRefundLine.layout", GetRootWidget());
		TextWidget text = TextWidget.Cast(newRow.FindAnyWidget("TextNotif"));
		TextWidget money = TextWidget.Cast(newRow.FindAnyWidget("MoneyNotif"));
		TextWidget xp = TextWidget.Cast(newRow.FindAnyWidget("XpNotif"));
		
		text.SetText("Refund vehicle");
		money.SetText(""+bonus+" $");
		xp.SetText("");
	}
	
}