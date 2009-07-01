// FPSCounter.h: interface for the CFPSCounter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FPSCOUNTER_H__563A051A_B127_4976_9F88_5B1FBD217B0B__INCLUDED_)
#define AFX_FPSCOUNTER_H__563A051A_B127_4976_9F88_5B1FBD217B0B__INCLUDED_

//namespace Lib2D
//{
    class CLib2D;
	class CFontWrapper;
//};

class CFPSCounter  
{
public:
	CFPSCounter();
	virtual ~CFPSCounter();

    //void Begin(int in_nCurrentTimeMillisec);
    void ToggleInfoShow();
    void Update();
	inline int getNumFrames() const { return m_nNumFrames;};
    void Draw(int in_nPosX, int in_nPosY, CLib2D& in_Lib2D,int in_nCurrentTimeMillisec);

	bool	toggledFromUpdateGame;
	bool	toggledFromGameLoop;
	int		numUpdates;

	inline void SetFont(CFontWrapper *font) { m_font = font; }

	inline bool GetShowInfo() {return showInfo;}

protected:
    int m_nCurrentFPS;
    int m_nTimeLastSec;
    int m_nNumFrames;
	bool showInfo;

	CFontWrapper *m_font;

	unsigned short	m_fpsInfoString[200];		
};

#endif // !defined(AFX_FPSCOUNTER_H__563A051A_B127_4976_9F88_5B1FBD217B0B__INCLUDED_)
