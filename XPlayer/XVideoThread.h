#pragma once
//此线程负责解码和显示视频

#include <QThread>
#include <mutex>
#include <list>
#include "IVideoCall.h"
#include "XDecodeThread.h""

struct AVCodecParameters;
//struct AVPacket;
//
//class XDecode;

class XVideoThread:public XDecodeThread
{
public:
	XVideoThread();
	virtual ~XVideoThread();

	//打开 不论成功与否 都清理
	virtual bool Open(AVCodecParameters *para, IVideoCall *call, int width, int height);
	


	void run();

	//同步时间 外部传入
	long long synpts = 0;

protected:
	IVideoCall *call = 0;
	std::mutex vmux;

};

