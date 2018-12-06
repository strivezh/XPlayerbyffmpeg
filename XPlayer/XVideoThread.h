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
	


	void run();

	//ͬ��ʱ�� �ⲿ����
	long long synpts = 0;

protected:
	IVideoCall *call = 0;
	std::mutex vmux;

};

