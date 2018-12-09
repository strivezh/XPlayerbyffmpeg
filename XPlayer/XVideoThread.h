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
	
	//暂停功能
	bool isPause = false;

	void SetPause(bool isPause);

	void run();

	//解码pts,如果接收到的解码数据pts > = seekPts ,返回true 并显示画面
	virtual bool RepaintPts(AVPacket *pkt, long long seekPts);

	//同步时间 外部传入
	long long synpts = 0;





protected:	
	std::mutex vmux;

	IVideoCall *call = 0;
};

