#ifndef MULTIARRAY_H
#define MULTIARRAY_H

template <typename T>
class CMultiArray
{
public:
	CMultiArray() : m_array0(0), m_count0(0), m_array1(0), m_count1(0)
	{}
	CMultiArray(T * i_array0, int i_count0, T * i_array1, int i_count1)
		: m_array0(i_array0), m_count0(i_count0), m_array1(i_array1), m_count1(i_count1)
	{}
	int size() const { return m_count0 + m_count1; }
	T const & operator[](int i) const
	{
		if (i >= m_count0)
		{
			return m_array1[i - m_count0];
		}
		return m_array0[i];
	}
	T & operator[](int i)
	{
		if (i >= m_count0)
		{
			return m_array1[i - m_count0];
		}
		return m_array0[i];
	}
private:
	T * m_array0;
	int m_count0;
	T * m_array1;
	int m_count1;
	// [NOTE] for now it's a fast double array.. could change it to true multi array if needed
};

#endif
