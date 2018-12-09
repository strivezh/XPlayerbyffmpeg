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
	//ѡ���ļ�
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("ѡ����Ƶ�ļ�"));
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

//��ʱ�� ��������ʾ
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

//�϶�������
void XPlayer::SliderPressed()
{
	isSliderPress = true;
}

void XPlayer::SliderReleased()
{
	isSliderPress = false;

	double pos = 0.0;

	//��ȡpos��ֵ
	pos = (double)ui.playPos->value() / (double)ui.playPos->maximum();

	dt->Seek(pos);
}


//���� resizeEvent
//���ڳߴ�仯
void XPlayer::resizeEvent(QResizeEvent *e)
{
	ui.playPos->move(50, this->height() - 50);
	ui.playPos->resize(this->width() - 100, ui.playPos->height());

	//������ťλ��
	ui.openFile->move(50, this->height() - 100);
	ui.isPlay->move(ui.openFile->x()+ui.openFile->width()+10, ui.openFile->y());

	ui.video->resize(this->size());
	
}

//˫��ȫ��
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
		//ui.isPlay->setText(u8"�� ��");
		ui.isPlay->setText(QString::fromLocal8Bit("�� ��"));
	}
	else
	{
		//ui.isPlay->setText(u8"�� ͣ");
		ui.isPlay->setText(QString::fromLocal8Bit("�� ͣ"));
	}
}

void XPlayer::PlayOrPause()
{
	//��ȡ��ǰ״̬ ��ȡ��
	bool isPause = !dt->isPause;

	//����״̬
	SetPause(isPause);
	dt->SetPause(isPause);

}
