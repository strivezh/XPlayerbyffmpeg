#pragma once

#include <mutex>

/*解封装类，负责打开视频文件，解封装后获得帧信息*/

struct AVPacket;
struct AVFormatContext;
struct AVCodecParameters;



class XDemux
{
public:
	//打开媒体文件或者流媒体 rtmp http rtsp
	virtual bool Open(const char * url);


	//空间需要调用者释放,释放AVPacket对象空间和数据空间  av_packet_free
	virtual AVPacket *Read();

	//只读视频，音频丢弃
	virtual AVPacket *ReadVideo();

	//判断 音频 or 视频
	virtual bool IsAudio(AVPacket *pkt);

	//获取视频参数 返回的空间需要清理  avcodec_parameters_free
	virtual AVCodecParameters *CopyVPara();

	//获取音频参数 返回的空间需要清理
	virtual AVCodecParameters *CopyAPara();


	//seek 位置 pos 0.0~1.0
	virtual bool Seek(double pos);

	//清理
	virtual void Clear();
	//关闭
	virtual void Close();


	XDemux();
	virtual ~XDemux();

	//媒体总时长 ms
	int totalMs;

	int width;
	int height;

	int sampleRate = 0;
	int channels = 0;



protected:
	std::mutex mux;
	//解封装上下文
	AVFormatContext *ic = NULL;

	//音视频索引
	int videoStream = 0;
	int audioStream = 1;



};

