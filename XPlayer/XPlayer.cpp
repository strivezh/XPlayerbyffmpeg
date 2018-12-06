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
}

