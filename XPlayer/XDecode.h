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
	//����������Ƶͨ�ã��˱�־λ��������
	bool isAudio = false;

	//��ǰ���뵽��pts
	long long pts = 0;

	//�򿪽����� ���۳ɹ�����ͷ�para�ռ�
	virtual bool Open(AVCodecParameters *para);
	
	virtual bool Clear();

	virtual bool Close();

	//���͵������߳� ���۳ɹ�����ͷ�pkt�ռ䣨�����ý�����ݣ�
	virtual bool Send(AVPacket *pkt);

	//��ȡ�������ݣ�һ��send������Ҫ���Recv,��ȡ�����е�����send NULL��Recv���
	//ÿ�θ���һ�ݣ��ɵ������ͷţ�av_frame_free
	virtual AVFrame *Recv();



	XDecode();
	virtual ~XDecode();

protected:
	AVCodecContext *codec = 0;
	//ͷ�ļ��о�����Ҫ���������ռ�
	std::mutex mux;
};

