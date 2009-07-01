
#ifndef VIDEO_H__
#define VIDEO_H__

class CVideo
{
private:
	char *m_fileName;

public:
	CVideo(){};
	~CVideo(){};

	void Load(char *fileName);
	void Play();
	void Stop();
	void Close();
	bool IsLoaded();
	bool IsPlaying();
	bool IsPlayCompleted();
};

#endif // VIDEO_H__