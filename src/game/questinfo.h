#ifndef GAME_QUESTINFO_H
#define GAME_QUESTINFO_H

#include <cstring>
#include <generated/protocol.h>



enum Quests
{
	QUEST_NONE,
	QUEST_KILLREMAININGENEMIES,
	QUEST_REACHDOOR,
	QUEST_SURVIVEWAVE,
	QUEST_SURVIVEWAVETIME,
};

enum WaveTypes
{
	WAVE_NONE,
	WAVE_ALIENS,
	WAVE_ROBOTS,
	WAVE_SKELETONS,
	WAVE_FURRIES,
	WAVE_CYBORGS,
	NUM_WAVES,
};


const char *GetQuestDisplayName(int Quest);
const char *GetQuestStartMessage(int Quest, int WaveType = WAVE_NONE);
const char *GetQuestCompletedMessage(int Quest, int WaveType = WAVE_NONE);

#endif
