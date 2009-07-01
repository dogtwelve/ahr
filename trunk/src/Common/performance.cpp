



#pragma warning(disable:4786)


#include "performance.h"


#if CALC_PERFORMANCE
#include <stdio.h>



Performance gPerformance;




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Performance::Performance()
{
	::QueryPerformanceFrequency(&m_frequency);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Performance::~Performance()
{
	Points::const_iterator i;

	LONGLONG	totalTime = 0;	

	for(i = m_points.begin();i!=m_points.end();++i)	
		totalTime += i->second.m_total;		

	FILE* f = ::fopen("D:\\performance.txt","at");
	if(f)
	{
		const double ms = 1000.0 / double(m_frequency.QuadPart);

		fprintf(f,"\n\n------PERFORMANCE REPORT----------\n");		
		fprintf(f,"F: %f ms\n\n",   ms);

		for(i = m_points.begin();i!=m_points.end();++i)
		{
			fprintf(f,"%s\t",i->first);

			fprintf(f,"T: %f ms\t",double(i->second.m_total) * ms);
			fprintf(f,"C: %d\t",i->second.m_nbCall);

			fprintf(f,"A: %f ms\t",double(i->second.m_total) / double(i->second.m_nbCall) * ms);

			fprintf(f,"\n");
		}
		::fclose(f);
	}
}



#endif //#if CALC_PERFORMANCE