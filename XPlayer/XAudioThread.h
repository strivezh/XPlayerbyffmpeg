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
	//��ǰ��Ƶ���ŵ�pts
	long long pts = 0;

	//�� ���۳ɹ���� ������
	virtual bool Open(AVCodecParameters *para, int sample_rate, int channels);

	//ֹͣ�̣߳�������Դ
	virtual void Close();

	virtual void Clear();

	//��ͣ����
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

