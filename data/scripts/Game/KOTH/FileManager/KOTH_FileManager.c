class KOTH_FileManager
{
    private static const string FILE_PATH = "$profile:KOTH_Settings.json";
    private ref ConfigSettings settings;

    void KOTH_FileManager()
    {
        settings = new ConfigSettings();
        LoadSettings();
    }

    // Load settings from JSON file
    void LoadSettings()
    {
        if (!FileIO.FileExists(FILE_PATH))
        {
            LogWorkbench("KOTH_FileManager: No settings file found. Using defaults.");
            settings = new ConfigSettings();
            return;
        }

        if (settings.LoadFromFile(FILE_PATH))
        {
            LogWorkbench("KOTH_FileManager: Settings loaded successfully from: " + FILE_PATH);
        }
        else
        {	
			//fallback if corrupted or unreadable.. console?
            LogWorkbench("KOTH_FileManager: Failed to load settings from: " + FILE_PATH);
            settings = new ConfigSettings();
        }
    }

    // Save settings to JSON file
    void SaveSettings()
    {
        if (settings.PackToFile(FILE_PATH))
        {
            LogWorkbench("KOTH_FileManager: Settings saved successfully to: " + FILE_PATH);
        }
        else
        {
            LogWorkbench("KOTH_FileManager: Failed to save settings to: " + FILE_PATH);
        }
    }

    HudSettings GetHudSettings()
	{
	    HudSettings hudSettings = HudSettings.Cast(settings.hudSettings);
	    if (!hudSettings)
	    {
	        // Create default settings if none exist
			LogWorkbench("KOTH_FileManager: Creating Defaults for HudSettings as they do not exist.");
	        hudSettings = HudSettings.CreateDefault();
	        settings.hudSettings = hudSettings;
	        SaveSettings();
	    }
	    return hudSettings;
	}

   
}

class ConfigSettings : JsonApiStruct
{
    ref HudSettings hudSettings;
    //ref array<ref WeaponLoadout> weaponLoadouts;

    void ConfigSettings()
    {
        RegV("hudSettings");
        //RegV("weaponLoadouts");
    }
}