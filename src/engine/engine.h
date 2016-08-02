

#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include "kernel.h"
#include <engine/shared/jobs.h>

class CHostLookup
{
public:
	CJob m_Job;
	char m_aHostname[128];
	int m_Nettype;
	NETADDR m_Addr;
};

class IEngine : public IInterface
{
	MACRO_INTERFACE("engine", 0)

protected:
	class CJobPool m_JobPool;

public:
	virtual void Init() = 0;
	virtual void InitLogfile() = 0;
	virtual void HostLookup(CHostLookup *pLookup, const char *pHostname, int Nettype) = 0;
	virtual void AddJob(CJob *pJob, JOBFUNC pfnFunc, void *pData) = 0;
};

extern IEngine *CreateEngine(const char *pAppname);

#endif
