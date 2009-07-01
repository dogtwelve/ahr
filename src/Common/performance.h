#ifndef _PERFORMANCE_H_
#define _PERFORMANCE_H_

#include "config.h"


#if CALC_PERFORMANCE

#ifndef WIN32
#error ONLY ON WINDOWS
#endif

#include <map>
#include "windows.h"




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
struct PerfPoint
{
	PerfPoint():m_total(0),m_nbCall(0){};

	LONGLONG			m_total;
	unsigned int	m_nbCall;
};

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class PerfMeter
{
public:
	PerfMeter(PerfPoint& p):m_point(p)
	{
		::QueryPerformanceCounter(&m_start);
	}

	~PerfMeter()
	{
		LARGE_INTEGER	end;
		::QueryPerformanceCounter(&end);
		m_point.m_total += (end.QuadPart - m_start.QuadPart);
		++m_point.m_nbCall;
	}

private:
	LARGE_INTEGER	m_start;
	PerfPoint&		m_point;
};



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class Performance
{
public:
	Performance();
	~Performance();

	PerfPoint&	Get(const char* name)	{return m_points[name];}


private:
	typedef std::map<const char*,PerfPoint>	Points;

	Points				m_points;
	LARGE_INTEGER	m_frequency;

};



extern Performance gPerformance;

#define PERF_COUNTER(x)	PerfMeter x(gPerformance.Get(#x));


#else //CALC_PERFORMANCE
	#define PERF_COUNTER(x)
#endif //CALC_PERFORMANCE



#endif // _PERFORMANCE_H_
