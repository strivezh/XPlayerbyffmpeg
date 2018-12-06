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

	//�������װ ��Ƶ���� ��Ƶ�����߳�
	virtual void Start();

	//�ر��߳� ������Դ
	virtual void Close();
	void run();

	bool isExit = false;

	XDemuxThread();
	virtual ~XDemuxThread();

protected:
	std::mutex mux;
	XDemux *demux = 0;
	XVideoThread *vt = 0;
	XAudioThread *at = 0;
};

