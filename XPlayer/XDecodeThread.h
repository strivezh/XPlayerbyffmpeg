#pragma once
#include <QThread>
#include <list>
#include <mutex>


//解码和运行视频
struct AVPacket;
class XDecode;


class XDecodeThread:public QThread
{
public:
	XDecodeThread();
	virtual ~XDecodeThread();

	virtual void Push(AVPacket *pkt);

	//清理队列
	virtual void Clear();

	//清理资源 停止线程
	virtual void Close();



	//队列中的数据出栈，如果没有返回NULL
	virtual AVPacket*Pop(); 
	//队列最大长度
	int maxList = 100;
	//线程退出函数
	bool isExit = false;

protected:
	std::mutex mux;
	//链表 存放外部传入的AVPacket  先进先出
	std::list<AVPacket *> packs;
	XDecode *decode = 0;
};

