#ifndef	__HEADER_MOUNT_SYSTEM__
#define	__HEADER_MOUNT_SYSTEM__

class CHARACTER;

class CMountActor
{
protected:
	friend class CMountSystem;
	CMountActor(LPCHARACTER owner, uint32_t vnum);

	virtual ~CMountActor();
	virtual bool	Update(uint32_t deltaTime);

protected:
	virtual bool	_UpdateFollowAI();
	virtual bool	_UpdatAloneActionAI(float fMinDist, float fMaxDist);

private:
	bool Follow(float fMinDistance = 50.f);

public:
	LPCHARACTER		GetCharacter()	const { return m_pkCharacter; }
	LPCHARACTER		GetOwner()	const { return m_pkOwner; }
	uint32_t			GetVnum() const { return m_dwVnum; }
	uint32_t			GetVID() const { return m_dwVID; }

	void			SetName(const char* MountName);
	uint32_t			Mount();
	uint32_t			Unmount();

	void			Summon(const char* MounName, uint32_t mobVnum);
	void			Unsummon();

	bool			IsSummoned() const { return 0 != m_pkCharacter; }
	bool			IsMounting() const { return m_dwMounted; }

	void			UpdateTime();

private:
	uint32_t			m_dwVnum;
	uint32_t			m_dwVID;
	uint32_t			m_dwLastActionTime;
	uint32_t			m_dwUpdatePeriod;
	uint32_t			m_dwLastUpdateTime;
	short			m_OMoveSpeed;
	bool			m_dwMounted;
	std::string		m_Name;

	LPCHARACTER		m_pkCharacter;
	LPCHARACTER		m_pkOwner;
};

class CMountSystem
{
public:
	typedef	std::map<uint32_t, CMountActor*> TMountActorMap;

public:
	CMountSystem(LPCHARACTER owner);
	virtual ~CMountSystem();

	CMountActor* GetByVID(uint32_t vid) const;
	CMountActor* GetByVnum(uint32_t vnum) const;

	bool		Update(uint32_t deltaTime);
	void		Destroy();

	size_t		CountSummoned() const;

public:
	void		SetUpdatePeriod(uint32_t ms);

	CMountActor* Summon(uint32_t mobVnum, const char* MountName);
	CMountActor* SummonSilent(uint32_t mobVnum, const char* MountName);
	CMountActor* Mount(uint32_t mobVnum);
	CMountActor* Unmount(uint32_t mobVnum);

	void		Unsummon(uint32_t mobVnum, bool bDeleteFromList = false);
	void		Unsummon(CMountActor* MountActor, bool bDeleteFromList = false);
	void        UnsummonAll();

	CMountActor* Mount(uint32_t mobVnum, const char* MountName);

	void		DeleteMount(uint32_t mobVnum);
	void		DeleteMount(CMountActor* MountActor);
	bool		IsActiveMount();
	bool		IsMounting(uint32_t mobVnum);
	void		UpdateTime();
	void EnsureUpdateEventStarted();

private:
	TMountActorMap	m_MountActorMap;
	LPCHARACTER		m_pkOwner;

	uint32_t			m_dwUpdatePeriod;
	uint32_t			m_dwLastUpdateTime;
	LPEVENT			m_pkMountSystemUpdateEvent;
};

#endif
