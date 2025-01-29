class HudSettings : JsonApiStruct
{
    bool showTeamNames;
    bool showScoreUI;
    bool showWayPointUI;

    void HudSettings()
    {
        showTeamNames = true;
        showScoreUI = true;
        showWayPointUI = true;

        RegV("showTeamNames");
        RegV("showScoreUI");
        RegV("showWayPointUI");
    }

    static HudSettings CreateDefault()
    {
        HudSettings defaultSettings = new HudSettings();
        defaultSettings.showTeamNames = true;
        defaultSettings.showScoreUI = true;
        defaultSettings.showWayPointUI = true;
        return defaultSettings;
    }
}