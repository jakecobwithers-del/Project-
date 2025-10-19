class KitItem
{
    string ClassName;
    int Quantity;
    float Health = -1;
    ref array<ref KitItem> Attachments;
    ref array<ref KitItem> Contents;

    void KitItem(string className = "", int quantity = 1, float health = -1)
    {
        ClassName = className;
        Quantity = quantity;
        Health = health;
    }

    void AddAttachment(KitItem attachment)
    {
        if (!Attachments)
            Attachments = new array<ref KitItem>;
        Attachments.Insert(attachment);
    }

    void AddContent(KitItem content)
    {
        if (!Contents)
            Contents = new array<ref KitItem>;
        Contents.Insert(content);
    }
}

class Kit
{
    string Name;
    ref array<ref KitItem> Items;
    bool OneTimeOnly = false;
    float CooldownHours = 0;
    string RequiredPermission = "";
    ref map<string, string> Messages;

    void Kit(string name)
    {
        Name = name;
        Items = new array<ref KitItem>;
        Messages = new map<string, string>;
    }
}
