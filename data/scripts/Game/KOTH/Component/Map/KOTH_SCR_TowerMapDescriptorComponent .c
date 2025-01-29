[ComponentEditorProps(category: "GameScripted/KOTH", description: "tower map markers")]
class KOTH_SCR_TowerMapDescriptorComponentClass : SCR_MapDescriptorComponentClass{};
class KOTH_SCR_TowerMapDescriptorComponent : SCR_MapDescriptorComponent
{
	protected MapItem m_item;

	[Attribute("", UIWidgets.EditBox, desc: "imageset quad koth")]
	string m_sImageQuadKoth;

	void KOTH_SCR_TowerMapDescriptorComponent (IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		GetGame().GetCallqueue().CallLater(CreateMarker, 500, false);
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
		m_item.SetImageDef(m_sImageQuadKoth);
		m_item.SetPriority(1);

		MapDescriptorProps props = m_item.GetProps();
		props.SetDetail(96);
		props.SetIconSize(128, 0.33, 0.33);
		props.SetFrontColor(Color.White);
		props.SetTextVisible(false);
		props.SetIconVisible(true);
		props.Activate(true);

		m_item.SetProps(props);
	}
}
