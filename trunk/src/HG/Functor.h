// Functor.h: interface for the CFunctor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FUNCTOR_H__B24EEFB6_8028_4142_BEED_31D6700D893C__INCLUDED_)
#define AFX_FUNCTOR_H__B24EEFB6_8028_4142_BEED_31D6700D893C__INCLUDED_

// abstract base class
class CFunctor  
{
public:
	CFunctor();
	virtual ~CFunctor();

    virtual void operator()(void) = 0;
};

// derived template class
template <class TClass> class CSpecificFunctor : 
    public CFunctor
{

public:

    CSpecificFunctor(TClass* in_Pt2Object, void(TClass::*in_FuncPtr)(void))
    { 
        m_Pt2Object = in_Pt2Object;
        m_FuncPtr = in_FuncPtr;
    }

    virtual void operator()(void)
    {
        (*m_Pt2Object.*m_FuncPtr)();
    }

protected:

    void (TClass::*m_FuncPtr)(void);        // pointer to member function
    TClass* m_Pt2Object;                    // pointer to object

};

class CSpecificFunctorNoObj : 
    public CFunctor
{

public:

    CSpecificFunctorNoObj(void(*in_FuncPtr)(void))
    { 
        m_FuncPtr = in_FuncPtr;
    }

    virtual void operator()(void)
    {
        (*m_FuncPtr)();
    }

protected:

    void (*m_FuncPtr)(void);                // pointer to member function

};

#endif // !defined(AFX_FUNCTOR_H__B24EEFB6_8028_4142_BEED_31D6700D893C__INCLUDED_)
