#include "XPlayer.h"
#include <QFileDialog>
#include <qDebug>
#include "XDemuxThread.h"
#include <QMessageBox>
static XDemuxThread *dt = 0;

XPlayer::XPlayer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	dt = new XDemuxThread();
	dt->Start();
	startTimer(40);
}

XPlayer::~XPlayer()
{
	dt->Close();
}

void XPlayer::OpenFile()
{
	//选择文件
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择视频文件"));
	//qDebug() << name;

	if (name.isEmpty()) return;
	this->setWindowTitle(name);

	if (!dt->Open(name.toLocal8Bit(), ui.video))
	{
		QMessageBox::information(0, "error", "open file failed !");
		return;
	}

	SetPause(dt->isPause);
}

//定时器 滑动条显示
void  XPlayer::timerEvent(QTimerEvent *e)
{
	if (isSliderPress) return;
	long long total = dt->totalMs;
	if (total > 0)
	{
		double pos = (double)dt->pts / (double)total;
		int v = ui.playPos->maximum() * pos;
		ui.playPos->setValue(v);
	}
}

//拖动滑动条
void XPlayer::SliderPressed()
{
	isSliderPress = true;
}

void XPlayer::SliderReleased()
{
	isSliderPress = false;

	double pos = 0.0;

	//获取pos比值
	pos = (double)ui.playPos->value() / (double)ui.playPos->maximum();

	dt->Seek(pos);
}


//重载 resizeEvent
//窗口尺寸变化
void XPlayer::resizeEvent(QResizeEvent *e)
{
	ui.playPos->move(50, this->height() - 50);
	ui.playPos->resize(this->width() - 100, ui.playPos->height());

	//调整按钮位置
	ui.openFile->move(50, this->height() - 100);
	ui.isPlay->move(ui.openFile->x()+ui.openFile->width()+10, ui.openFile->y());

	ui.video->resize(this->size());
	
}

//双击全屏
void XPlayer::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (isFullScreen())
		this->showNormal();
	else
		this->showFullScreen();
}

void XPlayer::SetPause(bool isPause)
{
	if (isPause)
	{
		//ui.isPlay->setText(u8"播 放");
		ui.isPlay->setText(QString::fromLocal8Bit("播 放"));
	}
	else
	{
		//ui.isPlay->setText(u8"暂 停");
		ui.isPlay->setText(QString::fromLocal8Bit("暂 停"));
	}
}

void XPlayer::PlayOrPause()
{
	//获取当前状态 并取反
	bool isPause = !dt->isPause;

	//设置状态
	SetPause(isPause);
	dt->SetPause(isPause);

}
