#ifndef EVENT_CODES_H
#define EVENT_CODES_H

typedef enum {
	EVENT_SAY = 0,
	EVENT_ITEM,			//being given an item
	EVENT_DEATH,		//being killed
	EVENT_SPAWN,		//triggered when we first spawn
	EVENT_ATTACK,		//being attacked (resets after an interval of not being attacked)
	EVENT_COMBAT,		//being attacked or attacking (resets after an interval of not being attacked)
	EVENT_AGGRO,		//entering combat mode due to a PC attack
	EVENT_SLAY,			//killing a PC
	EVENT_NPC_SLAY,		//killing an NPC
	EVENT_WAYPOINT_ARRIVE,	// reaching a waypoint on a grid
	EVENT_WAYPOINT_DEPART,	// departing a waypoint on a grid
	EVENT_TIMER,
	EVENT_SIGNAL,
	EVENT_HP,
	EVENT_ENTER,		//PC entering your set proximity
	EVENT_EXIT,			//PC leaving your set proximity
	EVENT_ENTERZONE,		//PC only, you enter zone
	EVENT_CLICKDOOR,		//pc only, you click a door
	EVENT_LOOT,			//pc only
	EVENT_ZONE,			//pc only
	EVENT_LEVEL_UP,		//pc only
	EVENT_KILLED_MERIT, //killed by a PC or group, gave experience; will repeat several times for groups
	EVENT_CAST_ON,		//pc casted a spell on npc
	EVENT_TASKACCEPTED,	//pc accepted a task
	EVENT_TASK_STAGE_COMPLETE,
	EVENT_TASK_UPDATE,
	EVENT_TASK_COMPLETE,
	EVENT_TASK_FAIL,
	EVENT_AGGRO_SAY,
	EVENT_PLAYER_PICKUP,
	EVENT_POPUPRESPONSE,
	EVENT_PROXIMITY_SAY,
	EVENT_CAST,
	EVENT_SCALE_CALC,
	EVENT_ITEM_ENTERZONE,
	EVENT_TARGET_CHANGE,	//target selected, target changed, or target removed
	EVENT_HATE_LIST,
	EVENT_SPELL_EFFECT_CLIENT,
	EVENT_SPELL_EFFECT_NPC,
	EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT,
	EVENT_SPELL_EFFECT_BUFF_TIC_NPC,
	EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE,
	EVENT_COMBINE_SUCCESS, //PC successfully combined a recipe
	EVENT_COMBINE_FAILURE, //PC failed to combine a recipe
	EVENT_ITEM_CLICK,	//SoF+ Item Right Clicked from worn or main/top inventory slot
	EVENT_ITEM_CLICK_CAST,
	EVENT_GROUP_CHANGE,
	EVENT_FORAGE_SUCCESS,
	EVENT_FORAGE_FAILURE,
	EVENT_FISH_START,
	EVENT_FISH_SUCCESS,
	EVENT_FISH_FAILURE,
	EVENT_CLICK_OBJECT,
	EVENT_DISCOVER_ITEM,
	EVENT_DISCONNECT,
	EVENT_CONNECT,
	EVENT_ITEM_TICK,
	EVENT_DUEL_WIN,
	EVENT_DUEL_LOSE,

	_LargestEventID
} QuestEventID;

extern const char *QuestEventSubroutines[_LargestEventID];

#endif

