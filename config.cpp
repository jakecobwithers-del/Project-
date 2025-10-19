class CfgPatches
{
    class SloppyBuildKits
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"LBmaster_GroupsServer"}; // Required dependency
    };
};

class CfgMods
{
    class SloppyBuildKits
    {
        dir = "SloppyBuildKits";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "SloppyBuildKits";
        credits = "YourName";
        author = "YourName";
        authorID = "";
        version = "1.0";
        extra = 0;
        type = "mod";
        
        dependencies[] = {"World"};
        
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"SloppyBuildKits/Scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"SloppyBuildKits/Scripts/5_Mission"};
            };
        };
    };
};
