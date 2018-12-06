#include "XPlayer.h"
#include <QtWidgets/QApplication>
#include <XDemux.h>
#include <XDecode.h>
#include <XResample.h>
#include <QThread>
#include <XAudioPlay.h>
#include <iostream>

#include "XAudioThread.h""
#include "XVideoThread.h"

#include "XDemuxThread.h"
using namespace std;

#if 0
class TestThread:public QThread
{
public:

	XDemux demux;
	XDecode vdecode;
	//XDecode adecode;

	XVideoWidget *video = 0;
	//XResample resample;
	XAudioThread at;
	XVideoThread vt;


	void Init()
	{
		//测试XDemux
		//url 香港财经 rtmp://202.69.69.180:443/webcast/bshdlive-pc
		//url 湖南卫视 rtmp://58.200.131.2:1935/livetv/hunantv
		//char *url = "rtmp://58.200.131.2:1935/livetv/hunantv";
		//char *url = "../v1080p.mp4";
		char *url = "../v1080.mp4";
		//XDemux demux;
		cout << "demux.Open return " << demux.Open(url) << endl;
		demux.Read();
		//demux.Clear();
		//demux.Close();

		cout << "demux.Open return " << demux.Open(url) << endl;

		cout << "demux.CopyVPara return " << demux.CopyVPara() << endl;
		cout << "demux.CopyAPara return " << demux.CopyAPara() << endl;

		//cout << "demux.Seek return " << demux.Seek(0.99) << endl;

		//XDecode vdecode;
		cout << "Vdecode.Open() " << vdecode.Open(demux.CopyVPara()) << endl;
		//cout << "vdecode.clear()" << vdecode.Clear() << endl;
		//cout << "vdecode.close()" << vdecode.Close() << endl;

		//XDecode adecode;
	//cout << "Adecode.Open()" << adecode.Open(demux.CopyAPara()) << endl;

	//	cout << "resample.Open()" << resample.Open(demux.CopyAPara()) << endl;

		//XAudioPlay::Get()->channels = demux.channels;
    	//XAudioPlay::Get()->sampleRate = demux.sampleRate;

		//cout << demux.channels << " " << demux.sampleRate << endl;

	//	cout << "XAudioPlay::Get()->Open()" << XAudioPlay::Get()->Open() << endl;

		cout << "ap-> Open  " << at.Open(demux.CopyAPara(), demux.sampleRate, demux.channels) << endl;

		vt.Open(demux.CopyVPara(), video, demux.width, demux.height);
		at.start();
		vt.start();
		

 	}

	unsigned char*pcm = new unsigned char[1024 * 1024];

	void run()
	{
		for (;;)
		{
			AVPacket *pkt = demux.Read();
			if (demux.IsAudio(pkt))
			{
				at.Push(pkt);
				
#if 0 //手动测试
				//先解码
				adecode.Send(pkt);
				AVFrame *frame = adecode.Recv();
				//再重采样
				int len = resample.Resample(frame, pcm);				
				cout << "resample.resample" <<len << endl;

				while (len > 0)
				{
					if (XAudioPlay::Get()->GetFree() >= len)
					{
						XAudioPlay::Get()->Write(pcm, len);
						break;
					}
					msleep(1);
				}

#endif 
				//cout << "audio:" << frame << endl;
			}
			else
			{

#if 1
				vt.Push(pkt);
				
#else  //手动测试视频解码播放
				vdecode.Send(pkt);
				AVFrame *frame = vdecode.Recv();
				video->Repaint(frame);
				cout << "video:" << frame << endl;
				//msleep(40);
#endif
			}
			if (!pkt) break;
		}
	}

};

#endif

//文件 ../v1080p.mp4
int main(int argc, char *argv[])
{


	////测试XDemux
	////url 香港财经 rtmp://202.69.69.180:443/webcast/bshdlive-pc
	////url 湖南卫视 rtmp://58.200.131.2:1935/livetv/hunantv
	////char *url = "rtmp://58.200.131.2:1935/livetv/hunantv";
	//char *url = "../v1080p.mp4";
	//XDemux demux;
	//cout << "demux.Open return " << demux.Open(url) << endl;
	//demux.Read();
	////demux.Clear();
	////demux.Close();

	//cout << "demux.Open return " << demux.Open(url) << endl;

	//cout << "demux.CopyVPara return " << demux.CopyVPara() << endl;
	//cout << "demux.CopyAPara return " << demux.CopyAPara() << endl;

	//cout << "demux.Seek return " << demux.Seek(0.99) << endl;

	//XDecode vdecode;
	//cout << "Vdecode.Open() " << vdecode.Open(demux.CopyVPara()) << endl;
	////cout << "vdecode.clear()" << vdecode.Clear() << endl;
	////cout << "vdecode.close()" << vdecode.Close() << endl;
	//
	//XDecode adecode;
	//cout << "Adecode.Open()" << adecode.Open(demux.CopyAPara()) << endl;

	//初始化显示
	
	//for (;;)
	//{
	//	AVPacket *pkt = demux.Read();
	//	if (demux.IsAudio(pkt))
	//	{
	//		adecode.Send(pkt);
	//		AVFrame *frame = adecode.Recv();
	//		cout << "audio:" << frame << endl;
	//	}
	//	else
	//	{
	//		vdecode.Send(pkt);
	//		AVFrame *frame = vdecode.Recv();
	//		cout << "video:" << frame << endl;

	//	}
	//	if (!pkt) break;
	//}

	//添加一个线程用来解码


	//TestThread tt;
	QApplication a(argc, argv);	
	XPlayer w;
	w.show();

	//XDemuxThread dt;
	//char *url = "rtmp://58.200.131.2:1935/livetv/hunantv";
	////char *url = "../v1080p.mp4";
	//dt.Open(url,w.ui.video);
	//dt.Start();


	return a.exec();

#if 0

	//初始化gl窗口
	tt.video = w.ui.video;

	//w.ui.video->Init(tt.demux.width, tt.demux.height);
	tt.Init();
	tt.start();
#endif
	
}


