#pragma once
#include <QThread>
#include <mutex>
#include <list>
#include "XDecodeThread.h"

struct AVCodecParameters;
//struct AVPacket;

//class XDecode;
class XAudioPlay;
class XResample;

class XAudioThread:public XDecodeThread
{
public:
	//当前音频播放的pts
	long long pts = 0;

	//打开 不论成功与否 都清理
	virtual bool Open(AVCodecParameters *para, int sample_rate, int channels);

	//停止线程，清理资源
	virtual void Close();

	virtual void Clear();

	//暂停功能
	bool isPause = false;

	void SetPause(bool isPause);

	void run();

	XAudioThread();
	virtual ~XAudioThread();

protected:

	XAudioPlay *ap = 0;
	XResample *res = 0;

	std::mutex amux;

	
};

