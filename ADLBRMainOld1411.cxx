
//#include "../seg/nonLocalMean.cxx"
#include "AnisotropicLBR.cxx"
#include "geodesic.cxx"
#include "watershed.cxx"
#include "createMask.cxx"
#include "fastmarching.cxx"
#include "preprocessing.cxx"
#include "math.h"
#include "utility.h"
#include "itkImageMomentsCalculator.h"
#include "itkSignedDanielssonDistanceMapImageFilter.h"
//#include "itkInvertIntensityImageFilter.h.";
int main(int argc, char *argv[])
{
	//cout << argc << endl;
	if (argc != 16) {
		cout << "Input_image Output_image SeedX SeedY SeedZ" << std::endl;
		return 0;
	}
	itk::TimeProbe clock;
	clock.Start();
	char * name = argv[5];
	char * file = argv[7];
	std::string newName;
	float *seedPointPhy = 0;
	seedPointPhy = new float[3];
	seedPointPhy[0] = atof(argv[2])+ atof(argv[15]); // (pixels)
	seedPointPhy[1] = atof(argv[3])- atof(argv[15]); // (pixels)
	seedPointPhy[2] = atof(argv[4])- atof(argv[15]); // (pixels)

	IndexType axesImage;
	axesImage[0] = 75; // atoi(argv[16])*2;      // diameter (mm)
	axesImage[1] = 75;// atoi(argv[17])*2;      // (mm)
	axesImage[2] = 75; //atoi(argv[18])*2;      // (mm)

	IndexType axes;
	axes[0] = axesImage[0] / 2;      // (mm)
	axes[1] = axesImage[0] / 2;      // (mm)
	axes[2] = axesImage[0] / 2;      // (mm)

	IndexType axesSeed;
	axesSeed[0] = 5;      // diameter (mm)
	axesSeed[1] = 5;      // (mm)
	axesSeed[2] = 5;      // (mm)

	float* sigMuStd = 0;
	sigMuStd = new float[2];
	float* sigMuStdseed = 0;
	sigMuStdseed = new float[2];

	//to store dice and other evaluation measure
	float *labelMeasure = 0;
	labelMeasure = new float[3];

	//Perform algorithm
	InternalImageType::Pointer  diacomImage, gtImage, smoothimage;
	OutputImageType::Pointer segmentationImageGeo, segmentationImageFastMarch, gtImageout, segmentationImageWater;
	std::stringstream sss;
	std::string stn;



	myfile2.open(file, ios::out | ios::app);
	diacomImage = ReadImage(argv[1]);
	double sourceImageSpacing[] = { diacomImage->GetSpacing()[0],diacomImage->GetSpacing()[1],diacomImage->GetSpacing()[2] }; //get the spacing of the image
	double origin[] = { diacomImage->GetOrigin()[0],diacomImage->GetOrigin()[1], diacomImage->GetOrigin()[2] };
	/*myfile2 << "\n" << argv[5] << "\n" << endl;
	myfile2 << "physical co-ordinates " << atof(argv[2]) << "," << atof(argv[3]) << "," << atof(argv[4]) << "\n";
	myfile2 << "spacing " << sourceImageSpacing[0] << ", " << sourceImageSpacing[1] << ", " << sourceImageSpacing[2] << "\n";
	myfile2 << "Origin" << origin[0] << "," << origin[1] << "," << origin[2] << "\n";
	cout << "\n" << argv[5] << "\n" << endl;
	cout << "physical co-ordinates " << atof(argv[2]) << "," << atof(argv[3]) << "," << atof(argv[4]) << "\n";
	cout << "spacing " << sourceImageSpacing[0] << ", " << sourceImageSpacing[1] << ", " << sourceImageSpacing[2] << "\n";
	cout << "Origin" << origin[0] << "," << origin[1] << "," << origin[2] << "\n";*/
	IndexType seedPoint;
	seedPoint[0] = floor(((seedPointPhy[0] - origin[0]) / sourceImageSpacing[0]) + 0.5);
	seedPoint[1] = floor(((seedPointPhy[1] - origin[1]) / sourceImageSpacing[1]) + 0.5);
	seedPoint[2] = floor(((seedPointPhy[2] - origin[2]) / sourceImageSpacing[2]) + 0.5);
	


	/*myfile2 << "seedPoint  " << seedPoint[0] << "," << seedPoint[1] << "," << seedPoint[2] << "\n";
	cout << "seedPoint  " << seedPoint[0] << "," << seedPoint[1] << "," << seedPoint[2] << "\n";*/

	gtImage = ReadImage(argv[6]);
/*
	typedef itk::ImageMomentsCalculator< InternalImageType > ImageCalculatorType;
	ImageCalculatorType::Pointer Calculator = ImageCalculatorType::New();
	Calculator->SetImage(gtImage);
	Calculator->Compute();
	Calculator->GetTotalMass();
	ImageCalculatorType::VectorType Center = Calculator->GetCenterOfGravity();
	std::cout << Center[0] << " " << Center[1] << " " << Center[2] << std::endl;

	IndexType seedPoint;
	seedPoint[0] = floor(((Center[0] - origin[0]) / sourceImageSpacing[0]) + 0.5);
	seedPoint[1] = floor(((Center[1] - origin[1]) / sourceImageSpacing[1]) + 0.5);
	seedPoint[2] = floor(((Center[2] - origin[2]) / sourceImageSpacing[2]) + 0.5);
	cout << "seedPoint  " << seedPoint[0] << "," << seedPoint[1] << "," << seedPoint[2] << "\n"; */
	//myfile2 << "Intensity at seed in GT before normalization  " << gtImage->GetPixel(seedPoint) << endl;
	//cout << "Intensity at seed in GT before normalization  " << gtImage->GetPixel(seedPoint) << endl;
	string sb = "";
	gtImageout = ThresholdFilterGT(gtImage);
	/*sb = "_GT_threshold.nii.gz";
	Write(gtImageout, name + sb);
	cout << "Intensity at seed in GT " << gtImageout->GetPixel(seedPoint) << endl;
	myfile2 << "Intensity at seed in GT " << gtImageout->GetPixel(seedPoint) << endl;*/
	InternalImageType::Pointer MaskImage = CreateMask(diacomImage, diacomImage, axesImage, seedPoint);
	sigMuStdseed = getMuSigma(MaskImage);
	//myfile2 << "75 mm mask's mean and std ###############################------------" << "," << "," << sigMuStdseed[0] << " , " << sigMuStdseed[1] << std::endl;
	//cout << "75 mm  mask's mean and std ###############################------------" << "," << "," << sigMuStdseed[0] << " , " << sigMuStdseed[1] << std::endl;
	////MaskImage = NULL;

	int	diffusionTime = atoi(argv[8]);
	double  lambda = atof(argv[9]);
	char* enhancement = argv[10];
	double	featureScale = atoi(argv[11]);
	double exponent = atoi(argv[12]);
	double noiseScale = 0.6;// sigMuStdseed[1] * 0.025;
	double diffparam = (sigMuStdseed[0] - sigMuStdseed[1])*0.01;
	double snr = sigMuStdseed[0] / sigMuStdseed[1];
	/*cout << "diffparam  " << diffparam << endl;
	myfile2 << "diffparam " << "," << diffparam << endl;
	cout << "noiseScale before  " << noiseScale << endl;
	myfile2 << "noiseScale before " << "," << noiseScale << endl;*/
	diffusionTime = 1.5; //0.2 mm images
	if (snr < 1.5)
	{
		noiseScale = 0.3;
		lambda = 0.03;
		diffusionTime = 1.0;
		enhancement = "cCED";//cCED
		//featureScale = 4; //4
	}
	else if (1.5 <= snr && snr < 1.8)
	{
		noiseScale = 0.8;//1.0
		lambda = 0.003;  //0.001
		diffusionTime = 2; //1.5
		enhancement = "cEED"; //CED
		//featureScale = 3; //4
	}
	else if (1.8 <= snr && snr < 2)
	{
		noiseScale = 0.7;//1.0
		lambda = 0.001;  //0.001
		diffusionTime = 1.5; //1.5
		enhancement = "cEED"; //CED
							  //featureScale = 3; //4
	}
	else if (2 <= snr && snr < 2.4)
	{

		noiseScale = 0.8; //0.5
		lambda = 0.005;  //0.005
		diffusionTime =1.5; //1.5
		enhancement = "cEED"; //cEED
		//featureScale = 3; //4
		cout << enhancement;
	}
	else if (2.4 <= snr && snr < 2.6)
	{
		noiseScale = 0.8; //0.8
		lambda = 0.005;  //0.005
		diffusionTime = 1.5; //1.5
		enhancement = "CED"; //CED
							 //featureScale = 4; //4
		cout << enhancement;
	}
	else if (2.6 <= snr && snr <3.1)
	{
		noiseScale = 0.6; //0.8
		lambda = 0.005;  //0.005
		diffusionTime = 1.5; //1.5
		enhancement = "cEED"; //CED
		featureScale = 3; //4
		cout << enhancement;
	}
	//else if (2.9 <= snr && snr < 2.92)
	//{
	//	noiseScale = 0.6; //0.8
	//	lambda = 0.05;  //0.005
	//	diffusionTime = 2; //1.5
	//	enhancement = "cCED"; //CED
	//						  //featureScale = 4; //4
	//	cout << enhancement;
	//}
	//else if (2.92 <= snr && snr < 3.1)
	//{
	//	noiseScale = 0.6; //0.8
	//	lambda = 0.005;  //0.005
	//	diffusionTime = 1.5; //1.5
	//	enhancement = "cEED"; //CED
	//						  //featureScale = 4; //4
	//	cout << enhancement;
	//}
	else if (3.1 <= snr && snr < 3.4)
	{
		noiseScale = 0.6; //0.6
		lambda = 0.005;  //0.005
		diffusionTime = 2.0; //1.5
		enhancement = "cEED"; //cEED
		//featureScale = 4; //4
		cout << enhancement;
	}
	//else if (3.2 <= snr && snr < 3.4)
	//{
	//	noiseScale = 0.6; //0.6
	//	lambda = 0.005;  //0.005
	//	diffusionTime = 1.5; //1.5
	//	enhancement = "cEED"; //cEED
	//						  //featureScale = 2; //4
	//	cout << enhancement;
	//}
	else if (3.4 <= snr && snr < 4.0)
	{
		noiseScale = 0.6;//1//0.6 //0.2
		lambda = 0.5;//0.5//0.05 //0.005
		diffusionTime = 1.0;//2 //1
		enhancement = "cEED"; //cCED
		//featureScale = 3;
		//exponent = 3;
	}
	//else if (4.0 <= snr && snr < 4.5)
	//{
	//	noiseScale = 0.5;//1//0.6
	//	lambda = 0.005;//0.5//0.05//0.00005
	//	diffusionTime = 2.0;//2
	//	enhancement = "CED";//cEED
	//	//featureScale = 3;
	//}
	else
	{
		noiseScale = 0.5;//1//0.6
		lambda = 0.0005;//0.5//0.05//0.00005
		diffusionTime = 1.5;//2
		enhancement = "CED";//cEED
		//featureScale = 3;
		//exponent = 3;

	}
	/*cout << "noiseScale  " << noiseScale << endl;
	myfile2 << "noiseScale  " << "," << noiseScale << endl;
	cout << "enhancement  " << enhancement << endl;
	myfile2 << "enhancement  " << "," << enhancement << endl;
	cout << "lambda  " << lambda << endl;
	myfile2 << "lambda  " << "," << lambda << endl;*/
	smoothimage = coherenceAnisotropic(MaskImage, diffusionTime, lambda, enhancement, noiseScale, featureScale, exponent);
	myfile2 << "stats of non local image output" << std::endl;
	string s = enhancement;
	stn = "_.nii.gz";
	Write(MaskImage, name + s + stn);
	stn.clear();
	MaskImage = NULL;
	sigMuStd[0] = 0; sigMuStd[1] = 0;
	MaskImage = CreateMask(smoothimage, smoothimage, axesImage, seedPoint);
	sigMuStd = getMuSigma(MaskImage);
	myfile2 << "after smoothing ###############################------------" << ", " << ", " << sigMuStd[0] << " , " << sigMuStd[1] << std::endl;
	//cout << "after smoothing ###############################------------" << ", " << ", " << sigMuStd[0] << " , " << sigMuStd[1] << std::endl;


	//calculating the image and gt gradient image or height function
	InternalImageType::Pointer  heightImage;
	//stn = "_GM.nii.gz";
	heightImage = HeightFunction(smoothimage);
	//Write(heightImage, name + stn);
	stn.clear();

	// creating the mask again for the image to avoide the edges artifact due to masking
	InternalImageType::Pointer maskSmoothImage;
	//stn = "_GM_Mask.nii.gz";       newName = name + stn;
	maskSmoothImage = CreateMask(diacomImage, heightImage, axes, seedPoint);
	//Write(maskSmoothImage, newName);
	newName.clear(); stn.clear();

	//Water shed segmentation
	segmentationImageWater = watershedSegmentation(maskSmoothImage, 0.005, 0.8, seedPoint, name);
	string wa = "_watershedSgmentation.nii.gz";
	Write(segmentationImageWater, name + wa);
	//sigMuStd[0] = 0; sigMuStd[1] = 0;
	////creating the groundtruth Mask image to get the mean and standard deviation of the ground truth after gradient image
	////InternalImageType::Pointer MaskImageNew = CreateMask(heightImage, heightImage, axesSeed, seedPoint);
	//InternalImageType::Pointer MaskImageNew = getMask(heightImage, segmentationImageWater);
	////stn = "maskwatershedGM.nii.gz";
	////Write(MaskImageNew, name + stn);
	//sigMuStd = getMuSigma(MaskImageNew);
	////
	//myfile2 << "mu sigma near seed point at 10mm #########------------: " << sigMuStd[0] << " , " << sigMuStd[1] << "\n" << std::endl;
	////cout << "mu sigma near seed point at 10mm #########------------: " << sigMuStd[0] << " , " << sigMuStd[1] << "\n" << std::endl;
	//float k1, k2;
	//k2 = sigMuStd[0];
	//k1 = sigMuStd[0] - sigMuStd[1] * 3;
	//float alpha, beta;
	//alpha = -sigMuStd[1] / 2;
	//beta = (k2 + k1) / 2;
	//myfile2 << "k1, k2  #########------------: " << k1 << " , " << k2 << "\n" << std::endl;
	//cout << "k1, k2  #########------------: " << k1 << " , " << k2 << "\n" << std::endl;
	//myfile2 << "alpha, beta  #########------------: " << alpha << " , " << beta << "\n" << std::endl;
	//cout << "alpha, beta  #########------------: " << alpha << " , " << beta << "\n" << std::endl;

	//InternalImageType::Pointer  edgeImage, geoImage, FastMarchingImagegeo, FastMarchingImage, Fastimagegeo;

	//edgeImage = sigmoid(maskSmoothImage, alpha, beta, name);

	//double stoppingTime = 15;
	//FastMarchingImage = FastMarching(edgeImage, seedPoint, name, stoppingTime);
	//segmentationImageFastMarch = ThresholdFilterFast(FastMarchingImage, seedPoint, stoppingTime);
	//Fastimagegeo = ThresholdFilterFastGeo(FastMarchingImage, seedPoint, stoppingTime);

	//string fa = "_segmented_fastMarching.nii.gz";
	//Write(segmentationImageFastMarch, name + fa);
	///*string temp = "_inverstedFast.nii.gz";
	//Write(Fastimagegeo, name + temp);*/

	//typedef itk::SignedDanielssonDistanceMapImageFilter<
	//	OutputImageType,
	//	InternalImageType >  FilterTypeDaniel;
	//FilterTypeDaniel::Pointer filterDaniel = FilterTypeDaniel::New();
	//filterDaniel->SetInput(segmentationImageWater);
	////filterDaniel->SetUseImageSpacing(TRUE);
	////filterDaniel->SetInsideIsPositive(TRUE);
	//try {
	//	filterDaniel->Update();
	//}
	//catch (itk::ExceptionObject & excep)
	//{
	//	std::cerr << "Exception caught in fast marching !" << std::endl;
	//	std::cerr << excep << std::endl;
	//}
	////string temp1 = "_DanielDistance.nii.gz";
	////Write(filterDaniel->GetOutput(), name + temp1);
	//////exit(1);
	//FastMarchingImagegeo = filterDaniel->GetOutput();//FastMarchingGeo(edgeImage, seedPoint, name); //;

	//geoImage = geodesicFilter(FastMarchingImagegeo, edgeImage, seedPoint, name);

	//string t = "_geoImage.nii.gz";
	//Write(geoImage, name + t);
	//segmentationImageGeo = ThresholdFilterGeo(geoImage, seedPoint);
	//string test = "_segmented_geodesic.nii.gz";
	//Write(segmentationImageGeo, name + test);



	/*cout << "Volume of the Gound truth... ";
	myfile2 << "Volume of the Gound truth...";
	*/float volumeGt = getVolume(gtImageout, sourceImageSpacing);

	/*cout << "Volume of the watershed segmentation... ";
	myfile2 << "Volume of the watershed segmentation... ";*/
	float volumeSegWater = getVolume(segmentationImageWater, sourceImageSpacing);

	float* vecWater = 0;
	vecWater = new float[3];
	vecWater = evaluate(gtImageout, segmentationImageWater);
	myfile2 << "Union (jaccard)" << "," << vecWater[0] << "\n";
	myfile2 << "Mean (dice)" << "," << vecWater[1] << "\n";
	myfile2 << "False negative" << "," << vecWater[2] << "\n";
	myfile2 << "False positive" << "," << vecWater[3] << "\n";

	//myfile2 << "Volume of the geodesic segmentation... ";
	////cout << "Volume of the geodesic segmentation... ";
	//float volumeSegGeo = getVolume(segmentationImageGeo, sourceImageSpacing);

	//float* vecGeo = 0;
	//vecGeo = new float[3];
	//vecGeo = evaluate(gtImageout, segmentationImageGeo);
	//myfile2 << "Union (jaccard)" << "," << vecGeo[0] << "\n";
	//myfile2 << "Mean (dice)" << "," << vecGeo[1] << "\n";
	//myfile2 << "False negative" << "," << vecGeo[2] << "\n";
	//myfile2 << "False positive" << "," << vecGeo[3] << "\n";

	//myfile2 << "Volume of the fastmarching segmentation... ";
	////cout << "Volume of the fastmarching segmentation... ";
	//float volumeSegFast = getVolume(segmentationImageFastMarch, sourceImageSpacing);

	//float* vecFast = 0;
	//vecFast = new float[3];
	//vecFast = evaluate(gtImageout, segmentationImageFastMarch);

	//myfile2 << "Union (jaccard)" << "," << vecFast[0] << "\n";
	//myfile2 << "Mean (dice)" << "," << vecFast[1] << "\n";
	//myfile2 << "False negative" << "," << vecFast[2] << "\n";
	//myfile2 << "False positive" << "," << vecFast[3] << "\n";

	clock.Stop();

	myfile2.close();

	myfile2.open(argv[14], ios::out | ios::app);
	myfile2 << argv[13] << "," << volumeGt << "," << snr << "," << ",";
	myfile2 << vecWater[0] << "," << vecWater[1] << "," << vecWater[2] << "," << vecWater[3] << "," << volumeSegWater << "," << endl;
	/*myfile2 << vecGeo[0] << "," << vecGeo[1] << "," << vecGeo[2] << "," << vecGeo[3] << "," << volumeSegGeo << "," << ",";
	myfile2 << vecFast[0] << "," << vecFast[1] << "," << vecFast[2] << "," << vecFast[3] << "," << volumeSegFast << "," << endl;*/

	myfile2.close();


	// print computation time at screen
	myfile2.open(file, ios::out | ios::app);
	myfile2 << "###############################" << std::endl;
	myfile2 << "SegmentationABUS" << std::endl;
	myfile2 << "Total time: " << clock.GetTotal() / 60.0 << " min." << std::endl;
	myfile2 << "Total time: " << clock.GetTotal() << " s." << std::endl;
	myfile2 << "###############################" << std::endl;
	myfile2 << "***************************************************************" << std::endl << endl;
	myfile2.close();

	cout << "###############################" << std::endl;
	cout << "SegmentationABUS" << std::endl;
	cout << "Total time: " << clock.GetTotal() / 60.0 << " min." << std::endl;
	cout << "Total time: " << clock.GetTotal() << " s." << std::endl;
	cout << "###############################" << std::endl;
	cout << "***************************************************************" << std::endl << endl;
}