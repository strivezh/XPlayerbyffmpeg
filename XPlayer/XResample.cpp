#include "XResample.h"
#include <iostream>

extern "C"
{
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}

#pragma comment(lib,"swresample.lib")

//����������������һ�£����˲�����ʽ�����ΪS16
bool XResample::Open(AVCodecParameters *para, bool isClearPara)
{
	if(!para) return false;
	mux.lock();
	//��Ƶ�ز���  �����ĳ�ʼ��
	
	//if(!actx)	
	//	actx = swr_alloc();
	//���actxΪNULL �����ռ�
	actx = swr_alloc_set_opts(
		actx,
		av_get_default_channel_layout(2),		//���ͨ������
		(AVSampleFormat)outFormat,				//���������ʽ /*AV_SAMPLE_FMT_S16*/
		para->sample_rate,						//���������

		av_get_default_channel_layout(para->channels),		//����ͨ����
		(AVSampleFormat)para->format,						//����������ʽ
		para->sample_rate,									//���������
		0, 0
	);
	//������ͷ�para
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

//�����ز������С ���۳ɹ���񣬶��ͷ�indata
int XResample::Resample(AVFrame *indata, unsigned char *outdata)
{
	if (!indata || !outdata) return 0;
	if(!outdata)
	{
		av_frame_free(&indata);
		return 0;
	}
	//��Ƶ
	uint8_t *data[2] = { 0 }; 
	data[0] = outdata;
	int ret = swr_convert(
			actx,
			data,		//���
			indata->nb_samples,
			(const uint8_t **)indata->data,		//����
			indata->nb_samples
	);
	if (ret <= 0) return ret;
	int outsize = ret * indata->channels * av_get_bytes_per_sample((AVSampleFormat)outFormat);
	av_frame_free(&indata);
	//����������С
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
