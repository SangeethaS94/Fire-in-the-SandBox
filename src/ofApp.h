/***********************************************************************
ofApp.h - main openframeworks app

Copyright (c) 2017 Charu Manivannan, Mina Karamesouti, Sangeetha Shankar, Zhihao Liu
Univeristy of Muenster, Germany

Based on Magic Sand by Thomas Wolf (2016)

This file is part of the project "Fire in the Sandbox".
Guided by Junior Prof. Dr. Judith Verstegen

The "Fire in the Sandbox" is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation.

The "Fire in the Sandbox" is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

***********************************************************************/

#pragma once

#include "ofMain.h"
#include "ofxDatGui.h"
#include "KinectProjector/KinectProjector.h"
#include "SandSurfaceRenderer/SandSurfaceRenderer.h"
#include "vehicle.h"

class ofApp : public ofBaseApp {

private:
	std::shared_ptr<KinectProjector> kinectProjector;
	SandSurfaceRenderer* sandSurfaceRenderer;
	
	// Projector and kinect variables
	ofVec2f projRes;
	ofVec2f kinectRes;
	ofRectangle kinectROI;
	
	// FBos
	ofFbo fboVehicles;
	ofFbo fboFireman;
	ofFbo fboHouse;

	//Vectors
	vector<Fire> Fires;
	vector<Fire> FiresToBeDrawn;
	vector<Fire> FiresThatCanSpawn;
	vector<Marker> Markers;
	vector<House> Houses;
	vector<houseWithBarrier> HousesWithBarrier;
	vector<Fireman> Firemen;
	
	ofPoint motherFire;
	bool showMotherFire;

	// Check variables
	bool showhouseWithBarrier;
	bool FiremanSet;
	bool firemanNearHouse;
	bool burnHouse;

	// Starting point of Fire
	int StartX;
	int StartY;

	// Starting point set on Slider
	int StartXSlider;
	int StartYSlider;
	
	// GUI
	ofxDatGui* gui;

	/***
	2D grid to store cell states
	0 - unburnt
	1 - burning
	2 - extinguished
	(3 - House With Barrier)
	Grid size based on the kinect ROI
	***/
	int grid[261][157];

public:
	// Model parameters
	string windSpeed;
	string windDirection;
	string vegType;

	void setup();

	void addNewFire(float x, float y);
	void addNewMarker(float x, float y);

	void addHouse();
	bool addHouseWithBarrier(float h_x, float h_y);
	void addFireman();

	void update();

	void draw();
	void drawProjWindow(ofEventArgs& args);
	void drawVehicles();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void setupGui();
	void onButtonEvent(ofxDatGuiButtonEvent e);
	void onToggleEvent(ofxDatGuiToggleEvent e);
	void onSliderEvent(ofxDatGuiSliderEvent e);
	void onDropdownEvent(ofxDatGuiDropdownEvent e);

	void addMarker(int x, int y);
	bool setMarkerLocation(ofRectangle area, bool liveInWater);

	std::shared_ptr<ofAppBaseWindow> projWindow;
	void spreadFire();
	float windAndSlopeEffects(float new_x, float new_y, float current_x, float current_y, float slope);
	bool headingDirection(float new_x, float new_y, float current_x, float current_y);
	bool backingDirection(float new_x, float new_y, float current_x, float current_y);
	bool checkSameDirection(float new_x, float new_y, float current_x, float current_y);

	float vegetationEffect(float probFromSlopeWind);
	
	bool Effects(float new_x, float new_y, float current_x, float current_y, float slope);
};
