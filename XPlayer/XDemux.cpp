
/*视频解封装*/
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
	//参数设置
	AVDictionary *opts = NULL;

	//设置trsp流以tcp协议打开
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//网络延时时间
	av_dict_set(&opts, "max_delay", "500", 0);

	//加锁保障多线程安全
	mux.lock();
	//解封装上下文
	//AVFormatContext *ic = NULL;
	int ret_code = avformat_open_input(
		&ic,
		url,
		0,		//传入0 表示自动选择解封器
		&opts	//参数设置，比如rtsp的延时时间等
	);

	//打开失败 打印错误信息
	if (ret_code != 0)
	{
		mux.unlock();
		char buf[1024] = { 0 };
		av_strerror(ret_code, buf, sizeof(buf) - 1);
		cout << "open" << url << " failed!:" << buf << endl;		
		return false;
	}

	//打开成功
	cout << "Open" << url << " Success!:" << endl;

	//获取流信息
	ret_code = avformat_find_stream_info(ic, 0);


	//总时长 ms
	//int totalMs;
	totalMs = ic->duration / AV_TIME_BASE * 1000; //duration的单位是 10^6  直接除以基数得到秒数 乘以1000 得到ms数

	cout << "total: " << totalMs << " ms" << endl;

	//打印视频流的详细信息
	av_dump_format(ic, 0, url, 0);
	//音视频索引
	//int videoStream = 0;
	//int audioStream = 1;

	//获取音视频流 （遍历或者函数获取）
	//调用函数获取视频流信息
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream *as = ic->streams[videoStream];

	width = as->codecpar->width;
	height = as->codecpar->height;

	cout << "codec_id=" << as->codecpar->codec_id << endl;
	cout << "format=" << as->codecpar->format << endl;
	cout << "=================================================================" << endl;
	cout << videoStream << " 视频信息" << endl;	
	cout << "width = " << as->codecpar->width << endl;
	cout << "height = " << as->codecpar->height << endl;
	//帧率
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;

	//获取音频流
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = ic->streams[audioStream];

	sampleRate = as->codecpar->sample_rate;
	channels = as->codecpar->channels;

	cout << "=================================================================" << endl;
	cout << audioStream << " 音频信息" << endl;
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
	mux.lock();		//open是一个线程，读是一个线程

	if (!ic)	//容错
	{
		mux.unlock();
		return 0;
	}
	AVPacket *pkt = av_packet_alloc();
	//读取一帧，并分配空间
	int ret = av_read_frame(ic, pkt);
	if (ret!=0)
	{
		mux.unlock();
		av_packet_free(&pkt);	//读取失败需要释放之前申请的内存
		return 0;
	}
	//pts转换为ms
	pkt->pts = pkt->pts*(1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts*(1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	
	mux.unlock();
	//cout << "pkt->pts =" << pkt->pts << " ms" << endl;
	return pkt;
 }

//此处存在bug， 真的有读不到数据的情况，后移进度条没影响，但是往前移动时，就挂了
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
	//防止阻塞
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


//seek 位置 pos 0.0~1.0 位置是一个百分比
bool XDemux::Seek(double pos)
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return false;
	}
	//清理读取缓冲 读网络流时需要注意
	avformat_flush(ic);

	long long seekPos = 0;
	//如果ic中不包含duration总时长，还可以自行计算
	seekPos = ic->streams[videoStream]->duration * pos;
	//int  ms = 3000;
	//long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
	
	int ret = av_seek_frame(ic, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	mux.unlock();
	if (ret < 0)return false;

	return true;
}

//清理 清理缓冲
void XDemux::Clear()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return;
	}
	//清理读取缓冲
	avformat_flush(ic);
	mux.unlock();
}
//关闭  清理ic
void XDemux::Close()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return;
	}
	//清理读取缓冲
	avformat_close_input(&ic);

	//清空媒体总时长
	totalMs = 0;
	mux.unlock();
}

XDemux::XDemux()
{
	static bool isFirst = true;
	//多线程安全
	static  std::mutex dmux;
	dmux.lock();
	if (isFirst)
	{
		//初始化封装库
		av_register_all();

		//初始化网络库 可以打开rtsp rtmp http 协议的流媒体视频
		avformat_network_init();
		isFirst = false;
	}
	dmux.unlock();	
}


XDemux::~XDemux()
{
}
