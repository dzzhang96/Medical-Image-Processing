#include "skeleton.h"


skeleton::skeleton(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_reader = vtkSmartPointer<vtkImageReader2>::New();

	connect(ui.iterationSlider, SIGNAL(valueChanged(int)), this, SLOT(SkeletonAnime(int)));

}

skeleton::~skeleton()
{
}

void skeleton::SkeletonAnime(int value)
{



	vtkSmartPointer<vtkImageSkeleton2D> skeleton =
		vtkSmartPointer<vtkImageSkeleton2D>::New();
	skeleton->SetInputConnection(m_reader->GetOutputPort());
	skeleton->SetPrune(1);
	skeleton->SetNumberOfIterations(value);

	//vtkImageData* a = skeleton->GetOutput();

	vtkSmartPointer<vtkImageViewer2> imageViewer =
		vtkSmartPointer<vtkImageViewer2>::New();
	imageViewer->SetInputConnection(skeleton->GetOutputPort());
	imageViewer->GetRenderer()->ResetCamera();

	vtkSmartPointer<vtkCamera> cam = vtkSmartPointer<vtkCamera>::New();
	cam = imageViewer->GetRenderer()->GetActiveCamera();
	cam->Zoom(1.4);

	//// Set up an interactor that does not respond to mouse events
	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
	//	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//imageViewer->GetRenderWindow()->SetInteractor(renderWindowInteractor);
	//renderWindowInteractor->SetInteractorStyle(0);


	ui.qvtkWidget_skeleton->SetRenderWindow(imageViewer->GetRenderWindow());
	ui.qvtkWidget_skeleton->show();
	ui.qvtkWidget_skeleton->update();

}