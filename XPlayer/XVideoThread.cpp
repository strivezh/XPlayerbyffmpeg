#include "XVideoThread.h"

#include <iostream>

#include <XDecode.h>



//�� ���۳ɹ���� ������
bool XVideoThread::Open(AVCodecParameters *para, IVideoCall *call, int width, int height)
{
	if (!para) return false;

	Clear();

	vmux.lock();
	synpts=0;
	//��ʼ����ʾ����
	this->call = call;
	if (call)
	{
		call->Init(width, height);
	}
	vmux.unlock();
	//�򿪽����� ���ڸ�������
	//if (!decode) decode = new XDecode();

	int re = true;
	//�ڲ������� para
	if (!decode->Open(para))
	{
		std::cout << "Video XDecode open Failed" << std::endl;
		re = false;
	}


	std::cout << "XVideo Thread::Open return:" << re << std::endl;
	return re;
}

bool isPause = false;

void XVideoThread::SetPause(bool isPause)
{
	vmux.lock();
	this->isPause = isPause;
	vmux.unlock();
}

void XVideoThread::run()
{

	while (!isExit)
	{
		vmux.lock();

		if (isPause)
		{
			vmux.unlock();
			msleep(5);
			continue;
		}

		//����Ƶͬ��
		if (synpts > 0 && synpts < decode->pts)
		{
			vmux.unlock();
			msleep(1);
			continue;
		}


		AVPacket *pkt = Pop();
		////û������
		//if (packs.empty() || !decode)
		//{
		//	vmux.unlock();
		//	msleep(1);
		//	continue;
		//}

		//
		////ͷ��ȡ��
		//AVPacket *pkt = packs.front();
		//packs.pop_front();	//��һ��ǰ��

		bool re = decode->Send(pkt);		//����pkt ȥ����
		if (!re)
		{
			vmux.unlock();
			msleep(1);
			continue;
		}

#if 0
		//һ��send ���recv
		while (!isExit)
		{
			AVFrame * frame = decode->Recv();
			if (!frame)
			{
				break;
			}
			//��ʾ��Ƶ
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

		//��ʾ��Ƶ
		if (call)
		{
			call->Repaint(frame);
		}
		vmux.unlock();
#endif


	}
}

bool XVideoThread::RepaintPts(AVPacket *pkt, long long seekPts)
{
	vmux.lock();
	bool re = decode->Send(pkt);
	if (!re)
	{
		vmux.unlock();
		return true;		//��ʾ��������
	}
	decode->pts = 0;

	AVFrame *frame = decode->Recv();

	if (!frame)
	{
		vmux.unlock();
		return false;
	}

	//����λ��
	if (decode->pts >= seekPts)
	{
		if (call)
			call->Repaint(frame);
		vmux.unlock();
		return true;
	}
	XFreeFrame(&frame);	
	vmux.unlock();
	return false;
}



XVideoThread::XVideoThread()
{
}


XVideoThread::~XVideoThread()
{

}
