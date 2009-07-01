// Random.h: interface for the CRandom class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RANDOM_H__C58FC575_0AED_4091_A9C8_3E52E63880C7__INCLUDED_)
#define AFX_RANDOM_H__C58FC575_0AED_4091_A9C8_3E52E63880C7__INCLUDED_

class CRandom
{
public:
	CRandom(unsigned int in_Seed);
	CRandom();
	~CRandom();

    inline void SetSeed(unsigned int in_Seed);
	inline unsigned int GetNumber(int min,int max);
    inline unsigned int GetNumber(unsigned int in_RandMax);
    inline unsigned int GetNumber();

private:
    unsigned int m_Seed;
};

inline void CRandom::SetSeed(unsigned int in_Seed)
{
    m_Seed = in_Seed;
}


inline unsigned int CRandom::GetNumber(int min,int max)
{
	return min + GetNumber(max-min);
}



inline unsigned int CRandom::GetNumber(unsigned int in_RandMax)
{
	if (in_RandMax == 0)
		return 0;
	else
		return GetNumber() % in_RandMax;
}

inline unsigned int CRandom::GetNumber()
{
    m_Seed = 1664525 * m_Seed + 1013904223;
	return ((m_Seed >> 16));
}


#endif // !defined(AFX_RANDOM_H__C58FC575_0AED_4091_A9C8_3E52E63880C7__INCLUDED_)
