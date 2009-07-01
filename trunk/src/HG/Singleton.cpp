
#include "GenDef.h"
#include "Singleton.h"
#include "DevUtil.h"

#ifdef __BREW__
	#include "brewmain.h"
#endif

#if defined _WINDOWS && !defined(__BREW__)

CSingletonManager* CSingletonManager::c_pTls = 0;
static CSingletonManager* gLastVal = NULL;

#endif
#if defined( __SYMBIAN32__ )
#if defined(SYMBIAN9) || defined (NGI)
CSingletonManager* CSingletonManager::c_pTls = 0;
#endif
#endif


//-----------------------------------------------------------------------------
CSingletonManager::CSingletonManager()
:   m_ppSingletons(0)
{
    m_ppSingletons = NEW ISingleton*[k_nNumSingletons];
    for(int i = 0; i < k_nNumSingletons; i++)
    {
        m_ppSingletons[i] = 0;
    }

#if defined _WINDOWS && !defined(__BREW__)
    c_pTls = this;
#endif
#ifdef  __SYMBIAN32__
#ifdef SYMBIAN8
	Dll::SetTls((TAny*)this);
#else
	c_pTls = this;
#endif
#endif
}

//-----------------------------------------------------------------------------
CSingletonManager::~CSingletonManager()
{
    for(int i = 0; i < k_nNumSingletons; i++)
    {
        ISingleton* pSingleton = m_ppSingletons[i];
        MM_DELETE pSingleton;
        m_ppSingletons[i] = 0;
    }
    DELETE_ARRAY m_ppSingletons;

#if defined _WINDOWS && !defined(__BREW__)
    c_pTls = 0;
#endif
#ifdef  __SYMBIAN32__
#ifdef SYMBIAN8
	Dll::SetTls(0);
#else
	c_pTls = 0;
#endif
#endif
}

//-----------------------------------------------------------------------------
void CSingletonManager::Create()
{
#if defined _WINDOWS && !defined(__BREW__)
    if (!c_pTls)
        STATIC_NEW CSingletonManager();
#endif
#ifdef __SYMBIAN32__
        STATIC_NEW CSingletonManager();
#endif
#ifdef __BREW__
	IApplet * pApplet = GETAPPINSTANCE();
	gamestruct * pMe = (gamestruct *)pApplet;
	pMe->pSingletonManager = STATIC_NEW CSingletonManager();
#endif
}

//-----------------------------------------------------------------------------
void CSingletonManager::RegisterSingleton(ISingleton* in_pSingleton, int ID)
{
    GetInstance()->Inst_RegisterSingleton(in_pSingleton, ID);
}
//-----------------------------------------------------------------------------
void CSingletonManager::Inst_RegisterSingleton(ISingleton* in_pSingleton, int ID)
{
    A_ASSERT(0 <= ID && ID < k_nNumSingletons);
    A_ASSERT(m_ppSingletons[ID] == 0);
    m_ppSingletons[ID] = in_pSingleton;
}
//-----------------------------------------------------------------------------
void CSingletonManager::Shutdown()
{
    MM_DELETE GetInstance();
}
//-----------------------------------------------------------------------------
CSingletonManager* CSingletonManager::GetInstance()
{
#ifdef  __SYMBIAN32__
#ifdef SYMBIAN8
	return (CSingletonManager*)Dll::Tls();
#else
	if (!c_pTls)
    {
        Create();
    }
    return c_pTls;
#endif
#else
#ifdef __BREW__

	IApplet * pApplet = GETAPPINSTANCE();

	gamestruct * pMe = (gamestruct *)pApplet;


	if ( !pMe )
		return NULL;


	if ( !pMe->pSingletonManager )
	{
		Create();
	}


	return pMe->pSingletonManager;
#endif
#if defined _WINDOWS && !defined(__BREW__)		
    if (!c_pTls)
    {
        Create();
    }
    return c_pTls;
#endif
#endif
}
