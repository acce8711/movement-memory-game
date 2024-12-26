#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowShape(Config::APP_WINDOW_WIDTH, Config::APP_WINDOW_HEIGHT);
	ofSetFrameRate(Config::APP_DESIRED_FRAMERATE);
	ofSetRectMode(OF_RECTMODE_CENTER);

	//load video
	
	m_videoPlayer.load(Config::VIDEO_PATH_COLOR);
	m_videoPlayer.setLoopState(OF_LOOP_NORMAL);
	m_videoPlayer.play();
	//setup live camera
	m_videoCam.setVerbose(true);
	m_videoCam.setup(Config::VIDEO_WIDTH, Config::VIDEO_HEIGHT);
	

	//m_videoPlayer.setLoopState(OF_LOOP_NORMAL);

	//allocate CV images to create space for it so it's just not garbage and not random in memory
	m_colorImage.allocate(Config::VIDEO_WIDTH, Config::VIDEO_HEIGHT);
	m_diffImage.allocate(Config::VIDEO_WIDTH, Config::VIDEO_HEIGHT);
	m_diffImage.set(0);
	m_grayscaleImage.allocate(Config::VIDEO_WIDTH, Config::VIDEO_HEIGHT);
	m_grayscaleBg.allocate(Config::VIDEO_WIDTH, Config::VIDEO_HEIGHT);

	//init vars
	m_threshold = 30;
	m_numContourConsidered = 1;
	m_minArea = 20.0f;
	m_maxArea = 300000.0f;
	m_collisionThreshold = 40.0f;
	m_depthThreshold = 9000;
	m_videoType = 0;
	m_backgroundMusic.load(Config::SOUND_PATH_BACKGROUND);
	m_backgroundMusic.setLoop(true);
	m_backgroundMusic.play();
	m_backgroundMusic.setPaused(true);
	m_correctSound.load(Config::SOUND_PATH_CORRECT);
	m_incorrectSound.load(Config::SOUND_PATH_INCORRECT);
	m_collisionSound.load(Config::SOUND_PATH_COLLISION);
	m_movementSound.load(Config::SOUND_PATH_MOVEMENT);

	//init gui vars
	m_detectionMode = 0;
	m_debugMode = true;
	m_frameCounter = 0;

	//bg subtraction inits
	m_captureBg = true;
	m_colourObj1 = new ColourObjectTracker(m_threshold, m_numContourConsidered, m_minArea, m_maxArea, 1, Config::DETECTION_DEBUG_TEXT_POS, m_depthThreshold);
	m_colourObj2 = new ColourObjectTracker(m_threshold, m_numContourConsidered, m_minArea, m_maxArea, 2, Config::DETECTION_DEBUG_TEXT2_POS, m_depthThreshold);

	m_colourObj1->movementTracking();
	m_colourObj2->movementTracking();

	//init game vars
	m_currMovementIndex = 0;
	m_currPatternIndex = 0;
	m_currMovement = Config::LEVEL_PATTERNS[m_currPatternIndex][m_currMovementIndex];
	m_guessingMode = false;
	m_currGuessIndex = 0;
	m_bgColourFrameCounter = 0;
	m_continueRound = false;
	m_beginGuessing = true;

	m_obj1Guessed = false;
	m_obj2Guessed = false;

	m_movementMatchesObj1 = false;
	m_movementMatchesObj2 = false;

	m_gameComplete = false;

	m_currRoundText.load(Config::FONT, Config::LARGE_FONT);

	m_score = 0;
	m_scoreText.load(Config::FONT, Config::LARGE_FONT);

	m_backgroundImage.load(Config::GAME_BACKGROUND);

	m_collisionState = false;

	//game over vars
	m_finalScoreText.load(Config::FONT, Config::LARGE_FONT);
	m_finalScoreLabel = "";

	//init images
	m_instructionsVisual.pos = { Config::APP_WINDOW_WIDTH/2, Config::APP_WINDOW_HEIGHT/2 };
	m_instructionsVisual.image.allocate(Config::INSTRUCTION_IMAGE_WIDTH, Config::INSTRUCTION_IMAGE_HEIGHT, OF_IMAGE_COLOR);

	m_obj1Visual.pos = { 0,0 };
	m_obj1Visual.image.allocate(Config::MOVEMENT_IMAGE_SIZE, Config::MOVEMENT_IMAGE_SIZE, OF_IMAGE_COLOR);
	m_obj2Visual.pos = { 0,0 };
	m_obj2Visual.image.allocate(Config::MOVEMENT_IMAGE_SIZE, Config::MOVEMENT_IMAGE_SIZE, OF_IMAGE_COLOR);

	m_showInstructionsVisual = false;
	m_showInstructionsVisual = false;
	m_showInstructionsVisual = false;

	m_instructionMoveText.load(Config::FONT, Config::LARGE_FONT);
	m_instructionMoveLabel = " ";

	m_collisionText.load(Config::FONT, Config::SMALL_FONT);
	m_collisionLabel = "";

	//init gui
	m_gui.setup();
	m_gui.addFont(Config::FONT, 14, nullptr);
}

//--------------------------------------------------------------
void ofApp::update() {
	//if webcam is being used
	if (m_videoType == 0)
	{
		m_videoCam.update();
		//video may have a diff frmewrate than program
		if (m_videoCam.isFrameNew())
		{
			ofxCvColorImage tempImage;
			tempImage.setFromPixels(m_videoCam.getPixels()); //copy video frame pixel data to an image
			videoToImage(tempImage); //converting the temp image into the colour image
			newFrameActions(); //doing collision detection, movement detection, etc. fo the tracked objects
		}
	}
	//if recorded video is being used
	else
	{
		m_videoPlayer.update();

		//video may have a diff frmewrate than program
		if (m_videoPlayer.isFrameNew())
		{
			ofxCvColorImage tempImage;
			tempImage.setFromPixels(m_videoPlayer.getPixels()); //copy video frame pixel data to an image
			videoToImage(tempImage);//converting the temp image into the colour image
			newFrameActions(); //doing collision detection, movement detection, etc. fo the tracked objects
		}
	}

	//if the program is in debug mode, tracking the object movements and updating their visuals
	if (m_debugMode)
		debugMode();

	//if playing game
	else
	{
		//disaplying the patterns to remember
		patternDemo();
		//user will guess the pattern
		guessingMode();
		//user will move onto the next round
		nextRound();
	}	
}

//--------------------------------------------------------------
void ofApp::draw(){
	
	ofBackground(Config::GAME_BACKGROUND_COLOUR);
	
	//displaying the gui
	guiUI();

	//displaying the debug visuals
	if (m_debugMode)
		debugUI();
	//displaying the game UI
	else
		gameUI();


	//drawing object detection info
	m_colourObj1->detectionInfo();
	m_colourObj2->detectionInfo();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	//add some video controls. E.g., a pause toggle
	//or key == 32
	if (key == ' ' && m_videoType == 1)
	{
		m_videoPlayer.setPaused(!m_videoPlayer.isPaused());
	}
	if (key == 49 || key == 50)
	{
		//need to convert from screen space coordinates  to video-based corrdinates
		//so we don't set an invalid pixel coordinate when clicking outside th video

		//making a ofrectangle that represet the position and size of our colour video (top-left corner)
		ofRectangle videoRect = ofRectangle(0,0,
			Config::VIDEO_WIDTH,
			Config::VIDEO_HEIGHT);
		//step 1 - clamp
		int converScreeToVideoSpace_X = ofClamp(ofGetMouseX(), videoRect.getMinX(), videoRect.getMaxX());
		int converScreeToVideoSpace_Y = ofClamp(ofGetMouseY(), videoRect.getMinY(), videoRect.getMaxY());

		//steps 2 - shift to image space
		converScreeToVideoSpace_X -= videoRect.getMinX();
		converScreeToVideoSpace_Y -= videoRect.getMinY();

		//step 3, find that pixel
		//use function: pixelIndex = x+y * width;
		const int index = (converScreeToVideoSpace_X + converScreeToVideoSpace_Y * Config::VIDEO_WIDTH) * m_colorImage.getPixels().getBytesPerPixel();
		if (key == 49)
		{
			m_trackedColor1[0] = m_colorImage.getPixels()[index + 0] / 255.0f;    //r, and convert to float colour 
			m_trackedColor1[1] = m_colorImage.getPixels()[index + 1] / 255.0f;    //g
			m_trackedColor1[2] = m_colorImage.getPixels()[index + 2] / 255.0f;    //b
			m_colourObj1->updateColour(m_trackedColor1);
		}
		if (key == 50)
		{
			m_trackedColor2[0] = m_colorImage.getPixels()[index + 0] / 255.0f;    //r, and convert to float colour 
			m_trackedColor2[1] = m_colorImage.getPixels()[index + 1] / 255.0f;    //g
			m_trackedColor2[2] = m_colorImage.getPixels()[index + 2] / 255.0f;    //b
			m_colourObj2->updateColour(m_trackedColor2);
		}
	}
}



//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	//we have to set the tracked colour to our selected colour (with mouse)
	if (button == OF_MOUSE_BUTTON_MIDDLE || button == OF_MOUSE_BUTTON_RIGHT)
	{
		//need to convert from screen space coordinates  to video-based corrdinates
		//so we don't set an invalid pixel coordinate when clicking outside th video

		//making a ofrectangle that represet the position and size of our colour video (top-left corner)
		ofRectangle videoRect = ofRectangle(0, 0,
											Config::VIDEO_WIDTH, 
											Config::VIDEO_HEIGHT);
		//step 1 - clamp
		int converScreeToVideoSpace_X = ofClamp(x, videoRect.getMinX(), videoRect.getMaxX());
		int converScreeToVideoSpace_Y = ofClamp(y, videoRect.getMinY(), videoRect.getMaxY());

		//steps 2 - shift to image space
		converScreeToVideoSpace_X -= videoRect.getMinX();
		converScreeToVideoSpace_Y -= videoRect.getMinY();

		//step 3, find that pixel
		//use function: pixelIndex = x+y * width;
		const int index = (converScreeToVideoSpace_X + converScreeToVideoSpace_Y * Config::VIDEO_WIDTH) * m_colorImage.getPixels().getBytesPerPixel();
		if (button == OF_MOUSE_BUTTON_RIGHT)
		{
			m_trackedColor1[0] = m_colorImage.getPixels()[index + 0] / 255.0f;    //r, and convert to float colour 
			m_trackedColor1[1] = m_colorImage.getPixels()[index + 1] / 255.0f;    //g
			m_trackedColor1[2] = m_colorImage.getPixels()[index + 2] / 255.0f;    //b
			m_colourObj1->updateColour(m_trackedColor1);
		}
		if (button == OF_MOUSE_BUTTON_MIDDLE)
		{
			m_trackedColor2[0] = m_colorImage.getPixels()[index + 0] / 255.0f;    //r, and convert to float colour 
			m_trackedColor2[1] = m_colorImage.getPixels()[index + 1] / 255.0f;    //g
			m_trackedColor2[2] = m_colorImage.getPixels()[index + 2] / 255.0f;    //b
			m_colourObj2->updateColour(m_trackedColor2);
		} 
	}
}


void ofApp::processBackground()
{
	//checking if bg needs to be captured
	if (m_captureBg) {
		m_grayscaleBg = m_grayscaleImage;
		m_captureBg = false;
	}

	//get the difference between images
	m_diffImage.absDiff(m_grayscaleBg, m_grayscaleImage);

	//maybe try to processing
	m_diffImage.blurGaussian(3);
	//m_grayscaleDiff.dilate_3x3();
	//m_grayscaleDiff.erode_3x3();

	//adjust threshold
	m_diffImage.threshold(m_threshold);

	//now look for contours/blobs
	m_contourFinder.findContours(m_diffImage, m_minArea, m_maxArea, m_numContourConsidered, false, true);
}

//takes a video frame scales it to the the allocated image size
void ofApp::videoToImage(ofxCvColorImage tempImage)
{
	m_colorImage.scaleIntoMe(tempImage);
	m_colorImage.mirror(false, true);
	m_grayscaleImage = m_colorImage;
	m_diffImage = m_colorImage;
}

//doing actions that need to be done everytime a new video frame is detected
void ofApp::newFrameActions()
{
	//processing the colours and looking for objects with the corresponding colour
	m_colourObj1->processColour(m_colorImage);
	m_colourObj2->processColour(m_colorImage);

	diffImageColourGenerator();

	//detecting if the coloured objects are on screen or off screen
	m_colourObj1->objectDetection();
	m_colourObj2->objectDetection();

	//monitoring depth change for the indicator radius
	m_colourObj1->indicatorSizeChange();
	m_colourObj2->indicatorSizeChange();

	//updating collision label
	m_collisionLabel = colourCollision() ? "Objects collided" : " ";
}


//function checks for object movements in the debug mode
void ofApp::debugMode()
{
	//getting the object current movements
	int currMovementObj1 = m_colourObj1->movementTracking();
	int currMovementObj2 = m_colourObj2->movementTracking();

	//checking if the object MOVEMENT_IMAGES match the MOVEMENT_IMAGES in the pattern
	if (m_colourObj1->contoursFound() && m_colourObj2->contoursFound())
	{
		//checking if there has been a chnage is obj1  movement and updating the correspingd visuals
		if (currMovementObj1 != -1)
		{
			movementImages(currMovementObj1, "obj1");
			m_movementSound.play();
			m_showObj1Visual = true;
		}
		//checking if there has been a change is obj2  movement and updating the correspingd visuals
		if (currMovementObj2 != -1)
		{
			movementImages(currMovementObj2, "obj2");
			m_showObj2Visual = true;
			m_movementSound.play();
		}
	}
}

void ofApp::patternDemo()
{
	//if this is the first movement in the pattern, initilize the first movement
	if (m_currMovementIndex == 0)
	{
		setDemoMovement();
		m_showInstructionsVisual = true;
	}
	//checking if the current pattern is still being shown
	if (m_currMovementIndex < 4 && m_frameCounter >= Config::APP_DESIRED_FRAMERATE)
	{
		//incrementing to the next movement in the pattern		
		m_currMovementIndex++;

		//if this is the last movement in the pattern, begin guessing mode
		if (m_currMovementIndex == 4)
		{
			m_guessingMode = true;
			m_beginGuessing = true;
			m_instructionMoveLabel = "Repeat the pattern";
			m_showInstructionsVisual = false;
		}
		//increment movement and corresponding images/sounds
		else
		{
			m_frameCounter = 0;
			m_currMovement = Config::LEVEL_PATTERNS[m_currPatternIndex][m_currMovementIndex];
			setDemoMovement();
		}
	}

	//checking if the current movement is still being shown, if yes, incrementing the timer
	if (m_frameCounter < Config::APP_DESIRED_FRAMERATE)
	{
		m_frameCounter++;
	}
}

void ofApp::setDemoMovement()
{
	m_movementSound.play();
	m_instructionsVisual.image.load(Config::INSTRUCTION_MOVEMENT_IMAGES[m_currMovement]);
	m_instructionMoveLabel = "Remember the pattern: " + Config::MOVEMENT_LABELS[m_currMovement];
}

void ofApp::guessingMode()
{
	//guessing will only work if both objects are active and the guess index is less than the number of movements in a pattern
	if (m_guessingMode && m_currGuessIndex < 4 && m_colourObj1->detectionState && m_colourObj2->detectionState)
	{
		//calibrating movement when beggining to guess
		if (m_beginGuessing)
		{
			m_backgroundImage.load(Config::GAME_BACKGROUND);
			m_beginGuessing = false;
			m_colourObj1->movementTracking();
			m_colourObj1->movementTracking();
			m_colourObj2->movementTracking();
			m_colourObj2->movementTracking();
		}
		//checking if the current object movement matches up with the current movement in the pattern
		else {
			int currMovementObj1 = m_colourObj1->movementTracking();
			int currMovementObj2 = m_colourObj2->movementTracking();

			//hiding the previous movement visuals
			if (!m_obj1Guessed && !m_obj2Guessed)
			{
				m_showObj1Visual = false;
				m_showObj2Visual = false;
			}

			//checking if the object MOVEMENT match the MOVEMENT in the pattern
			if (currMovementObj1 != -1)
			{
				m_movementMatchesObj1 = sameMovementCheck(currMovementObj1);
				movementImages(currMovementObj1, "obj1");
				m_obj1Guessed = true;
				m_backgroundImage.load(Config::GAME_BACKGROUND);
				m_movementSound.play();

			}
			if (currMovementObj2 != -1)
			{
				m_movementMatchesObj2 = sameMovementCheck(currMovementObj2);
				movementImages(currMovementObj2, "obj2");
				m_obj2Guessed = true;
				m_backgroundImage.load(Config::GAME_BACKGROUND);
				m_movementSound.play();
			}

			//if both movements have been guessed
			if (m_obj1Guessed && m_obj2Guessed)
			{
				//if both of the MOVEMENTS are correct
				if (m_movementMatchesObj1 && m_movementMatchesObj2)
				{
					m_correctSound.play();
					m_backgroundImage.load(Config::GAME_BACKGROUND_CORRECT_GUESS);
					m_currGuessIndex++;
					m_movementMatchesObj1 = false;
					m_movementMatchesObj2 = false;
					m_obj1Guessed = false;
					m_obj2Guessed = false;
				}
				//if at least one movement is incorrect
				else
				{
					m_incorrectSound.play();
					m_backgroundImage.load(Config::GAME_BACKGROUND_LOST);
					endRound();
				}
			}

			//if this is the last movement in the pattern, incrementing score and moving to the next round
			if (m_currGuessIndex == 4)
			{
				m_score++;
				m_backgroundImage.load(Config::GAME_BACKGROUND_WON);
				endRound();
			}
		}
	}
	//if at least one of the objects is not detected
	else if (m_guessingMode)
	{
		detectionInfo();
	}
}

void ofApp::detectionInfo()
{
	//if obj 1 is not detected, resetting its visuals and movement
	if (!m_colourObj1->contoursFound())
	{
		m_showObj1Visual = false;
		m_obj1Guessed = false;
	}
	//if obj 2 is not detected, resetting its visuals and movement
	if (!m_colourObj2->contoursFound())
	{
		m_showObj2Visual = false;
		m_obj2Guessed = false;
	}
	m_backgroundImage.load(Config::GAME_BACKGROUND_DETECTION_NOTICE);
	m_beginGuessing = true;
}

//function moves to the next round when there is a collision and the user is done guessing
void ofApp::nextRound()
{
	//setting collision state to false if it is tru to prevent several collisions being recorded
	if (m_collisionState)
		m_collisionState = colourCollision();

	//checking if objects have collided
	if (m_continueRound && colourCollision() && !m_collisionState)
	{
		m_collisionState = true;
		m_collisionSound.play();

		//if this is the last round, display end screen
		if (m_currPatternIndex == Config::NUM_LEVELS - 1)
			gameEnd();
		//if user collides object on end screen, reset game
		else if (m_currPatternIndex == Config::NUM_LEVELS)
		{
			endRound();
			resetGameplay();
		}
		//moving onto the next round
		else
		{
			m_backgroundImage.load(Config::GAME_BACKGROUND);
			m_showObj1Visual = false;
			m_showObj2Visual = false;
			m_continueRound = false;
			m_guessingMode = false;
			m_frameCounter = 0;
			m_currGuessIndex = 0;
			m_currPatternIndex++;
			m_currMovementIndex = 0;
			m_currMovement = Config::LEVEL_PATTERNS[m_currPatternIndex][m_currMovementIndex];
			m_movementMatchesObj1 = false;
			m_movementMatchesObj2 = false;
			m_obj1Guessed = false;
			m_obj2Guessed = false;
		}
	}
}

//detecting a collision between two coloured objects
bool ofApp::colourCollision()
{
	//making sure that both objects are detected before checking for their collision
	if (m_colourObj1->contoursFound() && m_colourObj2->contoursFound())
	{
		//scaling the contours a bit to allow detection to occur when object are very near eachother 
		ofRectangle obj1 = m_colourObj1->contourFinder.blobs[0].boundingRect;
		obj1.setSize(obj1.getWidth() + m_collisionThreshold, obj1.getHeight() + m_collisionThreshold);
		ofRectangle obj2 = m_colourObj2->contourFinder.blobs[0].boundingRect;
		obj2.setSize(obj2.getWidth() + m_collisionThreshold, obj2.getHeight() + m_collisionThreshold);

		//checking if a collision between the objects occured
		if (collisionDetection(obj1, obj2))
			return true;
		return false;
	}
}

//detecting collision between two rectangels
bool ofApp::collisionDetection(ofRectangle obj1, ofRectangle obj2)
{
	return obj1.intersects(obj2);
}

//function takes the diff images from both objects and combines them
void ofApp::diffImageColourGenerator()
{
	//checking if objetc one has a detected object
	if (m_colourObj1->colorSelected)
	{
		m_diffImage = m_colourObj1->diffImage;
		if (m_colourObj2->contoursFound())
			m_diffImage.drawBlobIntoMe(m_colourObj2->contourFinder.blobs[0], 255);
	}
	else 
	{
		m_diffImage = m_colourObj2->diffImage;
		if (m_colourObj1->contoursFound())
			m_diffImage.drawBlobIntoMe(m_colourObj1->contourFinder.blobs[0], 255);
	}
		
}



//function checks if the inputed movement matches the movement in the array
bool ofApp::sameMovementCheck(int objMovement)
{

		if (objMovement == Config::LEVEL_PATTERNS[m_currPatternIndex][m_currGuessIndex])
		{
			return true;
		}
		return false;	
}


//setting the proper images to match the object movement and pos
void ofApp::movementImages(int objMovement, string objName)
{

	//checking which image is on the left side of screen in order to flip the horizontal movement
	//if obj 1 is on left side
	if (objName == "obj1")
	{
		if (m_colourObj1->contourFinder.blobs[0].boundingRect.x < m_colourObj2->contourFinder.blobs[0].boundingRect.x)
		{
			m_obj1Visual.pos = ofVec2f(Config::APP_WINDOW_WIDTH / 2 - 200, Config::APP_WINDOW_HEIGHT / 2);
			//selecting to opposite horizontal movement if inputed movement is horizontal
			if (objMovement == 0 || objMovement == 1)
				objMovement = abs(objMovement - 1);
		}
		else
			m_obj1Visual.pos = ofVec2f(Config::APP_WINDOW_WIDTH / 2 + 200, Config::APP_WINDOW_HEIGHT / 2);
		
		imageSet(&m_obj1Visual.image, Config::MOVEMENT_IMAGES[objMovement]);
		m_showObj1Visual = true;
	}
		
	//if obj 2 is on left side
	else if (objName == "obj2")
	{
		if (m_colourObj2->contourFinder.blobs[0].boundingRect.x < m_colourObj1->contourFinder.blobs[0].boundingRect.x)
		{
			m_obj2Visual.pos = ofVec2f(Config::APP_WINDOW_WIDTH / 2 - 200, Config::APP_WINDOW_HEIGHT / 2);
			//selecting to opposite horizontal movement if inputed movement is horizontal
			if (objMovement == 0 || objMovement == 1)
				objMovement = abs(objMovement - 1);
		}
		else
			m_obj2Visual.pos = ofVec2f(Config::APP_WINDOW_WIDTH / 2 + 200, Config::APP_WINDOW_HEIGHT / 2);

		imageSet(&m_obj2Visual.image, Config::MOVEMENT_IMAGES[objMovement]);
		m_showObj2Visual = true;
	}
}

void ofApp::imageSet(ofImage* imageHolder, string imageToLoad)
{
	imageHolder->load(imageToLoad);
}

//toggling debug varaibles
void ofApp::toggleDebug()
{
	if (m_debugMode)
	{
		m_colourObj1->detectionTextPos = Config::DETECTION_DEBUG_TEXT_POS;
		m_colourObj2->detectionTextPos = Config::DETECTION_DEBUG_TEXT2_POS;
		m_backgroundMusic.setPaused(true);

	}
	else
	{
		m_colourObj1->detectionTextPos = Config::DETECTION_TEXT_POS;
		m_colourObj2->detectionTextPos = Config::DETECTION_TEXT2_POS;
		m_backgroundMusic.setPaused(false);
		m_showObj1Visual = false;
		m_showObj2Visual = false;
	}
}



//function runs when a round has ended and resets the needed game varaibles
void ofApp::endRound()
{
	m_instructionMoveLabel = "";

	m_guessingMode = false;
	m_continueRound = true;

	m_movementMatchesObj1 = false;
	m_movementMatchesObj2 = false;

	m_obj1Guessed = false;
	m_obj2Guessed = false;


}

//displaying the lose screen
void ofApp::looseScreen()
{
	ofPushMatrix();
	ofSetColor(255, 0, 0);
	ofDrawRectRounded(Config::APP_WINDOW_WIDTH/2, Config::APP_WINDOW_HEIGHT / 2, 400, 200, 10);
	ofPopMatrix();
}

//displaying end game screen and setting variables
void ofApp::gameEnd()
{
	m_backgroundImage.load(Config::GAME_BACKGROUND_END);
	m_finalScoreLabel = "Final score: " + ofToString(m_score) + "/" + ofToString(Config::NUM_LEVELS);
	m_currPatternIndex++;
	m_continueRound = true;
	m_gameComplete = true;
	m_guessingMode = false;
	m_showObj1Visual = false;
	m_showObj2Visual = false;
}

//reseting the gameplay
void ofApp::resetGameplay()
{
	m_gameComplete = false;
	m_currPatternIndex = 0;
	m_currMovementIndex = 0;
	m_currGuessIndex = 0;
	//m_frameCounter = 0;
	m_backgroundImage.load(Config::GAME_BACKGROUND);
	m_continueRound = false;
	m_guessingMode = false;
	m_frameCounter = 0;

	m_score = 0;

	m_backgroundImage.load(Config::GAME_BACKGROUND);
	m_showObj1Visual = false;
	m_showObj2Visual = false;
	m_continueRound = false;

	m_currMovement = Config::LEVEL_PATTERNS[m_currPatternIndex][m_currMovementIndex];
	m_movementMatchesObj1 = false;
	m_movementMatchesObj2 = false;
	m_obj1Guessed = false;
	m_obj2Guessed = false;
}

void ofApp::resetVideo()
{
	m_videoPlayer.firstFrame();
}

void ofApp::pauseVideo()
{
	m_videoPlayer.setPaused(true);
	m_videoPlayer.setPaused(false);
}


//drawing the gui
void ofApp::guiUI()
{
	//GUI for debug
	m_gui.begin();
	//debug toggle UI
	ImGui::SetNextWindowSize({ 150, 30 });
	ImGui::SetNextWindowPos({ Config::APP_WINDOW_WIDTH - Config::VIDEO_BORDER_SIZE * 2 - 150, 15 });
	ImGui::Begin("Debug");
	if (ImGui::Checkbox("Debug Mode", &m_debugMode))
	{
		toggleDebug();
	}
	ImGui::End();

	//debug callibration UI
	if (m_debugMode)
	{
		ImGui::SetNextWindowPos({ Config::VIDEO_WIDTH + Config::VIDEO_BORDER_SIZE, 100 });
		ImGui::SetNextWindowSize({ Config::APP_WINDOW_WIDTH - Config::VIDEO_WIDTH - Config::VIDEO_BORDER_SIZE * 2, 600 });
		ImGuiStyle& style = ImGui::GetStyle();
		style.ItemSpacing = { 10, 10 };
		ImGui::Begin("Controls");
		ImGui::SliderInt("Threshold", &m_threshold, 0, 255);
		ImGui::SliderFloat("Min. Area", &m_minArea, 0.0, Config::APP_WINDOW_WIDTH * Config::APP_WINDOW_HEIGHT);
		ImGui::SliderFloat("Max. Area", &m_maxArea, 0.0, Config::APP_WINDOW_WIDTH * Config::APP_WINDOW_HEIGHT);
		ImGui::Separator();
		ImGui::SliderInt("Collision distance", &m_collisionThreshold, 0, 100);
		ImGui::SliderInt("Depth threshold", &m_depthThreshold, 500, 12000);
		//ImGui::SliderInt("Collision detection distance", &m_collisionThreshold, 0, 100);
		ImGui::Separator();
		ImGui::RadioButton("colour detection", &m_detectionMode, 0);
		ImGui::Separator();
		if (ImGui::RadioButton("Webcam", &m_videoType, 0))
			pauseVideo();
		if (ImGui::RadioButton("Recorded video", &m_videoType, 1))
			resetVideo();
		ImGui::Separator();
		//colour selection
		ImGui::Text("Hover over the area to sample and press 1 OR right click");
		if (ImGui::ColorEdit3("Color 1", &m_trackedColor1[0]))
		{
			m_colourObj1->updateColour(m_trackedColor1);
		}
		ImGui::Text("Hover over the area to sample and press 2 OR middle click");
		if (ImGui::ColorEdit3("Color 2", &m_trackedColor2[0]))
		{
			m_colourObj2->updateColour(m_trackedColor2);
		}
		ImGui::Text("\nInstructions: \n- Press space bar to toggle play/pause \n- Keep one object on each half of the screen \n- Move objects slowly when guessing moves");
		ImGui::End();
	}
	m_gui.end();
}


//drawing the debug UI
void ofApp::debugUI()
{
	//draw videos
	ofSetColor(255); //reset any color state
	m_diffImage.draw(Config::VIDEO_WIDTH / 2, Config::VIDEO_HEIGHT + Config::VIDEO_HEIGHT / 2, Config::VIDEO_WIDTH, Config::VIDEO_HEIGHT); //step 2, grayscale(B/W) image
	m_colorImage.draw(Config::VIDEO_WIDTH / 2, Config::VIDEO_HEIGHT / 2); //step1, color image
	m_colourObj1->drawContour();
	m_colourObj2->drawContour();
	m_collisionText.drawString(m_collisionLabel, Config::COLLISION_DEBUG_TEXT_POS.x, Config::COLLISION_DEBUG_TEXT_POS.y);

	ofPushMatrix();
		ofSetColor(255);
		if (m_showObj1Visual)
			m_obj1Visual.image.draw(m_obj1Visual.pos.x / Config::VIDEO_SCALE_RATIO, 50, Config::MOVEMENT_IMAGE_SIZE / 2, Config::MOVEMENT_IMAGE_SIZE / 2);
		if (m_showObj2Visual)
			m_obj2Visual.image.draw(m_obj2Visual.pos.x / Config::VIDEO_SCALE_RATIO, 50, Config::MOVEMENT_IMAGE_SIZE / 2, Config::MOVEMENT_IMAGE_SIZE / 2);
	ofPopMatrix();
}

//drwing the gameUI
void ofApp::gameUI()
{
	ofPushMatrix();
	//drawing the game UI
	ofSetColor(255);
	m_backgroundImage.draw(Config::APP_WINDOW_WIDTH / 2, Config::APP_WINDOW_HEIGHT / 2);
	m_instructionMoveText.drawString(m_instructionMoveLabel, Config::INSTRUCTION_TEXT_POS.x - m_instructionMoveText.stringWidth(m_instructionMoveLabel) / 2, Config::INSTRUCTION_TEXT_POS.y);

	if (m_showInstructionsVisual)
		m_instructionsVisual.image.draw(m_instructionsVisual.pos.x, m_instructionsVisual.pos.y);

	//drawing circles to represent tracked objects 
	ofPushMatrix();

	ofSetColor(255, 255, 255, 100);
	ofScale(Config::VIDEO_SCALE_RATIO);
	if (m_colourObj1->contoursFound())
		ofDrawCircle(m_colourObj1->contourFinder.blobs[0].boundingRect.getCenter(), m_colourObj1->indicatorSize);
	if (m_colourObj2->contoursFound())
		ofDrawCircle(m_colourObj2->contourFinder.blobs[0].boundingRect.getCenter(), m_colourObj2->indicatorSize);
	ofPopMatrix();
	ofPopMatrix();

	ofPushMatrix();
	ofSetColor(255);
	if (m_showObj1Visual)
		m_obj1Visual.image.draw(m_obj1Visual.pos.x, m_obj1Visual.pos.y, Config::MOVEMENT_IMAGE_SIZE / 3);
	if (m_showObj2Visual)
		m_obj2Visual.image.draw(m_obj2Visual.pos.x, m_obj2Visual.pos.y, Config::MOVEMENT_IMAGE_SIZE / 3);
	ofPopMatrix();


	//printing score UI based on game state
	ofPushMatrix();
	ofSetColor(255);
	if (m_gameComplete)
		m_finalScoreText.drawString(m_finalScoreLabel, Config::APP_WINDOW_WIDTH / 2 - m_finalScoreText.stringWidth(m_finalScoreLabel) / 2, 200);
	else
	{
		m_currRoundText.drawString("Round: " + ofToString(m_currPatternIndex) + "/" + ofToString(Config::NUM_LEVELS), Config::ROUND_TEXT_POS.x, Config::ROUND_TEXT_POS.y);
		m_scoreText.drawString("Score: " + ofToString(m_score), Config::SCORE_TEXT_POS.x, Config::SCORE_TEXT_POS.y);
	}
	ofPopMatrix();
}