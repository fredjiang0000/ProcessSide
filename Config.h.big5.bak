/*
For more information, please refer to:

Chun-Te Chu <ctchu@u.washington.edu>
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <ctype.h>
#include <iostream>
#include <fstream>
#include <string.h>

enum ColorSpace
{
	GREY,
	HSV,
	BGR
};

class Config
{
public:
	Config();
	~Config();
	bool loadConf();

	char* getDirectory()	{return Directory;}
	bool getOutputImage()		{return OutputImage;}
	int getInitBB()		{return InitBB; }
	int* getBB()		{return BB;}
	int getKernelNumber()		{return KernelNumber;}
	float getFraction1()		{return Fraction1;}
	float getFraction2()		{return Fraction2;}
	float getFraction3()		{return Fraction1;}
	float getFraction4()		{return Fraction2;}
	int getBinInOneChannel()		{return BinInOneChannel;}
	ColorSpace getColorModel()		{return ColorModel;}
	int getHistogramChannelNumber()		{return HistogramChannelNumber;}
	int getImageChannelNumber()		{return ImageChannelNumber;}
	float getConstraintThreshold()	{return ConstraintThreshold;}
	float getCostThreshold()		{return CostThreshold;}
	int getMaxIteration()			{return MaxIteration;}
	bool getScaleUpdate()	{return ScaleUpdate; }
	float getScaleSmooth()		{return ScaleSmooth;}
	int getEnableSmoothVelocity()	{return EnableSmoothVelocity;}
	int* getConstantVelocity()		{return ConstantVelocity;}

private:
	int ParametersNum;

	char* Directory;			// the directory of the input video
	bool OutputImage;			// output the result image
	int InitBB;					// initialization bounding box	0: mouse select each kernel, 1: coordinate,  2: mouse select whole object
	int* BB;					// initialization bounding box by coordinate  x y wid hei
	int KernelNumber;			// number of kernels
	float Fraction1;			// fraction of the kernel1_hei/obj_hei
	float Fraction2;			// fraction of the kernel2_hei/obj_hei
	float Fraction3;			// fraction of the kernel1_hei/obj_hei
	float Fraction4;			// fraction of the kernel2_hei/obj_hei
	int BinInOneChannel;		// number of bins in one channel
	ColorSpace ColorModel;		// color space
	int HistogramChannelNumber;			// number of total channels in the histogram
	int ImageChannelNumber;			// number of image channels to be used for histogram construction
	float ConstraintThreshold;	// the epsilonC values for constraints functions
	float CostThreshold;		// the epsilonJ values for cost function
	int MaxIteration;			// the maximum number of iteration allowed
	bool ScaleUpdate;			// 1: do the scale update,   0: do not do the scale update
	float ScaleSmooth;			// smooth factor for scaling update
	int EnableSmoothVelocity;	// 1: use average velocity for prediction; 0: use fixed amount of value as speed
	int* ConstantVelocity;		// fixed amount of velocity [x y]
};



#endif