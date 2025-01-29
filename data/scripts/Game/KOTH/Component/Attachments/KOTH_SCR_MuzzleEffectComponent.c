modded class SCR_MuzzleEffectComponent
{
	// thx Zelik for the idea of handling it this way
	// Character subcribe to delete projectile when entering/activate the player protection trigger

	protected ref ScriptInvoker m_OnFired = new ScriptInvoker();
	protected bool isSubscribed = false;
	
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
    {
		super.OnFired(effectEntity, muzzle, projectileEntity);

		m_OnFired.Invoke(projectileEntity);
    }

	void Subscribe(bool doSubscribe)
    {
        if (doSubscribe)
        {
			if (isSubscribed)
				return;

            m_OnFired.Insert(DeleteProjectile);
			isSubscribed = true;
        }
		else
		{
			if (!isSubscribed)
				return;

	        m_OnFired.Remove(DeleteProjectile);
			isSubscribed = false;
		}
    }

	void DeleteProjectile(IEntity ent)
    {
        SCR_EntityHelper.DeleteEntityAndChildren(ent);
    }  
}
