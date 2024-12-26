#pragma once
#include "ofMain.h"


namespace Config {
	static const int APP_WINDOW_WIDTH = 1280;
	static const int APP_WINDOW_HEIGHT = 720;
	static const int APP_DESIRED_FRAMERATE = 60;

	static const string VIDEO_PATH_COLOR = "video.mp4";
	static const string SOUND_PATH_BACKGROUND = "background.mp3";
	static const string SOUND_PATH_CORRECT = "correct.mp3";
	static const string SOUND_PATH_INCORRECT = "incorrect.mp3";
	static const string SOUND_PATH_COLLISION = "collision.mp3";
	static const string SOUND_PATH_MOVEMENT = "selectMove.mp3";
	static const float VIDEO_SCALE_RATIO = 2;
	static const int VIDEO_WIDTH = APP_WINDOW_WIDTH/ VIDEO_SCALE_RATIO;
	static const int VIDEO_HEIGHT = APP_WINDOW_HEIGHT/ VIDEO_SCALE_RATIO;

	static const int VIDEO_BORDER_SIZE = 10;

	static const int DEPTH_THRESHOLD = 9000;

	//Game UI
	string FONT = "font.ttf";
	int LARGE_FONT = 16;
	int SMALL_FONT = 8;

	static const ofVec2f ROUND_TEXT_POS(1000, 100);
	static const ofVec2f SCORE_TEXT_POS(100, 100);
	static const ofVec2f INSTRUCTION_TEXT_POS(APP_WINDOW_WIDTH/2, 100);

	//DETECTION TEXT
	static const ofVec2f DETECTION_TEXT_POS(100, 625);
	static const ofVec2f DETECTION_TEXT2_POS(100, 650);
	static const ofVec2f DETECTION_DEBUG_TEXT_POS(VIDEO_WIDTH+VIDEO_BORDER_SIZE, 50);
	static const ofVec2f DETECTION_DEBUG_TEXT2_POS(VIDEO_WIDTH + VIDEO_BORDER_SIZE, 75);
	static const ofVec2f COLLISION_DEBUG_TEXT_POS(VIDEO_WIDTH + VIDEO_BORDER_SIZE, 25);

	static const ofColor DETECTION_TEXT_FOUND = ofColor(49, 189, 86);
	static const ofColor DETECTION_TEXT_LOST = ofColor(255, 97, 97);

	//backgrounds
	static const ofColor GAME_BACKGROUND_COLOUR = ofColor(1, 0, 26);
	static const string GAME_BACKGROUND = "background.png";
	static const string GAME_BACKGROUND_WON = "backgroundCorrect.png";
	static const string GAME_BACKGROUND_CORRECT_GUESS = "backgroundCorrectMove.png";
	static const string GAME_BACKGROUND_LOST = "backgroundWrong.png";
	static const string GAME_BACKGROUND_DETECTION_NOTICE = "backgroundDetectionNotice.png";
	static const string GAME_BACKGROUND_END = "backgroundEnd.png";

	//images
	static const int INSTRUCTION_IMAGE_WIDTH = 704;
	static const int INSTRUCTION_IMAGE_HEIGHT = 230;
	static const int MOVEMENT_IMAGE_SIZE = 200;


	//levels
	static const int NUM_LEVELS = 1;
	
	static const int LEVEL_PATTERNS[10][4] = {
		{0, 1, 2, 3}, //out, in, closer, far
		{2, 1, 2, 3}, //closer, in,closer,far
		{1, 0, 1, 0},
		{1, 3, 0, 3},
		{0, 1, 0, 1},
		{2, 3, 2, 1},
		{3, 2, 1, 0},
		{2, 3, 2, 3},
		{2, 1, 0, 1},
		{1, 0, 3, 2}
	};
	
	//Movements
	static const string MOVEMENT_LABELS[] = { "OUT", "IN", "CLOSER", "FURTHER" };

	static const string MOVEMENT_IMAGES[] = { "out.png", "in.png", "closer.png", "further.png"};

	static const string INSTRUCTION_MOVEMENT_IMAGES[] = { "outInstruction.png", "inInstruction.png", "closerInstruction.png", "furtherInstruction.png" };
}