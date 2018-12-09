
/*视频解码*/

#include "XDecode.h"

#include <iostream>

using namespace std;

extern "C"
{
#include <libavcodec/avcodec.h>
}

#pragma comment(lib,"avcodec.lib")

void XFreePakcet(AVPacket **pkt)
{
	if (!pkt || (*pkt)) return;
	av_packet_free(pkt);
}

void XFreeFrame(AVFrame **frame)
{
	if (!frame || (*frame)) return;
	av_frame_free(frame);
}

bool XDecode::Open(AVCodecParameters *para)
{
	if (!para) return false;
	Close();

	//找到解码器
	AVCodec *vcodec = avcodec_find_decoder(para->codec_id);
	if (!vcodec)
	{
		cout << "can't find the codec id " << para->codec_id << endl;
		avcodec_parameters_free(&para);
		return false;
	}
	cout << "find the avcodec " << para->codec_id << endl;

	mux.lock();
	///创建解码器上下文
	codec = avcodec_alloc_context3(vcodec);

	///配置解码器上下文参数
	avcodec_parameters_to_context(codec, para);
	avcodec_parameters_free(&para);
	//设置8线程解码
	codec->thread_count = 8;

	//打开解码器上下文
	int ret = avcodec_open2(codec, 0, 0);
	if (ret != 0)
	{
		avcodec_free_context(&codec);

		mux.unlock();
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		cout << "avcodec_open2 failed:" << buf << endl;		
		return false;
	}
	mux.unlock();
	cout << "video avcodec open2 success" << endl;
	//avcodec_parameters_free(&para);
	return true;
}

//发送到解码线程 不论成功与否都释放pkt空间（对象和媒体内容）
bool XDecode::Send(AVPacket *pkt)
{
	if (!pkt || pkt->size <= 0 || !pkt->data) return false;
	mux.lock();
	if (!codec)
	{
		mux.unlock();
		return false;
	}
	int re = avcodec_send_packet(codec, pkt);
	mux.unlock();
	av_packet_free(&pkt);
	if (re != 0)
		return false;
	return true;
}

//获取解码数据，一次send可能多次recv
 AVFrame  *XDecode::Recv()
{
	 mux.lock();
	 if (!codec)
	 {
		 mux.unlock();
		 return NULL;
	 }

	 AVFrame *frame = av_frame_alloc();
	 int ret = avcodec_receive_frame(codec, frame);
	 mux.unlock();
	 if (ret != 0)
	 {
		 av_frame_free(&frame);
		 return NULL;
	 }
	// cout << "[" << frame->linesize[0] << "] " << flush;
	 pts = frame->pts;
	 return frame;
}



bool XDecode::Clear()
{
	mux.lock();
	//清理解码缓冲
	if (codec)
		avcodec_flush_buffers(codec);
	mux.unlock();
	return true;

}

 bool  XDecode::Close()
{
	 mux.lock();
	 if (codec)
	 {
		 avcodec_close(codec);
		 avcodec_free_context(&codec);
	 }
	 pts = 0;
	 mux.unlock();
	 return true;
}



XDecode::XDecode()
{
}


XDecode::~XDecode()
{
}
