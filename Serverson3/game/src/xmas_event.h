#ifndef __INC_XMAS_EVENT_H
#define __INC_XMAS_EVENT_H

namespace xmas
{
	enum
	{
		MOB_SANTA_VNUM = 20031,
		MOB_XMAS_TREE_VNUM = 20032,
		MOB_XMAS_FIRWORK_SELLER_VNUM = 9004,
	};

	void ProcessEventFlag(const std::string& name, int prev_value, int value);
	void SpawnSanta(long lMapIndex, int iTimeGapSec);
	void SpawnEventHelper(bool spawn);
}
#endif
//martysama0134's 8e0aa8057d3f54320e391131a48866b4
