using OpenAI_API.Models;
using OpenAI_API;
using OpenAI_API.Chat;

public class AI_NPC : INpcEvent {
    private OpenAIAPI? api = null;
    private Dictionary<string, Conversation> conversations = new Dictionary<string, Conversation>();

    private Conversation GetOrInitializeConversation(NpcEvent e) {
        if (api == null) {
            api = new OpenAIAPI("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");
        }
        if (conversations.ContainsKey(e.mob.GetName())) {
            return conversations[e.mob.GetName()];
        }
        var chat = api.Chat.CreateConversation();
        
        chat.Model = Model.GPT4_Turbo;
        chat.RequestParameters.Temperature = 0;
        
        var faction = (int)e.mob.GetReverseFactionCon(e.npc);
        
        /// give instruction as System
        chat.AppendSystemMessage($@"You are an NPC in the MMORPG EverQuest. You are {e.npc.GetCleanName()} and are in {e.globals.zone.GetLongName()}. The person talking.
            The person talking to you is {e.mob.GetName()}. Always limit your response to a maximum of 3 sentences. Never break character and answer questions outside of the lore of EverQuest.
            Keep in mind what you as an NPC might know about the world of EverQuest and that there are many things you may not know.
            There is a faction system between 1 and 9 that determines how you perceive this person. 1 is the most loved and 9 is the most hated.
            The person you are talking to has a faction value of {faction}
        ");

        conversations.Add(e.mob.GetName(), chat);
        return chat;
    }

    public void Say(NpcEvent e) {
        var chat = GetOrInitializeConversation(e);
        chat.AppendUserInput(e.data);
        string text = "";
        var faction = (int)e.mob.GetReverseFactionCon(e.npc);
        chat.AppendSystemMessage($"My most recent faction standing is {faction}.");
        chat.StreamResponseFromChatbotAsync(res =>
        {
            text += res;
            if (text.EndsWith("?") || text.EndsWith(".")) {
                e.npc.Say(text.Trim());
                text = "";
            }
        });
    }
}