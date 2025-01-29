modded class SCR_EditorManagerEntity
{
	// remove arma vision rights
	override SCR_EditorModeEntity CreateEditorMode(EEditorMode mode, bool isInit, ResourceName prefab = "")
	{
		SCR_EditorModeEntity modeEntity = super.CreateEditorMode(mode, isInit, prefab);
		
		foreach (SCR_EditorModeEntity curmode : m_Modes)
		{
			if (curmode.GetModeType() == EEditorMode.PHOTO)
			{
				RemoveMode(curmode, false);
			}
		}

		return modeEntity;
	}
	
	override protected void AddMode(notnull SCR_EditorModeEntity modeEntity, bool isInit)
	{
		//--- Mode already registered, ignore
		if (m_Modes.Find(modeEntity) >= 0) return;
		
		int order = modeEntity.GetOrder();
		if (order < 0)
		{
			//--- When default order is used, place at the back
			m_Modes.Insert(modeEntity);
		}
		else
		{
			//--- Place according to custom order
			int index = 0;
			for (int c = m_Modes.Count() - 1; c >= 0; c--)
			{
				index = c;
				if (m_Modes[c] && order > m_Modes[c].GetOrder())
				{
					index++;
					break;
				}
			}
			m_Modes.InsertAt(modeEntity, index);
		}
		
		//removed Notification for sneaky admins 	
			
		UpdateLimited();
		Event_OnModeAdd.Invoke(modeEntity);
	}
	
	override void RemoveMode(notnull SCR_EditorModeEntity modeEntity, bool OnDisconnnect)
	{
		if (!m_Modes.Contains(modeEntity))
			return;
		
		if (IsOwner())
		{
			m_ProcessedMode = modeEntity.GetModeType();
			StartEvents(EEditorEventOperation.MODE_DELETE);
			Rpc(RemoveModeServer, modeEntity.GetModeType());
		}
		
		m_Modes.RemoveItem(modeEntity);
		UpdateLimited();
		
		Event_OnModeRemove.Invoke(modeEntity);
	
		// removed notification of removing arma vision spam
	}
}