#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include "config.h"
#include "DevUtil.h" // A_ASSERT

enum
{
	k_nSingletonMenuContainer,
    k_nSingletonHighGear,
	k_nSingletonArchivedFilesManager,
	k_nSingletonDemoMenu,
	k_nSingletonLanguageSelector,
//SEFU ADD
    k_nSingletonSoundManager,
	//////a2 stuff
	k_nSingletonFontManager,
	//k_nSingletonSoundWrapper,
    //k_nSingletonSystem,
	//k_nSingletonStringTable,
	k_nSingletonUniqueResourcesBitmapPal,
    k_nSingletonUniqueResourcesBitmapRLE,
	k_nSingletonUniqueResourcesBitmapAlpha,
	//k_nSingletonCarAnimationsCollection,

#if MANUAL_STACK_TRACE_ON_NGAGE
	k_nSingletonManualStackTracer,
#endif
#ifdef USE_TOUCH_SCREEN
	k_nSingletonTouchZones,
#endif

	//k_nSingletonApplicationStateSet,
	//k_nSingletonFxPlayer,
 //   k_nSingletonCharacterStateSet,
	//k_nSingletonPlayerStateSet,
	//k_nSingletonOptionManager,
	//k_nSingletonGameCameraStateSet,
	//k_nSingletonGunStateSet,
	//k_nSingletonTextureManager,
	//k_nSingletonThrowableFactory,
	//k_nSingleton3dSoundManager,
	//k_nSingletonCheatManager,
	//k_nSingletonAlarmManager,
	//k_nSingletonExplosionManager,
	//k_nSingletonScriptManager,
	//k_nSingletonDamagerSingleton,
	//k_nSingletonUniqueResourcesAnimatedObjectMesh,
 //   k_nSingletonUniqueResourcesAnimatedObjectAnimationsSet,
 //   k_nSingletonUniqueResourcesAnimatedObjectTexture,
	//k_nSingletonUniqueResourcesTexturePal,
	//k_nSingletonUniqueResourcesRefCountedGeneralMesh,
	//k_nSingletonObjectiveIndicator,
	//k_nSingletonCameraRadar,
	//k_nSingletonLoadManager,

	/////////////////////////////


    k_nNumSingletons
};

class ISingleton;

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class CSingletonManager
{
public:
    virtual ~CSingletonManager();

    static void Create();
    static CSingletonManager* GetInstance();
    static void Shutdown();

    static void RegisterSingleton(ISingleton* in_pSingleton, int ID);

    static inline ISingleton* GetSingleton(int ID);
    static inline ISingleton& GetRefSingleton(int ID);

protected:
    CSingletonManager();

    void Inst_RegisterSingleton(ISingleton* in_pSingleton, int ID);

    inline ISingleton* Inst_GetSingleton(int ID);
    inline ISingleton& Inst_GetRefSingleton(int ID);

protected:
    ISingleton** m_ppSingletons;

//#if defined _WINDOWS && !defined(__BREW__)
    static CSingletonManager* c_pTls;
//#endif
};

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class ISingleton
{
public:
    virtual ~ISingleton(){}
};

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
template<int ID, class T>
class CSingletonNoAutoConstruct : public ISingleton
{
public:
		typedef T TStateSet;
		enum
		{
			kId = ID
		};
public:
    CSingletonNoAutoConstruct()
    {
        CSingletonManager::RegisterSingleton(this, ID);
    }

    virtual ~CSingletonNoAutoConstruct()
    {
    }

    static T* GetInstance()
    {
			T* pInstance = _GetInstance();
			return pInstance;
    }

    static T& GetRefInstance()
    {
        return *GetInstance();
    }
protected:
	static T* _GetInstance()	{return (T*)CSingletonManager::GetSingleton(ID);}
};


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
template<int ID, class T>
class CSingleton : public CSingletonNoAutoConstruct<ID,T>
{
	typedef CSingletonNoAutoConstruct<ID,T> inherited;
public:
    CSingleton()
    {
    }

    virtual ~CSingleton()
    {
    }

    static T* GetInstance()
    {
		T* pInstance = CSingletonNoAutoConstruct<ID,T>::_GetInstance();
		if (pInstance)
			return pInstance;
		else
			return STATIC_NEW T;
    }

    static T& GetRefInstance()
    {
        return *GetInstance();
    }
};





// ---------------------------------------------------------------------------
// INLINE definitions
// ---------------------------------------------------------------------------
ISingleton* CSingletonManager::GetSingleton(int ID)
{
    return GetInstance()->Inst_GetSingleton(ID);
}

ISingleton& CSingletonManager::GetRefSingleton(int ID)
{
    return GetInstance()->Inst_GetRefSingleton(ID);
}

ISingleton* CSingletonManager::Inst_GetSingleton(int ID)
{
    return m_ppSingletons[ID];
}

ISingleton& CSingletonManager::Inst_GetRefSingleton(int ID)
{
    return *(m_ppSingletons[ID]);
}

#endif
