typedef func Log;
typedef func LogWorkbench;

void Log(string log, LogLevel level = LogLevel.NORMAL)
{
    Print("----- KOTH: " + log, level);
}

void Log(int log, LogLevel level = LogLevel.NORMAL)
{
    Print("----- KOTH: " + log, level);
}

void Log(Managed log, LogLevel level = LogLevel.NORMAL)
{
    Print("----- KOTH: " + log, level);
}

void LogWorkbench(string log, LogLevel level = LogLevel.NORMAL)
{
    #ifdef WORKBENCH
    Print("----- KOTH (Workbench): " + log, level);
    #endif
}

void LogWorkbench(int log, LogLevel level = LogLevel.NORMAL)
{
    #ifdef WORKBENCH
    Print("----- KOTH (Workbench): " + log, level);
    #endif
}

void LogWorkbench(Managed log, LogLevel level = LogLevel.NORMAL)
{
    #ifdef WORKBENCH
    Print("----- KOTH (Workbench): " + log, level);
    #endif
}