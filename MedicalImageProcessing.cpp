#include "MedicalImageProcessing.h"


#include <qfiledialog.h>
#include <qcolordialog.h>
#include <vtkAutoInit.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
//VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)
//VTK_MODULE_INIT(vtkRenderingContextOpenGL)
VTK_MODULE_INIT(vtkRenderingFreeType)


MedicalImageProcessing::MedicalImageProcessing(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	m_skeleton = new skeleton;

	reader = vtkSmartPointer<vtkImageReader2>::New();
	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer2 = vtkSmartPointer<vtkRenderer>::New();
	renderer3 = vtkSmartPointer<vtkRenderer>::New();
	renderer_otsu = vtkSmartPointer<vtkRenderer>::New();
	renderer_entropy = vtkSmartPointer<vtkRenderer>::New();
	renderer_conv = vtkSmartPointer<vtkRenderer>::New();
	


	initSlot();

	ui.qvtkWidget_original->GetRenderWindow()->AddRenderer(renderer);
	ui.qvtkWidget_histogram->GetRenderWindow()->AddRenderer(renderer2);
	ui.qvtkWidget_threshold->GetRenderWindow()->AddRenderer(renderer3);
	ui.qvtkWidget_otsu->GetRenderWindow()->AddRenderer(renderer_otsu);
	ui.qvtkWidget_entropy->GetRenderWindow()->AddRenderer(renderer_entropy);
	ui.qvtkWidget_conv->GetRenderWindow()->AddRenderer(renderer_conv);


	
	
}

void MedicalImageProcessing::initSlot()
{
	this->connect(ui.loadimage, SIGNAL(triggered()), this, SLOT(OnAddImageFile_clicked()));
	this->connect(ui.ThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(Threshold(int)));
	this->connect(ui.pushButton_gaussiannoise, SIGNAL(clicked()), this, SLOT(AddGaussianNoise()));
	this->connect(ui.pushButton_gaussian, SIGNAL(clicked()), this, SLOT(RestoreGaussian()));	
	this->connect(ui.GaussianSlider, SIGNAL(valueChanged(int)), this, SLOT(GaussianFilter(int)));
	this->connect(ui.pushButton_addnoise, SIGNAL(clicked()), this, SLOT(Addnoise()));
	this->connect(ui.pushButton_median, SIGNAL(clicked()), this, SLOT(MedianFilter()));
	this->connect(ui.pushButton_sobel, SIGNAL(clicked()), this, SLOT(Sobel()));
	this->connect(ui.pushButton_roberts, SIGNAL(clicked()), this, SLOT(Roberts()));
	this->connect(ui.pushButton_prewitt, SIGNAL(clicked()), this, SLOT(Prewitt()));

	this->ui.label_1->setText("");
	this->ui.label_2->setText("");
	this->ui.label_3->setText("");
	this->ui.label_4->setText("");

	this->timer = new QTimer;
	this->timer->setInterval(100);

	this->connect(ui.SEOK, SIGNAL(clicked()), this, SLOT(SelectElement()));
	this->connect(ui.pushButton_dilation, SIGNAL(clicked()), this, SLOT(Dilation()));
	this->connect(ui.pushButton_erosion, SIGNAL(clicked()), this, SLOT(Erosion()));
	this->connect(ui.pushButton_opening, SIGNAL(clicked()), this, SLOT(Opening()));
	this->connect(ui.pushButton_closing, SIGNAL(clicked()), this, SLOT(Closing()));
	this->connect(timer, SIGNAL(timeout()), this, SLOT(DistanceImageUPdate()));
	this->connect(ui.pushButton_distance, SIGNAL(clicked()), this, SLOT(DistanceTransform()));
	this->connect(ui.pushButton_skeletonanime, SIGNAL(clicked()), this, SLOT(OnClickSkeleton()));
	this->connect(ui.pushButton_skeleton, SIGNAL(clicked()), this, SLOT(Skeleton()));
	this->connect(ui.pushButton_skeletonrec, SIGNAL(clicked()), this, SLOT(SkeletonReconstruct()));
	this->connect(ui.pushButton_medgedetect, SIGNAL(clicked()), this, SLOT(MEdgeDetect()));
	this->connect(ui.pushButton_mgradient, SIGNAL(clicked()), this, SLOT(MGradient()));
	this->connect(ui.pushButton_obr, SIGNAL(clicked()), this, SLOT(OBR()));
	this->connect(ui.pushButton_cbr, SIGNAL(clicked()), this, SLOT(CBR()));
	this->connect(ui.pushButton_condilation, SIGNAL(clicked()), this, SLOT(ConditionDilation()));
	this->connect(ui.pushButton_ROISelect, SIGNAL(clicked()), this, SLOT(ROISelect()));
}

void MedicalImageProcessing::OnClickSkeleton() {
	
	m_skeleton->show();

}

void MedicalImageProcessing::OnAddImageFile_clicked()
{
	renderer->Clear();
	renderer->RemoveAllViewProps();

	QString filepath = QFileDialog::getOpenFileName(0, "Open a Picture");

	QString file_name;
	QFileInfo fileinfo;
	fileinfo = QFileInfo(filepath);
	file_name = fileinfo.fileName();


	if (filepath.isEmpty())
	{
		QMessageBox::warning(0, "Warning", "No File Be Opened");
		return;
	}
	// 支持带中文路径的读取  
	QByteArray ba = filepath.toLocal8Bit();
	const char *fileName_str = ba.data();

	//读取图像  

	vtkSmartPointer<vtkImageReader2Factory> readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();
	reader = readerFactory->CreateImageReader2(fileName_str);
    reader->SetFileName(fileName_str);
    reader->Update();

	//////////////
	m_skeleton->m_reader = reader;
	//////////////

	vtkSmartPointer<vtkImageViewer2> imageViewer =
	vtkSmartPointer<vtkImageViewer2>::New();
	imageViewer->SetInputConnection(reader->GetOutputPort());
	imageViewer->GetRenderer()->ResetCamera();

	vtkSmartPointer<vtkCamera> cam = vtkSmartPointer<vtkCamera>::New();
	cam = imageViewer->GetRenderer()->GetActiveCamera();
	cam->Zoom(1.4);

	// Set up an interactor that does not respond to mouse events
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	imageViewer->GetRenderWindow()->SetInteractor(renderWindowInteractor);
	renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_original->SetRenderWindow(imageViewer->GetRenderWindow());
	ui.qvtkWidget_original->GetRenderWindow()->Render();


	ui.label_name->setText(file_name);
	int dims[3];
	reader->GetOutput()->GetDimensions(dims);
	QString dimension= QString::number(dims[0], 10);
	dimension.append(" x ");
	dimension.append(QString::number(dims[1], 10));
	dimension.append(" x ");
	dimension.append(QString::number(dims[2], 10));
	ui.label_dimension->setText(dimension);

	Histogram();
	ReadHistogram();
	Threshold(100);
	Otsu();
	Entropy();
}

void MedicalImageProcessing::Histogram() {

	renderer2->Clear();
	renderer2->RemoveAllViewProps();

	int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();

	// Create a vtkXYPlotActor
	vtkSmartPointer<vtkXYPlotActor> plot =
		vtkSmartPointer<vtkXYPlotActor>::New();
	plot->ExchangeAxesOff();
	plot->SetLabelFormat("%g");
	plot->SetXTitle("Level");
	plot->SetYTitle("Frequency");
	plot->SetYTitlePositionToVCenter();
	plot->SetXValuesToValue();
	plot->GetPositionCoordinate()->SetValue(0, 0);
	plot->GetPosition2Coordinate()->SetValue(1, 1);

	vtkTextProperty *p = vtkTextProperty::New();
	//p->SetColor(0, 1.0, 0.0);
	p->SetFontSize(100);
	p->SetBold(5);
	plot->SetAxisLabelTextProperty(p);

	double xmax = 0.;
	double ymax = 0.;

	double colors[3][3] = {
	  { 1, 0, 0 },
	  { 0, 1, 0 },
	  { 0, 0, 1 } };

	const char* labels[3] = {
	  "Red", "Green", "Blue" };

	// Process the image, extracting and plotting a histogram for each
	// component
	for (int i = 0; i < numComponents; ++i)
	{
		vtkSmartPointer<vtkImageExtractComponents> extract =
			vtkSmartPointer<vtkImageExtractComponents>::New();
		extract->SetInputConnection(reader->GetOutputPort());
		extract->SetComponents(i);
		extract->Update();

		double range[2];
		extract->GetOutput()->GetScalarRange(range);

		vtkSmartPointer<vtkImageAccumulate> histogram =
			vtkSmartPointer<vtkImageAccumulate>::New();
		histogram->SetInputConnection(extract->GetOutputPort());
		histogram->SetComponentExtent(
			0,
			static_cast<int>(range[1]) - static_cast<int>(range[0]) - 1, 0, 0, 0, 0);
		histogram->SetComponentOrigin(range[0], 0, 0);
		histogram->SetComponentSpacing(1, 0, 0);
		//histogram->SetIgnoreZero(ignoreZero);
		histogram->Update();

		if (range[1] > xmax)
		{
			xmax = range[1];
		}
		if (histogram->GetOutput()->GetScalarRange()[1] > ymax)
		{
			ymax = histogram->GetOutput()->GetScalarRange()[1];
		}

#if VTK_MAJOR_VERSION <= 5
		plot->AddInput(histogram->GetOutput());
#else
		plot->AddDataSetInputConnection(histogram->GetOutputPort());
#endif
		if (numComponents > 1)
		{
			plot->SetPlotColor(i, colors[i]);
			plot->SetPlotLabel(i, labels[i]);
			plot->LegendOn();
		}
	}

	plot->SetXRange(0, xmax);
	plot->SetYRange(0, ymax);

	// Visualize the histogram(s)
	renderer2->AddActor(plot);   

	ui.qvtkWidget_histogram->GetRenderWindow()->AddRenderer(renderer2);
	ui.qvtkWidget_histogram->show();
}

void MedicalImageProcessing::ReadHistogram() {
	int dims[3];
	reader->GetOutput()->GetDimensions(dims);

	int nbOfComp;
	nbOfComp = reader->GetOutput()->GetNumberOfScalarComponents();

	for (int k = 0; k < dims[2]; k++)
	{
		for (int j = 0; j < dims[1]; j++)
		{
			for (int i = 0; i < dims[0]; i++)
			{
				if (i < dims[0] && j < dims[1])
				{
					unsigned char * pixel =
						(unsigned char *)(reader->GetOutput()->GetScalarPointer(i, j, k));
					//*pixel = 15;
					histogram_vector.push_back(*pixel);

				}
			}
		}
	}

}

void MedicalImageProcessing::Threshold(int value) {
	renderer3->Clear();
	renderer3->RemoveAllViewProps();

	vtkSmartPointer<vtkImageThreshold> threshould =
		vtkSmartPointer<vtkImageThreshold>::New();
	threshould->SetInputConnection(reader->GetOutputPort());
	threshould->ThresholdByUpper(value);
	threshould->SetInValue(255);
	threshould->SetOutValue(0);
	threshould->Update(); //算法执行后必须添加更新消息！！！

	vtkSmartPointer<vtkImageActor> binaryActor =
		vtkSmartPointer<vtkImageActor>::New();
	binaryActor->SetInputData(threshould->GetOutput());

	vtkSmartPointer<vtkRenderer> renderer3 =
		vtkSmartPointer<vtkRenderer>::New();
	renderer3->AddActor(binaryActor);
	renderer3->ResetCamera();
	renderer3->GetActiveCamera()->Zoom(1.4);
	
	// Set up an interactor that does not respond to mouse events
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkRenderWindow> rw =
		vtkSmartPointer<vtkRenderWindow>::New();
	rw->AddRenderer(renderer3);
	rw->SetInteractor(renderWindowInteractor);
	renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_threshold->SetRenderWindow(rw);
	ui.qvtkWidget_threshold->GetRenderWindow()->Render();
}

void MedicalImageProcessing::Otsu() {
	renderer_otsu->Clear();
	renderer_otsu->RemoveAllViewProps();

	int* histogram = new int[histogram_vector.size()];
	for (int ii = 0; ii < histogram_vector.size(); ii++)
		histogram[ii] = histogram_vector.at(ii);

	int X, Y, Amount = 0;
	int PixelBack = 0, PixelFore = 0, PixelIntegralBack = 0, PixelIntegralFore = 0, PixelIntegral = 0;
	double OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma;              // 类间方差;
	int MinValue, MaxValue;
	int Threshold_otsu = 0;

	for (MinValue = 0; MinValue < 256 && histogram[MinValue] == 0; MinValue++);
	for (MaxValue = 255; MaxValue > MinValue && histogram[MinValue] == 0; MaxValue--);
	if (MaxValue == MinValue) Threshold_otsu=MaxValue;          // 图像中只有一个颜色             
	if (MinValue + 1 == MaxValue) Threshold_otsu=MinValue;      // 图像中只有二个颜色

	for (Y = MinValue; Y <= MaxValue; Y++) Amount += histogram[Y];        //  像素总数

	PixelIntegral = 0;
	for (Y = MinValue; Y <= MaxValue; Y++) PixelIntegral += histogram[Y] * Y;
	SigmaB = -1;
	for (Y = MinValue; Y < MaxValue; Y++)
	{
		PixelBack = PixelBack + histogram[Y];
		PixelFore = Amount - PixelBack;
		OmegaBack = (double)PixelBack / Amount;
		OmegaFore = (double)PixelFore / Amount;
		PixelIntegralBack += histogram[Y] * Y;
		PixelIntegralFore = PixelIntegral - PixelIntegralBack;
		MicroBack = (double)PixelIntegralBack / PixelBack;
		MicroFore = (double)PixelIntegralFore / PixelFore;
		Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);
		if (Sigma > SigmaB)
		{
			SigmaB = Sigma;
			Threshold_otsu = Y;
		}
	}

	vtkSmartPointer<vtkImageThreshold> threshould =
		vtkSmartPointer<vtkImageThreshold>::New();
	threshould->SetInputConnection(reader->GetOutputPort());
	threshould->ThresholdByUpper(Threshold_otsu);
	threshould->SetInValue(255);
	threshould->SetOutValue(0);
	threshould->Update(); //算法执行后必须添加更新消息！！！

	vtkSmartPointer<vtkImageActor> binaryActor =
		vtkSmartPointer<vtkImageActor>::New();
	binaryActor->SetInputData(threshould->GetOutput());

	vtkSmartPointer<vtkRenderer> renderer_otsu =
		vtkSmartPointer<vtkRenderer>::New();
	renderer_otsu->AddActor(binaryActor);
	renderer_otsu->ResetCamera();
	renderer_otsu->GetActiveCamera()->Zoom(1.4);

	// Set up an interactor that does not respond to mouse events
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkRenderWindow> rw =
		vtkSmartPointer<vtkRenderWindow>::New();
	rw->AddRenderer(renderer_otsu);
	rw->SetInteractor(renderWindowInteractor);
	renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_otsu->SetRenderWindow(rw);
	ui.qvtkWidget_otsu->GetRenderWindow()->Render();
	QString s = QString::number(Threshold_otsu, 10);
	ui.label_otsu->setText(s);

}

void MedicalImageProcessing::Entropy(){
	renderer_entropy->Clear();
	renderer_entropy->RemoveAllViewProps();

	int* histogram = new int[histogram_vector.size()];
	for (int ii = 0; ii < histogram_vector.size(); ii++)
		histogram[ii] = histogram_vector.at(ii);

	int  X, Y, Amount = 0;
	double HistGramD[256];
	double SumIntegral, EntropyBack, EntropyFore, MaxEntropy;
	int MinValue = 255, MaxValue = 0;
	int Threshold_entropy = 0;

	for (MinValue = 0; MinValue < 256 && histogram[MinValue] == 0; MinValue++);
	for (MaxValue = 255; MaxValue > MinValue && histogram[MinValue] == 0; MaxValue--);
	if (MaxValue == MinValue) Threshold_entropy=MaxValue;          // 图像中只有一个颜色             
	if (MinValue + 1 == MaxValue) Threshold_entropy=MinValue;      // 图像中只有二个颜色

	for (Y = MinValue; Y <= MaxValue; Y++) Amount += histogram[Y];        //  像素总数

	for (Y = MinValue; Y <= MaxValue; Y++)   HistGramD[Y] = (double)histogram[Y] / Amount + 1e-17;

	MaxEntropy = double(MinValue);
	for (Y = MinValue + 1; Y < MaxValue; Y++)
	{
		SumIntegral = 0;
		for (X = MinValue; X <= Y; X++) SumIntegral += HistGramD[X];
		EntropyBack = 0;
		for (X = MinValue; X <= Y; X++) EntropyBack += (-HistGramD[X] / SumIntegral * log(HistGramD[X] / SumIntegral));
		EntropyFore = 0;
		for (X = Y + 1; X <= MaxValue; X++) EntropyFore += (-HistGramD[X] / (1 - SumIntegral) * log(HistGramD[X] / (1 - SumIntegral)));
		if (MaxEntropy < EntropyBack + EntropyFore)
		{
			Threshold_entropy = Y;
			MaxEntropy = EntropyBack + EntropyFore;
		}
	}

	vtkSmartPointer<vtkImageThreshold> threshould =
		vtkSmartPointer<vtkImageThreshold>::New();
	threshould->SetInputConnection(reader->GetOutputPort());
	threshould->ThresholdByUpper(Threshold_entropy);
	threshould->SetInValue(255);
	threshould->SetOutValue(0);
	threshould->Update(); //算法执行后必须添加更新消息！！！

	vtkSmartPointer<vtkImageActor> binaryActor =
		vtkSmartPointer<vtkImageActor>::New();
	binaryActor->SetInputData(threshould->GetOutput());

	vtkSmartPointer<vtkRenderer> renderer_entropy =
		vtkSmartPointer<vtkRenderer>::New();
	renderer_entropy->AddActor(binaryActor);
	renderer_entropy->ResetCamera();
	renderer_entropy->GetActiveCamera()->Zoom(1.4);

	// Set up an interactor that does not respond to mouse events
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkRenderWindow> rw =
		vtkSmartPointer<vtkRenderWindow>::New();
	rw->AddRenderer(renderer_entropy);
	rw->SetInteractor(renderWindowInteractor);
	renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_entropy->SetRenderWindow(rw);
	ui.qvtkWidget_entropy->GetRenderWindow()->Render();
	QString s = QString::number(Threshold_entropy, 10);
	ui.label_entropy->setText(s);
}

double MedicalImageProcessing::generateGaussianNoise(double mu, double sigma) {
	//定义小值
	const double epsilon = numeric_limits<double>::min();
	static double z0, z1;
	static bool flag = false;
	flag = !flag;
	//flag为假构造高斯随机变量X
	if (!flag)
		return z1 * sigma + mu;
	double u1, u2;
	//构造随机变量
	do
	{
		u1 = rand() * (1.0 / RAND_MAX);
		u2 = rand() * (1.0 / RAND_MAX);
	} while (u1 <= epsilon);
	//flag为真构造高斯随机变量
	z0 = sqrt(-2.0*log(u1))*cos(2 * CV_PI*u2);
	z1 = sqrt(-2.0*log(u1))*sin(2 * CV_PI*u2);
	return z0 * sigma + mu;

}

void MedicalImageProcessing::AddGaussianNoise() {
	renderer_conv->Clear();
	renderer_conv->RemoveAllViewProps();
	
	Mat dstImage = Vtkimage2mat();

	int channels = dstImage.channels();
	int rowsNumber = dstImage.rows;
	int colsNumber = dstImage.cols*channels;
	//判断图像的连续性
	if (dstImage.isContinuous())
	{
		colsNumber *= rowsNumber;
		rowsNumber = 1;
	}
	for (int i = 0; i < rowsNumber; i++)
	{
		for (int j = 0; j < colsNumber; j++)
		{
			//添加高斯噪声
			int val = dstImage.ptr<uchar>(i)[j] +
				generateGaussianNoise(2, 0.8) * 32;
			if (val < 0)
				val = 0;
			if (val > 255)
				val = 255;
			dstImage.ptr<uchar>(i)[j] = (uchar)val;
		}
	}


	NoiseImage = mat2Vtkimage(dstImage, 1);
	// Create actor
	vtkSmartPointer<vtkImageActor> imageActor =
		vtkSmartPointer<vtkImageActor>::New();
	imageActor->GetMapper()->SetInputData(NoiseImage);

	vtkSmartPointer<vtkRenderer> renderer_conv =
		vtkSmartPointer<vtkRenderer>::New();
	renderer_conv->AddActor(imageActor);
	renderer_conv->ResetCamera();
	renderer_conv->GetActiveCamera()->Zoom(1.4);

	// Set up an interactor that does not respond to mouse events
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkRenderWindow> rw =
		vtkSmartPointer<vtkRenderWindow>::New();
	rw->AddRenderer(renderer_conv);
	rw->SetInteractor(renderWindowInteractor);
	renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_conv->SetRenderWindow(rw);
	ui.qvtkWidget_conv->GetRenderWindow()->Render();


}

void MedicalImageProcessing::RestoreGaussian() {
	GaussianFilter(3);
}

void MedicalImageProcessing::GaussianFilter(int value) {

	renderer_conv->Clear();
	renderer_conv->RemoveAllViewProps();
	
	vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter =
		vtkSmartPointer<vtkImageGaussianSmooth>::New();
	gaussianSmoothFilter->SetInputData(NoiseImage);
	gaussianSmoothFilter->SetDimensionality(2);
	gaussianSmoothFilter->SetRadiusFactor(5);
	gaussianSmoothFilter->SetStandardDeviation(value);
	gaussianSmoothFilter->Update();

	vtkSmartPointer<vtkImageActor> smoothedActor =
		vtkSmartPointer<vtkImageActor>::New();
	smoothedActor->SetInputData(gaussianSmoothFilter->GetOutput());
	smoothedActor->Update();

	renderer_conv->AddActor(smoothedActor);
	renderer_conv->ResetCamera();
	renderer_conv->GetActiveCamera()->Zoom(1.4);	

	//// Set up an interactor that does not respond to mouse events
	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
	//	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//renderer_conv->GetRenderWindow()->SetInteractor(renderWindowInteractor);
	//renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_conv->GetRenderWindow()->AddRenderer(renderer_conv);
	ui.qvtkWidget_conv->show();
	ui.qvtkWidget_conv->update();

}

void MedicalImageProcessing::Addnoise() {
	renderer_conv->Clear();
	renderer_conv->RemoveAllViewProps();

	int n=3000;
	Mat dstImage = Vtkimage2mat();

	for (int k = 0; k < n; k++)
	{
		//随机取值行列
		int i = rand() % dstImage.rows;
		int j = rand() % dstImage.cols;
		//图像通道判定
		if (dstImage.channels() == 1)
		{
			dstImage.at<uchar>(i, j) = 255;		//盐噪声
		}
		else
		{
			dstImage.at<Vec3b>(i, j)[0] = 255;
			dstImage.at<Vec3b>(i, j)[1] = 255;
			//dstImage.at<Vec3b>(i, j)[2] = 255;
		}
	}
	for (int k = 0; k < n; k++)
	{
		//随机取值行列
		int i = rand() % dstImage.rows;
		int j = rand() % dstImage.cols;
		//图像通道判定
		if (dstImage.channels() == 1)
		{
			dstImage.at<uchar>(i, j) = 0;		//椒噪声
		}
		else
		{
			dstImage.at<Vec3b>(i, j)[0] = 0;
			dstImage.at<Vec3b>(i, j)[1] = 0;
			//dstImage.at<Vec3b>(i, j)[2] = 0;
		}
	}

	NoiseImage = mat2Vtkimage(dstImage, 1);

	// Create actor
	vtkSmartPointer<vtkImageActor> imageActor =
		vtkSmartPointer<vtkImageActor>::New();
	imageActor->GetMapper()->SetInputData(NoiseImage);

	vtkSmartPointer<vtkRenderer> renderer_conv =
		vtkSmartPointer<vtkRenderer>::New();
	renderer_conv->AddActor(imageActor);
	renderer_conv->ResetCamera();
	renderer_conv->GetActiveCamera()->Zoom(1.4);

	// Set up an interactor that does not respond to mouse events
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkRenderWindow> rw =
		vtkSmartPointer<vtkRenderWindow>::New();
	rw->AddRenderer(renderer_conv);
	rw->SetInteractor(renderWindowInteractor);
	renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_conv->SetRenderWindow(rw);
	ui.qvtkWidget_conv->GetRenderWindow()->Render();


}

void MedicalImageProcessing::MedianFilter() {
	renderer_conv->Clear();
	renderer_conv->RemoveAllViewProps();

	vtkSmartPointer<vtkImageHybridMedian2D> hybridMedian =
		vtkSmartPointer<vtkImageHybridMedian2D>::New();
	hybridMedian->SetInputData(NoiseImage);
	hybridMedian->Update();

	vtkSmartPointer<vtkImageActor> hybridMedianActor =
		vtkSmartPointer<vtkImageActor>::New();
	hybridMedianActor->SetInputData(hybridMedian->GetOutput());

	renderer_conv->AddActor(hybridMedianActor);
	renderer_conv->ResetCamera();
	renderer_conv->GetActiveCamera()->Zoom(1.4);

	//// Set up an interactor that does not respond to mouse events
	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
	//	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//renderer_conv->GetRenderWindow()->SetInteractor(renderWindowInteractor);
	//renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_conv->GetRenderWindow()->AddRenderer(renderer_conv);
	ui.qvtkWidget_conv->show();
	ui.qvtkWidget_conv->update();

}

void MedicalImageProcessing::Sobel() {
	//先转成灰度图
	renderer_conv->Clear();
	renderer_conv->RemoveAllViewProps();

	vtkSmartPointer<vtkImageLuminance> luminanceFilter =
		vtkSmartPointer<vtkImageLuminance>::New();
	luminanceFilter->SetInputConnection(reader->GetOutputPort());
	luminanceFilter->Update();

	vtkSmartPointer<vtkImageSobel2D> sobelFilter =
		vtkSmartPointer<vtkImageSobel2D>::New();
	sobelFilter->SetInputConnection(luminanceFilter->GetOutputPort());

	vtkSmartPointer<vtkImageExtractComponents> extractXFilter =
		vtkSmartPointer<vtkImageExtractComponents>::New();
	if (ui.comb_sob->currentText() == "X-Axis")
	{
		extractXFilter->SetComponents(1);
	}
	else {
		extractXFilter->SetComponents(0);
	}
		
	
	extractXFilter->SetInputConnection(sobelFilter->GetOutputPort());
	extractXFilter->Update();

	double xRange[2];
	extractXFilter->GetOutput()->GetScalarRange(xRange);

	vtkSmartPointer<vtkImageMathematics> xImageAbs =
		vtkSmartPointer<vtkImageMathematics>::New();
	xImageAbs->SetOperationToAbsoluteValue();
	xImageAbs->SetInputConnection(extractXFilter->GetOutputPort());
	xImageAbs->Update();

	vtkSmartPointer<vtkImageShiftScale> xShiftScale =
		vtkSmartPointer<vtkImageShiftScale>::New();
	xShiftScale->SetOutputScalarTypeToUnsignedChar();
	xShiftScale->SetScale(255 / xRange[1]);
	xShiftScale->SetInputConnection(xImageAbs->GetOutputPort());
	xShiftScale->Update();

	vtkSmartPointer<vtkImageActor> xActor =
		vtkSmartPointer<vtkImageActor>::New();
	xActor->SetInputData(xShiftScale->GetOutput());

	renderer_conv->AddActor(xActor);
	renderer_conv->ResetCamera();
	renderer_conv->GetActiveCamera()->Zoom(1.4);

	//// Set up an interactor that does not respond to mouse events
	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
	//	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//renderer_conv->GetRenderWindow()->SetInteractor(renderWindowInteractor);
	//renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_conv->GetRenderWindow()->AddRenderer(renderer_conv);
	ui.qvtkWidget_conv->show();
	ui.qvtkWidget_conv->update();
}

void MedicalImageProcessing::Prewitt(){
	renderer_conv->Clear();
	renderer_conv->RemoveAllViewProps();

	vtkSmartPointer<vtkImageCast> Prewitt = vtkSmartPointer<vtkImageCast>::New();
	Prewitt->SetInputConnection(reader->GetOutputPort());
	Prewitt->SetOutputScalarTypeToFloat();
	Prewitt->Update();

	vtkSmartPointer<vtkImageConvolve>XPrewittConv = vtkSmartPointer<vtkImageConvolve>::New();
	XPrewittConv->SetInputConnection(Prewitt->GetOutputPort());
	if (ui.comb_pre->currentText() == "X-Axis")
	{
		double kernelx[9] = { 1, 1, 1, 0, 0, 0, -1, -1, -1 };
		XPrewittConv->SetKernel3x3(kernelx);
	}
	else {
		double kernelx[9] = { 1, 0, -1, 1, 0, -1, 1, 0, -1 };
		XPrewittConv->SetKernel3x3(kernelx);
	}
	
	
	
	XPrewittConv->Update();

	vtkSmartPointer<vtkImageCast>Axcast = vtkSmartPointer<vtkImageCast>::New();
	Axcast->SetInputConnection(XPrewittConv->GetOutputPort());
	Axcast->SetOutputScalarTypeToChar();
	Axcast->Update();

	vtkSmartPointer<vtkImageViewer2> imageViewer =
		vtkSmartPointer<vtkImageViewer2>::New();

	imageViewer->SetInputConnection(XPrewittConv->GetOutputPort());
	imageViewer->GetRenderer()->ResetCamera();
	imageViewer->GetRenderer()->GetActiveCamera()->Zoom(1.4);
	
	// Set up an interactor that does not respond to mouse events
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	imageViewer->GetRenderWindow()->SetInteractor(renderWindowInteractor);
	renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_conv->SetRenderWindow(imageViewer->GetRenderWindow());
	ui.qvtkWidget_conv->GetRenderWindow()->Render();

}

Mat MedicalImageProcessing::Vtkimage2mat() {
	reader->Update();

	vtkImageData* image = reader->GetOutput();
	// Check number of components. 
	const int numComponents = image->GetNumberOfScalarComponents(); // 3
	// Construct the OpenCv Mat 
	int dims[3];
	image->GetDimensions(dims);
	cv::Mat openCVImage(dims[0], dims[1], CV_8UC3, image->GetScalarPointer()); // Unsigned int, 3 channels 

	cvtColor(openCVImage, openCVImage, CV_BGRA2GRAY);

	// Flip because of different origins between vtk and OpenCV 
	cv::flip(openCVImage, openCVImage, 0);
	
	return openCVImage;


	//vtkImageData* a = reader->GetOutput();

	//int dims[3];
	//a->GetDimensions(dims);// vtkImage的类型是vktImageData  
	//Mat srcMat(dims[1], dims[0], CV_8UC1, a->GetScalarPointer());

	//double spacing[3];
	//a->GetSpacing(spacing);
	//cv::Size dsize;
	//dsize.width = dims[0] * spacing[0];
	//dsize.height = dims[1] * spacing[1];
	//Mat distMat = srcMat;
	//cv::resize(srcMat, distMat, dsize);
	/////////////////////////////////////////////////////////////////////
	//	//图像旋转180
	//cv::Point2f center(distMat.cols / 2, distMat.rows / 2);
	//cv::Mat rot = cv::getRotationMatrix2D(center, -180, 1);
	//cv::Rect bbox = cv::RotatedRect(center, distMat.size(), -180).boundingRect();
	//rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	//rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;
	//cv::Mat dst;
	//cv::warpAffine(distMat, dst, rot, bbox.size());
	////cv::flip(dst, dst, 0);
	////////////////////////////////////////////////////////////////////////////////

	//return dst;

}

vtkSmartPointer<vtkImageData> MedicalImageProcessing::mat2Vtkimage(const cv::Mat &sourceCVImage, bool flipOverXAxis) {
	vtkSmartPointer<vtkImageData> outputVtkImage = vtkSmartPointer<vtkImageData>::New();

	int numOfChannels = sourceCVImage.channels();

	// dimension set to 1 for z since it's 2D
	outputVtkImage->SetDimensions(sourceCVImage.cols, sourceCVImage.rows, 1);

	// NOTE: if your image isn't uchar for some reason you'll need to change this type
	outputVtkImage->AllocateScalars(VTK_UNSIGNED_CHAR, numOfChannels);

	// the flipped image data gets put into tempCVImage
	cv::Mat tempCVImage;
	cv::flip(sourceCVImage, tempCVImage, 0);


	// the number of byes in the cv::Mat, assuming the data type is uchar
	size_t byte_count = sourceCVImage.cols * sourceCVImage.rows * numOfChannels * sizeof(unsigned char);

	// copy the internal cv::Mat data into the vtkImageData pointer
	memcpy(outputVtkImage->GetScalarPointer(), tempCVImage.data, byte_count);

	outputVtkImage->Modified();
	return outputVtkImage;
}

void MedicalImageProcessing::Roberts() {
	renderer_conv->Clear();
	renderer_conv->RemoveAllViewProps();

	Mat GrayImage = Vtkimage2mat();
	Mat dstImage = GrayImage.clone();
	int nRows = dstImage.rows;
	int nCols = dstImage.cols;
	for (int i = 0; i < nRows - 1; i++)
	{
		for (int j = 0; j < nCols - 1; j++)
		{
			int t1 = (GrayImage.at<uchar>(i, j) -
				GrayImage.at<uchar>(i + 1, j + 1)) *
				(GrayImage.at<uchar>(i, j) -
					GrayImage.at<uchar>(i + 1, j + 1));
			int t2 = (GrayImage.at<uchar>(i + 1, j) -
				GrayImage.at<uchar>(i, j + 1)) *
				(GrayImage.at<uchar>(i + 1, j) -
					GrayImage.at<uchar>(i, j + 1));
			dstImage.at<uchar>(i, j) = (uchar)sqrt(t1 + t2);
		}
	}

	vtkSmartPointer<vtkImageData> outputVtkImage = vtkSmartPointer<vtkImageData>::New();
    outputVtkImage= mat2Vtkimage(dstImage, 1);

	// Create actor
	vtkSmartPointer<vtkImageActor> imageActor =
		vtkSmartPointer<vtkImageActor>::New();
	imageActor->GetMapper()->SetInputData(outputVtkImage);

	vtkSmartPointer<vtkRenderer> renderer_conv =
		vtkSmartPointer<vtkRenderer>::New();
	renderer_conv->AddActor(imageActor);
	renderer_conv->ResetCamera();
	renderer_conv->GetActiveCamera()->Zoom(1.4);

	// Set up an interactor that does not respond to mouse events
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkRenderWindow> rw =
		vtkSmartPointer<vtkRenderWindow>::New();
	rw->AddRenderer(renderer_conv);
	rw->SetInteractor(renderWindowInteractor);
	renderWindowInteractor->SetInteractorStyle(0);

	ui.qvtkWidget_conv->SetRenderWindow(rw);
	ui.qvtkWidget_conv->GetRenderWindow()->Render();
}

void MedicalImageProcessing::QtDisplayMat(Mat A, QLabel* DisWindow)
{
	/*cvtColor(A, rgb, CV_BGR2RGB);*/
	QImage image1 = QImage((uchar*)(A.data),
		A.cols, A.rows,
		A.cols*A.channels(),
		QImage::Format_Indexed8);
	//注意图像格式，对于8位灰度图是Format_Indexed8，会根据灰度值在颜色表中查找出
	//相应的灰度颜色以显示。
	//对于彩色图，是先转换为grb顺序，如果是8位3通道的，则选择Format_RGB888
	
	QPixmap pixmap = QPixmap::fromImage(image1);
	QPixmap fitpixmap = pixmap.scaled(DisWindow->size(), Qt::KeepAspectRatio);;
	DisWindow->setAlignment(Qt::AlignCenter);
	DisWindow->setPixmap(fitpixmap);
	//DisWindow->resize(DisWindow->pixmap()->size());
	//DisWindow->setScaledContents(true);
}

void MedicalImageProcessing::SelectElement()
{
	RNG rng;
	int SEGSize;
	if (ui.SESize->text() != "")
	{
		SEGSize = ui.SESize->text().toInt();
	}
	else
	{
		SEGSize = 3;
		ui.SESize->setText("3");
	}
	//Structure element的值随机产生
	Element = Mat(SEGSize, SEGSize, CV_8UC1);
	rng.fill(Element, RNG::UNIFORM, 1, 100);

	//选择Structure element的形状
	Mat Modulelist;
	if (ui.SEOption->currentText() == "Cross")
	{
		Modulelist = getStructuringElement(MORPH_CROSS, Size(SEGSize, SEGSize));

	}
	else if (ui.SEOption->currentText() == "Circle")
	{
		Modulelist = getStructuringElement(MORPH_ELLIPSE, Size(SEGSize, SEGSize));
	}
	else
	{
		Modulelist = getStructuringElement(MORPH_RECT, Size(SEGSize, SEGSize));
	}
	Element = Element.mul(Modulelist);
}

void MedicalImageProcessing::Dilation()
{
	Mat Dil;
	dilate(Vtkimage2mat(), Dil, Element);
	this->QtDisplayMat(Dil, ui.image_1);
	ui.label_1->setText("DilationResult");
}

void MedicalImageProcessing::Erosion()
{
	Mat Ero;
	erode(Vtkimage2mat(), Ero, Element);
	this->QtDisplayMat(Ero, ui.image_2);

	ui.label_2->setText("ErosionResult");
}

Mat MedicalImageProcessing::Opening()
{
	Mat temp, Ope;
	erode(Vtkimage2mat(), temp, Element);

	dilate(temp, Ope, Element);
	this->QtDisplayMat(Ope, ui.image_3);
	ui.label_3->setText("OpeningResult");
	return Ope;
}

Mat MedicalImageProcessing::Closing()
{
	Mat temp, Clo;
	dilate(Vtkimage2mat(), temp, Element);
	erode(temp, Clo, Element);
	this->QtDisplayMat(Clo, ui.image_4);
	ui.label_4->setText("ClosingResult");
	return Clo;
}

void MedicalImageProcessing::DistanceTransform()
{
	////备用，采用opencv的方法进行DistanceTransform，但是为了后续SkeletonReconstruct的distance需求，采用逐渐遍历的方法/////
	//BinaryImage = Vtkimage2mat();
 //   //定义距离变换矩阵中的最大值
 //   float maxValue = 0;
 //   //BinaryImage = ~BinaryImage;
 //   threshold(BinaryImage, BinaryImage, 20, 200, CV_THRESH_BINARY);
 //   // 定义保存距离变换结果的Mat矩阵
 //   Mat imageThin(BinaryImage.size(), CV_32FC1);
 //   
 //   // 距离变换
 //   distanceTransform(BinaryImage, imageThin, CV_DIST_L2, 3);
 //   //为了显示清晰，做了0~255归一化
 //   normalize(imageThin, imageThin, 0, 255, CV_MINMAX);
 //   Mat distShow;
 //   
 //   // 定义最终显示的图像
 //   distShow = Mat::zeros(BinaryImage.size(), CV_8UC1);
 //   
 //   for (int i = 0; i < imageThin.rows; i++)
 //   {
 //   	for (int j = 0; j < imageThin.cols; j++)
 //   	{
 //   		distShow.at<uchar>(i, j) = imageThin.at<float>(i, j);
 //   	}
 //   }
 //   this->QtDisplayMat(distShow, ui.image_1);
	//ui.label_1->setText("DistanceTransformResult");

	/*逐次erosion，获取distanceTransform*/
	Mat img;
	img = Vtkimage2mat();
	Nr = img.rows;
	Nc = img.cols;
	Nc = Nc * img.channels();
	BinaryImage = img.clone();
	Distance = Mat::zeros(Nr, Nc, CV_8U);
	Distance2 = Mat::zeros(Nr, Nc, CV_8U);
	Medum = Mat::zeros(Nr, Nc, CV_8U);
	Out = Mat::zeros(Nr, Nc, CV_8U);
	Count = 0;
	max = 10;
	timer->start();
	this->QtDisplayMat(BinaryImage, ui.image_1);
	ui.label_1->setText("Loading...");
}

void MedicalImageProcessing::DistanceImageUPdate()
{
	if (max == 0)
	{
		timer->stop();
	}
	else
	{
		/*BinaryImage.copyTo(Out);*/
		erode(BinaryImage, Out, Element);
		cv::minMaxIdx(Out, 0, &max);
		Medum = BinaryImage - Out;
		Count++;
		for (int i = 0; i < Nr; i++)
		{
			for (int j = 0; j < Nc; j++)
			{
				if (Medum.at<uchar>(i, j) != 0)//灰度图像操作
				{
					Distance.at<uchar>(i, j) = Count;//
					Distance2.at<uchar>(i, j) = Count * 5;//增加亮度，应该去掉*5才能做骨架重建
				}
				/*	if (Medum.at<cv::Vec3b>(i, j)[0] != 0)//彩色图像操作
					{
						Distance.at<cv::Vec3b>(i, j)[0] = Count;
					}*/
			}
		}

		Out.copyTo(BinaryImage);
		this->QtDisplayMat(Out, ui.image_1);
		this->QtDisplayMat(Distance2, ui.image_2);
		ui.label_2->setText("Distance Transform");
	}
}

void MedicalImageProcessing::Skeleton()
{

	Mat img, temp, eroded;
	img = Vtkimage2mat();

	Nr = img.rows;
	Nc = img.cols;
	Nc = Nc * img.channels();
	SkeletonMat = Mat::zeros(Nr, Nc, CV_8UC1);
	bool done;
	do
	{
		cv::erode(img, eroded, Element);
		cv::dilate(eroded, temp, Element); // temp = open(img)
		cv::subtract(img, temp, temp);
		cv::bitwise_or(SkeletonMat, temp, SkeletonMat);
		eroded.copyTo(img);

		done = (cv::countNonZero(img) == 0);
	} while (!done);
	/*skeleton is pixels cannot be recovered after opening */
	this->QtDisplayMat(SkeletonMat, ui.image_3);
	ui.label_3->setText("SkeletonResult");
	this->QtDisplayMat(Vtkimage2mat(), ui.image_1);
	ui.label_1->setText("OriginalImage");
}

void MedicalImageProcessing::SkeletonReconstruct()
{

	//DistanceImageUPdate();
	Mat SkeletonBit;
	cv::Mat Sn, Temp1, Temp;
	Temp = Mat::zeros(Nr, Nc, CV_8UC1);
	double Max1, MaxDistance;

	cv::threshold(SkeletonMat, SkeletonBit, 0, 1, 0);
	SkeletonBit = SkeletonBit.mul(Distance);
	//最终SkeletonBit为真Skeleton
	minMaxIdx(SkeletonBit, 0, &MaxDistance);
	Max1 = MaxDistance;

	bool done;
	do
	{
		if (Max1 == MaxDistance)
		{
			for (int i = 0; i < Nr; i++)
			{
				for (int j = 0; j < Nc; j++)
				{
					if (SkeletonBit.at<uchar>(i, j) == Max1)
					{
						Temp.at<uchar>(i, j) = Max1;
					}
				}
			}
			dilate(Temp, Temp1, Element);
			Sn = SkeletonBit - Temp;
		}
		else
		{
			dilate(Temp1, Temp1, Element);
			Sn = Sn - Temp;
		}

		minMaxIdx(Sn, 0, &Max1);
		Temp = Mat::zeros(Nr, Nc, CV_8U);
		for (int i = 0; i < Nr; i++)
		{
			for (int j = 0; j < Nc; j++)
			{
				if (SkeletonBit.at<uchar>(i, j) == Max1)
				{
					Temp.at<uchar>(i, j) = Max1;
				}
			}
		}
		cv::bitwise_or(Temp, Temp1, Temp1);
		done = (Max1 == 1);
		//终止条件
	} while (!done);

	//将结果二值化
	cv::threshold(Temp1, Temp1, 0, 255, 0);
	this->QtDisplayMat(Temp1, ui.image_4);
	ui.label_4->setText("SkeletonReconstruct");
}

void MedicalImageProcessing::MEdgeDetect()
{
	Mat MEdgeOut, tempp, tempp1;
	dilate(Vtkimage2mat(), tempp, Element);
	erode(Vtkimage2mat(), tempp1, Element);
	
	if (ui.comb_me->currentText() == "Standard")
	{
		MEdgeOut = tempp - tempp1;
	}
	else if (ui.comb_me->currentText() == "External")
	{
		MEdgeOut = tempp - Vtkimage2mat();
	}
	else 
	{
		MEdgeOut = Vtkimage2mat()-tempp1;
	}
	
	this->QtDisplayMat(MEdgeOut, ui.image_3);
	ui.label_3->setText("MEdge");
}

void MedicalImageProcessing::MGradient()
{
	Mat temp, temp1, MgraOut;
	dilate(Vtkimage2mat(), temp, Element);
	erode(Vtkimage2mat(), temp1, Element);

	if (ui.comb_mg->currentText() == "Standard")
	{
		MgraOut = 0.5*(temp - temp1);
	}
	else if (ui.comb_mg->currentText() == "External")
	{
		MgraOut = 0.5*(temp - Vtkimage2mat());
	}
	else
	{
		MgraOut = 0.5*(Vtkimage2mat() - temp1);
	}

	this->QtDisplayMat(MgraOut, ui.image_4);
	ui.label_4->setText("MGradient");
}

void MedicalImageProcessing::OBR()
{
	Mat mask = Opening();
	Srcimage = Vtkimage2mat();
	Mat GReConStruct, Temp = Mat::zeros(Srcimage.rows, Srcimage.cols, CV_8UC1);
	bool done = false;
	int counter = 0;
	while (!done)
	{
		if (counter == 0)
		{
			dilate(mask, Temp, Element);			
			counter++;
		}
		else
		{
			dilate(Temp, Temp, Element);
			counter++;
		}

		GReConStruct = Temp.clone();


		for (int i = 0; i < Srcimage.rows; i++)
		{
			for (int j = 0; j < Srcimage.cols; j++)
			{
				if (Temp.at<uchar>(i, j) > Srcimage.at<uchar>(i, j))
				{
					done = true;
				}
			}
		}
		
		
	}
	QtDisplayMat(GReConStruct, ui.image_1);
	ui.label_1->setText("OBR");
}

void MedicalImageProcessing::CBR()
{
	Mat mask = Closing();
	Srcimage = Vtkimage2mat();
	Mat GReConStruct, Temp = Mat::zeros(Srcimage.rows, Srcimage.cols, CV_8UC1);
	bool done = false;
	int counter = 0;
	while (!done)
	{
		if (counter == 0)
		{
			dilate(mask, Temp, Element);
			counter++;
		}
		else
		{
			dilate(Temp, Temp, Element);
			counter++;
		}

		GReConStruct = Temp.clone();

		for (int i = 0; i < Srcimage.rows; i++)
		{
			for (int j = 0; j < Srcimage.cols; j++)
			{
				if (Temp.at<uchar>(i, j) > Srcimage.at<uchar>(i, j))
				{
					done = true;
				}
			}
		}
	}
	QtDisplayMat(GReConStruct, ui.image_2);
	ui.label_2->setText("CBR");
}

void MedicalImageProcessing::ROISelect()
{	
	BinarySource= Vtkimage2mat();
	this->QtDisplayMat(BinarySource, ui.image_1);
	ui.label_1->setText("SourceImage");
	ui.image_1->setMouseTracking(false);
	ui.image_1->installEventFilter(this);
}

bool MedicalImageProcessing::eventFilter(QObject *obj, QEvent *event)
{
	/*int flag = 0;*/
	if (obj == ui.image_1 && (event->type() == QMouseEvent::MouseButtonPress))
	{
		if (!BinarySource.empty())
		{
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
			setCursor(Qt::CrossCursor);
			QPoint qPos = mouseEvent->globalPos();
			QPoint qPos1 = ui.image_1->mapFromGlobal(qPos);
			if (flag == 0)
			{
				ROI[0] = qPos1.x();
				ROI[1] = qPos1.y();
				PointCount++;
				flag = 1;
			}
			else
			{
				ROI[2] = qPos1.x();
				ROI[3] = qPos1.y();
				flag = 0;
				PointCount++;
			}
			 //cout<<qPos1.x()<<endl;
			 //cout<<qPos1.y()<<endl;
		}
	}
	else if (obj == ui.image_1 && (event->type() == QEvent::Leave))
	{
		setCursor(Qt::ArrowCursor);
	}


	if (PointCount == 2)
	{
		PointCount = 0;
		Marker = Mat::zeros(BinarySource.rows, BinarySource.cols, CV_8UC1);

		if (ROI[0] <= ROI[2])
		{
			for (int i = ROI[0]; i < ROI[2]; i++)
			{
				if (ROI[1] <= ROI[3])
				{
					for (int j = ROI[1]; j < ROI[3]; j++)
					{
						Marker.at<uchar>(j, i) = BinarySource.at<uchar>(j, i);
					}
				}
				else
				{
					for (int j = ROI[1]; j < ROI[3]; j--)
					{
						Marker.at<uchar>(j, i) = BinarySource.at<uchar>(j, i);
					}
				}
			}
		}
		else
			for (int i = ROI[0]; i < ROI[2]; i--)
			{
				if (ROI[1] <= ROI[3])
				{
					for (int j = ROI[1]; j < ROI[3]; j++)
					{
						Marker.at<uchar>(j, i) = BinarySource.at<uchar>(j, i);
					}
				}
				else
				{
					for (int j = ROI[1]; j < ROI[3]; j--)
					{
						Marker.at<uchar>(j, i) = BinarySource.at<uchar>(j, i);
					}
				}
			}
		QtDisplayMat(Marker, ui.image_2);
		ui.label_2->setText("Marker");
	}
	return QWidget::eventFilter(obj, event);
}

void MedicalImageProcessing::ConditionDilation()
{
	Mat GReConStruct;
	Mat Temp = Mat::zeros(BinarySource.rows, BinarySource.cols, CV_8UC1);
	Mat  Temp1 = Mat::zeros(BinarySource.rows, BinarySource.cols, CV_8UC1);
	bool done = false;
	int counter = 0;
	while (!done)
	{
		if (counter == 0)
		{
			dilate(Marker, Temp, Element);
			counter++;
			bitwise_and(Temp, BinarySource, Temp1);
			/*Temp.copyTo(Temp1);*/
		}
		else
		{
			Temp1.copyTo(Temp);
			dilate(Temp1, Temp1, Element);
			bitwise_and(Temp1, BinarySource, Temp1);
			counter++;
		}

		absdiff(Temp1, Temp, GReConStruct);
		if (cv::countNonZero(GReConStruct) == 0)
		{
			done = true;
		}
	}

	QtDisplayMat(Temp1, ui.image_3);
	ui.label_3->setText("ConditionDilation");
}