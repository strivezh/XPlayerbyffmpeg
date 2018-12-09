#include "XDemuxThread.h"

#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"

#include "XDecode.h"
#include <iostream>

extern "C"
{
#include <libavformat/avformat.h>
}
#pragma comment(lib,"avformat.lib")

void XDemuxThread::SetPause(bool isPause)
{
	mux.lock();
	this->isPause = isPause;
	if (at) at->SetPause(isPause);
	if (vt) vt->SetPause(isPause);
	mux.unlock();
}

void XDemuxThread::Clear()
{
	mux.lock();
	if (demux) demux->Clear();
	if (vt) vt->Clear();
	if (at) at->Clear();
	mux.unlock();


}

 void XDemuxThread::Seek(double pos)
{
	 //�建��
	 Clear();
	 mux.lock();
	 bool states = this->isPause;
	 mux.unlock();

	 //����ͣ
	 SetPause(true);

	 mux.lock();
	 if (demux)
		 demux->Seek(pos);
	 //ʵ��Ҫ��ʾ��λ��
	 long long seekPts = pos * demux->totalMs;

	 while (!isExit)
	 {
		 AVPacket *pkt = demux->ReadVideo();
		 if (!pkt) continue;

		 //������뵽seekPts
		 if (vt->RepaintPts(pkt, seekPts))
		 {
			 this->pts = seekPts;
			 break;
		 }
		
		 ////��Ƶ���� ����
		 //bool re = vt->decode->Send(pkt);

		 //if (!re) break;
		 //AVFrame *frame = vt->decode->Recv();

		 //if (!frame) continue;
		 ////����λ�� ��ʾ
		 //if (frame->pts >= seekPts)
		 //{
			// this->pts = frame->pts;
			// vt->call->Repaint(frame);
			// break;
		 //}
		 ////û��λ�� �ͷ�
		 //av_frame_free(&frame);
	 }
	 mux.unlock();

	 //Seekʱ����ͣ״̬
	  if (!states)
		 SetPause(false);
}

void XDemuxThread::run()
{
	while (!isExit)
	{
		
		mux.lock();

		if (isPause)
		{
			mux.unlock();
			msleep(5);
			continue;
		}

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
			pts = at->pts;
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

	//����Ƶʱ��Ϊ��������ʾ��ֵ ��ʱ��
	totalMs = demux->totalMs;
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
