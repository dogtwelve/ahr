// Generic.h: interface for the CGeneric class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNIQUERESOURCE_H__001C2E5A_5463_4493_86AA_6D9440D4C845__INCLUDED_)
#define AFX_UNIQUERESOURCE_H__001C2E5A_5463_4493_86AA_6D9440D4C845__INCLUDED_

//#pragma warning(disable:4786)
//#pragma warning(disable:4503)

#include "Singleton.h"
#include "Generic.h"
#include <map>

// Default factory for unique resource class
template <class ti_Class, class ti_Key>
class CDefaultFactory
{
public:
    static ti_Class* Generate(const ti_Key& in_Key);
};

template <class ti_Class, class ti_Key>
ti_Class* 
CDefaultFactory<ti_Class, ti_Key>::Generate(const ti_Key& in_Key)
{
    return STATIC_NEW ti_Class(in_Key);
}


// CUniqueResource
// usage:
// - derive Class from CUniqueResource<Class, Key, nSingletonID>
// - implement the constructor : Class(const Key&) or a factory class
//   with a Generate function
//
// This will allow to use the GetUnique() function to Create or Get an existant unique resource
//
template <  class ti_Class,
            class ti_Key,
            int ti_nSingletonID,
            class ti_FactoryClass = CDefaultFactory<ti_Class,ti_Key>,
            class ti_ParentClass = IGeneric>
class CUniqueResource : public CSmartPointed<ti_Class, ti_ParentClass>
{
protected:
    class CUniquePointerCollection :
        public CSingleton<ti_nSingletonID, CUniquePointerCollection>
    {
    private:
		typedef std::map<ti_Key, ti_Class*> TInstancesMap;


    public:
        ti_Class* Find(const ti_Key& in_Key)
        {
            if (m_UniqueInstances.find(in_Key) == m_UniqueInstances.end())
            {
                return 0;
            }
            return m_UniqueInstances.find(in_Key)->second;
        }

        void Register(const ti_Key& in_Key, ti_Class* in_pInstance)
        {
            m_UniqueInstances[in_Key] = in_pInstance;
        }

        void UnRegister(const ti_Key& in_Key)
        {
            m_UniqueInstances.erase(in_Key);
        }

        virtual ~CUniquePointerCollection()
        {
            A_ASSERT(m_UniqueInstances.empty());
#ifdef WIN_DEBUG
			for(TInstancesMap::iterator curIt = m_UniqueInstances.begin();curIt != m_UniqueInstances.end();++curIt)
			{
				ti_Key	 key = (*curIt).first;
				ti_Class* pClass = (*curIt).second;
			}			
#endif
        }

    private:
        TInstancesMap m_UniqueInstances;
    };

public:
    // Create or get a unique resource

	static CSmartPtr<ti_Class> GetUnique(const ti_Key& in_Key,const ti_FactoryClass& factory)
	{
		ti_Class* pInstance = CUniquePointerCollection::GetInstance()->Find(in_Key);
		if (pInstance)
		{
			pInstance->AddRef();
		}
		else
		{
			pInstance = factory.Generate(in_Key);
			if (pInstance)
			{
				pInstance->m_UniqueKey = in_Key;
				CUniquePointerCollection::GetInstance()->Register(pInstance->m_UniqueKey, pInstance);
			}
		}
		//JOGY//
		return ti_Class::P(pInstance);
	}

	static CSmartPtr<ti_Class> GetUnique(const ti_Key& in_Key)
	{
		ti_Class* pInstance = CUniquePointerCollection::GetInstance()->Find(in_Key);
		if (pInstance)
		{
			pInstance->AddRef();
		}
		else
		{
			pInstance = ti_FactoryClass::Generate(in_Key);
			if (pInstance)
			{
				pInstance->m_UniqueKey = in_Key;
				CUniquePointerCollection::GetInstance()->Register(pInstance->m_UniqueKey, pInstance);
			}
		}
		//JOGY//
		return CSmartPtr<ti_Class>(pInstance);
	}

public:
	static void InstanciateCollection()
	{
		CUniquePointerCollection::GetInstance();
	}

protected:
    virtual ~CUniqueResource()
	{
		CUniquePointerCollection::GetInstance()->UnRegister(m_UniqueKey);
	}

protected:
    ti_Key m_UniqueKey;
};

#endif // !defined(AFX_UNIQUERESOURCE_H__001C2E5A_5463_4493_86AA_6D9440D4C845__INCLUDED_)
