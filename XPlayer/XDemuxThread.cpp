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
			//没有创建好，等待一会
			mux.unlock();
			msleep(5);
			continue;
		}

		//音视频同步 把音频pts时间 传入视频中，视频中在做同步
		if (vt && at)
		{
			vt->synpts = at->pts;
		}

		AVPacket *pkt = demux->Read();
		if (!pkt) 
		{
			//未读到数据
			mux.unlock();
			msleep(5);
			continue;
		}
		//读到音频数据
		if (demux->IsAudio(pkt))
		{
			//不一定有音频数据，加个保护
			if(at) 
				at->Push(pkt);
		}
		else  //视频数据
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

	//打开解封装
	bool re = demux->Open(url);
	if (!re)
	{
		std::cout << "demux->Open(url) failed" << std::endl;
		mux.unlock();
		return false;
	}
	//打开视频解码器和处理线程
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height))
	{
		
		re = false;
		std::cout << "vt->Open failed" << std::endl;
	}
	
	//打开音频解码器和处理线程
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

	//启动当前线程
	QThread::start();
	//启动VT 
	if (vt) vt->start();
	//启动AT
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
