#include "XResample.h"
#include <iostream>

extern "C"
{
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}

#pragma comment(lib,"swresample.lib")

//输出参数和输入参数一致，除了采样格式，输出为S16
bool XResample::Open(AVCodecParameters *para, bool isClearPara)
{
	if(!para) return false;
	mux.lock();
	//音频重采样  上下文初始化
	
	//if(!actx)	
	//	actx = swr_alloc();
	//如果actx为NULL 会分配空间
	actx = swr_alloc_set_opts(
		actx,
		av_get_default_channel_layout(2),		//输出通道数量
		(AVSampleFormat)outFormat,				//输出样本格式 /*AV_SAMPLE_FMT_S16*/
		para->sample_rate,						//输出采样率

		av_get_default_channel_layout(para->channels),		//输入通道数
		(AVSampleFormat)para->format,						//输入样本格式
		para->sample_rate,									//输入采样率
		0, 0
	);
	//用完后释放para
	if(isClearPara)
		avcodec_parameters_free(&para);

	int ret = swr_init(actx);
	mux.unlock();
	if (ret != 0)
	{	
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		std::cout << "swr_init failed:" << buf << std::endl;
	}
	//unsigned char *pcm = NULL;

	return true;
}

//返回重采样后大小 不论成功与否，都释放indata
int XResample::Resample(AVFrame *indata, unsigned char *outdata)
{
	if (!indata || !outdata) return 0;
	if(!outdata)
	{
		av_frame_free(&indata);
		return 0;
	}
	//音频
	uint8_t *data[2] = { 0 }; 
	data[0] = outdata;
	int ret = swr_convert(
			actx,
			data,		//输出
			indata->nb_samples,
			(const uint8_t **)indata->data,		//输入
			indata->nb_samples
	);
	if (ret <= 0) return ret;
	int outsize = ret * indata->channels * av_get_bytes_per_sample((AVSampleFormat)outFormat);
	av_frame_free(&indata);
	//返回样本大小
	return outsize;

}


bool XResample::Close()
{
	mux.lock();

	if (actx)
		swr_free(&actx);

	mux.unlock();
	return true;
}

XResample::XResample()
{
}


XResample::~XResample()
{
}
