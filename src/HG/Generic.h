// Generic.h: interface for the CGeneric class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4786)
#if !defined(AFX_GENERIC_H__001C2E5A_5463_4493_86AA_6D9440D4C845__INCLUDED_)
#define AFX_GENERIC_H__001C2E5A_5463_4493_86AA_6D9440D4C845__INCLUDED_

#include "config.h"

// IGeneric
class IGeneric
{
public:
    virtual void AddRef()=0;
    virtual void Release()=0;
};

// CSmartPtr
template <class T>
class CSmartPtr
{
    T* p;
public:
    static inline CSmartPtr<T> Null(){return CSmartPtr<T>(0);}

public:
    CSmartPtr(T* p_ = 0) : p(p_)
    {
        // Do not AddRef in constructor
        // Should only be called when allocating a new T
    }

    CSmartPtr(const CSmartPtr<T>& p_) : p(p_.p)
    {
        if(p) p->AddRef();
    }

    ~CSmartPtr(void)
    {
        if (p) p->Release();
    }
    operator T*(void) const { return p;}
    T& operator*(void) { return *p; }
    const T& operator*(void) const { return *p; }
    T* operator->(void) const { return p; }
    CSmartPtr& operator=(const CSmartPtr<T> &p_)
    {
        return operator=((T * const) p_);
    }

    void Nullify()
    {
        (*this) = Null();
    }

    CSmartPtr<T>& operator=(const int& in_nZero)
    {
        return ((*this) = CSmartPtr<T>(0));
    }

	CSmartPtr<T> GetAndRelease()
	{
		CSmartPtr<T> smartPtrCopy = *this;
		Nullify();
		return smartPtrCopy;
	}

private:
    CSmartPtr& operator=(T * const p_)
    {
        if (p) p->Release();
        p = p_;
        if (p) p->AddRef();
        return *this;
    }
};

// CGeneric
template <class ti_ParentClass>
class CGeneric : public ti_ParentClass
{
public:
    CGeneric()
    {
        m_nRefCount = 1;
    }

    virtual ~CGeneric(){}

    virtual void AddRef()
    {
        ++m_nRefCount;
    }

    virtual void Release()
    {
        if(--m_nRefCount == 0)
        {
            MM_DELETE this;
        }
    }

    inline int GetRefCount() { return m_nRefCount; }

protected:
    int m_nRefCount;
};

// CSmartPointed
template <class ti_Class, class ti_ParentClass = IGeneric>
class CSmartPointed : public CGeneric<ti_ParentClass>
{
public:
    typedef CSmartPtr<ti_Class> P;
};

#endif // !defined(AFX_GENERIC_H__001C2E5A_5463_4493_86AA_6D9440D4C845__INCLUDED_)
