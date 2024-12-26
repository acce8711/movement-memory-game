#pragma once
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "Config.h"

class ObjectTracker
{
public:
	ofxCvContourFinder contourFinder;
	ofxCvGrayscaleImage diffImage;

	int * threshold;
	int *  numContourConsidered;
	float * minArea;
	float * maxArea;
	bool detectionState;
	int prevArea;
	int prevXPos;
	int prevYPos;
	int objNum;
	float indicatorSize;
	int * depthThreshold;
	ofTrueTypeFont detectionText;
	string detectionLabel;
	ofColor detectionColour;
	ofVec2f detectionTextPos;

	ObjectTracker(int &threshold, int &numContourConsidered, float &minArea, float& maxArea, int objNum, ofVec2f detectionTextPos, int& depthThreshold);
	~ObjectTracker();

	void objectDetection();
	void virtual detectionInfo();
	void drawContour();
	bool contoursFound();
	int movementTracking();
	void indicatorSizeChange();
	float lerp(float start, float end, float percent);
};

class ColourObjectTracker : public ObjectTracker
{
public:
	float trackedColor[4];
	bool colorSelected;

	ColourObjectTracker(int& threshold, int& numContourConsidered, float& minArea, float& maxArea, int objNum, ofVec2f detectionTextPos, int& depthThreshold);
	~ColourObjectTracker();

	void processColour(ofxCvColorImage colorImage);
	void updateColour(float* colour);
	//void detectionInfo();

};


class BgObjectTracker : public ObjectTracker
{
public:
	BgObjectTracker();
	~BgObjectTracker();
	void processBackground();
};