#pragma once
//���̸߳���������ʾ��Ƶ

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

	//�� ���۳ɹ���� ������
	virtual bool Open(AVCodecParameters *para, IVideoCall *call, int width, int height);
	
	//��ͣ����
	bool isPause = false;

	void SetPause(bool isPause);

	void run();

	//����pts,������յ��Ľ�������pts > = seekPts ,����true ����ʾ����
	virtual bool RepaintPts(AVPacket *pkt, long long seekPts);

	//ͬ��ʱ�� �ⲿ����
	long long synpts = 0;





protected:	
	std::mutex vmux;

	IVideoCall *call = 0;
};

