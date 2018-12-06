#pragma once


class XAudioPlay
{
public:

	int sampleRate = 44100;
	int sampleSize = 16;		//位数
	int channels = 2;

	//打开音频播放 定义为纯虚函数
	virtual bool Open() = 0;

	virtual bool Close() = 0;

	//返回缓冲中还未播放的时间 ms
	virtual long long GetNoPlayMs() = 0;

	//播放音频 
	virtual bool Write(const unsigned char *data, int datasize) = 0;

	virtual int GetFree() = 0;

	static XAudioPlay* Get();

	XAudioPlay();
	virtual ~XAudioPlay();
};

