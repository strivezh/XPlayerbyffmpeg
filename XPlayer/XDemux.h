#pragma once

#include <mutex>

/*���װ�࣬�������Ƶ�ļ������װ����֡��Ϣ*/

struct AVPacket;
struct AVFormatContext;
struct AVCodecParameters;



class XDemux
{
public:
	//��ý���ļ�������ý�� rtmp http rtsp
	virtual bool Open(const char * url);


	//�ռ���Ҫ�������ͷ�,�ͷ�AVPacket����ռ�����ݿռ�  av_packet_free
	virtual AVPacket *Read();

	//ֻ����Ƶ����Ƶ����
	virtual AVPacket *ReadVideo();

	//�ж� ��Ƶ or ��Ƶ
	virtual bool IsAudio(AVPacket *pkt);

	//��ȡ��Ƶ���� ���صĿռ���Ҫ����  avcodec_parameters_free
	virtual AVCodecParameters *CopyVPara();

	//��ȡ��Ƶ���� ���صĿռ���Ҫ����
	virtual AVCodecParameters *CopyAPara();


	//seek λ�� pos 0.0~1.0
	virtual bool Seek(double pos);

	//����
	virtual void Clear();
	//�ر�
	virtual void Close();


	XDemux();
	virtual ~XDemux();

	//ý����ʱ�� ms
	int totalMs;

	int width;
	int height;

	int sampleRate = 0;
	int channels = 0;



protected:
	std::mutex mux;
	//���װ������
	AVFormatContext *ic = NULL;

	//����Ƶ����
	int videoStream = 0;
	int audioStream = 1;



};

