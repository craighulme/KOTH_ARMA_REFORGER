[BaseContainerProps(configRoot:true)]
class KOTH_Scenarios
{
	[Attribute("", UIWidgets.Auto)]
	protected ref array<ref KOTH_ScenarioEntry> m_items;

	array<ref KOTH_ScenarioEntry> GetItems()
	{
		return m_items;
	}
}

[BaseContainerProps()]
class KOTH_ScenarioEntry
{
	[Attribute("", UIWidgets.Auto, params: "conf")]
	protected ref array<ResourceName> m_items;

	array<ResourceName> GetItems()
	{
		return m_items;
	}
}