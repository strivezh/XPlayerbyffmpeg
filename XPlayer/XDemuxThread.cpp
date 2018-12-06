#include "XDemuxThread.h"

#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"

#include <iostream>


void XDemuxThread::run()
{
	while (!isExit)
	{
		mux.lock();
		if (!demux)
		{
			//û�д����ã��ȴ�һ��
			mux.unlock();
			msleep(5);
			continue;
		}

		//����Ƶͬ�� ����Ƶptsʱ�� ������Ƶ�У���Ƶ������ͬ��
		if (vt && at)
		{
			vt->synpts = at->pts;
		}

		AVPacket *pkt = demux->Read();
		if (!pkt) 
		{
			//δ��������
			mux.unlock();
			msleep(5);
			continue;
		}
		//������Ƶ����
		if (demux->IsAudio(pkt))
		{
			//��һ������Ƶ���ݣ��Ӹ�����
			if(at) 
				at->Push(pkt);
		}
		else  //��Ƶ����
		{
			if(vt)
				vt->Push(pkt);
		}

		mux.unlock();
		msleep(1);
	}
}



bool XDemuxThread::Open(const char *url, IVideoCall *call)
{
	if (url == 0 || url[0] == '\0')
		return false;

	mux.lock();
	if (!demux) demux = new XDemux();
	if (!vt) vt = new XVideoThread();
	if (!at) at = new XAudioThread();

	//�򿪽��װ
	bool re = demux->Open(url);
	if (!re)
	{
		std::cout << "demux->Open(url) failed" << std::endl;
		mux.unlock();
		return false;
	}
	//����Ƶ�������ʹ����߳�
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height))
	{
		
		re = false;
		std::cout << "vt->Open failed" << std::endl;
	}
	
	//����Ƶ�������ʹ����߳�
	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels))
	{
		re = false;
		std::cout << "at->Open failed" << std::endl;
	}

	mux.unlock();

	std::cout << "XDemux Open return " << re << std::endl;
	return re;
}

void XDemuxThread::Start()
{
	mux.lock();
	if (!demux) demux = new XDemux();
	if (!vt) vt = new XVideoThread();
	if (!at) at = new XAudioThread();

	//������ǰ�߳�
	QThread::start();
	//����VT 
	if (vt) vt->start();
	//����AT
	if (at) at->start();
	mux.unlock();
}

void XDemuxThread::Close()
{
	isExit = true;
	wait();
	if (vt) vt->Close();
	if (at) at->Close();
	mux.lock();
	delete vt;
	delete at;
	vt = NULL;
	at = NULL;
	mux.unlock();
}

XDemuxThread::XDemuxThread()
{
}


XDemuxThread::~XDemuxThread()
{
	isExit = true;
	wait();
}
