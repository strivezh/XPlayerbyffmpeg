#include "XAudioThread.h"
#include <iostream>
#include <XDecode.h>
#include <XAudioPlay.h>
#include <XResample.h>

void XAudioThread::Clear()
{
	XDecodeThread::Clear();
	mux.lock();

	if (ap) ap->Clear();

	mux.unlock();
}

void XAudioThread::Close()
{
	XDecodeThread::Close();
	if (res)
	{
		res->Close();
		amux.lock();
		delete res;
		res = NULL;
		amux.unlock();
	}
	if (ap)
	{
		ap->Close();
		amux.lock();
		ap = NULL;	
		amux.unlock();
	}
}

bool XAudioThread::Open(AVCodecParameters *para, int sample_rate, int channels)
{

	if (!para) return false;

	Clear();

	amux.lock();
	pts = 0;
	/*if (!decode) decode = new XDecode();
	if (!res) res = new XResample();
	if (!ap) ap = XAudioPlay::Get();*/
	bool re = true;
	//一方面用re判断三步是否都成功了
	//另一个方面是的三步都执行，最后一步释放para
	//此处添加false 不清理para，否则后面调不到了
	if (!res->Open(para, false))
	{
		re = false;
		//amux.unlock;
		//return false;
	}
	ap->sampleRate = sample_rate;
	ap->channels = channels;

	if (!ap->Open())
	{
		re = false;
		std::cout << "XAudio Play Failed" << std::endl;
		//amux.unlock;
		//return false;
	}

	//内部会清理 para
	if (!decode->Open(para))
	{
		std::cout << "Audio XDecode open Failed" << std::endl;
		re = false;
	}

	amux.unlock();
	std::cout << "XAudio Thread::Open return:" << re << std::endl;
	return re;
}

void XAudioThread::SetPause(bool isPause)
{
	//amux.lock();
	this->isPause = isPause;
	if (ap)
	{
		ap->SetPause(isPause);
	}
	//amux.unlock();
}

void XAudioThread::run()
{
	unsigned char * pcm = new unsigned char[1024 * 1024 * 10];
	while (!isExit)
	{
		amux.lock();

		if (isPause)
		{
			amux.unlock();
			msleep(5);
			continue;
		}
		//if (packs.empty() || !decode || !res || !ap)
		//{
		//	amux.unlock();
		//	msleep(1);
		//	continue;
		//}
		////头部取出
		//AVPacket *pkt = packs.front();
		//packs.pop_front();	//后一个前移

		AVPacket *pkt = Pop();

		bool re = decode->Send(pkt);		//发送pkt 去解码
		if (!re)
		{
			amux.unlock();
			msleep(1);
			continue;
		}

		//一次send 多次recv
		while (!isExit)
		{
			AVFrame * frame = decode->Recv();
			if (!frame)break;
			//减去缓冲中未播放的时间 单位ms
			pts = decode->pts - ap->GetNoPlayMs();

			//重采样
			int size = res->Resample(frame, pcm);

			//播放音频
			while (!isExit)
			{
				if (size <= 0) break;
				//缓冲未播完 空间不够
				if (ap->GetFree() < size || isPause)
				{
					msleep(1);
					continue;
				}
				ap->Write(pcm, size);
				break;
			}
		}

		amux.unlock();
	}
	delete pcm;
}

XAudioThread::XAudioThread()
{
	if (!res) res = new XResample();
	if (!ap) ap = XAudioPlay::Get();
}


XAudioThread::~XAudioThread()
{
	//析构函数中设置此位
	isExit = true;
	wait();
}
