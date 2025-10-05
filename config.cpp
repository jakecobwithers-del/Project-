class CfgPatches
{
    class BuildKit
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "LBmaster_GroupsServer"};
    };
};

class CfgMods
{
    class BuildKit
    {
        dir = "BuildKit";
        picture = "";
        action = "";
        hideName = 0;
        hidePicture = 0;
        name = "BuildKit";
        credits = "YourName";
        author = "YourName";
        version = "2.0";
        type = "mod";
        dependencies[] = {"Mission"};

        class defs
        {
            class missionScriptModule
            {
                value = "";
                files[] = {"BuildKit/scripts/5_Mission"};
            };
        };
    };
};
