modded class SCR_BudgetEditorComponent
{
    override void OnPostInit(IEntity owner)
    {
        // Prevent any initialization logic from running
        if (!m_Owner)
            return;

        // Unregister all events and clear all budget-related settings
        Deactivate(owner);
        ClearAllEvents();

        // Ensure no delayed calls are processed
        m_DelayedSetMaxBudgetsMap.Clear();
        m_bListenToMaxBudgetDelay = false;

        // Clear the budget settings map to prevent any logic
        m_BudgetSettingsMap.Clear();
    }

    protected void ClearAllEvents()
    {
        // Disable budget-related events
        if (Event_OnBudgetUpdated)
            Event_OnBudgetUpdated.Clear();

        if (Event_OnBudgetMaxReached)
            Event_OnBudgetMaxReached.Clear();

        if (Event_OnBudgetMaxUpdated)
            Event_OnBudgetMaxUpdated.Clear();

        if (Event_OnBudgetPreviewUpdated)
            Event_OnBudgetPreviewUpdated.Clear();

        if (Event_OnBudgetPreviewReset)
            Event_OnBudgetPreviewReset.Clear();
    }

    override protected void EOnEditorActivate()
    {
        // Prevent activation
    }

    override protected void EOnEditorInit()
    {
        // Prevent initialization logic
    }

    override protected void EOnEditorInitServer()
    {
        // Prevent server initialization logic
    }

    override protected void EOnEditorDeleteServer()
    {
        // Prevent server delete logic
    }

    override bool IsBudgetCapEnabled()
    {
        // Disable the budget cap entirely
        return false;
    }

    override void RefreshBudgetSettings()
    {
        // Prevent any budget settings from being refreshed
        m_BudgetSettingsMap.Clear();
    }

    override void SetMaxBudgetValue(EEditableEntityBudget type, int newValue)
    {
        // Prevent any max budget updates
    }

    override bool CanPlaceEntityInfo(SCR_EditableEntityUIInfo info, out EEditableEntityBudget blockingBudget, bool showNotification)
    {
        // Always allow placement
        return true;
    }

    override bool CanPlace(notnull array<ref SCR_EntityBudgetValue> budgetCosts, out EEditableEntityBudget blockingBudget)
    {
        // Always allow placement
        return true;
    }

    override void RpcServer_UpdateBudget()
    {
        // Prevent RPC calls
    }

    override void RpcOwner_UpdateBudget(EEditableEntityBudget budgetType, int currentBudget)
    {
        // Prevent RPC calls
    }

    override void OnEntityCoreBudgetUpdated(EEditableEntityBudget entityBudget, int originalBudgetValue, int budgetChange, int updatedBudgetValue, SCR_EditableEntityComponent entity)
    {
        // Prevent budget update logic
    }

    override void OnEntityCoreBudgetUpdatedOwner(EEditableEntityBudget entityBudget, int budgetValue, int budgetChange, bool sendBudgetMaxEvent, bool budgetMaxReached)
    {
        // Prevent budget update logic
    }
	
	override void GetEntityTypeBudgetCost(EEditableEntityType entityType, out array<ref SCR_EntityBudgetValue> budgetCosts)
	{
	    // Avoid null pointer errors
	    if (!m_EntityCore)
	    {
	        budgetCosts = {};
	        return;
	    }
	
	    EEditableEntityBudget entityBudgetType = m_EntityCore.GetBudgetForEntityType(entityType);
	    int minBudgetCost = GetEntityTypeBudgetCost(entityBudgetType);
	    
	    budgetCosts = { new SCR_EntityBudgetValue(entityBudgetType, minBudgetCost) };
	}
	
	override bool GetEntityPreviewBudgetCosts(SCR_EditableEntityUIInfo entityUIInfo, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
	    // Provide fallback behavior
	    if (!entityUIInfo || !m_EntityCore)
	    {
	        budgetCosts = {};
	        return false;
	    }
	
	    // Original logic
	    return super.GetEntityPreviewBudgetCosts(entityUIInfo, budgetCosts);
	}
}
