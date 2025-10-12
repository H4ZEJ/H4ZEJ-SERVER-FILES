#ifndef __INC_METIN2_COMMON_DEFINES_H__
#define __INC_METIN2_COMMON_DEFINES_H__
#pragma once

//////////////////////////////////////////////////////////////////////////
// ### Standard Features ###
//#define _IMPROVED_PACKET_ENCRYPTION_
#ifdef _IMPROVED_PACKET_ENCRYPTION_
#define USE_IMPROVED_PACKET_DECRYPTED_BUFFER
#else
#define USE_NO_PACKET_ENCRYPTION
#endif

// ### END Standard Features ###
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ### New Features ###
#define ENABLE_NO_MOUNT_CHECK
#define ENABLE_D_NJGUILD
#define ENABLE_FULL_NOTICE
#define ENABLE_NEWSTUFF
#define ENABLE_PORT_SECURITY
#define ENABLE_BELT_INVENTORY_EX
#define ENABLE_CMD_WARP_IN_DUNGEON
// #define ENABLE_ITEM_ATTR_COSTUME
#define ENABLE_DICE_SYSTEM
#define ENABLE_EXTEND_INVEN_SYSTEM
#define ENABLE_WEAPON_COSTUME_SYSTEM
#define ENABLE_QUEST_DIE_EVENT
#define ENABLE_QUEST_BOOT_EVENT
#define ENABLE_QUEST_DND_EVENT
#define ENABLE_SKILL_FLAG_PARTY
#define ENABLE_NO_DSS_QUALIFICATION
// #define ENABLE_NO_SELL_PRICE_DIVIDED_BY_5
#define ENABLE_CHECK_SELL_PRICE
#define ENABLE_GOTO_LAG_FIX
#define ENABLE_MOVE_CHANNEL
#define ENABLE_QUIVER_SYSTEM
#define ENABLE_REDUCED_ENTITY_VIEW
#define ENABLE_GUILD_TOKEN_AUTH
// #define ENABLE_DS_GRADE_MYTH
#define ENABLE_DB_SQL_LOG

#define __PET_SYSTEM__

enum eCommonDefines {
	MAP_ALLOW_LIMIT = 32, // 32 default
};

// ### END New Features ###
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ### Ex Features ###
#define DISABLE_STOP_RIDING_WHEN_DIE //	if DISABLE_TOP_RIDING_WHEN_DIE is defined, the player doesn't lose the horse after dying
#define ENABLE_HIGHLIGHT_NEW_ITEM //if you want to see highlighted a new item when dropped or when exchanged
#define ENABLE_KILL_EVENT_FIX //if you want to fix the 0 exp problem about the when kill lua event (recommended)
// #define ENABLE_SYSLOG_PACKET_SENT // debug purposes

#define ENABLE_EXTEND_ITEM_AWARD //slight adjustement
#ifdef ENABLE_EXTEND_ITEM_AWARD
	// #define USE_ITEM_AWARD_CHECK_ATTRIBUTES //it prevents bonuses higher than item_attr lvl1-lvl5 min-max range limit
#endif

// ### END Ex Features ###
//////////////////////////////////////////////////////////////////////////

#define ENABLE_EXCHANGE_RENEWAL
#define ENABLE_MOUNT_COSTUME_SYSTEM

#endif
//martysama0134's 8e0aa8057d3f54320e391131a48866b4
