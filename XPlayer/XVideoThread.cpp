#include "XVideoThread.h"

#include <iostream>

#include <XDecode.h>



//打开 不论成功与否 都清理
bool XVideoThread::Open(AVCodecParameters *para, IVideoCall *call, int width, int height)
{
	if (!para) return false;

	Clear();

	vmux.lock();
	synpts=0;
	//初始化显示窗口
	this->call = call;
	if (call)
	{
		call->Init(width, height);
	}
	vmux.unlock();
	//打开解码器 放在父类中做
	//if (!decode) decode = new XDecode();

	int re = true;
	//内部会清理 para
	if (!decode->Open(para))
	{
		std::cout << "Video XDecode open Failed" << std::endl;
		re = false;
	}


	std::cout << "XVideo Thread::Open return:" << re << std::endl;
	return re;
}



void XVideoThread::run()
{

	while (!isExit)
	{
		vmux.lock();

		//音视频同步
		if (synpts>0 && synpts < decode->pts)
		{
			vmux.unlock();
			msleep(1);
			continue;
		}
		

		AVPacket *pkt = Pop();
		////没有数据
		//if (packs.empty() || !decode)
		//{
		//	vmux.unlock();
		//	msleep(1);
		//	continue;
		//}

		//
		////头部取出
		//AVPacket *pkt = packs.front();
		//packs.pop_front();	//后一个前移

		bool re = decode->Send(pkt);		//发送pkt 去解码
		if (!re)
		{
			vmux.unlock();
			msleep(1);
			continue;
		}
		
#if 0
		//一次send 多次recv
		while (!isExit)
		{
			AVFrame * frame = decode->Recv();
			if (!frame)
			{
				break;
			}
			//显示视频
			if (call)
			{
				call->Repaint(frame);
			}
		}
		vmux.unlock();
#else
		AVFrame * frame = decode->Recv();
		if (!frame)
		{
			vmux.unlock();
			continue;
		}
			
		//显示视频
		if (call)
		{
			call->Repaint(frame);
		}
		vmux.unlock();
#endif
	

	}
}

XVideoThread::XVideoThread()
{
}


XVideoThread::~XVideoThread()
{

}
