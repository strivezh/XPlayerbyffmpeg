
/*��Ƶ���װ*/
#include "XDemux.h"
#include <iostream>

using namespace std;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

bool XDemux::Open(const char * url)
{
	Close();
	//��������
	AVDictionary *opts = NULL;

	//����trsp����tcpЭ���
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//������ʱʱ��
	av_dict_set(&opts, "max_delay", "500", 0);

	//�������϶��̰߳�ȫ
	mux.lock();
	//���װ������
	//AVFormatContext *ic = NULL;
	int ret_code = avformat_open_input(
		&ic,
		url,
		0,		//����0 ��ʾ�Զ�ѡ������
		&opts	//�������ã�����rtsp����ʱʱ���
	);

	//��ʧ�� ��ӡ������Ϣ
	if (ret_code != 0)
	{
		mux.unlock();
		char buf[1024] = { 0 };
		av_strerror(ret_code, buf, sizeof(buf) - 1);
		cout << "open" << url << " failed!:" << buf << endl;		
		return false;
	}

	//�򿪳ɹ�
	cout << "Open" << url << " Success!:" << endl;

	//��ȡ����Ϣ
	ret_code = avformat_find_stream_info(ic, 0);


	//��ʱ�� ms
	//int totalMs;
	totalMs = ic->duration / AV_TIME_BASE * 1000; //duration�ĵ�λ�� 10^6  ֱ�ӳ��Ի����õ����� ����1000 �õ�ms��

	cout << "total: " << totalMs << " ms" << endl;

	//��ӡ��Ƶ������ϸ��Ϣ
	av_dump_format(ic, 0, url, 0);
	//����Ƶ����
	//int videoStream = 0;
	//int audioStream = 1;

	//��ȡ����Ƶ�� ���������ߺ�����ȡ��
	//���ú�����ȡ��Ƶ����Ϣ
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream *as = ic->streams[videoStream];

	width = as->codecpar->width;
	height = as->codecpar->height;

	cout << "codec_id=" << as->codecpar->codec_id << endl;
	cout << "format=" << as->codecpar->format << endl;
	cout << "=================================================================" << endl;
	cout << videoStream << " ��Ƶ��Ϣ" << endl;	
	cout << "width = " << as->codecpar->width << endl;
	cout << "height = " << as->codecpar->height << endl;
	//֡��
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;

	//��ȡ��Ƶ��
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = ic->streams[audioStream];

	sampleRate = as->codecpar->sample_rate;
	channels = as->codecpar->channels;

	cout << "=================================================================" << endl;
	cout << audioStream << " ��Ƶ��Ϣ" << endl;
	cout << "sample_rate = " << as->codecpar->sample_rate << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "channel = " << as->codecpar->channels << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "audio fps =" << r2d(as->avg_frame_rate) << endl;
	cout << "=================================================================" << endl;

	mux.unlock();
	return true;
}

bool XDemux::IsAudio(AVPacket *pkt)
{
	if (!pkt) return false;
	if (pkt->stream_index == videoStream)
		return false;
	return true;
}


AVPacket *XDemux::Read()
{
	mux.lock();		//open��һ���̣߳�����һ���߳�

	if (!ic)	//�ݴ�
	{
		mux.unlock();
		return 0;
	}
	AVPacket *pkt = av_packet_alloc();
	//��ȡһ֡��������ռ�
	int ret = av_read_frame(ic, pkt);
	if (ret!=0)
	{
		mux.unlock();
		av_packet_free(&pkt);	//��ȡʧ����Ҫ�ͷ�֮ǰ������ڴ�
		return 0;
	}
	//ptsת��Ϊms
	pkt->pts = pkt->pts*(1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts*(1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	
	mux.unlock();
	//cout << "pkt->pts =" << pkt->pts << " ms" << endl;
	return pkt;
 }

//�˴�����bug�� ����ж��������ݵ���������ƽ�����ûӰ�죬������ǰ�ƶ�ʱ���͹���
AVPacket *XDemux::ReadVideo()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return 0;		
	}
	mux.unlock();

	AVPacket *pkt = NULL;
	//��ֹ����
	for (int i = 0; i < 20; i++)
	{
		pkt = Read();
		if (!pkt)
		{
			break;
		}
		if (pkt->stream_index == videoStream)
		{	
			break;
		}
		av_packet_free(&pkt);
	}

	return pkt;
}


AVCodecParameters *XDemux::CopyVPara()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return NULL;
	}
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[videoStream]->codecpar);
	mux.unlock();
	return pa;
}


AVCodecParameters *XDemux::CopyAPara()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return NULL;
	}
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[audioStream]->codecpar);
	mux.unlock();
	return pa;
}


//seek λ�� pos 0.0~1.0 λ����һ���ٷֱ�
bool XDemux::Seek(double pos)
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return false;
	}
	//�����ȡ���� ��������ʱ��Ҫע��
	avformat_flush(ic);

	long long seekPos = 0;
	//���ic�в�����duration��ʱ�������������м���
	seekPos = ic->streams[videoStream]->duration * pos;
	//int  ms = 3000;
	//long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
	
	int ret = av_seek_frame(ic, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	mux.unlock();
	if (ret < 0)return false;

	return true;
}

//���� ������
void XDemux::Clear()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return;
	}
	//�����ȡ����
	avformat_flush(ic);
	mux.unlock();
}
//�ر�  ����ic
void XDemux::Close()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return;
	}
	//�����ȡ����
	avformat_close_input(&ic);

	//���ý����ʱ��
	totalMs = 0;
	mux.unlock();
}

XDemux::XDemux()
{
	static bool isFirst = true;
	//���̰߳�ȫ
	static  std::mutex dmux;
	dmux.lock();
	if (isFirst)
	{
		//��ʼ����װ��
		av_register_all();

		//��ʼ������� ���Դ�rtsp rtmp http Э�����ý����Ƶ
		avformat_network_init();
		isFirst = false;
	}
	dmux.unlock();	
}


XDemux::~XDemux()
{
}
