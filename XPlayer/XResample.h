#pragma once
#include <mutex>

struct AVCodecParameters;
struct SwrContext;
struct AVFrame;

class XResample
{
public:
	//输出参数和输入参数一致，除了采样格式，输出为S16  释放para
	virtual bool Open(AVCodecParameters *para, bool isClearPara=false);

	virtual bool Close();

	//返回重采样后大小 不论成功与否，都释放indata
	virtual int Resample(AVFrame *indata, unsigned char *data);


	XResample();
	~XResample();

	int outFormat = 1; //AV_SAMPLE_FMT_S16

protected:
	std::mutex mux;

	SwrContext *actx = 0;
};

