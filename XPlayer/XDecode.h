#pragma once
#include <mutex>
struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

extern void XFreePakcet(AVPacket **pkt);
extern void XFreeFrame(AVFrame **frame);

class XDecode
{
public:
	//解码器音视频通用，此标志位用来区分
	bool isAudio = false;

	//当前解码到的pts
	long long pts = 0;

	//打开解码器 不论成功与否都释放para空间
	virtual bool Open(AVCodecParameters *para);
	
	virtual bool Clear();

	virtual bool Close();

	//发送到解码线程 不论成功与否都释放pkt空间（对象和媒体内容）
	virtual bool Send(AVPacket *pkt);

	//获取解码数据，一次send可能需要多次Recv,获取缓冲中的数据send NULL在Recv多次
	//每次复制一份，由调用者释放，av_frame_free
	virtual AVFrame *Recv();



	XDecode();
	virtual ~XDecode();

protected:
	AVCodecContext *codec = 0;
	//头文件中尽量不要引用命名空间
	std::mutex mux;
};

