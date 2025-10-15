#include "stdafx.h"
#include "config.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "MountSystem.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"

EVENTINFO(mountsystem_event_info)
{
	CMountSystem* pMountSystem;
};

EVENTFUNC(mountsystem_update_event)
{
	mountsystem_event_info* info = dynamic_cast<mountsystem_event_info*>( event->info );
	if ( info == NULL )
	{
		sys_err( "<mountsystem_update_event> <Factor> Null pointer" );
		return 0;
	}

	CMountSystem*	pMountSystem = info->pMountSystem;

	if (NULL == pMountSystem)
		return 0;


	pMountSystem->Update(0);
	return PASSES_PER_SEC(1) / 4;
}

EVENTINFO(mount_leave_event_info)
{
    DWORD vid;
};

EVENTFUNC(mount_leave_vanish_event)
{
    mount_leave_event_info* info = dynamic_cast<mount_leave_event_info*>(event->info);
    if (!info) return 0;

    LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(info->vid);
    if (ch)
        M2_DESTROY_CHARACTER(ch);

    return 0; // tek seferlik
}

///////////////////////////////////////////////////////////////////////////////////////
//  CMountActor
///////////////////////////////////////////////////////////////////////////////////////

CMountActor::CMountActor(LPCHARACTER owner, DWORD vnum)
{
	m_dwVnum = vnum;
	m_dwVID = 0;
	m_dwLastActionTime = 0;

	m_pkChar = 0;
	m_pkOwner = owner;

	m_originalMoveSpeed = 0;

	m_dwSummonItemVID = 0;
	m_dwSummonItemVnum = 0;
    m_bLeaveScheduled = false; // <<< yeni
}

CMountActor::~CMountActor()
{
	this->Unsummon();
	m_pkOwner = 0;
}

void CMountActor::SetName()
{
    char buf[64];

    // Sahip adı güvenli şekilde al
    const char* ownerName = (m_pkOwner ? m_pkOwner->GetName() : "");

    if (ownerName && ownerName[0] != '\0')
        snprintf(buf, sizeof(buf), "Binek ~ |cFFCD5C5C|h%s", ownerName);
    else
    {
        const char* mobName = (m_pkChar ? m_pkChar->GetMobTable().szLocaleName : "Mount");
        snprintf(buf, sizeof(buf), "%s", mobName);
    }

    if (IsSummoned() && m_pkChar)
        m_pkChar->SetName(buf);

    m_name = buf;
}


bool CMountActor::Mount(LPITEM mountItem)
{
	if (0 == m_pkOwner)
		return false;
	
	if(!mountItem)
		return false;

	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
	
	if (m_pkOwner->GetHorse())
		m_pkOwner->HorseSummon(false);
	
	Unmount();

	m_pkOwner->AddAffect(AFFECT_MOUNT, POINT_MOUNT, m_dwVnum, AFF_NONE, (DWORD)mountItem->GetSocket(0) - time(0), 0, true);
	
	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		if (mountItem->GetProto()->aApplies[i].bType == APPLY_NONE)
			continue;

		m_pkOwner->AddAffect(AFFECT_MOUNT_BONUS, aApplyInfo[mountItem->GetProto()->aApplies[i].bType].bPointType, mountItem->GetProto()->aApplies[i].lValue, AFF_NONE, (DWORD)mountItem->GetSocket(0) - time(0), 0, false);
	}
	
	return m_pkOwner->GetMountVnum() == m_dwVnum;
}

void CMountActor::Unmount()
{
	if (0 == m_pkOwner)
		return;
	
	if (!m_pkOwner->GetMountVnum())
		return;
	
	m_pkOwner->RemoveAffect(AFFECT_MOUNT);
	m_pkOwner->RemoveAffect(AFFECT_MOUNT_BONUS);
	m_pkOwner->MountVnum(0);
	
	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
	
	if (m_pkOwner->GetHorse())
		m_pkOwner->HorseSummon(false);
	
	m_pkOwner->MountVnum(0);
}

void CMountActor::Unsummon(bool bLeaveFar)
{
    if (!IsSummoned())
        return;

    this->SetSummonItem(NULL);

    if (NULL != m_pkChar)
    {
        m_pkChar->SetRider(NULL);

        if (bLeaveFar && NULL != m_pkOwner)
        {
            m_pkChar->SetNowWalking(false);

            float fx, fy;
            m_pkChar->SetRotation(GetDegreeFromPositionXY(
                m_pkChar->GetX(), m_pkChar->GetY(),
                m_pkOwner->GetX(), m_pkOwner->GetY()) + 180);
            GetDeltaByDegree(m_pkChar->GetRotation(), 3500.f, &fx, &fy);

            // 1) Uzaklaşma komutu
            m_pkChar->Goto(static_cast<long>(m_pkChar->GetX() + fx),
                           static_cast<long>(m_pkChar->GetY() + fy));
            m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

            // 2) Birkaç saniye sonra kesin yok et (tek sefer)
            if (!m_bLeaveScheduled)
            {
                m_bLeaveScheduled = true;
                mount_leave_event_info* info = AllocEventInfo<mount_leave_event_info>();
                info->vid = m_pkChar->GetVID();
                event_create(mount_leave_vanish_event, info, PASSES_PER_SEC(3)); // 3 sn sonra
            }
        }
        else
        {
            M2_DESTROY_CHARACTER(m_pkChar);
        }
    }

    m_pkChar = 0;
    m_dwVID  = 0;
    m_bLeaveScheduled = false; // temizlik
}


DWORD CMountActor::Summon(LPITEM pSummonItem, bool bSpawnFar)
{
    if (!m_pkOwner)
        return 0;

    long x = m_pkOwner->GetX();
    long y = m_pkOwner->GetY();
    long z = m_pkOwner->GetZ();

    if (bSpawnFar)
    {
        // Sahibin yönünün TERSİ, 2000..2500
        const float rad = (m_pkOwner->GetRotation() + 180.0f) * 3.141592f / 180.0f;
        const int dist = number(2000, 2500);
        x += (long)(cos(rad) * dist);
        y += (long)(sin(rad) * dist);
    }
    else
    {
        x += number(-100, 100);
        y += number(-100, 100);
    }

    if (m_pkChar && (m_pkChar->IsDead() || CHARACTER_MANAGER::instance().Find(m_pkChar->GetVID()) != m_pkChar))
    {
        M2_DESTROY_CHARACTER(m_pkChar);
        m_pkChar = NULL;
        m_dwVID = 0;
    }

    if (!m_pkChar)
    {
        m_pkChar = CHARACTER_MANAGER::instance().SpawnMob(
            m_dwVnum, m_pkOwner->GetMapIndex(), x, y, z, false,
            (int)(m_pkOwner->GetRotation() + 180.0f), false);
        if (!m_pkChar)
        {
            sys_err("[CMountActor::Summon] Failed to summon (vnum:%d)", m_dwVnum);
            return 0;
        }
        m_pkChar->SetMount();
        m_pkChar->SetEmpire(m_pkOwner->GetEmpire());
    }

    m_pkChar->SetRider(m_pkOwner);
    m_dwVID = m_pkChar->GetVID();
    SetName();

    if (!m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y, z))
    {
        sys_err("[CMountActor::Summon] Show failed (vnum:%d)", m_dwVnum);
        M2_DESTROY_CHARACTER(m_pkChar);
        m_pkChar = NULL;
        m_dwVID = 0;
        return 0;
    }

    m_dwLastActionTime = 0;

    SetSummonItem(pSummonItem);
    return m_dwVID;
}

bool CMountActor::_UpdateFollowAI()
{
    if (!m_pkChar || !m_pkOwner)
        return false;
    if (!m_pkChar->m_pkMobData)
        return false;

    if (0 == m_originalMoveSpeed)
    {
        const CMob* mobData = CMobManager::Instance().Get(m_dwVnum);
        if (mobData)
            m_originalMoveSpeed = mobData->m_table.sMovingSpeed;
    }

    // At paritesi
    const float START_FOLLOW_DISTANCE = 400.0f;   // takip yalnızca bu uzaklıktan sonra başlar
    const float START_RUN_DISTANCE    = 700.0f;   // koşma eşiği
    const float RESPAWN_DISTANCE      = 4500.0f;  // çok uzaksa arkaya ışınla

    // Stabil yaklaşma mesafesi (150..300) — deterministik olsun ki jitter olmasın
    const int MIN_APPROACH = 150;
    const int MAX_APPROACH = 300;
    const int approach = MIN_APPROACH + (int)((m_pkOwner->GetVID() ^ m_dwVnum) % (MAX_APPROACH - MIN_APPROACH + 1));

    const DWORD now = get_dword_time();

    const long ownerX = m_pkOwner->GetX();
    const long ownerY = m_pkOwner->GetY();
    const long ownerZ = m_pkOwner->GetZ();

    long charX = m_pkChar->GetX();
    long charY = m_pkChar->GetY();

    // Farklı harita ise önce aynı haritaya getir
    if (m_pkChar->GetMapIndex() != m_pkOwner->GetMapIndex())
    {
        if (!m_pkChar->Show(m_pkOwner->GetMapIndex(), ownerX, ownerY, ownerZ))
            return false;
        charX = m_pkChar->GetX();
        charY = m_pkChar->GetY();
    }

    const float fDist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

    // Aşırı uzak: sahibin ARKASINA (rot+180°) 'approach' kadar ışınla ve bekle
    if (fDist >= RESPAWN_DISTANCE)
    {
        const float rad = (m_pkOwner->GetRotation() + 180.0f) * 3.141592f / 180.0f;
        const long nx = ownerX + (long)(cos(rad) * approach);
        const long ny = ownerY + (long)(sin(rad) * approach);
        m_pkChar->Show(m_pkOwner->GetMapIndex(), nx, ny, ownerZ);
        m_pkChar->SetRotation(m_pkOwner->GetRotation());
        m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
        return true;
    }

    if (fDist >= START_FOLLOW_DISTANCE)
    {
        // Sadece uzaksa takip: 400+ → takip, 700+ → koş (aksi halde yürüyüş)
        const bool bRun = (fDist >= START_RUN_DISTANCE);
        m_pkChar->SetNowWalking(!bRun);

        // Hedefi hesaplayan basit takip (CHARACTER::Follow kullanmadan)
        Follow((float)approach);

        m_pkChar->SetLastAttacked(now);
        m_dwLastActionTime = now;
    }
    else
    {
        // Yakın: WANDER VAR (pet gibi sürekli takip yok)
        // 5–12 sn'de bir, 200–400 birim yürüyerek kısa bir gezinme yap
        if (now > m_dwLastActionTime)
        {
            m_dwLastActionTime = now + number(5000, 12000);

            m_pkChar->SetNowWalking(true);
            const int wanderDeg  = number(0, 359);
            const int wanderDist = number(200, 400);

            float fx, fy;
            GetDeltaByDegree(wanderDeg, (float)wanderDist, &fx, &fy);

            const long tx = m_pkChar->GetX() + (long)fx;
            const long ty = m_pkChar->GetY() + (long)fy;

            // Engel/zemin kontrolü YOK — doğrudan dene; başaramazsa bekle
            m_pkChar->SetRotation(wanderDeg);
            if (m_pkChar->Goto(tx, ty))
                m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
            else
                m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
        }
        else
        {
            // Gezinme aralığı dışında → sadece bekle, sahibin yönüne dön
            m_pkChar->SetRotation(m_pkOwner->GetRotation());
            m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
        }
    }

    return true;
}

bool CMountActor::Update(DWORD deltaTime)
{
	bool bResult = true;

	if (m_pkOwner->IsDead() || (IsSummoned() && m_pkChar->IsDead())
		|| NULL == ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())
		|| ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetOwner() != this->GetOwner()
		)
	{
		this->Unsummon();
		return true;
	}

	if (this->IsSummoned())
		bResult = bResult && this->_UpdateFollowAI();

	return bResult;
}

bool CMountActor::Follow(float fMinDistance)
{
    if (!m_pkOwner || !m_pkChar)
        return false;

    // Hedef: sahibin arkasında fMinDistance kadar
    const float rad = (m_pkOwner->GetRotation() + 180.0f) * 3.141592f / 180.0f;

    const long targetX = m_pkOwner->GetX() + (long)(cos(rad) * fMinDistance);
    const long targetY = m_pkOwner->GetY() + (long)(sin(rad) * fMinDistance);

    // Hedefe yeterince yakınsa yeni Goto gönderme → jitter yok
    const float toTarget = DISTANCE_APPROX(m_pkChar->GetX() - targetX, m_pkChar->GetY() - targetY);
    if (toTarget <= 25.0f) // küçük bir dead-zone
    {
        m_pkChar->SetRotation(m_pkOwner->GetRotation());
        m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);
        return true;
    }

    // Doğrudan hedefe dön ve tek Goto dene (engel/yan kaçış YOK)
    m_pkChar->SetRotationToXY(targetX, targetY);

    if (!m_pkChar->Goto(targetX, targetY))
    {
        // Rota çizilemediyse ek sapma/çarpışma çözümü yok; bir sonraki döngüde tekrar denenecek
        return false;
    }

    m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);
    return true;
}

void CMountActor::SetSummonItem(LPITEM pItem)
{
	if (NULL == pItem)
	{
		m_dwSummonItemVID = 0;
		m_dwSummonItemVnum = 0;
		return;
	}

	m_dwSummonItemVID = pItem->GetVID();
	m_dwSummonItemVnum = pItem->GetVnum();
}

///////////////////////////////////////////////////////////////////////////////////////
//  CMountSystem
///////////////////////////////////////////////////////////////////////////////////////

CMountSystem::CMountSystem(LPCHARACTER owner)
{
	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;

	m_dwLastUpdateTime = 0;
}

CMountSystem::~CMountSystem()
{
	Destroy();
}

void CMountSystem::Destroy()
{
	for (TMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CMountActor* mountActor = iter->second;

		if (0 != mountActor)
		{
			delete mountActor;
		}
	}
	event_cancel(&m_pkMountSystemUpdateEvent);
	m_mountActorMap.clear();
}

bool CMountSystem::Update(DWORD deltaTime)
{
	bool bResult = true;

	DWORD currentTime = get_dword_time();

	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;

	std::vector <CMountActor*> v_garbageActor;

	for (TMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CMountActor* mountActor = iter->second;

		if (0 != mountActor && mountActor->IsSummoned())
		{
			LPCHARACTER pMount = mountActor->GetCharacter();

			if (NULL == CHARACTER_MANAGER::instance().Find(pMount->GetVID()))
			{
				v_garbageActor.push_back(mountActor);
			}
			else
			{
				bResult = bResult && mountActor->Update(deltaTime);
			}
		}
	}
	for (std::vector<CMountActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeleteMount(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CMountSystem::DeleteMount(DWORD mobVnum)
{
	TMountActorMap::iterator iter = m_mountActorMap.find(mobVnum);

	if (m_mountActorMap.end() == iter)
	{
		sys_err("[CMountSystem::DeleteMount] Can't find mount on my list (VNUM: %d)", mobVnum);
		return;
	}

	CMountActor* mountActor = iter->second;

	if (0 == mountActor)
		sys_err("[CMountSystem::DeleteMount] Null Pointer (mountActor)");
	else
		delete mountActor;

	m_mountActorMap.erase(iter);
}

void CMountSystem::DeleteMount(CMountActor* mountActor)
{
	for (TMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		if (iter->second == mountActor)
		{
			delete mountActor;
			m_mountActorMap.erase(iter);

			return;
		}
	}

	sys_err("[CMountSystem::DeleteMount] Can't find mountActor(0x%x) on my list(size: %d) ", mountActor, m_mountActorMap.size());
}

void CMountSystem::Unsummon(DWORD vnum, bool bDeleteFromList, bool bFromFar)
{
	CMountActor* actor = this->GetByVnum(vnum);

	if (0 == actor)
	{
		sys_err("[CMountSystem::Unsummon(%d)] Null Pointer (actor)", vnum);
		return;
	}

	actor->Unsummon(bFromFar);

	if (true == bDeleteFromList)
	{
		this->DeleteMount(actor);
	}

	bool bActive = false;
	for (TMountActorMap::iterator it = m_mountActorMap.begin(); it != m_mountActorMap.end(); ++it)
	{
		bActive |= it->second->IsSummoned();
	}

	if (false == bActive)
	{
		event_cancel(&m_pkMountSystemUpdateEvent);
		m_pkMountSystemUpdateEvent = NULL;
	}
}

void CMountSystem::Unsummon(CMountActor* mountActor, bool bDeleteFromList, bool bFromFar)
{
	if (NULL == mountActor)
	{
		sys_err("[CMountSystem::Unsummon] Null Pointer (mountActor)");
		return;
	}

	mountActor->Unsummon(bFromFar);

	if (true == bDeleteFromList)
	{
		DeleteMount(mountActor);
	}

	bool bActive = false;
	for (TMountActorMap::iterator it = m_mountActorMap.begin(); it != m_mountActorMap.end(); ++it)
	{
		bActive |= it->second->IsSummoned();
	}

	if (false == bActive)
	{
		event_cancel(&m_pkMountSystemUpdateEvent);
		m_pkMountSystemUpdateEvent = NULL;
	}
}

void CMountSystem::Summon(DWORD mobVnum, LPITEM pSummonItem, bool bSpawnFar)
{
	//if (m_pkOwner->IncreaseMountCounter() >= 5)
	//{
	//	m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("_TRANSLATE_CHAT_TYPE_PACKETS_FROM_SOURCE_TO_GLOBAL__TRANSLATE_LIST_110"));
	//	return;
	//}
	
	CMountActor* mountActor = this->GetByVnum(mobVnum);

	if (0 == mountActor)
	{
		mountActor = M2_NEW CMountActor(m_pkOwner, mobVnum);
		m_mountActorMap.insert(std::make_pair(mobVnum, mountActor));
	}

	DWORD mountVID = mountActor->Summon(pSummonItem, bSpawnFar);

	if (!mountVID)
		sys_err("[CMountSystem::Summon(%d)] Null Pointer (mountVID)", pSummonItem->GetID());

	if (NULL == m_pkMountSystemUpdateEvent)
	{
		mountsystem_event_info* info = AllocEventInfo<mountsystem_event_info>();

		info->pMountSystem = this;

		m_pkMountSystemUpdateEvent = event_create(mountsystem_update_event, info, PASSES_PER_SEC(1) / 4);
	}

	//return mountActor;
}

void CMountSystem::Mount(DWORD mobVnum, LPITEM mountItem)
{
	CMountActor* mountActor = this->GetByVnum(mobVnum);

	if (!mountActor)
	{
		sys_err("[CMountSystem::Mount] Null Pointer (mountActor)");
		return;
	}
	
	if(!mountItem)
		return;
	
	//check timer
	// if (m_pkOwner->IncreaseMountCounter() >= 5)
	// {
		// m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "<Mount> Asteapta 5 secunde pentru a face aceasta actiune");
		// return;
	// }

	this->Unsummon(mobVnum, false, false);
	mountActor->Mount(mountItem);
}

void CMountSystem::Unmount(DWORD mobVnum)
{
	CMountActor* mountActor = this->GetByVnum(mobVnum);

	if (!mountActor)
	{
		sys_err("[CMountSystem::Mount] Null Pointer (mountActor)");
		return;
	}
	//check timer
	// if (m_pkOwner->IncreaseMountCounter() >= 5)
	// {
		// m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "<Mount> Asteapta 5 secunde pentru a face aceasta actiune");
		// return;
	// }
	
	if(LPITEM pSummonItem = m_pkOwner->GetWear(WEAR_COSTUME_MOUNT))
	{
		this->Summon(mobVnum, pSummonItem, false);
	}
	
	mountActor->Unmount();
}

CMountActor* CMountSystem::GetByVID(DWORD vid) const
{
	CMountActor* mountActor = 0;

	bool bFound = false;

	for (TMountActorMap::const_iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		mountActor = iter->second;

		if (0 == mountActor)
		{
			sys_err("[CMountSystem::GetByVID(%d)] Null Pointer (mountActor)", vid);
			continue;
		}

		bFound = mountActor->GetVID() == vid;

		if (true == bFound)
			break;
	}

	return bFound ? mountActor : 0;
}

CMountActor* CMountSystem::GetByVnum(DWORD vnum) const
{
	CMountActor* mountActor = 0;

	TMountActorMap::const_iterator iter = m_mountActorMap.find(vnum);

	if (m_mountActorMap.end() != iter)
		mountActor = iter->second;

	return mountActor;
}

size_t CMountSystem::CountSummoned() const
{
	size_t count = 0;

	for (TMountActorMap::const_iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CMountActor* mountActor = iter->second;

		if (0 != mountActor)
		{
			if (mountActor->IsSummoned())
				++count;
		}
	}

	return count;
}
