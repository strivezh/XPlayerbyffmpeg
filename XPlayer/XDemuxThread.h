#pragma once
#include <QThread>
#include "IVideoCall.h"
#include <mutex>
class XDemux;
class XVideoThread;
class XAudioThread;

class XDemuxThread:public QThread
{
public:

	virtual bool Open(const char *url, IVideoCall *call);

	//启动解封装 音频解码 视频解码线程
	virtual void Start();

	//关闭线程 清理资源
	virtual void Close();
	void run();

	virtual void Seek(double pos);

	

	virtual void Clear();

	virtual void SetPause(bool isPause);

	bool isExit = false; 

	long long pts = 0;

	long long totalMs = 0;

	bool isPause = false;




	XDemuxThread();
	virtual ~XDemuxThread();

protected:
	std::mutex mux;
	XDemux *demux = 0;
	XVideoThread *vt = 0;
	XAudioThread *at = 0;
};

