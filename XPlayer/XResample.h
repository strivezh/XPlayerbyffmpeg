#pragma once
#include <mutex>

struct AVCodecParameters;
struct SwrContext;
struct AVFrame;

class XResample
{
public:
	//����������������һ�£����˲�����ʽ�����ΪS16  �ͷ�para
	virtual bool Open(AVCodecParameters *para, bool isClearPara=false);

	virtual bool Close();

	//�����ز������С ���۳ɹ���񣬶��ͷ�indata
	virtual int Resample(AVFrame *indata, unsigned char *data);


	XResample();
	~XResample();

	int outFormat = 1; //AV_SAMPLE_FMT_S16

protected:
	std::mutex mux;

	SwrContext *actx = 0;
};

