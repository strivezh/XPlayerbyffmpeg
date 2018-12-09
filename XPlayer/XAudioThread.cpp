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
	//һ������re�ж������Ƿ񶼳ɹ���
	//��һ�������ǵ�������ִ�У����һ���ͷ�para
	//�˴����false ������para����������������
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

	//�ڲ������� para
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
		////ͷ��ȡ��
		//AVPacket *pkt = packs.front();
		//packs.pop_front();	//��һ��ǰ��

		AVPacket *pkt = Pop();

		bool re = decode->Send(pkt);		//����pkt ȥ����
		if (!re)
		{
			amux.unlock();
			msleep(1);
			continue;
		}

		//һ��send ���recv
		while (!isExit)
		{
			AVFrame * frame = decode->Recv();
			if (!frame)break;
			//��ȥ������δ���ŵ�ʱ�� ��λms
			pts = decode->pts - ap->GetNoPlayMs();

			//�ز���
			int size = res->Resample(frame, pcm);

			//������Ƶ
			while (!isExit)
			{
				if (size <= 0) break;
				//����δ���� �ռ䲻��
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
	//�������������ô�λ
	isExit = true;
	wait();
}
