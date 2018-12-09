#pragma once

#include <QtWidgets/QWidget>
#include "ui_XPlayer.h"

class XPlayer : public QWidget
{
	Q_OBJECT

public:
	XPlayer(QWidget *parent = Q_NULLPTR);
	~XPlayer();


	//��ʱ�� ��������ʾ 
	void timerEvent(QTimerEvent *e);

	//���ڳߴ�仯
	void resizeEvent(QResizeEvent *e);
	
	//˫��ȫ��
	void mouseDoubleClickEvent(QMouseEvent *e);

	void SetPause(bool isPause);

	bool isSliderPress = false;

public slots:
	void OpenFile();
	void PlayOrPause();
	void SliderPressed();
	void SliderReleased();

private:

	Ui::XPlayerClass ui;
};
