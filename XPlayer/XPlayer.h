#pragma once

#include <QtWidgets/QWidget>
#include "ui_XPlayer.h"

class XPlayer : public QWidget
{
	Q_OBJECT

public:
	XPlayer(QWidget *parent = Q_NULLPTR);
	~XPlayer();
public slots:
	void OpenFile();

private:

	Ui::XPlayerClass ui;
};
