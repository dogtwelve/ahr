
#if !defined(_CONTROLLER_H__INCLUDED_)
#define _CONTROLLER_H__INCLUDED_

class CControlled;

//--------------------------------
// Controller interface
//--------------------------------
class IController
{
public:
    IController() : m_bInvincible(false), m_bFury(false) { }
    virtual void UpdateControlStatus(CControlled* in_pControlled)=0;

    bool IsInvincible() const { return m_bInvincible; }
    void SetInvincible(bool in_bInvincible) { m_bInvincible = in_bInvincible; }

	bool IsInFuryMode() const { return m_bFury; }
    void SetFuryMode(bool in_bFury) { m_bFury = in_bFury; }

protected:
    bool m_bInvincible;
	bool m_bFury;
};

#endif // !defined(_CONTROLLER_H__INCLUDED_)
