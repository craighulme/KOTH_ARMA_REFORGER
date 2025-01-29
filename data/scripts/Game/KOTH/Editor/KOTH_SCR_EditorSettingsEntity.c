modded class SCR_EditorSettingsEntity
{
    override bool GetSkipStreamingRules(notnull array<EEditableEntityType> skipStreamingRules)
    {
		//commented out line that errors
        //skipStreamingRules.InsertAll(m_SkipStreamingRules);
        //return !m_SkipStreamingRules.IsEmpty();
		
		//return false so we assume its always empty
		return false;
    }
}