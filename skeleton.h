#pragma once
#include "MedicalImageProcessing.h"
#include <QWidget>
#include "ui_skeleton.h"

class skeleton : public QWidget
{
	Q_OBJECT

public:
	skeleton(QWidget *parent = Q_NULLPTR);
	~skeleton();
	
	vtkSmartPointer<vtkImageReader2> m_reader;

public slots:
	void SkeletonAnime(int value);

private:
	Ui::skeleton ui;

};