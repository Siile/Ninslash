#include "questinfo.h"


const char *GetQuestDisplayName(int Quest)
{
	switch (Quest)
	{
		case QUEST_KILLREMAININGENEMIES: return "Terminate the enemies";
		case QUEST_REACHDOOR: return "Reach the door";
		case QUEST_SURVIVEWAVE: return "Survive the wave of enemies";
		case QUEST_SURVIVEWAVETIME: return "Survive the wave of enemies";
		default: return "";
	}
}

const char *GetQuestStartMessage(int Quest, int WaveType)
{
	if (Quest == QUEST_SURVIVEWAVE || Quest == QUEST_SURVIVEWAVETIME)
	{
		switch (WaveType)
		{
			case WAVE_ALIENS: return "Wave of aliens incoming";
			case WAVE_ROBOTS: return "Wave of robots incoming";
			case WAVE_SKELETONS: return "Wave of skeletons incoming";
			case WAVE_FURRIES: return "Wave of furries incoming";
			case WAVE_CYBORGS: return "Wave of cyborgs incoming";
			default: return "Wave incoming";
		}
	}
	
	if (Quest == QUEST_KILLREMAININGENEMIES)
	{
		switch (WaveType)
		{
			case WAVE_ALIENS: return "Terminate the aliens";
			case WAVE_ROBOTS: return "Terminate the robots";
			case WAVE_SKELETONS: return "Terminate the skeletons";
			case WAVE_FURRIES: return "Terminate the furries";
			case WAVE_CYBORGS: return "Terminate the cyborgs";
			default: return "Terminate the enemies";
		}
	}
	
	switch (Quest)
	{
		case QUEST_KILLREMAININGENEMIES: return "Terminate the enemies";
		case QUEST_REACHDOOR: return "Seek the door";
		case QUEST_SURVIVEWAVE: return "Wave incoming";
		case QUEST_SURVIVEWAVETIME: return "Wave incoming";
		default: return "";
	}
}

const char *GetQuestCompletedMessage(int Quest, int WaveType)
{
	if (Quest == QUEST_SURVIVEWAVE || Quest == QUEST_SURVIVEWAVETIME)
	{
		switch (WaveType)
		{
			case WAVE_ALIENS: return "Alien wave cleared";
			case WAVE_ROBOTS: return "Robot wave cleared";
			case WAVE_FURRIES: return "Furry wave cleared";
			case WAVE_SKELETONS: return "Skeleton wave cleared";
			case WAVE_CYBORGS: return "Cyborg wave cleared";
			default: return "Wave cleared";
		}
	}
	
	if (Quest == QUEST_KILLREMAININGENEMIES)
	{
		switch (WaveType)
		{
			case WAVE_ALIENS: return "Aliens terminated";
			case WAVE_ROBOTS: return "Robots terminated";
			case WAVE_SKELETONS: return "Skeletons terminated";
			case WAVE_FURRIES: return "Furries terminated";
			case WAVE_CYBORGS: return "Cyborgs terminated";
			default: return "Enemies terminated";
		}
	}
	
	switch (Quest)
	{
		case QUEST_REACHDOOR: return "";
		default: return "";
	}
}