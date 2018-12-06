#pragma once
#include <QThread>
#include <list>
#include <mutex>


//�����������Ƶ
struct AVPacket;
class XDecode;


class XDecodeThread:public QThread
{
public:
	XDecodeThread();
	virtual ~XDecodeThread();

	virtual void Push(AVPacket *pkt);

	//�������
	virtual void Clear();

	//������Դ ֹͣ�߳�
	virtual void Close();



	//�����е����ݳ�ջ�����û�з���NULL
	virtual AVPacket*Pop(); 
	//������󳤶�
	int maxList = 100;
	//�߳��˳�����
	bool isExit = false;

protected:
	std::mutex mux;
	//���� ����ⲿ�����AVPacket  �Ƚ��ȳ�
	std::list<AVPacket *> packs;
	XDecode *decode = 0;
};

