[ComponentEditorProps(category: "GameScripted/KOTH", description: "priority area map marker")]
class KOTH_SCR_PriorityAreaMapDescriptorComponentClass : SCR_MapDescriptorComponentClass{};
class KOTH_SCR_PriorityAreaMapDescriptorComponent : SCR_MapDescriptorComponent
{	
	protected MapItem m_item;
	protected SCR_MapEntity m_MapEntity;
	
	void KOTH_SCR_PriorityAreaMapDescriptorComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		GetGame().GetCallqueue().CallLater(CreateMarker, 500, false);
		m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
	
	void CreateMarker()
	{
		m_item = Item();
		if (!m_item)
		{
			GetGame().GetCallqueue().CallLater(CreateMarker, 1000, false);
			return;
		}

		m_item.SetBaseType(EMapDescriptorType.MDT_ICON);
		m_item.SetImageDef("KOTH_Zone");
		m_item.SetPriority(10);

		MapDescriptorProps props = m_item.GetProps();
		Color color = Color.Yellow;
		color.SetA(0.5);
		
		props.SetDetail(96);
		props.SetIconSize(256, 1, 1);
		props.SetFrontColor(color);
		props.SetTextVisible(true);
		props.SetIconVisible(true);
		props.Activate(true);

		m_item.SetProps(props);
		
		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return;
	}
}
