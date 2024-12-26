#pragma once

#include "ofMain.h"

#include "ofxOpenCv.h"
#include "ofxImGui.h"
#include "ObjectTracker.h"


#include "Config.h"


class ofApp : public ofBaseApp{

	public:

		struct ImageInfo
		{
			ofImage image;
			ofVec2f pos;
		};

		ofVideoGrabber m_videoCam;
		ofVideoPlayer m_videoPlayer;
		ofxCvContourFinder m_contourFinder;
		ofxCvColorImage m_colorImage;
		ofxCvGrayscaleImage m_diffImage;

		ColourObjectTracker* m_colourObj1;
		ColourObjectTracker* m_colourObj2;

		//GUI
		ofxImGui::Gui m_gui;
		int m_threshold;
		int m_numContourConsidered;
		float m_minArea;
		float m_maxArea;
		int m_detectionMode;
		bool m_debugMode;
		int m_videoType;

		//background subtraction vars
		bool m_captureBg;
		ofxCvGrayscaleImage	m_grayscaleImage;
		ofxCvGrayscaleImage m_grayscaleBg;

		//game vars
		int m_currPatternIndex;
		int m_currMovement;
		int m_currMovementIndex;
		ImageInfo m_instructionsVisual;
		ImageInfo m_obj1Visual;
		ImageInfo m_obj2Visual;
		bool m_showInstructionsVisual;
		bool m_showObj1Visual;
		bool m_showObj2Visual;
		int m_frameCounter;
		bool m_continueRound;

		bool m_obj1Guessed;
		bool m_obj2Guessed;

		bool m_movementMatchesObj1;
		bool m_movementMatchesObj2;

		ofTrueTypeFont m_currRoundText;

		int m_score;
		ofTrueTypeFont m_scoreText;

		ofImage m_backgroundImage;

		float m_lastRecordedTime;
		float m_currTime;

		bool m_gameComplete;

		ofSoundPlayer m_backgroundMusic;
		ofSoundPlayer m_correctSound;
		ofSoundPlayer m_incorrectSound;
		ofSoundPlayer m_collisionSound;
		ofSoundPlayer m_movementSound;

		//end game vars
		ofTrueTypeFont m_finalScoreText;
		string m_finalScoreLabel;

		//pattern instructions
		vector <ImageInfo> m_patternVisual;
		ofTrueTypeFont m_instructionMoveText;
		string m_instructionMoveLabel;

		int m_bgColourFrameCounter;

		int m_currGuessIndex;

		bool m_guessingMode;
		bool m_beginGuessing;
		bool m_guessed;

		bool m_collisionState;


		//tracking color vars
		float m_trackedColor1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float m_trackedColor2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		ofTrueTypeFont m_obj1Text;
		ofTrueTypeFont m_obj2Text;

		ofTrueTypeFont m_collisionText;
		string m_collisionLabel;

		int m_obj1Size;
		int m_obj2Size;

		int m_collisionThreshold;
		int m_depthThreshold;

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void mousePressed(int x, int y, int button);


		//custom functions
		void processBackground();
		void videoToImage(ofxCvColorImage tempImage);
		void newFrameActions();
		void debugMode();
		void patternDemo();
		void setDemoMovement();
		void guessingMode();
		void detectionInfo();
		void nextRound();
		bool colourCollision();
		bool collisionDetection(ofRectangle obj1, ofRectangle obj2);
		void diffImageColourGenerator();
		bool sameMovementCheck(int objMovement);
		void movementImages(int objMovement, string objName);
		void imageSet(ofImage* imageHolder, string imageToLoad);
		void toggleDebug();
		void endRound();
		void looseScreen();
		void gameEnd();
		void adjustBallSize(string objName);
		void resetGameplay();
		void pauseVideo();
		void resetVideo();
		void guiUI();
		void debugUI();
		void gameUI();
		
		
};
