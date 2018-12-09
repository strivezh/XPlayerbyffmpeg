#pragma once

#include <QtWidgets/QWidget>
#include "ui_XPlayer.h"

class XPlayer : public QWidget
{
	Q_OBJECT

public:
	XPlayer(QWidget *parent = Q_NULLPTR);
	~XPlayer();


	//定时器 滑动条显示 
	void timerEvent(QTimerEvent *e);

	//窗口尺寸变化
	void resizeEvent(QResizeEvent *e);
	
	//双击全屏
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
