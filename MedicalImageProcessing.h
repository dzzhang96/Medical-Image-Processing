#pragma once



#include <qmessagebox.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <QtWidgets/QMainWindow>
#include <QWidget>
#include <QObject>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSphereSource.h>
#include <QVTKWidget.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkEventQtSlotConnect.h>
#include <QCheckBox.h>
#include <vtkFloatArray.h>
#include <vtkPolyLine.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h> //×ø±êÏµ½»»¥
#include <vtkExtractSurface.h>
#include "vtkImageReader2Factory.h"
#include "vtkImageReader2.h"
#include "vtkImageViewer2.h"
#include <vtkVersion.h>
#include <vtkImageAccumulate.h>
#include <vtkXYPlotActor.h>
#include <vtkCamera.h>
#include <vtkTextProperty.h>
#include <vtkImageNoiseSource.h>
#include <vtkImageThreshold.h>
#include <vtkInteractorStyleImage.h>
#include <vtkImageActor.h>
#include <vtkImageEllipsoidSource.h>
#include <vtkImageCast.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageData.h>
#include <vtkImageCast.h>
#include <vtkImageHybridMedian2D.h>
#include <vtkImageSobel2D.h>
#include <vtkImageMathematics.h>
#include <vtkImageMagnitude.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageShiftScale.h>
#include <vtkImageConvolve.h>
#include <vtkImageMapper3D.h>
#include <vtkImageLuminance.h>
#include <vtkImageSkeleton2D.h>



#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>  
#include <opencv2/core/utility.hpp>  
#include <opencv2/core/ocl.hpp>  
#include <opencv2/imgcodecs.hpp>    
using namespace cv;

#include "vtkAutoInit.h"
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QMouseEvent>
#include <qtimer.h>
#include "ui_MedicalImageProcessing.h"
#include "skeleton.h"
using namespace std;

class skeleton;

class MedicalImageProcessing : public QMainWindow
{
	Q_OBJECT

public:
	MedicalImageProcessing(QWidget *parent = Q_NULLPTR);
	vtkSmartPointer<vtkImageReader2> reader;
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkRenderer> renderer2;
	vtkSmartPointer<vtkRenderer> renderer3;
	vtkSmartPointer<vtkRenderer> renderer_otsu;
	vtkSmartPointer<vtkRenderer> renderer_entropy;
	vtkSmartPointer<vtkRenderer> renderer_conv;
	vtkSmartPointer<vtkImageData> NoiseImage;

	vector<int> histogram_vector;
	void ReadHistogram();
	Mat Vtkimage2mat();
	vtkSmartPointer<vtkImageData> mat2Vtkimage(const cv::Mat &sourceCVImage, bool flipOverXAxis);

	void initSlot();
	QString fileName;

	cv::Mat Srcimage;

public slots:
	void OnClickSkeleton();
	void OnAddImageFile_clicked();
	void Threshold(int value);
	void AddGaussianNoise();
	
	void GaussianFilter(int value);
	void RestoreGaussian();
	void Addnoise();
	void MedianFilter();
	void Sobel();
	void Prewitt();
	void Roberts();
	/////////////OpenCV///////////////
	void SelectElement();
	void Dilation();
	void Erosion();
	Mat Opening();
	Mat Closing();
	void DistanceImageUPdate();
	void DistanceTransform();
	void Skeleton();
	void SkeletonReconstruct();
	//void SkeletonReconstruct();
	void MEdgeDetect();
	void MGradient();
	void CBR();
	void OBR();
	void ROISelect();
	void ConditionDilation();

	/////////////OpenCV////////////////



private:
	Ui::MedicalImageProcessingClass ui;
	skeleton *m_skeleton;

	//vtkEventQtSlotConnect* m_EventQtSlotConnect;
	QTimer*timer;
	void Histogram();
	void Otsu();
	void Entropy();
	double generateGaussianNoise(double mu, double sigma);
	/////////////OpenCV///////////////
	cv::Mat Element;
	void QtDisplayMat(cv::Mat, QLabel*);
	cv::Mat BinaryImage, BinarySource, Distance, Distance2;
	cv::Mat SkeletonMat;
	cv::Mat Out;
	cv::Mat Medum;
	int Nr, Nc, Count;
	int ROI[4];
	double max;
	/////////////OpenCV////////////////
	int flag = 0, PointCount = 0;
	cv::Mat Marker;
	virtual bool eventFilter(QObject *, QEvent *);
};
