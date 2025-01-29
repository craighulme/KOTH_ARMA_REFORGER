class KOTH_VoteMapUI
{
	static Widget m_wRoot;
	static ref array<Widget> m_wdgMapList = {};
	static string m_selectedMap;

	static void ShowVoteMap()
	{
		m_wRoot = GetGame().GetWorkspace().CreateWidgets("{263604A70BD346A8}UI/Layouts/Vote/VoteMapLayout.layout");

		PlayerController controller = GetGame().GetPlayerController();
		KOTH_SCR_PlayerProfileComponent playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(controller.FindComponent(KOTH_SCR_PlayerProfileComponent));
		ShowChoices(playerProfileComp.m_mapList);
		GetGame().GetCallqueue().CallLater(RemoveUI, 28000, false);
	}
	
	static void RemoveUI()
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
	}
	
	static void ShowChoices(array<ResourceName> mapList)
	{
		foreach(int index, string mapItem : mapList)
		{
			SCR_MissionHeader mission = SCR_MissionHeader.Cast(SCR_MissionHeader.ReadMissionHeader(mapItem));
			
			if (!mission)
				continue;

			Widget container;
			if (index < 4)
				container = m_wRoot.FindAnyWidget("MapList1");
			
			if (index > 3 && index < 8)
				container = m_wRoot.FindAnyWidget("MapList2");

			if (index > 7)
				container = m_wRoot.FindAnyWidget("MapList3");
			
			// Break loop after list is full
			if (index > 11)
				break;

			
			Widget row = GetGame().GetWorkspace().CreateWidgets("{75DC434FC01BA79B}UI/Layouts/Vote/VoteMapItem.layout", container);
			m_wdgMapList.Insert(row);
			ImageWidget mapImage = ImageWidget.Cast(row.FindAnyWidget("MapImage"));
			mapImage.LoadImageTexture(0, mission.m_sPreviewImage);

			TextWidget mapTitle = TextWidget.Cast(row.FindAnyWidget("MapTitle"));
			mapTitle.SetText(mission.m_sName);
			
			TextWidget mapConf = TextWidget.Cast(row.FindAnyWidget("MapConf"));
			mapConf.SetText(mapItem);
			
			SCR_TileBaseComponent tileComp = SCR_TileBaseComponent.Cast(row.FindHandler(SCR_TileBaseComponent));
			if (tileComp) { tileComp.m_OnClick.Insert(OnClicked); }
		}
	}
	
	static void OnClicked(SCR_TileBaseComponent instance)
	{
		TextWidget mapConf = TextWidget.Cast(instance.GetRootWidget().FindAnyWidget("MapConf"));
		m_selectedMap = mapConf.GetText();
		
		PlayerController controller = GetGame().GetPlayerController();
		KOTH_SCR_PlayerProfileComponent playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(controller.FindComponent(KOTH_SCR_PlayerProfileComponent));
		playerProfileComp.AskRpc_SendVote(m_selectedMap);
		m_wRoot.RemoveFromHierarchy();
	}
}