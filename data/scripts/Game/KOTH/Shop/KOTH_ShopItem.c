[BaseContainerProps(configRoot:true)]
class KOTH_ShopItem
{
	[Attribute("baguette", desc: "name")]
	string m_itemName;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(KOTH_ShopItemCategory))]
	KOTH_ShopItemCategory m_category;
	
	[Attribute(desc: "Prefab of the Arsenal item", params: "et")]
	ResourceName m_itemResource;
	
	[Attribute(desc: "Prefab of the magazine", params: "et")]
	ResourceName m_magazineResource;
	
	[Attribute("5", desc: "Number of magazines added when equiped")]
	int m_magazineNumber;
	
	[Attribute(desc: "Prefab of the secondary magazine", params: "et")]
	ResourceName m_secondaryMagazineResource;

	[Attribute("5", desc: "Number of magazines added when equiped")]
	int m_secondaryMagazineNumber;

	[Attribute("1000", desc: "Price buy once")]
	int m_priceOnce;

	[Attribute("100000", desc: "Price buy permanent")]
	int m_pricePermanent;
	
	[Attribute("100", desc: "Re-Arm Cost (Vehicles/Weapon)")]
	int m_rearmCost;

	[Attribute("5", desc: "Level required to buy the item")]
	int m_level;
}

enum KOTH_ShopItemCategory
{
	Handgun,
	Primary,
	Launcher,
	Optics,
	Muzzle,
	Bayonet,
	Flashlight,
	Binocular,
	Grenade,
	Smoke,
	Mine,
	Vehicle,
	VehicleArmed,
	Armor,
	Accessory,
	ViperHood
}
