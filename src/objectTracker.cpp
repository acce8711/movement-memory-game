#include "objectTracker.h"


ObjectTracker::ObjectTracker(int& threshold, int& numContourConsidered, float& minArea, float& maxArea, int objNum, ofVec2f detectionTextPos, int& depthThreshold)
{
	diffImage.allocate(Config::VIDEO_WIDTH, Config::VIDEO_HEIGHT);
	detectionState = false;
	prevArea = 0;
	prevXPos = 0;
	prevYPos = 0;
	indicatorSize = 0;
	detectionText.load(Config::FONT, Config::SMALL_FONT);
	detectionLabel = "Object " + ofToString(objNum) + " not detected";
	detectionColour = Config::DETECTION_TEXT_LOST;
	this->depthThreshold = &depthThreshold;
	this->detectionTextPos = detectionTextPos;
	this->objNum = objNum;
	this->threshold = &threshold;
	this->numContourConsidered = &numContourConsidered;
	this->minArea = &minArea;
	this->maxArea = &maxArea;
}

ObjectTracker::~ObjectTracker()
{

}



void ObjectTracker::objectDetection()
{
	//if object is no longer on screen
	if (contourFinder.nBlobs == 0 && detectionState)
	{
		detectionState = false;	
		detectionLabel = "Object " + ofToString(objNum) + " not detected";
		detectionColour = Config::DETECTION_TEXT_LOST;
	}
	//if object has entered the screen
	if (contourFinder.nBlobs == 1 && !detectionState)
	{
		detectionState = true;
		detectionLabel =  "Object " + ofToString(objNum) + " detected";
		detectionColour = Config::DETECTION_TEXT_FOUND;
	}

}

void ObjectTracker::detectionInfo()
{
	ofPushMatrix();
		ofSetColor(0, 0, 0, 100);
		ofDrawRectangle(detectionTextPos.x+90, detectionTextPos.y-5, 200, 20);
	ofPopMatrix();
	ofPushMatrix();
		ofSetColor(detectionColour);
		detectionText.drawString(detectionLabel, detectionTextPos.x, detectionTextPos.y);
	ofPopMatrix();
}

void ObjectTracker::drawContour()
{
	//step 3, draw the contours
	static ofVec2f contourCenter; //re-use vars for each contour in loop
	static float contourArea = 0.0f;

	ofPushMatrix();
	//so all contours are drawn to the right of the two videos below
	//ofTranslate(Config::VIDEO_BORDER_SIZE, Config::VIDEO_HEIGHT + Config::VIDEO_BORDER_SIZE);
	ofSetRectMode(OF_RECTMODE_CORNER);
	for (int i = 0; i < contourFinder.blobs.size(); i++) {
		contourCenter.set(contourFinder.blobs[i].boundingRect.getCenter().x, contourFinder.blobs[i].boundingRect.getCenter().y);
		contourArea = contourFinder.blobs[i].area;

		//draw contour
		contourFinder.blobs[i].draw();

		//draw centre
		ofSetColor(ofColor::coral);
		ofDrawCircle(contourCenter.x, contourCenter.y, 10.0f);

		//draw textual info
		ofSetColor(ofColor::green);
		ofDrawBitmapString("Centre" + ofToString(contourCenter.x) + ", " + ofToString(contourCenter.y),
			contourFinder.blobs[i].boundingRect.getMaxX() + Config::VIDEO_BORDER_SIZE,
			contourCenter.y);
		ofDrawBitmapString("Area" + ofToString(contourArea),
			contourFinder.blobs[i].boundingRect.getMaxX() + Config::VIDEO_BORDER_SIZE,
			contourCenter.y + 20.0f);
	}
	ofSetRectMode(OF_RECTMODE_CENTER);
	ofPopMatrix();
}


bool ObjectTracker::contoursFound()
{
	return contourFinder.nBlobs;
}


int ObjectTracker::movementTracking()
{
	int movementDetected = -1;
	//making sure that there are contours present to track the object
	if (contoursFound())
	{
		
		//if depth change has been detected
		if (abs(prevArea - contourFinder.blobs[0].boundingRect.getArea()) > *depthThreshold)
		{
			
			//object is closer to camera -> forward movement
			if (prevArea < contourFinder.blobs[0].boundingRect.getArea())
			{
				movementDetected = 2;
			}
				
			//object is further from camera -> back movement
			else
				movementDetected = 3;
			prevArea = contourFinder.blobs[0].boundingRect.getArea();
		}

		//if object has moved horizontally
		else if (abs(prevXPos - contourFinder.blobs[0].boundingRect.getCenter().x) > Config::VIDEO_WIDTH / 7)
		{
			if (abs(Config::VIDEO_WIDTH / 2 - contourFinder.blobs[0].boundingRect.getCenter().x) < abs(Config::VIDEO_WIDTH / 2 - prevXPos))
				movementDetected = 1;
			//object has moved away from the center -> out movement
			else
				movementDetected = 0;

			prevXPos = contourFinder.blobs[0].boundingRect.getCenter().x;
		}
	}
	return movementDetected;
}

void ObjectTracker::indicatorSizeChange()
{
	if (contoursFound())
		indicatorSize = lerp(indicatorSize, contourFinder.blobs[0].boundingRect.getWidth() / 2, 0.5f);
}

float ObjectTracker::lerp(float start, float end, float percent)
{
	return (start + percent * (end - start));
}

//colour tracking object
ColourObjectTracker::ColourObjectTracker(int& threshold, int& numContourConsidered, float& minArea, float& maxArea, int objNum, ofVec2f detectionTextPos, int& depthThreshold) : ObjectTracker(threshold, numContourConsidered, minArea, maxArea, objNum, detectionTextPos, depthThreshold)
{
	trackedColor[0] = 0.0f;
	trackedColor[1] = 0.0f;
	trackedColor[2] = 0.0f;
	trackedColor[3] = 0.0f;
	colorSelected = false;
}

ColourObjectTracker::~ColourObjectTracker()
{

}


void ColourObjectTracker::processColour(ofxCvColorImage colorImage)
{
	//loop through all the pixels of color image and determine if each pixel is that colour (within threshold)
	//if color found, write a white pixel. if not, write a black pixels to our grayscale images

	if (colorSelected)
	{
		const int numChannels = Config::VIDEO_WIDTH * Config::VIDEO_HEIGHT * 3; //there are 3 channels per pixel in a color image (R,G,B)
		static float pixel[3] = { 0.0f, 0.0f, 0.0f };

		//loop through all channels (vut consdier how every channels in one pixel)
		for (int i = 0; i < numChannels; i += 3)
		{
			//define our current "pixel/color"
			pixel[0] = colorImage.getPixels()[i + 0]; //r
			pixel[1] = colorImage.getPixels()[i + 1]; //g
			pixel[2] = colorImage.getPixels()[i + 2]; //b

			//determine if the current pixel "matches" our tracked colour
			//FLT_EPSILON can sue as comparison instad of 0

			//checking if one of the selected colours have been detected
			if (
				(abs(pixel[0] - trackedColor[0] * 255.0f) < *threshold &&
					abs(pixel[1] - trackedColor[1] * 255.0f) < *threshold &&
					abs(pixel[2] - trackedColor[2] * 255.0f) < *threshold)
				)

			{
				//the same colour
				//greyscale only has one chanel not like rgb which has 3 
				diffImage.getPixels()[i / 3] = 255;
			}
			else
			{
				//not the same colour
				diffImage.getPixels()[i / 3] = 0;
			}


		}
		//update texture
		diffImage.flagImageChanged(); //ofxCv version of "ofImage::update()"

		//find contours now
		contourFinder.findContours(diffImage, *minArea, *maxArea, 1, false, true);
		
	}
	
}

void ColourObjectTracker::updateColour(float* colour)
{
	colorSelected = true;
	trackedColor[0] = colour[0];
	trackedColor[1] = colour[1];
	trackedColor[2] = colour[2];
	trackedColor[3] = colour[3];
}

/*
void ColourObjectTracker::detectionInfo()
{
	
	if (detectionState && colorSelected)
		ofSetColor(detectionColour);
		detectionText.drawString(detectionLabel, detectionTextPos.x, detectionTextPos.y);
	if (!detectionState && colorSelected)
		ofDrawBitmapString("Object " + ofToString(objNum) + " Un-detected", 100, 500);
}
*/


void BgObjectTracker::processBackground()
{

}
