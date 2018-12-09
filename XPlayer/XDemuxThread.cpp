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
	 //清缓存
	 Clear();
	 mux.lock();
	 bool states = this->isPause;
	 mux.unlock();

	 //先暂停
	 SetPause(true);

	 mux.lock();
	 if (demux)
		 demux->Seek(pos);
	 //实际要显示的位置
	 long long seekPts = pos * demux->totalMs;

	 while (!isExit)
	 {
		 AVPacket *pkt = demux->ReadVideo();
		 if (!pkt) continue;

		 //如果解码到seekPts
		 if (vt->RepaintPts(pkt, seekPts))
		 {
			 this->pts = seekPts;
			 break;
		 }
		
		 ////视频数据 解码
		 //bool re = vt->decode->Send(pkt);

		 //if (!re) break;
		 //AVFrame *frame = vt->decode->Recv();

		 //if (!frame) continue;
		 ////到达位置 显示
		 //if (frame->pts >= seekPts)
		 //{
			// this->pts = frame->pts;
			// vt->call->Repaint(frame);
			// break;
		 //}
		 ////没到位置 释放
		 //av_frame_free(&frame);
	 }
	 mux.unlock();

	 //Seek时非暂停状态
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
			//没有创建好，等待一会
			mux.unlock();
			msleep(5);
			continue;
		}

		//音视频同步 把音频pts时间 传入视频中，视频中在做同步
		if (vt && at)
		{
			pts = at->pts;
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

	//打开视频时，为进度条显示赋值 总时长
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
