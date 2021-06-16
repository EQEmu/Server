#ifndef EVENT_CODES_H
#define EVENT_CODES_H

typedef enum {
	EVENT_SAY = 0,
	EVENT_TRADE,		//being given an item or money
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
	EVENT_ENTER_ZONE,		//PC only, you enter zone
	EVENT_CLICK_DOOR,		//pc only, you click a door
	EVENT_LOOT,			//pc only
	EVENT_ZONE,			//pc only
	EVENT_LEVEL_UP,		//pc only
	EVENT_KILLED_MERIT, //killed by a PC or group, gave experience; will repeat several times for groups
	EVENT_CAST_ON,		//pc casted a spell on npc
	EVENT_TASK_ACCEPTED,	//pc accepted a task
	EVENT_TASK_STAGE_COMPLETE,
	EVENT_TASK_UPDATE,
	EVENT_TASK_COMPLETE,
	EVENT_TASK_FAIL,
	EVENT_AGGRO_SAY,
	EVENT_PLAYER_PICKUP,
	EVENT_POPUP_RESPONSE,
	EVENT_ENVIRONMENTAL_DAMAGE,
	EVENT_PROXIMITY_SAY,
	EVENT_CAST,
	EVENT_CAST_BEGIN,
	EVENT_SCALE_CALC,
	EVENT_ITEM_ENTER_ZONE,
	EVENT_TARGET_CHANGE,	//target selected, target changed, or target removed
	EVENT_HATE_LIST,
	EVENT_SPELL_EFFECT_CLIENT,
	EVENT_SPELL_EFFECT_NPC,
	EVENT_SPELL_BUFF_TIC_CLIENT,
	EVENT_SPELL_BUFF_TIC_NPC,
	EVENT_SPELL_FADE,
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
	EVENT_ENCOUNTER_LOAD,
	EVENT_ENCOUNTER_UNLOAD,
	EVENT_COMMAND,
	EVENT_DROP_ITEM,
	EVENT_DESTROY_ITEM,
	EVENT_FEIGN_DEATH,
	EVENT_WEAPON_PROC,
	EVENT_EQUIP_ITEM,
	EVENT_UNEQUIP_ITEM,
	EVENT_AUGMENT_ITEM,
	EVENT_UNAUGMENT_ITEM,
	EVENT_AUGMENT_INSERT,
	EVENT_AUGMENT_REMOVE,
	EVENT_ENTER_AREA,
	EVENT_LEAVE_AREA,
	EVENT_RESPAWN,
	EVENT_DEATH_COMPLETE,
	EVENT_UNHANDLED_OPCODE,
	EVENT_TICK,
	EVENT_SPAWN_ZONE,
	EVENT_DEATH_ZONE,
	EVENT_USE_SKILL,
	EVENT_COMBINE_VALIDATE,
	EVENT_BOT_COMMAND,
	EVENT_TEST_BUFF,
	_LargestEventID
} QuestEventID;

extern const char *QuestEventSubroutines[_LargestEventID];

#endif

