[EntityEditorProps(category: "GameScripted/Scoreboard", description: "Scoreboard component")]
class KOTH_ScoreboardComponentClass : ScriptComponentClass { }
class KOTH_ScoreboardComponent : ScriptComponent
{
	SoundComponent m_soundComponent;
	TextWidget m_MaintextWidget;
	TextWidget m_leveltextWidget;
	TextWidget m_ZonetextWidget;
	ImageWidget m_ImageWidget;
	RTTextureWidget m_renderTargetTexture;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Path to layout file to create", params: "layout")]
	ResourceName m_sLayoutPath;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode())
			return;

		Widget root = GetGame().GetWorkspace().CreateWidgets(m_sLayoutPath);
		if (!root)
		{
			Log("KOTH_ScoreboardComponent | Unable to create layout", LogLevel.ERROR);
			return;
		}

		m_renderTargetTexture = RTTextureWidget.Cast(root.FindAnyWidget("RTTexture0"));
		if (!m_renderTargetTexture)
		{
			Log("KOTH_ScoreboardComponent | Couldn't find RT Texture widget", LogLevel.ERROR);
			return;
		}

		m_renderTargetTexture.SetGUIWidget(owner, 0);
	}

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	};
}
