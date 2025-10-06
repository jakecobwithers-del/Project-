#define _ARMA_

class CfgPatches
{
    class lbbuildkit
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = { "DZ_Data", "LBmaster_AdminTools", "LBmaster_GroupsServer" };
        type = "mod";
    };
};

class CfgMods
{
    class lbbuildkit
    {
        dir = "lbbuildkit";
        name = "LB Build Kit";
        author = "sloppy";
        version = "1.0";
        type = "mod";
        dependencies[] = { "Game", "World", "Mission" };
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = { "lbbuildkit/scripts/3_Game" };
            };
            class worldScriptModule
            {
                value = "";
                files[] = { "lbbuildkit/scripts/4_World" };
            };
            class missionScriptModule
            {
                value = "";
                files[] = { "lbbuildkit/scripts/5_Mission" };
            };
        };
    };
};
