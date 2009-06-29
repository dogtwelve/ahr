#pragma once

class CIniContainer
{
private :
	char ** m_lines;
	const char * m_fileName;
	int CountLines();
	void LoadLines();
	
public:
	CIniContainer(const char * fileName);
	~CIniContainer(void);
};
