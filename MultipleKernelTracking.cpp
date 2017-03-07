//+-----------------------------------------------------------------------
//
//  Copyright (c) 2013 Chun-Te Chu <ctchu@u.washington.edu>.  All rights reserved.
//
//  Description:
//      Implementation of Multiple-Kernel Tracking
//
//	For more details, please refer to :
//	Chun-Te Chu, Jenq-Neng Hwang, Hung-I Pai and Kung-Ming Lan �� Tracking Human Under Occlusion Based On Adaptive
//	Multiple Kernels With Projected Gradients,�� IEEE Trans. on Multimedia,15(7):1602-1615, November 2013.
//
//------------------------------------------------------------------------

#include "MultipleKernelTracking.h"

#include <ctime>
#include <QTime>
#include <QDebug>

double abs(double num)
{
    return (num<0 ? -num : num);
}

float abs(float num)
{
    return (num<0 ? -num : num);
}

//*************************************************************
//	Method:printMat
//
//	Description: print the content of a matrix
//*************************************************************
void printMat(char* matName, CvMat* mat, int rowsNum, int colsNum)
{
	printf("[%s]\n", matName);

	for(int i = 0; i < rowsNum; ++i)
	{
		for(int j = 0; j < colsNum; ++j)
			printf("%f ", float(cvmGet(mat, i, j)));

		printf("\n");
	}

    printf("\n");

}

//*************************************************************
//	Method:printImg
//
//	Description: print the content of an image
//				(only single channel image is allowed)
//*************************************************************
void printImg(IplImage* img, int rowsNum, int colsNum)
{

	int max = 0;
	for(int i = 0; i < rowsNum; ++i)
	{
		for(int j = 0; j < colsNum; ++j)
		{
			int index = ((uchar*)(img->imageData + i*img->widthStep))[j];
			printf("%d ", index);

			if (index > max)
				max = index;
		}

		printf("\n");
	}

	printf("\n");
	printf("max = %d\n", max);

}

//===================================== ObjInfo ==========================================

ObjInfo::ObjInfo()
{

}

ObjInfo::~ObjInfo()
{

}

void ObjInfo::initial(int frameWid, int frameHei, int histRow, int histCol)
{
	Hist = cvCreateMat(histRow, histCol, CV_32FC1);
	Mask = cvCreateMat(frameHei, frameWid, CV_32FC1);
	clearTrajectory();
	Weight = 1.0f;
}

void ObjInfo::setNode(XYpair<float> pos, XYpair<int> maxXY, XYpair<int> minXY, int area, CvMat *hist, int wid, int hei, CvMat *mask)
{


	Pos.setXY(pos);
	MaxXY.setXY(maxXY);
	MinXY.setXY(minXY);
	Area = area;

	Wid = wid;
	Hei = hei;

	cvCopy(hist, Hist);
	cvCopy(mask, Mask);
	setTrajectory(pos);
	setTrajectory(pos);				// set the first position twice as the first two histories

}

//*************************************************************
//	Method:shift
//
//	Description: shift the object
//*************************************************************
void ObjInfo::shift(int deltaX, int deltaY)
{
	Pos.shift(deltaX, deltaY);

	Area = 0;
	XYpair<int> tempMaxXY = MaxXY;
	XYpair<int> tempMinXY = MinXY;
	MaxXY.setXY(0, 0);
	MinXY.setXY(10000, 10000);
	CvMat* tempMask = cvCreateMat(Mask->rows, Mask->cols,  CV_32FC1);
	cvCopy(Mask, tempMask);
	cvZero(Mask);

	for (int x = tempMinXY.getX(); x < tempMaxXY.getX() + 1; ++x)
	{
		for (int y = tempMinXY.getY(); y < tempMaxXY.getY() + 1; ++y)
		{
			if ((tempMask->data.fl[y*tempMask->cols+x]) != 0)
			{
				if( (x + deltaX) >= 0 && (y + deltaY) >= 0 && (x + deltaX) < tempMask->cols && (y + deltaY) < tempMask->rows)
				{
					Mask->data.fl[(y + deltaY)*Mask->cols + ( x + deltaX)] = 255.0f;

					Area = Area + 1;

					if((x + deltaX) > MaxXY.getX() ) MaxXY.setX(x + deltaX);	//get max value of x
					if((y + deltaY) > MaxXY.getY() ) MaxXY.setY(y + deltaY);	//get max value of y

					if((x + deltaX) < MinXY.getX() ) MinXY.setX(x + deltaX);	//get min value of x
					if((y + deltaY) < MinXY.getY() ) MinXY.setY(y + deltaY);	//get min value of y

				}

			}
		}
	}

	Wid = MaxXY.getX()-MinXY.getX()+1;
	Hei = MaxXY.getY()-MinXY.getY()+1;

    cvReleaseMat(&tempMask);
}

//*************************************************************
//	Method:changeObjectScale
//
//	Description: change the scale of the object
//*************************************************************
void ObjInfo::changeObjectScale(float newScale, Config confLoader)
{
	Area = 0;

	float s = confLoader.getScaleSmooth()*1+(1-confLoader.getScaleSmooth())*newScale;				// smooth the change

	int minx = (int) (Pos.getX() + (MinXY.getX()-Pos.getX())*s + 0.5f);
	int miny = (int) (Pos.getY() + (MinXY.getY()-Pos.getY())*s + 0.5f);
	int maxx = (int) (Pos.getX() + (MaxXY.getX()-Pos.getX())*s + 0.5f);
	int maxy = (int) (Pos.getY() + (MaxXY.getY()-Pos.getY())*s + 0.5f);

	// To avoid underestimation of the change because of the original width or height being too small, if
	// either height or width changes more than aspectRatio, the other one should change a little bit, too.
	if(MaxXY.getX() - MinXY.getX() <= MaxXY.getY() - MinXY.getY())
	{
		float aspectRatio = (MaxXY.getY() - MinXY.getY()) / (MaxXY.getX() - MinXY.getX());
		if(abs((MaxXY.getY() - MinXY.getY()) - (maxy - miny)) >= aspectRatio   &&  abs((MaxXY.getX() - MinXY.getX()) - (maxx - minx)) == 0)
		{
			if(s<1)
				minx = MIN(minx+1, maxx);
			else if(s>1)
				minx = MAX(minx-1, 0);
		}
	}
	else
	{
		float aspectRatio = (MaxXY.getX() - MinXY.getX()) / (MaxXY.getY() - MinXY.getY());
		if(abs((MaxXY.getX() - MinXY.getX()) - (maxx - minx)) >= aspectRatio   &&  abs((MaxXY.getY() - MinXY.getY()) - (maxy - miny)) == 0)
		{
			if(s<1)
				miny = MIN(miny+1, maxy);
			else if(s>1)
				miny = MAX(miny-1, 0);
		}
	}

	MaxXY.setXY(maxx, maxy);	MinXY.setXY(minx, miny);

	float axisA;
	float axisB;
	float axisC;
	XYpair<float> focalP1;
	XYpair<float> focalP2;

	if(maxy-miny >= maxx-minx)					// vertical ellipse
	{
		axisA = (maxy-miny+1)/2.0f;
		axisB = (maxx-minx+1)/2.0f;
		axisC = sqrt(axisA*axisA-axisB*axisB);
		focalP1.setXY(Pos.getX(), Pos.getY()-axisC);
		focalP2.setXY(Pos.getX(), Pos.getY()+axisC);
	}
	else
	{
		axisB = (maxy-miny+1)/2.0f;
		axisA = (maxx-minx+1)/2.0f;
		axisC = sqrt(axisA*axisA-axisB*axisB);
		focalP1.setXY(Pos.getX()-axisC, Pos.getY());
		focalP2.setXY(Pos.getX()+axisC, Pos.getY());
	}

	for (int x = 0; x < Mask->width; ++x)
	{
		for (int y = 0; y < Mask->height; ++y)
		{
			float dist1 = sqrt((x-focalP1.getX())*(x-focalP1.getX()) + (y-focalP1.getY())*(y-focalP1.getY()));
			float dist2 = sqrt((x-focalP2.getX())*(x-focalP2.getX()) + (y-focalP2.getY())*(y-focalP2.getY()));
			if(dist1+dist2 <= 2*axisA+1 )						// if inside the ellipse
			{
				Mask->data.fl[y*Mask->cols+x] = 255.0f;
				Area++;
			}
			else
				Mask->data.fl[y*Mask->cols+x] = 0;
		}
	}

	Wid = MaxXY.getX()-MinXY.getX()+1;
	Hei = MaxXY.getY()-MinXY.getY()+1;

}

void ObjInfo::clearObjInfo()
{
	releaseHist();
	releaseMask();
	clearTrajectory();
	clearIterationTrajectory();
}
//===================================== ObjInfo ==========================================////


//===================================== Constraint ==========================================

Constraint::Constraint()
{

}


Constraint::~Constraint()
{

}

//*************************************************************
//	Method:calculateConstraintCost
//
//	Description: given two objects, calculate the constraint
//				function values
//*************************************************************
float Constraint::calculateConstraintCost(ObjInfo& a, ObjInfo& b)
{
	if (TypeOfConstraint == L2XY)				//	1: Euclidean distance
	{
		return (a.getPos().distances2(b.getPos())-OriDistance*OriDistance) / (OriDistance*OriDistance + 1);
	}
	else if (TypeOfConstraint == L2Y)			//  2: Euclidean distanc only for y
	{
		return ( (a.getPosY()- b.getPosY())*(a.getPosY()- b.getPosY())-OriDistance*OriDistance )  / (OriDistance*OriDistance + 1);
	}
	else
	{
		printf("[Error]: calculateConstraintCost():  invalid input for ConstraintCost computation\n");
		return -100;
	}
}

//===================================== Constraint ==========================================////



//===================================== MultipleKernelTracking ==========================================

MultipleKernelTracking::MultipleKernelTracking()
{
    int kernelNumber = 2;
    if(!confLoader.loadConf())
    {
        printf("[Error]: error in loading configuration\n");
    }
    _kernelNumber = kernelNumber;
    _target.clear();
}


MultipleKernelTracking::MultipleKernelTracking(int kernelNumber)
{
    if(!confLoader.loadConf())
    {
        printf("[Error]: error in loading configuration\n");
    }
	_kernelNumber = kernelNumber;
	_target.clear();
}

MultipleKernelTracking::~MultipleKernelTracking()
{

	for(unsigned int i = 0; i < _target.size(); ++i)
		_target[i].clearObjInfo();

	_target.clear();
    _constraintList.clear();
	_thresholdConstraint.clear();

}

//*************************************************************
//	Method:constructImagePlane
//
//	Description: transform the original input image frame to the
//				 image format based on the desired color space
//*************************************************************
void MultipleKernelTracking::constructImagePlane(IplImage *img, IplImage *useImg, ColorSpace choice)
{
	if(useImg->nChannels == 3)
	{
		if (choice == GREY)				// gray level
		{
			IplImage* singleChannelImg = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
			cvCvtColor(img, singleChannelImg, CV_BGR2GRAY);
			cvMerge(singleChannelImg, singleChannelImg, singleChannelImg, 0, useImg);
			cvReleaseImage(&singleChannelImg);
		}
		else if (choice == HSV)			// H or S or V value in HSV color space
		{
			cvCvtColor(img, useImg, CV_BGR2HSV);
		}
		else if (choice == BGR)			// B or G or R value in RGB color space
		{
			cvCopy(img, useImg);
		}
	}
	else if(useImg->nChannels == 1)
	{
		if (choice == GREY)				// gray level
		{
			cvCvtColor(img, useImg, CV_BGR2GRAY);
		}
		else if (choice == HSV)			// H or S or V value in HSV color space
		{
			IplImage* hsvImg = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 3);
			cvCvtColor(img, hsvImg, CV_BGR2HSV);
			cvCvtPixToPlane(hsvImg, useImg, NULL, NULL, NULL);
			cvReleaseImage(&hsvImg);
		}
		else if (choice == BGR)			// B or G or R value in RGB color space
		{
			cvCvtPixToPlane(img, useImg, NULL, NULL, NULL);
		}
	}
}

//*************************************************************
//	Method:calculateWeight
//
//	Description: calculate the weight value in the computation of
//				histogram based on the kernel type.
//
//*************************************************************
float MultipleKernelTracking::calculateWeight(XYpair<float> pos, int x, int y, KernelType kernelChoice, int wid, int hei, float s)		// calculate the weight used by histogram construction
{
    float weight = 0.0;
	wid = wid*s;			// scale change
	hei= hei*s;

	if (kernelChoice == Gaussian)				// gaussian
	{
		float kernelsigmaX = wid/2.0f;
		float kernelsigmaY = hei/2.0f;
		weight = (1/(2*PI*kernelsigmaX*kernelsigmaY))*exp(-(x-pos.getX())*(x-pos.getX())/(2*kernelsigmaX*kernelsigmaX))*exp(-(y-pos.getY())*(y-pos.getY())/(2*kernelsigmaY*kernelsigmaY));
	}
	else if (kernelChoice == Roof)				// roof
	{
		float roofSpan = sqrt(float (wid*wid+hei*hei));
		float roofLength = roofSpan*PI/6;
		float dist = sqrt((x-pos.getX())*(x-pos.getX()) + (y-pos.getY())*(y-pos.getY()));
		if (dist < (roofSpan/2) )
			weight = 4*(roofSpan/2-dist)/(roofLength*roofSpan*roofSpan);
		else
			weight = 0;
	}
	else if (kernelChoice == Epanechnikov)				// Epanechnikov
	{
		float dist = ((x-pos.getX())*(x-pos.getX()) + (y-pos.getY())*(y-pos.getY()))/(hei*hei/4 + wid*wid/4);
		if (dist<1)
			weight = 2*(1-dist)/PI;
		else
			weight = 0;
	}

	return weight;
}

//*************************************************************
//	Method:weightFunction
//
//	Description: calculate the weight value in the computation of
//				the mean shift vector based on the distance function.
//
//*************************************************************
float MultipleKernelTracking::weightFunction(CvMat* targetHist, CvMat* candidateHist, int bin, int channelIndex, int choice)				// calculate the weight used by mean shift vector calculation
{
    float weight = 0.0;
	if (choice ==1)					//correlation (original mean shift)
	{
		weight = sqrt(targetHist->data.fl[bin*targetHist->cols+channelIndex]/candidateHist->data.fl[bin*candidateHist->cols+channelIndex]);
	}
	else if (choice == 2)			//KL-distance
	{
		weight = (targetHist->data.fl[bin*targetHist->cols+channelIndex]/candidateHist->data.fl[bin*candidateHist->cols+channelIndex]);
	}
	else if (choice == 3)			//dual KL-distance
	{
		if (targetHist->data.fl[bin*targetHist->cols+channelIndex] >= candidateHist->data.fl[bin*candidateHist->cols+channelIndex])
			weight = 0.5f*(targetHist->data.fl[bin*targetHist->cols+channelIndex]/candidateHist->data.fl[bin*candidateHist->cols+channelIndex] - log(targetHist->data.fl[bin*targetHist->cols+channelIndex]/candidateHist->data.fl[bin*candidateHist->cols+channelIndex]) - 1);
		else
			weight = 0;
	}

	return weight;
}

//*************************************************************
//	Method:kernelDerivative
//
//	Description: calculate the derivative of the kernel in the computation of
//				the mean shift vector based on the kernel type.
//
//*************************************************************
float MultipleKernelTracking::kernelDerivative(XYpair<float> pos, int x, int y, KernelType type, int wid, int hei)			// calculate the kernel value used by mean shift vector calculation
{
    float weight = 0.0;

	if (type == Gaussian)				// gaussian
	{
		float kernelsigmaX = wid/2.0f;
		float kernelsigmaY = hei/2.0f;
		weight = (1/(2*PI*kernelsigmaX*kernelsigmaY))*exp(-(x-pos.getX())*(x-pos.getX())/(2*kernelsigmaX*kernelsigmaX))*exp(-(y-pos.getY())*(y-pos.getY())/(2*kernelsigmaY*kernelsigmaY));
	}
	else if (type == Roof)				// roof
	{
		float roofSpan = sqrt(float (wid*wid+hei*hei));
		float roofLength = roofSpan*PI/6;
		float dist = sqrt((x-pos.getX())*(x-pos.getX()) + (y-pos.getY())*(y-pos.getY()));
		if (dist < (roofSpan/2) )
		{
			weight = -4/(roofSpan*roofSpan*roofLength);							// derivative of roof kernel is constant
		}
		else
			weight = 0;
	}
	else if (type == Epanechnikov)				// Epanechnikov
	{
		float dist = ((x-pos.getX())*(x-pos.getX()) + (y-pos.getY())*(y-pos.getY()))/(hei*hei/4 + wid*wid/4);
		if (dist<1)
		{
			weight = -2/PI;							// derivative of Epanechnikov kernel is constant
		}
		else
			weight = 0;
	}


	return weight;
}

//*************************************************************
//	Method:constructHistogram
//
//	Description: build the histogram
//
//*************************************************************
CvMat* MultipleKernelTracking::constructHistogram(IplImage *useImg, CvMat *mask, XYpair<int> maxXY, XYpair<int> minXY, XYpair<float> pos, float scale)
{

	CvMat* currentHistogram = cvCreateMat(int (pow(float (confLoader.getBinInOneChannel()), float (confLoader.getImageChannelNumber())/confLoader.getHistogramChannelNumber())), confLoader.getHistogramChannelNumber(), CV_32FC1);
	cvZero(currentHistogram);

	int* histStep = new int[confLoader.getImageChannelNumber()];
	if(confLoader.getColorModel() == HSV)			// HSV
	{
		if(confLoader.getImageChannelNumber() == 3)
		{
            histStep[0] = 181/confLoader.getBinInOneChannel();		// H
            histStep[1] = 256/confLoader.getBinInOneChannel();		// S
            histStep[2] = 256/confLoader.getBinInOneChannel();		// V
		}
		else if(confLoader.getImageChannelNumber() == 1)
		{
			histStep[0] = 181/confLoader.getBinInOneChannel();
		}
	}
	else if(confLoader.getColorModel() == BGR)				// BGR
	{
		if(confLoader.getImageChannelNumber() == 3)
		{
			histStep[0] = 256/confLoader.getBinInOneChannel();
			histStep[1] = 256/confLoader.getBinInOneChannel();
			histStep[2] = 256/confLoader.getBinInOneChannel();
		}
		else if(confLoader.getImageChannelNumber() == 1)
		{
			histStep[0] = 256/confLoader.getBinInOneChannel();
		}
	}

	float totalWeight = 0;

	float weight = 1;

	for (int i = minXY.getX(); i < maxXY.getX()+1; ++i)
	{
		for (int j = minXY.getY(); j < maxXY.getY()+1; ++j)
		{

			if(mask->data.fl[j*mask->cols+i] != 0)
			{
				if( confLoader.getHistogramChannelNumber() != 1  || confLoader.getImageChannelNumber() != 3 )
				{
					for(int ch = 0; ch < confLoader.getHistogramChannelNumber(); ++ch)
					{
						int binIndex = ((uchar *) (useImg->imageData+j*useImg->widthStep))[confLoader.getHistogramChannelNumber()*i+ch]/histStep[ch];
						if (binIndex >= confLoader.getBinInOneChannel())
							binIndex = confLoader.getBinInOneChannel() - 1;

						weight = calculateWeight(pos, i, j, KERNEL_TYPE, maxXY.getX()-minXY.getX(), maxXY.getY()-minXY.getY(), scale);					// depends on the kernel type and distance
                        currentHistogram->data.fl[binIndex*currentHistogram->cols+ch] = currentHistogram->data.fl[binIndex*currentHistogram->cols+ch] + weight;
					}
					totalWeight = totalWeight + weight;
				}
				else								//  3D histogram
				{
					int individualIndex;
					int binIndex = ((uchar *) (useImg->imageData+j*useImg->widthStep))[3*i]/histStep[0];
					if (binIndex >= confLoader.getBinInOneChannel())
						binIndex = confLoader.getBinInOneChannel() - 1;

					individualIndex = ((uchar *) (useImg->imageData+j*useImg->widthStep))[3*i+1]/histStep[1];
					if (individualIndex >= confLoader.getBinInOneChannel())
						individualIndex = confLoader.getBinInOneChannel() - 1;

					binIndex = binIndex + individualIndex*confLoader.getBinInOneChannel();

					individualIndex = ((uchar *) (useImg->imageData+j*useImg->widthStep))[3*i+2]/histStep[2];
					if (individualIndex >= confLoader.getBinInOneChannel())
						individualIndex = confLoader.getBinInOneChannel() - 1;

					binIndex = binIndex + individualIndex*confLoader.getBinInOneChannel()*confLoader.getBinInOneChannel();

					weight = calculateWeight(pos, i, j, KERNEL_TYPE, maxXY.getX()-minXY.getX(), maxXY.getY()-minXY.getY(), scale);					// depends on the kernel type and distance
					currentHistogram->data.fl[binIndex*currentHistogram->cols] = currentHistogram->data.fl[binIndex*currentHistogram->cols] + weight;

					totalWeight = totalWeight + weight;
				}
			}
		}
	}

	delete [] histStep;

	cvAddS(currentHistogram, cvRealScalar(1.0f/INT_MAX), currentHistogram);
	totalWeight = totalWeight + currentHistogram->rows*currentHistogram->cols/INT_MAX;
	cvConvertScale(currentHistogram, currentHistogram, 1.0f/totalWeight);


	return currentHistogram;
}

//*************************************************************
//	Method:manualInitializeMask
//
//	Description: use the manual selected mask to initialize the model
//				of the kernel
//
//*************************************************************
void MultipleKernelTracking::manualInitializeMask(IplImage* frame, IplImage* mask)
{
	int maskMinX = INT_MAX;
	int maskMinY = INT_MAX;
	int maskMaxX = 0;
	int maskMaxY = 0;

	CvMat* maskMatrix = cvCreateMat(mask->height, mask->width,  CV_32FC1);
	IplImage* graymask = cvCreateImage(cvSize(mask->width, mask->height), IPL_DEPTH_8U, 1);
	cvCvtColor(mask, graymask, CV_BGR2GRAY);
	cvCmpS(graymask, 128, graymask, CV_CMP_GE);
	cvConvert(graymask, maskMatrix);

	////// find the boundary of the kernel
	for (int x = 0; x < frame->width; ++x)
	{
		for (int y = 0; y < frame->height; ++y)
		{
			if (maskMatrix->data.fl[y*maskMatrix->cols+x] >= 128)
			{
				if(x > maskMaxX ) maskMaxX = x;	//get max value of x
				if(y > maskMaxY ) maskMaxY = y;	//get max value of y

				if(x < maskMinX ) maskMinX = x;	//get min value of x
				if(y < maskMinY ) maskMinY = y;	//get min value of y
			}
		}
	}

	XYpair<float> maskPos((maskMinX+maskMaxX)/2.0f, (maskMinY+maskMaxY)/2.0f);
	XYpair<int> maskMaxXY(maskMaxX, maskMaxY);
	XYpair<int> maskMinXY(maskMinX, maskMinY);

	IplImage* useImg = cvCreateImage(cvSize(frame->width,frame->height), IPL_DEPTH_8U, confLoader.getImageChannelNumber());			// confLoader.getImageChannelNumber() channels
	constructImagePlane(frame, useImg, confLoader.getColorModel());

	CvMat* maskHistogram = constructHistogram(useImg, maskMatrix, maskMaxXY, maskMinXY, maskPos, 1);

	/// Ellipse parameters
	float axisA;
	float axisB;
	float axisC;
	XYpair<float> focalP1;
	XYpair<float> focalP2;

	if(maskMaxY-maskMinY >= maskMaxX-maskMinX)					// vertical ellipse
	{
		axisA = (maskMaxY-maskMinY+1)/2.0f;
		axisB = (maskMaxX-maskMinX+1)/2.0f;
		axisC = sqrt(axisA*axisA-axisB*axisB);
		focalP1.setXY(maskPos.getX(), maskPos.getY()-axisC);
		focalP2.setXY(maskPos.getX(), maskPos.getY()+axisC);
	}
	else
	{
		axisB = (maskMaxY-maskMinY+1)/2.0f;
		axisA = (maskMaxX-maskMinX+1)/2.0f;
		axisC = sqrt(axisA*axisA-axisB*axisB);
		focalP1.setXY(maskPos.getX()-axisC, maskPos.getY());
		focalP2.setXY(maskPos.getX()+axisC, maskPos.getY());
	}

	int maskArea = 0;
	for (int x = 0; x < frame->width; ++x)
	{
		for (int y = 0; y < frame->height; ++y)
		{
			float dist1 = sqrt((x-focalP1.getX())*(x-focalP1.getX()) + (y-focalP1.getY())*(y-focalP1.getY()));
			float dist2 = sqrt((x-focalP2.getX())*(x-focalP2.getX()) + (y-focalP2.getY())*(y-focalP2.getY()));
			if(dist1+dist2 <= 2*axisA+1 )						// if inside the ellipse
			{
				maskMatrix->data.fl[y*maskMatrix->cols+x] = 255;
				maskArea++;
			}
			else
			{
				maskMatrix->data.fl[y*maskMatrix->cols+x] = 0;
			}
		}
	}

	ObjInfo maskTarget;

	int histRow = int (pow(float (confLoader.getBinInOneChannel()), float (confLoader.getImageChannelNumber())/confLoader.getHistogramChannelNumber()));
	int histCol = confLoader.getHistogramChannelNumber();
	maskTarget.initial(maskMatrix->width, maskMatrix->height, histRow, histCol);

	maskTarget.setNode(maskPos, maskMaxXY, maskMinXY, maskArea, maskHistogram, maskMaxX-maskMinX+1, maskMaxY-maskMinY+1, maskMatrix);

	_target.push_back(maskTarget);

	cvReleaseMat(&maskHistogram);
	cvReleaseMat(&maskMatrix);
	cvReleaseImage(&graymask);
	cvReleaseImage(&useImg);

}

//*************************************************************
//	Method:autoInitializeMask
//
//	Description: use the manual selected mask (whole object) to
//				automatically divide the object into kernels and
//				initialize the models of the kernels.
//
//*************************************************************
void MultipleKernelTracking::autoInitializeMask(IplImage* frame, IplImage* mask)
{
    for (unsigned int i = 0; i < _target.size(); ++i)
    {
        _target[i].clearObjInfo();				// need to release the existing memory before initialize
    }
    _target.clear();						// clear the list

    int maskMinX = INT_MAX;
    int maskMinY = INT_MAX;
    int maskMaxX = 0;
    int maskMaxY = 0;

    CvMat* maskMatrix = cvCreateMat(mask->height, mask->width,  CV_32FC1);
    IplImage* graymask = cvCloneImage(mask);
    cvCmpS(graymask, 128, graymask, CV_CMP_GE);
    cvConvert(graymask, maskMatrix);

    ////// find the boundary of the whole object
    for (int x = 0; x < frame->width; ++x)
    {
        for (int y = 0; y < frame->height; ++y)
        {
            if (maskMatrix->data.fl[y*maskMatrix->cols+x] >= 128)
            {
                if(x > maskMaxX ) maskMaxX = x;	//get max value of x
                if(y > maskMaxY ) maskMaxY = y;	//get max value of y

                if(x < maskMinX ) maskMinX = x;	//get min value of x
                if(y < maskMinY ) maskMinY = y;	//get min value of y
            }
        }
    }

    ////// define the boundary of each kernel automatically
    int frameHeight = frame->height;
    int frameWidth = frame->width;

    float* startX = new float[_kernelNumber];
    float* endX = new float[_kernelNumber];
    float* startY = new float[_kernelNumber];
    float* endY = new float[_kernelNumber];


    // Design the kernels layput; Some overlapping between kernels is allowed
    if (_kernelNumber == 4)
    {
        /*//// another layout
        startX[0] = (float) maskMinX;
        endX[0] = (float) maskMinX + (maskMaxX - maskMinX)*confLoader.getFraction1();
        startX[1] = (float) maskMinX + (maskMaxX - maskMinX)*(1-confLoader.getFraction2());
        endX[1] = (float) maskMaxX;
        startX[2] = (float) maskMinX;
        endX[2] = (float) maskMinX + (maskMaxX - maskMinX)*confLoader.getFraction1();
        startX[3] = (float) maskMinX + (maskMaxX - maskMinX)*(1-confLoader.getFraction2());
        endX[3] = (float) maskMaxX;

        startY[0] = (float) maskMinY;
        endY[0] = (float) maskMinY + (maskMaxY - maskMinY)*confLoader.getFraction1();
        startY[2] = (float) maskMinY + (maskMaxY - maskMinY)*(1-confLoader.getFraction2());
        endY[2] = (float) maskMaxY;
        startY[1] = (float) maskMinY;
        endY[1] = (float) maskMinY + (maskMaxY - maskMinY)*confLoader.getFraction1();
        startY[3] = (float) maskMinY + (maskMaxY - maskMinY)*(1-confLoader.getFraction2());
        endY[3] = (float) maskMaxY;
        //// another layout*/


        startX[0] = (float) maskMinX;
        endX[0] = (float) maskMaxX;
        startX[1] = (float) maskMinX;
        endX[1] = (float) maskMaxX;
        startX[2] = (float) maskMinX;
        endX[2] = (float) maskMinX + (maskMaxX - maskMinX)*confLoader.getFraction3();
        startX[3] = (float) maskMinX + (maskMaxX - maskMinX)*(1-confLoader.getFraction4());
        endX[3] = (float) maskMaxX;

        startY[0] = (float) maskMinY;
        endY[0] = (float) maskMinY + (maskMaxY - maskMinY)*confLoader.getFraction1();
        startY[1] = (float) maskMinY + (maskMaxY - maskMinY)*(1-confLoader.getFraction2());
        endY[1] = (float) maskMaxY;
        startY[2] = (float) (maskMinY + maskMaxY)/2 - (maskMaxY - maskMinY)*confLoader.getFraction1()/2;
        endY[2] = (float) (maskMinY + maskMaxY)/2 + (maskMaxY - maskMinY)*confLoader.getFraction1()/2;
        startY[3] = (float) (maskMinY + maskMaxY)/2 - (maskMaxY - maskMinY)*confLoader.getFraction1()/2;
        endY[3] = (float) (maskMinY + maskMaxY)/2 + (maskMaxY - maskMinY)*confLoader.getFraction1()/2;
    }
    else if(_kernelNumber == 2)
    {
        startX[0] = (float) maskMinX;
        endX[0] = (float) maskMaxX;
        startX[1] = (float) maskMinX;
        endX[1] = (float) maskMaxX;
        startY[0] = (float) maskMinY;
        endY[0] = (float) maskMinY + (maskMaxY - maskMinY)*confLoader.getFraction1();
        startY[1] = (float) maskMinY + (maskMaxY - maskMinY)*(1-confLoader.getFraction2());
        endY[1] = (float) maskMaxY;
    }

    IplImage* useImg = cvCreateImage(cvSize(frame->width,frame->height), IPL_DEPTH_8U, confLoader.getImageChannelNumber());			// confLoader.getImageChannelNumber() channels
    constructImagePlane(frame, useImg, confLoader.getColorModel());

    ///// initialize each kernel
    for (int i = 0; i < _kernelNumber; ++i)				// for each kernel
    {
        CvMat* subMask = cvCreateMat(frameHeight, frameWidth,  CV_32FC1);
        cvZero(subMask);

        int subMaskMinX = INT_MAX;
        int subMaskMinY = INT_MAX;
        int subMaskMaxX = 0;
        int subMaskMaxY = 0;

        for (int y = (int) startY[i]; y <= (int) endY[i]; ++y)				// create the mask for each kernel i
        {
            for (int x = (int) startX[i]; x <= (int) endX[i]; ++x)
            {
                if ((maskMatrix->data.fl[y*maskMatrix->cols+x] >= 128) && (x>=startX[i]) && (x<=endX[i]) && (y>=startY[i]) && (y<=endY[i]))
                {
                    subMask->data.fl[y*subMask->cols+x] = 255;

                    if(x > subMaskMaxX ) subMaskMaxX = x;	//get max value of x
                    if(y > subMaskMaxY ) subMaskMaxY = y;	//get max value of y

                    if(x < subMaskMinX ) subMaskMinX = x;	//get min value of x
                    if(y < subMaskMinY ) subMaskMinY = y;	//get min value of y
                }
            }
        }

        XYpair<float> subMaskPos((subMaskMinX+subMaskMaxX)/2, (subMaskMinY+subMaskMaxY)/2);

        XYpair<int> subMaskMaxXY(subMaskMaxX, subMaskMaxY);
        XYpair<int> subMaskMinXY(subMaskMinX, subMaskMinY);

        CvMat* subMaskHistogram = constructHistogram(useImg, subMask, subMaskMaxXY, subMaskMinXY, subMaskPos, 1);

        /// Ellipse parameters
        float axisA;
        float axisB;
        float axisC;
        XYpair<float> focalP1;
        XYpair<float> focalP2;

        if(subMaskMaxY-subMaskMinY >= subMaskMaxX-subMaskMinX)					// vertical ellipse
        {
            axisA = (subMaskMaxY-subMaskMinY+1)/2.0f;
            axisB = (subMaskMaxX-subMaskMinX+1)/2.0f;
            axisC = sqrt(axisA*axisA-axisB*axisB);
            focalP1.setXY(subMaskPos.getX(), subMaskPos.getY()-axisC);
            focalP2.setXY(subMaskPos.getX(), subMaskPos.getY()+axisC);
        }
        else
        {
            axisB = (subMaskMaxY-subMaskMinY+1)/2.0f;
            axisA = (subMaskMaxX-subMaskMinX+1)/2.0f;
            axisC = sqrt(axisA*axisA-axisB*axisB);
            focalP1.setXY(subMaskPos.getX()-axisC, subMaskPos.getY());
            focalP2.setXY(subMaskPos.getX()+axisC, subMaskPos.getY());
        }

        int subMaskArea = 0;
        for (int x = 0; x < frame->width; ++x)
        {
            for (int y = 0; y < frame->height; ++y)
            {
                float dist1 = sqrt((x-focalP1.getX())*(x-focalP1.getX()) + (y-focalP1.getY())*(y-focalP1.getY()));
                float dist2 = sqrt((x-focalP2.getX())*(x-focalP2.getX()) + (y-focalP2.getY())*(y-focalP2.getY()));
                if(dist1+dist2 <= 2*axisA+1 )						// if inside the ellipse
                {
                    subMask->data.fl[y*subMask->cols+x] = 255;
                    subMaskArea++;
                }
                else
                    subMask->data.fl[y*subMask->cols+x] = 0;
            }
        }

        ObjInfo subMaskTarget;

        int histRow = int (pow(float (confLoader.getBinInOneChannel()), float (confLoader.getImageChannelNumber())/confLoader.getHistogramChannelNumber()));
        int histCol = confLoader.getHistogramChannelNumber();
        subMaskTarget.initial(subMask->width, subMask->height, histRow, histCol);

        subMaskTarget.setNode(subMaskPos, subMaskMaxXY, subMaskMinXY, subMaskArea, subMaskHistogram, subMaskMaxX-subMaskMinX+1, subMaskMaxY-subMaskMinY+1, subMask);

        _target.push_back(subMaskTarget);

        cvReleaseMat(&subMaskHistogram);
        cvReleaseMat(&subMask);
    }

    delete [] startY;
    delete [] endY;
    delete [] startX;
    delete [] endX;

    cvReleaseImage(&graymask);
    cvReleaseMat(&maskMatrix);
    cvReleaseImage(&useImg);

}

//*************************************************************
//	Method:autoInitializeCoordinate
//
//	Description: use the coordination information of the whole object to
//				automatically divide the object into kernels and
//				initialize the models of the kernels.
//
//*************************************************************
void MultipleKernelTracking::autoInitializeCoordinate(IplImage* frame, int* coordinate)
{
	for (unsigned int i = 0; i < _target.size(); ++i)
	{
		_target[i].clearObjInfo();				// need to release the existing memory before initialize
	}
	_target.clear();						// clear the list

	XYpair<int> maskMaxXY(coordinate[0]+coordinate[2]/2, coordinate[1]+coordinate[3]/2);
	XYpair<int> maskMinXY(coordinate[0]-coordinate[2]/2, coordinate[1]-coordinate[3]/2);
	CvMat* maskMatrix = cvCreateMat(frame->height, frame->width,  CV_32FC1);
	cvZero(maskMatrix);

	/// Ellipse parameters
	float axisA;
	float axisB;
	float axisC;
	XYpair<float> focalP1;
	XYpair<float> focalP2;

	if(coordinate[3] >= coordinate[2])					// vertical ellipse
	{
		axisA = coordinate[3]/2.0f;
		axisB = coordinate[2]/2.0f;
		axisC = sqrt(axisA*axisA-axisB*axisB);
		focalP1.setXY(coordinate[0], coordinate[1]-axisC);
		focalP2.setXY(coordinate[0], coordinate[1]+axisC);
	}
	else
	{
		axisA = coordinate[2]/2.0f;
		axisB = coordinate[3]/2.0f;
		axisC = sqrt(axisA*axisA-axisB*axisB);
		focalP1.setXY(coordinate[0]-axisC, coordinate[1]);
		focalP2.setXY(coordinate[0]+axisC, coordinate[1]);
	}

	////// find the boundary of the whole object
	for (int x = 0; x < frame->width; ++x)
	{
		for (int y = 0; y < frame->height; ++y)
		{
			float dist1 = sqrt((x-focalP1.getX())*(x-focalP1.getX()) + (y-focalP1.getY())*(y-focalP1.getY()));
			float dist2 = sqrt((x-focalP2.getX())*(x-focalP2.getX()) + (y-focalP2.getY())*(y-focalP2.getY()));
			if(dist1+dist2 <= 2*axisA+1 )						// if inside the ellipse
				maskMatrix->data.fl[y*maskMatrix->cols+x] = 255;
		}
	}

	////// define the boundary of each kernel automatically
	int frameHeight = frame->height;
	int frameWidth = frame->width;

	float* startX = new float[_kernelNumber];
	float* endX = new float[_kernelNumber];
	float* startY = new float[_kernelNumber];
	float* endY = new float[_kernelNumber];
	int maskMinX = maskMinXY.getX();
	int maskMinY = maskMinXY.getY();
	int maskMaxX = maskMaxXY.getX();
	int maskMaxY = maskMaxXY.getY();

	// Design the kernels layput; Some overlapping between kernels is allowed
	if (_kernelNumber == 4)
	{
		/*//// another layout
		startX[0] = (float) maskMinX;
		endX[0] = (float) maskMinX + (maskMaxX - maskMinX)*confLoader.getFraction1();
		startX[1] = (float) maskMinX + (maskMaxX - maskMinX)*(1-confLoader.getFraction2());
		endX[1] = (float) maskMaxX;
		startX[2] = (float) maskMinX;
		endX[2] = (float) maskMinX + (maskMaxX - maskMinX)*confLoader.getFraction1();
		startX[3] = (float) maskMinX + (maskMaxX - maskMinX)*(1-confLoader.getFraction2());
		endX[3] = (float) maskMaxX;

		startY[0] = (float) maskMinY;
		endY[0] = (float) maskMinY + (maskMaxY - maskMinY)*confLoader.getFraction1();
		startY[2] = (float) maskMinY + (maskMaxY - maskMinY)*(1-confLoader.getFraction2());
		endY[2] = (float) maskMaxY;
		startY[1] = (float) maskMinY;
		endY[1] = (float) maskMinY + (maskMaxY - maskMinY)*confLoader.getFraction1();
		startY[3] = (float) maskMinY + (maskMaxY - maskMinY)*(1-confLoader.getFraction2());
		endY[3] = (float) maskMaxY;
		//// another layout*/


		startX[0] = (float) maskMinX;
		endX[0] = (float) maskMaxX;
		startX[1] = (float) maskMinX;
		endX[1] = (float) maskMaxX;
		startX[2] = (float) maskMinX;
		endX[2] = (float) maskMinX + (maskMaxX - maskMinX)*confLoader.getFraction3();
		startX[3] = (float) maskMinX + (maskMaxX - maskMinX)*(1-confLoader.getFraction4());
		endX[3] = (float) maskMaxX;

		startY[0] = (float) maskMinY;
		endY[0] = (float) maskMinY + (maskMaxY - maskMinY)*confLoader.getFraction1();
		startY[1] = (float) maskMinY + (maskMaxY - maskMinY)*(1-confLoader.getFraction2());
		endY[1] = (float) maskMaxY;
		startY[2] = (float) (maskMinY + maskMaxY)/2 - (maskMaxY - maskMinY)*confLoader.getFraction1()/2;
		endY[2] = (float) (maskMinY + maskMaxY)/2 + (maskMaxY - maskMinY)*confLoader.getFraction1()/2;
		startY[3] = (float) (maskMinY + maskMaxY)/2 - (maskMaxY - maskMinY)*confLoader.getFraction1()/2;
		endY[3] = (float) (maskMinY + maskMaxY)/2 + (maskMaxY - maskMinY)*confLoader.getFraction1()/2;
	}
	else if(_kernelNumber == 2)
	{
		startX[0] = (float) maskMinX;
		endX[0] = (float) maskMaxX;
		startX[1] = (float) maskMinX;
		endX[1] = (float) maskMaxX;
		startY[0] = (float) maskMinY;
		endY[0] = (float) maskMinY + (maskMaxY - maskMinY)*confLoader.getFraction1();
		startY[1] = (float) maskMinY + (maskMaxY - maskMinY)*(1-confLoader.getFraction2());
		endY[1] = (float) maskMaxY;
	}

	IplImage* useImg = cvCreateImage(cvSize(frame->width,frame->height), IPL_DEPTH_8U, confLoader.getImageChannelNumber());			// confLoader.getImageChannelNumber() channels
	constructImagePlane(frame, useImg, confLoader.getColorModel());

	///// initialize each kernel
	for (int i =0; i < _kernelNumber; ++i)				// for each kernel
	{
		CvMat* subMask = cvCreateMat(frameHeight, frameWidth,  CV_32FC1);
		cvZero(subMask);

		int subMaskMinX = INT_MAX;
		int subMaskMinY = INT_MAX;
		int subMaskMaxX = 0;
		int subMaskMaxY = 0;

		for (int y = (int) startY[i]; y <= (int) endY[i]; ++y)				// create the mask for each kernel i
		{
			for (int x = (int) startX[i]; x <= (int) endX[i]; ++x)
			{
				if ((maskMatrix->data.fl[y*maskMatrix->cols+x] >= 128) && (x>=startX[i]) && (x<=endX[i]) && (y>=startY[i]) && (y<=endY[i]))
				{
					subMask->data.fl[y*subMask->cols+x] = 255;

					if(x > subMaskMaxX ) subMaskMaxX = x;	//get max value of x
					if(y > subMaskMaxY ) subMaskMaxY = y;	//get max value of y

					if(x < subMaskMinX ) subMaskMinX = x;	//get min value of x
					if(y < subMaskMinY ) subMaskMinY = y;	//get min value of y
				}
			}
		}

		XYpair<float> subMaskPos((subMaskMinX+subMaskMaxX)/2, (subMaskMinY+subMaskMaxY)/2);

		XYpair<int> subMaskMaxXY(subMaskMaxX, subMaskMaxY);
		XYpair<int> subMaskMinXY(subMaskMinX, subMaskMinY);

		CvMat* subMaskHistogram = constructHistogram(useImg, subMask, subMaskMaxXY, subMaskMinXY, subMaskPos, 1);

		/// Ellipse parameters
		float axisA;
		float axisB;
		float axisC;
		XYpair<float> focalP1;
		XYpair<float> focalP2;

		if(subMaskMaxY-subMaskMinY >= subMaskMaxX-subMaskMinX)					// vertical ellipse
		{
			axisA = (subMaskMaxY-subMaskMinY+1)/2.0f;
			axisB = (subMaskMaxX-subMaskMinX+1)/2.0f;
			axisC = sqrt(axisA*axisA-axisB*axisB);
			focalP1.setXY(subMaskPos.getX(), subMaskPos.getY()-axisC);
			focalP2.setXY(subMaskPos.getX(), subMaskPos.getY()+axisC);
		}
		else
		{
			axisB = (subMaskMaxY-subMaskMinY+1)/2.0f;
			axisA = (subMaskMaxX-subMaskMinX+1)/2.0f;
			axisC = sqrt(axisA*axisA-axisB*axisB);
			focalP1.setXY(subMaskPos.getX()-axisC, subMaskPos.getY());
			focalP2.setXY(subMaskPos.getX()+axisC, subMaskPos.getY());
		}
		int subMaskArea = 0;
		for (int x = 0; x < frame->width; ++x)
		{
			for (int y = 0; y < frame->height; ++y)
			{
				float dist1 = sqrt((x-focalP1.getX())*(x-focalP1.getX()) + (y-focalP1.getY())*(y-focalP1.getY()));
				float dist2 = sqrt((x-focalP2.getX())*(x-focalP2.getX()) + (y-focalP2.getY())*(y-focalP2.getY()));
				if(dist1+dist2 <= 2*axisA+1 )						// if inside the ellipse
				{
					subMask->data.fl[y*subMask->cols+x] = 255;
					subMaskArea++;
				}
				else
					subMask->data.fl[y*subMask->cols+x] = 0;
			}
		}

		ObjInfo subMaskTarget;

		int histRow = int (pow(float (confLoader.getBinInOneChannel()), float (confLoader.getImageChannelNumber())/confLoader.getHistogramChannelNumber()));
		int histCol = confLoader.getHistogramChannelNumber();
		subMaskTarget.initial(subMask->width, subMask->height, histRow, histCol);

		subMaskTarget.setNode(subMaskPos, subMaskMaxXY, subMaskMinXY, subMaskArea, subMaskHistogram, subMaskMaxX-subMaskMinX+1, subMaskMaxY-subMaskMinY+1, subMask);

		_target.push_back(subMaskTarget);

		cvReleaseMat(&subMaskHistogram);
		cvReleaseMat(&subMask);
	}

	delete [] startY;
	delete [] endY;
	delete [] startX;
	delete [] endX;
	cvReleaseMat(&maskMatrix);
	cvReleaseImage(&useImg);

}

//*************************************************************
//	Method:calculateSimi
//
//	Description: calculate the similarity between the candidata
//				and the target model.
//
//*************************************************************
float MultipleKernelTracking::calculateSimi(CvMat* targetmodel, CvMat* candidate, int choice)
{
	if (choice == 1)					//correlation (original mean shift)
	{
		CvMat* temp1 = cvCreateMat(targetmodel->rows, confLoader.getHistogramChannelNumber(), CV_32FC1);
		CvMat* temp2 = cvCreateMat(targetmodel->rows, confLoader.getHistogramChannelNumber(), CV_32FC1);

		cvMul(targetmodel, candidate, temp1);
		cvPow(temp1, temp2, 0.5);

		return cvNorm(temp2, NULL, CV_L1);

		cvReleaseMat(&temp2);
		cvReleaseMat(&temp1);
	}
	else if (choice == 2)			//KL-distance
	{
		float costValue = 0;
		for (int ch = 0; ch < confLoader.getHistogramChannelNumber(); ++ch)
		{
			for (int i = 0; i < targetmodel->rows; ++i)
			{
				costValue = costValue + (targetmodel->data.fl[i*targetmodel->cols+ch])*log((targetmodel->data.fl[i*targetmodel->cols+ch]/candidate->data.fl[i*candidate->cols+ch]));
			}
		}
		if (costValue < 0)
            costValue = -costValue;

		return 1/(1 + costValue);
	}
	else if (choice == 3)			//dual KL-distance
	{
		float temp = 0;
		for (int ch = 0; ch < confLoader.getHistogramChannelNumber(); ++ch)
		{
			for (int i = 0; i < targetmodel->rows; ++i)
			{
				temp = temp + 0.5f*(targetmodel->data.fl[i*targetmodel->cols+ch])*log((targetmodel->data.fl[i*targetmodel->cols+ch]/candidate->data.fl[i*candidate->cols+ch])) + 0.5*(candidate->data.fl[i*candidate->cols+ch])*log((candidate->data.fl[i*candidate->cols+ch]/targetmodel->data.fl[i*targetmodel->cols+ch]));
			}
		}

		return 1- temp;
	}
	else
	{
		printf("[Error]: calculateSimi(): invalid input for similarity computation\n");
		return -100;
	}

}

//*************************************************************
//	Method:calculateCost
//
//	Description: calculate the cost/distance between the candidata
//				and the target model.
//
//*************************************************************
float MultipleKernelTracking::calculateCost(CvMat* targetmodel, CvMat* candidate, int choice)
{
	if (choice == 1)					//correlation (original mean shift)
	{
		CvMat* temp1 = cvCreateMat(targetmodel->rows, confLoader.getHistogramChannelNumber(), CV_32FC1);
		CvMat* temp2 = cvCreateMat(targetmodel->rows, confLoader.getHistogramChannelNumber(), CV_32FC1);

		cvMul(targetmodel, candidate, temp1);
		cvPow(temp1, temp2, 0.5);

		return 1-cvNorm(temp2, NULL, CV_L1);

		cvReleaseMat(&temp2);
		cvReleaseMat(&temp1);
	}
	else if (choice == 2)			//KL-distance
	{
		float costValue = 0;
		for (int ch = 0; ch < confLoader.getHistogramChannelNumber(); ++ch)
		{
			for (int i = 0;i < targetmodel->rows; ++i)
			{
				costValue = costValue + (targetmodel->data.fl[i*targetmodel->cols+ch])*log((targetmodel->data.fl[i*targetmodel->cols+ch]/candidate->data.fl[i*candidate->cols+ch]));
			}
		}
		if (costValue < 0)
            costValue = -costValue;

		return costValue;
	}
	else if (choice == 3)			//dual KL-distance
	{
		float temp=0;
		for (int ch = 0; ch < confLoader.getHistogramChannelNumber(); ++ch)
		{
			for (int i = 0; i < targetmodel->rows; ++i)
			{
				temp = temp + 0.5f*(targetmodel->data.fl[i*targetmodel->cols+ch])*log((targetmodel->data.fl[i*targetmodel->cols+ch]/candidate->data.fl[i*candidate->cols+ch])) + 0.5*(candidate->data.fl[i*candidate->cols+ch])*log((candidate->data.fl[i*candidate->cols+ch]/targetmodel->data.fl[i*targetmodel->cols+ch]));
			}
		}
		if (temp < 0)
            temp = -temp;

		return temp;
	}
	else
	{
		printf("[Error]: calculateCost(): invalid input for cost computation\n");
		return -100;
	}
}

//*************************************************************
//	Method:setConstraintList
//
//	Description: set the constraint functions during initilization
//
//*************************************************************
void MultipleKernelTracking::setConstraintList()
{
    _constraintList.clear();
	for (unsigned int targeti = 0; targeti < _target.size()-1; ++targeti)
	{
		for (unsigned int targetj = targeti+1; targetj < _target.size(); ++targetj)
		{
			if (NCONSTRAINT >= 1)
			{
				Constraint cons;

				cons.setKernel1(targeti);
				cons.setKernel2(targetj);
				cons.setTypeOfConstraint(Constraint::L2XY);			// Euclidean distance
				cons.setOriDistance(_target[targeti].getPos().distances(_target[targetj].getPos()));

				_constraintList.push_back(cons);
			}
			if (NCONSTRAINT >= 2)
			{
				Constraint cons;

				cons.setKernel1(targeti);
				cons.setKernel2(targetj);
				cons.setTypeOfConstraint(Constraint::L2Y);			// Euclidean distance for only y
				cons.setOriDistance( abs(_target[targeti].getPosY()- _target[targetj].getPosY()));

				_constraintList.push_back(cons);
			}
		}
	}

	_thresholdConstraint.clear();
	for (unsigned int i = 0; i < _constraintList.size(); ++i)
		_thresholdConstraint.push_back(confLoader.getConstraintThreshold());
}

//*************************************************************
//	Method:computeConstraintGradient
//
//	Description: compute the gradient vector of constraint functions
//
//*************************************************************
CvMat* MultipleKernelTracking::computeConstraintGradient()
{
	CvMat* gradient = cvCreateMat(NSTATE*_kernelNumber, _constraintList.size(), CV_32FC1);
	cvZero(gradient);

	for (unsigned int constrainti = 0; constrainti < _constraintList.size(); ++constrainti)
	{
		int id1 = _constraintList[constrainti].getKernel1();
		int id2 = _constraintList[constrainti].getKernel2();

		if (_constraintList[constrainti].getTypeOfConstraint() == Constraint::L2XY)		//	1: Euclidean distance
		{
            gradient->data.fl[(NSTATE*id1)*gradient->cols+constrainti] = 2.0*(_target[id1].getPosX()-_target[id2].getPosX()) / (_constraintList[constrainti].getOriDistance() * _constraintList[constrainti].getOriDistance() + 1);		// 2*(x1-x2)
            gradient->data.fl[(NSTATE*id1+1)*gradient->cols+constrainti] = 2.0*(_target[id1].getPosY()-_target[id2].getPosY()) / (_constraintList[constrainti].getOriDistance() * _constraintList[constrainti].getOriDistance() + 1);	// 2*(y1-y2)
			gradient->data.fl[(NSTATE*id1+2)*gradient->cols+constrainti] = 0;
            gradient->data.fl[(NSTATE*id2)*gradient->cols+constrainti] = 2.0*(_target[id2].getPosX()-_target[id1].getPosX()) / (_constraintList[constrainti].getOriDistance() * _constraintList[constrainti].getOriDistance() + 1);		// 2*(x2-x1)
            gradient->data.fl[(NSTATE*id2+1)*gradient->cols+constrainti] = 2.0*(_target[id2].getPosY()-_target[id1].getPosY()) / (_constraintList[constrainti].getOriDistance() * _constraintList[constrainti].getOriDistance() + 1);	// 2*(y2-y1)
			gradient->data.fl[(NSTATE*id2+2)*gradient->cols+constrainti] = 0;
		}
		else if (_constraintList[constrainti].getTypeOfConstraint() == Constraint::L2Y)		//	2: Euclidean distance only for y
		{
			gradient->data.fl[(NSTATE*id1)*gradient->cols+constrainti] = 0;
            gradient->data.fl[(NSTATE*id1+1)*gradient->cols+constrainti] = 2.0*(_target[id1].getPosY()-_target[id2].getPosY()) / (_constraintList[constrainti].getOriDistance() * _constraintList[constrainti].getOriDistance() + 1);	// 2*(y1-y2)
			gradient->data.fl[(NSTATE*id1+2)*gradient->cols+constrainti] = 0;
			gradient->data.fl[(NSTATE*id2)*gradient->cols+constrainti] = 0;
            gradient->data.fl[(NSTATE*id2+1)*gradient->cols+constrainti] = 2.0*(_target[id2].getPosY()-_target[id1].getPosY()) / (_constraintList[constrainti].getOriDistance() * _constraintList[constrainti].getOriDistance() + 1);	// 2*(y2-y1)
			gradient->data.fl[(NSTATE*id2+2)*gradient->cols+constrainti] = 0;
		}
	}

	return gradient;
}

//*************************************************************
//	Method:computeConstraint
//
//	Description: compute the constraint functions values
//
//*************************************************************
CvMat* MultipleKernelTracking::computeConstraint()
{
	CvMat* constraintValues = cvCreateMat(_constraintList.size(), 1, CV_32FC1);
	float currentCost;

	for (unsigned int constrainti = 0; constrainti < _constraintList.size(); ++constrainti)
	{
		int id1 = _constraintList[constrainti].getKernel1();
		int id2 = _constraintList[constrainti].getKernel2();

		currentCost =  _constraintList[constrainti].calculateConstraintCost(_target[id1], _target[id2]);
        constraintValues->data.fl[constrainti*constraintValues->cols] = currentCost;
	}

	return constraintValues;
}

//*************************************************************
//	Method:calculateTotalCost
//
//	Description: calculate weighted total cost
//
//*************************************************************
float MultipleKernelTracking::calculateTotalCost(IplImage* img)
{
	float totalCost = 0;

	for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)
	{
		CvMat* candidateHist = constructHistogram(img, _target[targeti].getMask(), _target[targeti].getMaxXY(), _target[targeti].getMinXY(), _target[targeti].getPos(), 1);
		float trackcost = calculateCost(_target[targeti].getHist(), candidateHist, WEIGHT_TYPE);
		totalCost = totalCost + _target[targeti].getWeight()*trackcost;
		cvReleaseMat(&candidateHist);
	}
	return totalCost;
}

//*************************************************************
//	Method:calculateTotalSimi
//
//	Description: calculate weighted total similarity
//
//*************************************************************
float MultipleKernelTracking::calculateTotalSimi(IplImage* img, float s)
{
	float tempTotalSimi = 0;

	for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)
	{
        CvMat maskmat = *_target[targeti].getMask();
        //cv::imshow("target mask" , maskmat);
        //cvWaitKey(0);
    std::cout << "DDDDDDDDDDDEBUG : " << maskmat.cols << " , " << maskmat.rows << std::endl;
        CvMat* candidateHist = constructHistogram(img, _target[targeti].getMask(), _target[targeti].getMaxXY(), _target[targeti].getMinXY(), _target[targeti].getPos(), s);
		float tracksimi = calculateSimi(_target[targeti].getHist(), candidateHist, WEIGHT_TYPE);
		tempTotalSimi = tempTotalSimi + _target[targeti].getWeight()*tracksimi;
		cvReleaseMat(&candidateHist);
	}

	return tempTotalSimi;
}

//*************************************************************
//	Method:checkJ
//
//	Description: check whether the cost function is smaller than the threshold
//
//*************************************************************
bool MultipleKernelTracking::checkJ(IplImage* useImg)
{
	float costJ = calculateTotalCost(useImg);
#ifdef MKT_DEBUG
	printf("costJ = %f\n", float(costJ));
#endif
	if (costJ < confLoader.getCostThreshold()*_kernelNumber)
		return true;				// the cost is small enough
	else
		return false;
}

//*************************************************************
//	Method:checkC
//
//	Description: check whether the constraint functions are smaller than the thresholds
//
//*************************************************************
bool MultipleKernelTracking::checkC()
{
	bool converge = true;
	if (_constraintList.size() > 0)
	{
		CvMat* constraintC = computeConstraint();

		for(unsigned int i = 0; i < _constraintList.size(); ++i)
		{
#ifdef MKT_DEBUG
		printf("costraint %d = %f\n", i, float(abs(constraintC->data.fl[i*constraintC->cols])));
#endif
			if(abs(constraintC->data.fl[i*constraintC->cols]) >= _thresholdConstraint[i])
			{
				converge = false;												// at least one constraint function are not satisfied
				break;
			}
		}
		cvReleaseMat(&constraintC);
		return converge;
	}
	else
		return converge;
}

//*************************************************************
//	Method:adaptiveWeight
//
//	Description: apply adaptive weight
//
//*************************************************************
void MultipleKernelTracking::adaptiveWeight(CvMat* Jx)
{
	CvMat* W = cvCreateMat(_kernelNumber*NSTATE, _kernelNumber*NSTATE, CV_32FC1);
	cvZero(W);

	float* weight = new float [_kernelNumber];

	if(ADAPTIVE_WEIGHT_CHOICE == 1)
	{
		for(int i = 0; i < _kernelNumber; ++i)
		{
			weight[i] = _target[i].getSimi()*_target[i].getSimi();
		}
	}
	else if(ADAPTIVE_WEIGHT_CHOICE == 0)
	{
		for(int i = 0; i < _kernelNumber; ++i)
		{
			weight[i] = 1;
		}
		if(_kernelNumber == 2)
			weight[0] = 2;
		else if (_kernelNumber == 4)
		{
			weight[0] = 2;
			weight[1] = 2;
		}
	}
	float totalW = 0;
	for(int i = 0; i < _kernelNumber; ++i)
	{
		totalW += weight[i];
	}
	for(int i = 0; i < _kernelNumber; ++i)
	{
		weight[i] /= totalW;
		weight[i] *= _kernelNumber;
		_target[i].setWeight(weight[i]);
	}

	for (int i = 0; i < _kernelNumber; ++i)
	{
		W->data.fl[(i*NSTATE)*W->cols+i*NSTATE] = weight[i];
		W->data.fl[(i*NSTATE+1)*W->cols+i*NSTATE+1] = weight[i];
		W->data.fl[(i*NSTATE+2)*W->cols+i*NSTATE+2] = 1.0f;
	}
	cvMatMul(W, Jx, Jx);

	delete[] weight;
	cvReleaseMat(&W);
}

//*************************************************************
//	Method:predictUpdate
//
//	Description: simple prediction of the initial points in next frame
//
//*************************************************************
void MultipleKernelTracking::predictUpdate()
{
	for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)
	{
		if (_target[targeti].getTrajectoryLength() >= PREDICT_VELOCITY_SMOOTH)							// use the average velocity to predict
		{
			XYpair<float> previousEnd = _target[targeti].getTrajectory(_target[targeti].getTrajectoryLength() - 1);
			XYpair<float> previousStart = _target[targeti].getTrajectory(_target[targeti].getTrajectoryLength() - PREDICT_VELOCITY_SMOOTH);
			float aveX =  ((previousEnd.getX()-previousStart.getX())/PREDICT_VELOCITY_SMOOTH );
			float aveY =  ((previousEnd.getY()-previousStart.getY())/PREDICT_VELOCITY_SMOOTH );
			if(confLoader.getEnableSmoothVelocity() == 0)
			{
				if(aveX > 0)
					aveX = (confLoader.getConstantVelocity())[0];
				else if (aveX < 0)
					aveX = -(confLoader.getConstantVelocity())[0];

				if(aveY > 0)
					aveY = (confLoader.getConstantVelocity())[1];
				else if (aveY < 0)
					aveY = -(confLoader.getConstantVelocity())[1];
			}

			_target[targeti].shift((int) aveX, (int) aveY);
		}
		else if (_target[targeti].getTrajectoryLength() >= 2)
		{
			XYpair<float> previousEnd = _target[targeti].getTrajectory(_target[targeti].getTrajectoryLength() - 1);
			XYpair<float> previousStart = _target[targeti].getTrajectory(0);
			float aveX =  ((previousEnd.getX()-previousStart.getX())/_target[targeti].getTrajectoryLength() );
			float aveY =  ((previousEnd.getY()-previousStart.getY())/_target[targeti].getTrajectoryLength() );
			if(confLoader.getEnableSmoothVelocity() == 0)
			{
				if(aveX > 0)
					aveX = (confLoader.getConstantVelocity())[0];
				else if (aveX < 0)
					aveX = -(confLoader.getConstantVelocity())[0];

				if(aveY > 0)
					aveY = (confLoader.getConstantVelocity())[1];
				else if (aveY < 0)
					aveY = -(confLoader.getConstantVelocity())[1];
			}
			_target[targeti].shift((int) aveX, (int) aveY);
		}
	}
}

//*************************************************************
//	Method:updateModel
//
//	Description: update the model
//
//*************************************************************
void MultipleKernelTracking::updateModel(IplImage* useImg)
{
	for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)
	{
		CvMat* candidateHist = constructHistogram(useImg, _target[targeti].getMask(), _target[targeti].getMaxXY(), _target[targeti].getMinXY(), _target[targeti].getPos(), 1);
		float tracksimi = calculateSimi(_target[targeti].getHist(), candidateHist, WEIGHT_TYPE);
		_target[targeti].setSimi(tracksimi);

		if (tracksimi>=MIN_UPDATE_MODEL)					// update the model template
		{
			_target[targeti].setHist(candidateHist);
		}
		cvReleaseMat(&candidateHist);
	}
}

//*************************************************************
//	Method:calculateDeltaB
//
//	Description: calculate deltaXb
//
//*************************************************************
void MultipleKernelTracking::calculateDeltaB(CvMat* deltaB)
{
    if (_constraintList.size() > 0)
    {
        CvMat* Cx = computeConstraintGradient();
        CvMat* invCov = cvCreateMat(_constraintList.size(), _constraintList.size(), CV_32FC1);
        CvMat* C = computeConstraint();
        CvMat* temp1 = cvCreateMat(NSTATE*_kernelNumber, _constraintList.size(), CV_32FC1);

        cvMulTransposed(Cx, invCov, 1);
        if (cvDet(invCov) == 0)
        {
            cvZero(invCov);
        }
        else
        {
            cvInvert(invCov, invCov, CV_SVD);						// invCov = (Cx'*Cx)^-1
        }
        cvMatMul(Cx, invCov, temp1);
        cvMatMul(temp1, C, deltaB);
        cvConvertScale(deltaB, deltaB, -1);			// deltaB = -CxMC

        cvReleaseMat(&temp1);
        cvReleaseMat(&C);
        cvReleaseMat(&invCov);
        cvReleaseMat(&Cx);
    }
    else
    {
        cvZero(deltaB);
    }

}

//*************************************************************
//	Method:calculateDeltaA
//
//	Description: calculate deltaXa
//
//*************************************************************
void MultipleKernelTracking::calculateDeltaA(CvMat* deltaA, IplImage* useImg, float scale)
{
	CvMat* Jx = cvCreateMat(NSTATE*_kernelNumber, 1, CV_32FC1);

	int* histStep = new int[confLoader.getImageChannelNumber()];
	if(confLoader.getColorModel() == HSV)			// HSV
	{
		if(confLoader.getImageChannelNumber() == 3)
		{
			histStep[0] = 181/confLoader.getBinInOneChannel();		// H
			histStep[1] = 256/confLoader.getBinInOneChannel();		// S
			histStep[2] = 256/confLoader.getBinInOneChannel();		// V
		}
		else if(confLoader.getImageChannelNumber() == 1)
		{
			histStep[0] = 181/confLoader.getBinInOneChannel();
		}
	}
	else if(confLoader.getColorModel() == BGR)				// BGR
	{
		if(confLoader.getImageChannelNumber() == 3)
		{
			histStep[0] = 256/confLoader.getBinInOneChannel();
			histStep[1] = 256/confLoader.getBinInOneChannel();
			histStep[2] = 256/confLoader.getBinInOneChannel();
		}
		else if(confLoader.getImageChannelNumber() == 1)
		{
			histStep[0] = 256/confLoader.getBinInOneChannel();
		}
	}

	// calculate the Jx
	for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)
	{

		CvMat* candidateHist = constructHistogram(useImg, _target[targeti].getMask(), _target[targeti].getMaxXY(), _target[targeti].getMinXY(), _target[targeti].getPos(), scale);

		float normalizationFactor = 0;

		CvMat* newState = cvCreateMat(NSTATE, 1, CV_32FC1);
		CvMat* tempState = cvCreateMat(NSTATE, 1, CV_32FC1);
		CvMat* currentState = cvCreateMat(NSTATE, 1, CV_32FC1);
		CvMat* shiftVector = cvCreateMat(NSTATE, 1, CV_32FC1);

		cvZero(newState);
		cvZero(tempState);

		currentState->data.fl[0] = _target[targeti].getPosX();
		currentState->data.fl[1] = _target[targeti].getPosY();
		currentState->data.fl[2] = 1.0f;

		int nCount = 0;

		float summationKi = 0;
		float summationGvh2 = 0;
		float summationWk = 0;
		float summationWgvh2 = 0;
		float derivativeOfDensityEstimator = 0;			// derivative of f(h)

		for (int s = _target[targeti].getMinX(); s < _target[targeti].getMaxX() + 1; s++)
		{
			for (int r = _target[targeti].getMinY(); r < _target[targeti].getMaxY() + 1; r++)
			{
				if(_target[targeti].getMaskvalue(r,s) != 0)
				{
					float meanShiftWeight = 0;				// weight value in mean shift vector computation
					if( confLoader.getHistogramChannelNumber() != 1  || confLoader.getImageChannelNumber() != 3 )
					{
						for(int ch = 0; ch<confLoader.getHistogramChannelNumber(); ch++)
						{
							int binIndex = ((uchar *) (useImg->imageData+r*useImg->widthStep))[confLoader.getHistogramChannelNumber()*s+ch]/histStep[ch];
							if (binIndex >= confLoader.getBinInOneChannel())
								binIndex = confLoader.getBinInOneChannel() - 1;

							meanShiftWeight = meanShiftWeight + weightFunction(_target[targeti].getHist(), candidateHist, binIndex, ch, WEIGHT_TYPE);
						}
					}
					else
					{
						int individualIndex;
						int binIndex = ((uchar *) (useImg->imageData+r*useImg->widthStep))[3*s]/histStep[0];
						if (binIndex >= confLoader.getBinInOneChannel())
							binIndex = confLoader.getBinInOneChannel() - 1;

						individualIndex = ((uchar *) (useImg->imageData+r*useImg->widthStep))[3*s+1]/histStep[1];
						if (individualIndex >= confLoader.getBinInOneChannel())
							individualIndex = confLoader.getBinInOneChannel() - 1;

						binIndex = binIndex + individualIndex*confLoader.getBinInOneChannel();

						individualIndex = ((uchar *) (useImg->imageData+r*useImg->widthStep))[3*s+2]/histStep[2];
						if (individualIndex >= confLoader.getBinInOneChannel())
							individualIndex = confLoader.getBinInOneChannel() - 1;

						binIndex = binIndex + individualIndex*confLoader.getBinInOneChannel()*confLoader.getBinInOneChannel();

						meanShiftWeight = weightFunction(_target[targeti].getHist(), candidateHist, binIndex, 0, WEIGHT_TYPE);
					}

					float gi = -kernelDerivative(_target[targeti].getPos(), s, r, KERNEL_TYPE, _target[targeti].getWid(), _target[targeti].getHei());		// g = -k'
					tempState->data.fl[0] = s;
					tempState->data.fl[1] = r;
					cvScaleAdd(tempState, cvScalar(meanShiftWeight*gi), newState, newState);

					float ki = calculateWeight(_target[targeti].getPos(), s, r, KERNEL_TYPE, _target[targeti].getWid(), _target[targeti].getHei(), 1);

					summationKi = summationKi + ki;							// k
					summationGvh2 = summationGvh2 + (gi)*((_target[targeti].getPosX()-s)*(_target[targeti].getPosX()-s) + (_target[targeti].getPosY()-r)*(_target[targeti].getPosY()-r));		//g*v*h*h
					summationWk = summationWk + meanShiftWeight*ki;						// w*k
					summationWgvh2 = summationWgvh2 + meanShiftWeight*(gi)*((_target[targeti].getPosX()-s)*(_target[targeti].getPosX()-s) + (_target[targeti].getPosY()-r)*(_target[targeti].getPosY()-r));		//w*g*v*h*h

					normalizationFactor = normalizationFactor + meanShiftWeight*gi;

					nCount = nCount + 1;
				}
			}

		}
		float h = (sqrt(float (_target[targeti].getWid()*_target[targeti].getWid()+_target[targeti].getHei()*_target[targeti].getHei())) )/2;		// for scale change

		cvConvertScale(newState, newState, 1.0/normalizationFactor);
		cvSub(currentState, newState, shiftVector);

		float curSimi = calculateSimi(_target[targeti].getHist(), candidateHist, WEIGHT_TYPE);
		_target[targeti].setSimi(curSimi);

		float scaleChangeRatio;
		float beta = 9000.0f;
		derivativeOfDensityEstimator = -2*summationGvh2*summationWk/(pow(h,3)*summationKi*summationKi) + 2*summationWgvh2/(pow(h,3)*summationKi);	//	if derivativeOfDensityEstimator < 0, it	means the scale decreases
		scaleChangeRatio = -beta*summationKi*derivativeOfDensityEstimator/(summationWk*h);

		Jx->data.fl[(NSTATE*targeti)*Jx->cols] = shiftVector->data.fl[0]*1;
		Jx->data.fl[(NSTATE*targeti+1)*Jx->cols] = shiftVector->data.fl[1]*1;
		Jx->data.fl[(NSTATE*targeti+2)*Jx->cols] = scaleChangeRatio;

		cvReleaseMat(&shiftVector);
		cvReleaseMat(&currentState);
		cvReleaseMat(&tempState);
		cvReleaseMat(&newState);
		cvReleaseMat(&candidateHist);
	}																								// calculate the Jx
	if (_target.size() > 1)
		adaptiveWeight(Jx);

	if (_constraintList.size() > 0)
	{
		CvMat* Cx = computeConstraintGradient();
		CvMat* invCov = cvCreateMat(_constraintList.size(), _constraintList.size(), CV_32FC1);
		CvMat* CxT = cvCreateMat(_constraintList.size(), NSTATE*_kernelNumber, CV_32FC1);
		CvMat* temp1 = cvCreateMat(NSTATE*_kernelNumber, NSTATE*_kernelNumber, CV_32FC1);
		CvMat* temp2 = cvCreateMat(NSTATE*_kernelNumber, _constraintList.size(), CV_32FC1);
		CvMat* temp3 = cvCreateMat(NSTATE*_kernelNumber, 1, CV_32FC1);

		cvMulTransposed(Cx, invCov, 1);				// invCov = Cx'Cx
		if (cvDet(invCov) == 0)
		{
			cvZero(invCov);
		}
		else
		{
			cvInvert(invCov, invCov, CV_SVD);						// invCov = (Cx'*Cx)^-1
		}
		cvTranspose(Cx, CxT);
		cvMatMul(Cx, invCov, temp2);
		cvMatMul(temp2, CxT, temp1);
		cvMatMul(temp1, Jx, temp3);
		cvSub(temp3, Jx, deltaA);
		cvConvertScale(deltaA, deltaA, LEARNING_RATE);			// deltaA = a*(-I+CxMCx')Jx

		cvReleaseMat(&temp3);
		cvReleaseMat(&temp2);
		cvReleaseMat(&temp1);
		cvReleaseMat(&CxT);
		cvReleaseMat(&invCov);
		cvReleaseMat(&Cx);
	}
	else
	{
		cvConvertScale(Jx, deltaA, -1*LEARNING_RATE);
	}

	cvReleaseMat(&Jx);
}

//*************************************************************
//	Method:updateState
//
//	Description: update the state vector
//
//*************************************************************
void MultipleKernelTracking::updateState(CvMat* deltaX)
{
	for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)
	{
		int shiftX, shiftY;
		float dx = deltaX->data.fl[(NSTATE*targeti)*deltaX->cols];
        //std::cerr << dx << " ";
		if (dx >= 0)
			shiftX = int (dx + 0.5f);
		else
			shiftX = int (dx - 0.5f);

		float dy = deltaX->data.fl[(NSTATE*targeti+1)*deltaX->cols];
        //std::cerr << dy << "\n";
		if (dy >= 0)
			shiftY = int (dy + 0.5f);
		else
			shiftY = int (dy - 0.5f);

		if (shiftX != 0 || shiftY != 0)
		{
			_target[targeti].shift(shiftX, shiftY);
		}
	}

    /*for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)					// set the IterationTrajectory
	{
		_target[targeti].setIterationTrajectory(_target[targeti].getPos());
    }*/
}

//*************************************************************
//	Method:scaleChange
//
//	Description: apply scale change
//
//*************************************************************
void MultipleKernelTracking::scaleChange(float s)
{
	for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)
	{
        _target[targeti].changeObjectScale(s, confLoader);
	}

	for (unsigned int constrainti = 0; constrainti < _constraintList.size(); ++constrainti)
	{
		float smoothedScaleChange = confLoader.getScaleSmooth()*1+(1-confLoader.getScaleSmooth())*s;
		_constraintList[constrainti].setOriDistance(smoothedScaleChange*_constraintList[constrainti].getOriDistance());
	}
}

//*************************************************************
//	Method:drawBox
//
//	Description: visualization; draw bounding box
//
//*************************************************************
void MultipleKernelTracking::drawBox(IplImage* img)
{
    int minX = 10000;
	int minY = 10000;
	int maxX = -100;
	int maxY = -100;
	for (unsigned int k = 0; k < _target.size(); ++k)
	{
		if (_target[k].getMinX()<minX)
			minX = _target[k].getMinX();

		if (_target[k].getMinY()<minY)
			minY = _target[k].getMinY();

		if (_target[k].getMaxX()>maxX)
			maxX = _target[k].getMaxX();

		if (_target[k].getMaxY()>maxY)
			maxY = _target[k].getMaxY();
        cvRectangle(img, cvPoint(_target[k].getMinX(), _target[k].getMinY()), cvPoint(_target[k].getMaxX(), _target[k].getMaxY()), CV_RGB(255,0,0), 2);

    }
	//cvEllipse(img, cvPoint((maxX+minX)/2, (maxY+minY)/2 ), cvSize((maxX-minX)/2, (maxY-minY)/2), 0, 0, 360, CV_RGB(255,0,0), 2);
    //cvRectangle(img, cvPoint(minX, minY), cvPoint(maxX, maxY), CV_RGB(0,255,0), 2);
}

//*************************************************************
//	Method:GetObjRect
//
//	Description: Get the tracking object bounding box
//
//*************************************************************
CvRect MultipleKernelTracking::GetObjRect()
{
	int minX = 10000;
	int minY = 10000;
	int maxX = -100;
	int maxY = -100;
	for (unsigned int k = 0; k < _target.size(); ++k)
	{
		if (_target[k].getMinX()<minX)
			minX = _target[k].getMinX();

		if (_target[k].getMinY()<minY)
			minY = _target[k].getMinY();

		if (_target[k].getMaxX()>maxX)
			maxX = _target[k].getMaxX();

		if (_target[k].getMaxY()>maxY)
			maxY = _target[k].getMaxY();
    }
    return cvRect(minX, minY, maxX-minX, maxY-minY);
}

//*************************************************************
//	Method:outputCurrentState
//
//	Description: for debugging; output intermediate steps
//
//*************************************************************
void MultipleKernelTracking::outputCurrentState(IplImage* frame, std::string filename)
{
	IplImage* imgdA = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);
	cvCopy(frame, imgdA);
	drawBox(imgdA);
	cvSaveImage(filename.c_str(), imgdA);
	cvReleaseImage(&imgdA);
}

//*************************************************************
//	Method:tracking
//
//	Description: tracking
//
//*************************************************************
void MultipleKernelTracking::tracking(IplImage* frame)
{

	IplImage* useImg = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);
	constructImagePlane(frame, useImg, confLoader.getColorModel());

	CvMat* deltaB = cvCreateMat(NSTATE*_kernelNumber, 1, CV_32FC1);
	CvMat* deltaA = cvCreateMat(NSTATE*_kernelNumber, 1, CV_32FC1);

    /*for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)					// set the iterationtrajectory
	{
		_target[targeti].clearIterationTrajectory();
		_target[targeti].setIterationTrajectory(_target[targeti].getPos());
    }*/
	for (int i = 0; i < confLoader.getMaxIteration(); ++i)
	{

#ifdef MKT_DEBUG
		printf("	iteration = %d\n", i);
#endif
		bool indexJ = checkJ(useImg);
		bool indexC = checkC();

		if (indexJ == true && indexC == true)
		{
			break;
		}
		else if (indexJ == false && indexC == true)
		{
            calculateDeltaA(deltaA, useImg, 1);
            //std::cerr << "update:A\n";
			updateState(deltaA);
		}
		else if (indexJ == true && indexC == false)
		{
            calculateDeltaB(deltaB);
            //std::cerr << "update:B\n";
			updateState(deltaB);
		}
		else
		{
            calculateDeltaA(deltaA, useImg, 1);
            //std::cerr << "update:A\n";
			updateState(deltaA);

			calculateDeltaB(deltaB);
            //std::cerr << "update:B\n";
			updateState(deltaB);
		}
	}

    /*float changeRatio = 10000;
	float minChange = 10000;
	int numberOfObservable = 0;
    for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)
	{
		float currentScaleChange = deltaA->data.fl[(NSTATE*targeti+2)*deltaA->cols];
		if (abs(currentScaleChange) < abs(minChange))
		{
			minChange = currentScaleChange;
		}
		if ( (_target[targeti].getSimi() > MIN_UPDATE_SCALE))
		{
			numberOfObservable++;
		}
	}
    changeRatio = 1 + 6*minChange;
	if ( confLoader.getScaleUpdate() == 1 && (abs(changeRatio-1) < MAX_SCALE_CHANGE) && numberOfObservable >= 1)
	{
		scaleChange(changeRatio);
    }*/

	updateModel(useImg);
    /*for (unsigned int targeti = 0; targeti < _target.size(); ++targeti)					// set the trajectory
	{
		_target[targeti].setTrajectory(_target[targeti].getPos());
    }*/

    //predictUpdate();				// use the predicted location as the new initial location for next frame

	cvReleaseMat(&deltaB);
	cvReleaseMat(&deltaA);
	cvReleaseImage(&useImg);
}
