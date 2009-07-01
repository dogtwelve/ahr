
#if !defined(AFX_CONTROLLED_H__91DD2AFA_2745_422A_BBE0_FF7F88776E62__INCLUDED_)
#define AFX_CONTROLLED_H__91DD2AFA_2745_422A_BBE0_FF7F88776E62__INCLUDED_

#include "DevUtil.h"
#include "Controller.h"


//--------------------------------
// CControlled
//--------------------------------
class CControlled  
{
public:
    typedef unsigned int TControl;
    typedef unsigned int TControlStatus;
    enum {k_nMaxNumControls = 32};

    static inline TControlStatus ControlMask(const TControl& in_Control)
    {
        return 1<<in_Control;
    }

public:
    CControlled();

    // Controls update
    void ResetControl();
    void Control(TControl in_nControl);
    void SetControlStatus(TControlStatus in_nControlStatus);
    void AddControlStatus(TControlStatus in_nControlStatus);
    TControlStatus GetControlStatus() const;
    bool IsControlled(TControl in_nControl) const;
    bool IsJustControlled(TControl in_nControl) const;
    bool IsAnyJustControlled() const;
    bool IsAnyControlChanged() const;
    void UpdateControlStatus();

    void SetController(IController* in_pController);
    IController* GetController(){ return m_pController; }
    const IController* GetController() const{ return m_pController; }


	// stuff from a2;
	
    void ResetControlStatus();
    bool IsJustReleased(TControl in_nControl) const;


private:
    TControlStatus m_nControlStatus;
    TControlStatus m_nLastControlStatus;
    IController* m_pController;
};

//--------------------------------
// CControlled inline functions
//--------------------------------

inline CControlled::CControlled() :
    m_nControlStatus(0),
    m_nLastControlStatus(0)
{
}

inline void CControlled::ResetControl()
{
    m_nLastControlStatus = m_nControlStatus;
    m_nControlStatus = 0;
}

inline void CControlled::Control(TControl in_nControl)
{
    m_nControlStatus |= ControlMask(in_nControl);
}

inline void CControlled::SetControlStatus(TControlStatus in_nControlStatus)
{
    m_nControlStatus = in_nControlStatus;
}

inline void CControlled::AddControlStatus(TControlStatus in_nControlStatus)
{
    m_nControlStatus |= in_nControlStatus;
}

inline CControlled::TControlStatus CControlled::GetControlStatus() const
{
    return m_nControlStatus;
}

inline bool CControlled::IsControlled(CControlled::TControl in_nControl) const
{
    return (m_nControlStatus & ControlMask(in_nControl)) != 0;
}

inline bool CControlled::IsJustControlled(CControlled::TControl in_nControl) const
{
    return ((m_nControlStatus ^ m_nLastControlStatus) & m_nControlStatus & ControlMask(in_nControl)) != 0;
}

inline bool CControlled::IsAnyJustControlled() const
{
    return ((m_nControlStatus ^ m_nLastControlStatus) & m_nControlStatus) != 0;
}

inline bool CControlled::IsAnyControlChanged() const
{
    return (m_nControlStatus ^ m_nLastControlStatus) != 0;
}

inline void CControlled::UpdateControlStatus()
{
    A_ASSERT(m_pController);
    m_pController->UpdateControlStatus(this);
}

inline void CControlled::SetController(IController* in_pController)
{
    m_pController = in_pController;
    m_nControlStatus = 0;
    m_nLastControlStatus = 0;
}

inline bool CControlled::IsJustReleased(CControlled::TControl in_nControl) const
{
    return ((m_nControlStatus ^ m_nLastControlStatus) & m_nLastControlStatus & ControlMask(in_nControl)) != 0;
}


inline void CControlled::ResetControlStatus()
{
    m_nLastControlStatus = m_nControlStatus;
    m_nControlStatus = 0;
}

//--------------------------------

#endif // !defined(AFX_CONTROLLED_H__91DD2AFA_2745_422A_BBE0_FF7F88776E62__INCLUDED_)
