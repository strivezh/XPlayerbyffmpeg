#pragma once


class XAudioPlay
{
public:

	int sampleRate = 44100;
	int sampleSize = 16;		//λ��
	int channels = 2;

	//����Ƶ���� ����Ϊ���麯��
	virtual bool Open() = 0;

	virtual bool Close() = 0;

	//���ػ����л�δ���ŵ�ʱ�� ms
	virtual long long GetNoPlayMs() = 0;

	//������Ƶ 
	virtual bool Write(const unsigned char *data, int datasize) = 0;

	virtual int GetFree() = 0;

	static XAudioPlay* Get();

	XAudioPlay();
	virtual ~XAudioPlay();
};

