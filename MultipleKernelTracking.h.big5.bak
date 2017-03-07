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

#ifndef _MULTIPLEKERNELTRACKING_H
#define _MULTIPLEKERNELTRACKING_H


#include <vector>
#include <iostream>
#include <stdio.h>
#include <math.h>

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include "Config.h"


#define UNSET_VALUE					-100	//for un-set value
#ifndef PI
    #define PI							3.1415926	//for pi
#endif
#define NSTATE						3	// for dimension of state for each kernel
#define NCONSTRAINT					2	// for number of constraint per pair of kernels
#define LEARNING_RATE				1	// for learning rate
#define KERNEL_TYPE					Epanechnikov	//for choosing the type of kernel			1: Gaussian		2: roof kernel		3: Epanechnikov
#define WEIGHT_TYPE					2	// for choosing the type of the weight used by mean shift vector calculation (depends on the similarity function): 1: correlation (original mean shift)		2: KL-distance		3:dual KL-distance
#define PREDICT_VELOCITY_SMOOTH		15			// smoothvelocity = (trajectory(now) - trajectory(now-PREDICT_VELOCITY_SMOOTH))/PREDICT_VELOCITY_SMOOTH;
#define MIN_UPDATE_MODEL				1			// the minimum simi for allowing update the template model
#define MIN_UPDATE_SCALE				0.3			// the minimum simi for allowing update the scale
#define MAX_SCALE_CHANGE				0.4			// the maximum allowed scale change portion
#define ADAPTIVE_WEIGHT_CHOICE		1			// 0: use fixed weight value		1: use similarity based weight

#define SIGN_CHECK(a,b)	(a>=b) ? 1 : 0


//===================================== XY pairs ==========================================
template <class T>
class XYpair
{
public:
	XYpair() {X = UNSET_VALUE; Y = UNSET_VALUE; }
	XYpair(T x, T y) {X = x; Y= y;}
	void setX(T x) {X = x; }
	void setY(T y) {Y = y; }
	void setXY(T x, T y) {X = x; Y = y; }
	void setXY(XYpair<T> xy) {X = xy.getX(); Y = xy.getY(); }

	T getX() {return X;}
	T getY() {return Y;}

	void shift(T deltaX, T deltaY)		{X += deltaX;	Y += deltaY;}  //shift point with (deltaX, deltaY)
	float distances(XYpair<T> xy) {return sqrt((float)((X-xy.getX())*(X-xy.getX()) + (Y-xy.getY())*(Y-xy.getY())) ); }
	float distances2(XYpair<T> xy) {return ((float)(X-xy.getX())*(X-xy.getX()) + (Y-xy.getY())*(Y-xy.getY()) ); }

private:
	T X;
	T Y;

};
//===================================== XY pairs ==========================================////


//===================================== ObjInfo ==========================================
class ObjInfo
{
public:
	ObjInfo();
	~ObjInfo();
	void initial(int frameWid, int frameHei, int histRow, int histCol);
	void setNode(XYpair<float> pos, XYpair<int> maxXY, XYpair<int> minXY, int area, CvMat* hist, int wid, int hei, CvMat *mask);

	XYpair<float> getPos() {return Pos; }
	XYpair<int> getMaxXY() {return MaxXY; }
	XYpair<int> getMinXY() {return MinXY; }
	float getPosX() {return Pos.getX(); }
	float getPosY() {return Pos.getY(); }
	int getMaxX() {return MaxXY.getX(); }
	int getMaxY() {return MaxXY.getY(); }
	int getMinX() {return MinXY.getX(); }
	int getMinY() {return MinXY.getY(); }
	int getArea() {return Area; }
	float getObjHalfWid() {return (MaxXY.getX() - MinXY.getX())/(float)2; }
	float getObjHalfHei() {return (MaxXY.getY() - MinXY.getY())/(float)2; }
	int getWid() {return Wid; }
	int getHei() {return Hei; }
	float getHistvalue (int index)	{return Hist->data.fl[index*Hist->cols]; }
	int getMaskvalue (int row, int col)	{return (int) Mask->data.fl[row*Mask->cols+col]; }
	CvMat* getHist()	{return Hist;}
	CvMat* getMask()	{return Mask;}
	float getSimi()	{return Simi;}
	float getScale()	{return Scale;}
	float getWeight()	{return Weight;}
	XYpair<float> getTrajectory(int t)	{return Trajectory[t];}
	int getTrajectoryLength()	{return Trajectory.size();}
	XYpair<float> getIterationTrajectory(int t)	{return IterationTrajectory[t];}
	int getIterationTrajectoryLength()	{return IterationTrajectory.size();}

	void setArea (int a)		{Area = a;}
	void setHist(CvMat* a)		{cvCopy(a, Hist);}
	void setMask(CvMat* a)		{cvCopy(a, Mask);}
	void setSimi(float a )		{Simi = a;}
	void setScale(float a )		{Scale = a;}
	void setWeight(float a )		{Weight = a;}
	void setTrajectory(XYpair<float> a )		{Trajectory.push_back(a);}
	void setIterationTrajectory(XYpair<float> a )		{IterationTrajectory.push_back(a);}

	void shift(int deltaX, int deltaY);
    void changeObjectScale(float, Config confLoader);
	void releaseHist()			{cvReleaseMat(&Hist);}
	void releaseMask()			{cvReleaseMat(&Mask);}
	void clearTrajectory()		{Trajectory.clear();}
	void clearIterationTrajectory()		{IterationTrajectory.clear();}
	void clearObjInfo();

private:
	XYpair<float> Pos;
	XYpair<int> MaxXY;
	XYpair<int> MinXY;
	int Area;
	int Wid;
	int Hei;
	CvMat* Hist;
	CvMat* Mask;
	float Simi;
	float Scale;
	float Weight;
	std::vector<XYpair <float> > Trajectory;
	std::vector<XYpair <float> > IterationTrajectory;
};

//===================================== ObjInfo ==========================================////



//===================================== Constraint ==========================================
class Constraint
{
public:
	enum ConstraintType
	{
		L2X,
		L2Y,
		L2XY
	};

	Constraint();
	~Constraint();

	void setKernel1(int x) {Kernel1 = x; }
	void setKernel2(int x) {Kernel2 = x; }
	void setTypeOfConstraint(ConstraintType x) {TypeOfConstraint = x; }
	void setOriDistance(float x) {OriDistance = x; }

	int getKernel1()	{return Kernel1;}
	int getKernel2()	{return Kernel2;}
	ConstraintType getTypeOfConstraint()	{return TypeOfConstraint;}
	float getOriDistance()	{return OriDistance;}

	float calculateConstraintCost(ObjInfo& a, ObjInfo& b);

private:
	int Kernel1;
	int Kernel2;
	ConstraintType TypeOfConstraint;				//	L2X, L2Y, L2XY
	float OriDistance;
};
//===================================== Constraint ==========================================////


//===================================== MultipleKernelTracking ==========================================

class MultipleKernelTracking
{
public:
    MultipleKernelTracking();
	MultipleKernelTracking(int kernelNumber);
	~MultipleKernelTracking();

	void manualInitializeMask(IplImage* frame, IplImage* mask);
	void autoInitializeMask(IplImage* frame, IplImage* mask);
	void autoInitializeCoordinate(IplImage* frame, int* coordinate);
	void setConstraintList();

	void tracking(IplImage* frame);
    CvRect GetObjRect();
    void drawBox(IplImage* frame);

	enum KernelType
	{
		Gaussian,
		Roof,
		Epanechnikov
	};

private:
	float calculateSimi(CvMat* targetmodel, CvMat* candidate, int choice);
	float calculateTotalSimi(IplImage* img, float s);
	float calculateCost(CvMat* targetmodel, CvMat* candidate, int choice);
	float calculateTotalCost(IplImage* img);

	void constructImagePlane(IplImage* img, IplImage* useImg, ColorSpace choice);
	CvMat* constructHistogram(IplImage *useImg, CvMat *mask, XYpair<int> maxXY, XYpair<int> minXY, XYpair<float> pos, float scale);

	float calculateWeight(XYpair<float> pos, int x, int y, KernelType kernelChoice, int wid, int hei, float s);					// calculate the weight used by histogram construction
	float weightFunction(CvMat* targetHist, CvMat* candidateHist, int bin, int channelIndex, int choice);				// calculate the weight used by mean shift vector calculation
	float kernelDerivative(XYpair<float> pos, int x, int y, KernelType type, int wid, int hei);

	CvMat* computeConstraintGradient();
	CvMat* computeConstraint();
	void adaptiveWeight(CvMat* Jx);

	bool checkJ(IplImage* useImg);
	bool checkC();
	void calculateDeltaB(CvMat* deltaB);
    void calculateDeltaA(CvMat* deltaA, IplImage* useImg, float scale);
	void updateState(CvMat* deltaX);
	void scaleChange(float s);
	void predictUpdate();
	void updateModel(IplImage* useImg);

	void outputCurrentState(IplImage* frame, std::string filename);


    Config confLoader;
	int _kernelNumber;
	std::vector<ObjInfo> _target;
	std::vector<Constraint>	_constraintList;
	std::vector<float> _thresholdConstraint;
};
//===================================== MultipleKernelTracking ==========================================////

#endif
