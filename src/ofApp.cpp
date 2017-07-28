/***********************************************************************
ofApp.cpp - main openframeworks app

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

#include "ofApp.h"

void ofApp::setup() {
	//cout << "\nInside ofApp::setup function";

	// OF basics
	ofSetFrameRate(10);
	ofBackground(0);
	ofSetVerticalSync(true);
	ofSetLogLevel("ofThread", OF_LOG_WARNING);

	// Setup kinectProjector
	kinectProjector = std::make_shared<KinectProjector>(projWindow);
	kinectProjector->setup(true);
	
	// Setup sandSurfaceRenderer
	sandSurfaceRenderer = new SandSurfaceRenderer(kinectProjector, projWindow);
	sandSurfaceRenderer->setup(true);
	
	// Retrieve variables
	kinectRes = kinectProjector->getKinectRes();
	projRes = ofVec2f(projWindow->getWidth(), projWindow->getHeight());
	kinectROI = kinectProjector->getKinectROI();
	
	// Set width, height and alpha of ofFbos
	// Separate Fbo for fires, house and fireman (truck)
	fboVehicles.allocate(projRes.x, projRes.y, GL_RGBA);
	fboHouse.allocate(projRes.x, projRes.y, GL_RGBA);
	fboFireman.allocate(projRes.x, projRes.y, GL_RGBA);

	setupGui();

	// Vehicles
	showMotherFire = false;

	// initial value for the starting point of fire
	StartX = kinectROI.getLeft();
	StartY = kinectROI.getTop();

	// initial value for the sliders
	StartXSlider = kinectROI.getLeft();
	StartYSlider = kinectROI.getTop();

	// default option for wind
	windSpeed = "NoWind";
	// default option for vegetation
	vegType = "Evergreen";
	// default option for wind direction
	windDirection = "North";

	// initial value for check variables
	FiremanSet = false;
	firemanNearHouse = false;
	showhouseWithBarrier = false;
}

void ofApp::update() {
	//cout << "\nInside ofApp::update function";

    // Call kinectProjector->update() first during the update function()
	kinectProjector->update();
	sandSurfaceRenderer->update();
    
    if (kinectProjector->isROIUpdated())
        kinectROI = kinectProjector->getKinectROI();

	spreadFire();

	if (kinectProjector->isImageStabilized()) {
	    for (auto & r : Fires){
			//decrease the fuel availability of the burning fires by 1 in each time step
			r.fuel = r.fuel - 1;
			//redraw the fire when it is extinguished
			if (r.fuel == 0) {
				FiresToBeDrawn.push_back(r);
			}
	    }

		//update only the new fire instances created and the instances that extinguished in this time step
		for (auto & r : FiresToBeDrawn) {
			r.update();
		}

		for (auto & m : Markers) {
			m.update();
		}

		//check if the fire has reached the house (runs only until the house starts burning)
		if (!burnHouse) {
			for (auto & h : Houses) {
				h.update();
				int h_x = static_cast<int>(h.getLocation().x);
				int h_y = static_cast<int>(h.getLocation().y);
				float gridforHouse[16][16][2] = { 0 };
				for (int i = -15; i <= 15; i = i+2) {
					for (int j = -15; j <= 15; j =j+2) {
						gridforHouse[(i+15)/2][(j+15)/2][0] = h_x + i;
						gridforHouse[(i+15)/2][(j+15)/2][1] = h_y + j;
					}
				}
				//check if house is on a burning cell
				for (int i = 0; i < 16 & (!h.burningState); i++) {
					for (int j = 0; j < 16 & (!h.burningState); j++) {
						int h_temp_x = (gridforHouse[i][j][0] - kinectROI.getLeft()) / 2;
						int h_temp_y = (gridforHouse[i][j][1] - kinectROI.getTop()) / 2;
						if (grid[h_temp_x][h_temp_y] == 1) {
							h.image.load("house2.png");
							burnHouse = true;
							h.burningState = true;
						}
					}
				}
				
			}
		}
		
		for (auto & hb : HousesWithBarrier) {
			hb.update();
		}
		
		//check if the center of fireman (truck) is on fire!
		bool killFireman = false;
		for (auto & fm : Firemen) {
			fm.update();
			int fm_x = static_cast<int>(fm.getLocation().x);
			int fm_y = static_cast<int>(fm.getLocation().y);
			//check if fireman is on a burning cell
			int fm_temp_x = (fm_x - kinectROI.getLeft()) / 2;
			int fm_temp_y = (fm_y - kinectROI.getTop()) / 2;
			if (grid[fm_temp_x][fm_temp_y] == 1) {
				killFireman = true;
			}
		}
		if (killFireman) {
			Firemen.clear();
			FiremanSet = false;
		}

	    drawVehicles();
	}
	gui->update();
}


void ofApp::draw() {
	//cout << "\nInside ofApp::draw function";

	sandSurfaceRenderer->drawMainWindow(300, 30, 600, 450);//400, 20, 400, 300);
	fboVehicles.draw(300, 30, 600, 450);
	fboHouse.draw(300, 30, 600, 450);
	fboFireman.draw(300, 30, 600, 450);
	kinectProjector->drawMainWindow(300, 30, 600, 450);
	gui->draw();
}

void ofApp::drawProjWindow(ofEventArgs &args) {
	//cout << "\nInside ofApp::drawProjWindow function";

	kinectProjector->drawProjectorWindow();
	
	if (!kinectProjector->isCalibrating()){
	    sandSurfaceRenderer->drawProjectorWindow();
	    fboVehicles.draw(0,0);
		fboHouse.draw(0, 0);
		fboFireman.draw(0, 0);
	}
}

// Draw all new fire instances, extinguished instances, house/house with barrier, fireman (firetruck)
void ofApp::drawVehicles()
{
	//cout << "\nInside ofApp::drawVehicles function";

	/***** VEHICLE FBO ****/
    fboVehicles.begin();
    for (auto & r : FiresToBeDrawn){
        r.draw();
    }

	for (auto & m : Markers) {
		ofClear(255, 255, 255, 0);
		m.draw();
	}
	fboVehicles.end();
	
	/***** HOUSE FBO ****/
	fboHouse.begin();
	for (auto & h : Houses) {
		ofClear(255, 255, 255, 0);
		h.draw();
	}
	
	for (auto & hb : HousesWithBarrier) {
		ofClear(255, 255, 255, 0);
		hb.draw();
	}
	fboHouse.end();
	
	/***** FIREMAN FBO ****/
	fboFireman.begin();
	ofClear(255,255,255, 0);
	for (auto & fm : Firemen) {
		fm.draw();
	}
	fboFireman.end();
    
}

// Setting up the user interface of the application - PLACED AT BOTTOM RIGHT PART OF THE INTERFACE
void ofApp::setupGui(){
	//cout << "\nInside ofApp::setupGui function";

    // instantiate and position the gui //
    gui = new ofxDatGui();

	// set FrameRate
	gui->addSlider("Set FrameRate", 5, 20, 10)->setPrecision(0);

	// dropdown for Wind Speed (parent #0)
	vector<string> optionsWind{ "No Wind", "Low Wind Speed", "High Wind Speed" };
	gui->addDropdown("Options for wind", optionsWind);
	
	// dropdown for Wind Direction (parent #1)
	vector<string> optionsWindDirection{ "West - East", "East - West", "South - North", "North - South" };
	gui->addDropdown("Wind Direction", optionsWindDirection);

	// dropdown for Vegetation type (parent #2)
	vector<string> optionsVegetationType{ "Evergreen Forest", "Grasslands", "Pinus Forest" };
	gui->addDropdown("Vegetation type", optionsVegetationType);

	// Sliders to define the starting point of fire
	gui->addLabel("Starting Point of Fire");
	gui->addSlider("X coordinate", kinectROI.getLeft(), kinectROI.getRight(), Fires.size())->setPrecision(0);
	gui->addSlider("Y coordinate", kinectROI.getTop(), kinectROI.getBottom(), Fires.size())->setPrecision(0);
	
	// Buttons
	gui->addButton("Start");
	gui->addButton("Add a House");
	gui->addButton("Add Fireman");
    gui->addButton("Reset");
    gui->addBreak();
    

    gui->onButtonEvent(this, &ofApp::onButtonEvent);
    gui->onToggleEvent(this, &ofApp::onToggleEvent);
    gui->onSliderEvent(this, &ofApp::onSliderEvent);
	gui->onDropdownEvent(this, &ofApp::onDropdownEvent);
    gui->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    
    gui->setPosition(ofxDatGuiAnchor::BOTTOM_RIGHT); // This should be done at the end
	gui->setAutoDraw(false); // troubles with multiple windows drawings on Windows
}

// Button Events - runs when a button is clicked on the interface
void ofApp::onButtonEvent(ofxDatGuiButtonEvent e){
	//cout << "\nInside ofApp::onButtonEvent function";

	/*** 
	START BUTTON:
	Clears all Fire vectors, markers.
	Creates a new fire instance at the starting point selected in the slider
	Updates the value in that location in the 2D grid to 1 (burning)
	***/
	if (e.target->is("Start")) {
		Fires.clear();
		FiresThatCanSpawn.clear();
		FiresToBeDrawn.clear();
		Markers.clear();
		addNewFire(StartX, StartY);

		int temp_x = (StartX - kinectROI.getLeft()) / 2;
		int temp_y = (StartY - kinectROI.getTop()) / 2;
		grid[temp_x][temp_y] = 1;
	}

	/***
	RESET BUTTON:
	Clears all vectors and fbos; Reset grid and check variables
	***/
    if (e.target->is("Reset")) {
        
		// Clear all vectors
		Fires.clear();
		FiresThatCanSpawn.clear();
		FiresToBeDrawn.clear();
		Markers.clear();
		Houses.clear();
		HousesWithBarrier.clear();
		Firemen.clear();
		
		// Clear Fbos
		fboVehicles.begin();
		ofClear(255, 255, 255, 0);
		fboVehicles.end();
		fboHouse.begin();
		ofClear(255, 255, 255, 0);
		fboHouse.end();

        showMotherFire = false;
		
		// Reset all values in the grid to 0 (not burning)
		for (int i = 0; i < 261; i++)
			for (int j = 0; j < 157; j++)
				grid[i][j] = 0;

		// Reset Check Variables
		FiremanSet = false;
		firemanNearHouse = false;
		burnHouse = false;
    }

	/***
	'ADD A HOUSE' BUTTON:
	Clears Houses and HousesWithBarriers vector and adds a new house at a random location
	***/
	if (e.target->is("Add a House")) {
		Houses.clear();
		HousesWithBarrier.clear();
		showMotherFire = false;
		addHouse();
	}

	/***
	'ADD FIREMAN' BUTTON:
	Clears Firemen vector and adds a new fireman at a random location
	***/
	if (e.target->is("Add Fireman")) {
		Firemen.clear();
		addFireman();
	}
}

// Dropdown Events - runs when an option is chosen in one of the dropdowns on the interface
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e) {
	//cout << "\nInside ofApp::onDropdownEvent() function";
	
	/*** WIND SPEED DROPDOWN ***/
	//No Wind
	if (e.parent == 1 & e.child == 0) {
		windSpeed = "NoWind";
	}
	//Low Wind Speed
	if (e.parent == 1 & e.child == 1) {
		windSpeed = "LowWind";	// Velocity = 0.42 m/s
	}
	//High Wind Speed
	if (e.parent == 1 & e.child == 2) {
		windSpeed = "HighWind";	// Velocity = 1.15 m/s
	}
	
	/*** WIND DIRECTION DROPDOWN ***/
	if (e.parent == 2 & e.child == 0) {
		windDirection = "West";		// West to East
	}
	if (e.parent == 2 & e.child == 1) {
		windDirection = "East";		// East to West
	}
	if (e.parent == 2 & e.child == 2) {
		windDirection = "South";	// South to North
	}
	if (e.parent == 2 & e.child == 3) {
		windDirection = "North";	// North to South
	}

	/*** VEGETATION TYPE DROPDOWN ***/
	if (e.parent == 3 & e.child == 0) {
		vegType = "Evergreen";
	}
	if (e.parent == 3 & e.child == 1) {
		vegType = "Grassland";
	}
	if (e.parent == 3 & e.child == 2) {
		vegType = "Pinus";
	}
}

// Slider Events - runs when a slider value changes in the interface
void ofApp::onSliderEvent(ofxDatGuiSliderEvent e){
	//cout << "\nInside ofApp::onSliderEvent function";

	// Set the number of frames (timesteps) per second
	if (e.target->is("Set FrameRate")) {
		ofSetFrameRate(e.value);
	}

	// Sets the starting point of fire and adds a marker when slider value changes
	if (e.target->is("X coordinate")) {
		StartXSlider = e.value;
		if(Fires.size()==0)
			addMarker(StartX, StartY);
	}
	if (e.target->is("Y coordinate")) {
		StartYSlider = e.value;
		if (Fires.size() == 0)
			addMarker(StartX, StartY);
	}
}

// Key Pressed Events - runs when a key is pressed on the keyboard
// Control the movement of fireman using arrow keys (truck)
void ofApp::keyPressed(int key) {
	//cout << "\nInside ofApp::keyPressed function";
	if (FiremanSet) {
		for (auto & fm : Firemen) {
			if ((key == OF_KEY_UP) | (key == OF_KEY_DOWN) | (key == OF_KEY_LEFT) | (key == OF_KEY_RIGHT)) {
				
				// Move the fireman when an arrow key is pressed
				fm.moveFireman(key);

				// Check if Fireman is near house
				// If yes, convert the house to house with barrier
				if (!firemanNearHouse) {
					int x;
					int y;
					for (auto & h : Houses) {
						x = static_cast<int>(h.getLocation().x);
						y = static_cast<int>(h.getLocation().y);
						int fm_x = static_cast<int>(fm.getLocation().x);
						int fm_y = static_cast<int>(fm.getLocation().y);
						
						//check if fireman is near the House
						for (int i = -15; (i <= 15); i++) {
							for (int j = -15; (j <= 15); j++) {
								if ((fm_x == (x + i)) & (fm_y == (y + j)))
									firemanNearHouse = true;
							}
						}
					}
					if (firemanNearHouse & !showhouseWithBarrier) {
						Houses.clear();
						HousesWithBarrier.clear();
						showMotherFire = false;
						addHouseWithBarrier(x, y);
						showhouseWithBarrier = true;
					}
				}
			}
		}
	}
}

// Adds the first fire instance and adds it in all the Fire vectors
void ofApp::addNewFire(float x, float y) {
	//cout << "\nInside ofApp::addNewFire function";

	ofVec2f location = ofVec2f(x, y);
	auto r = Fire(kinectProjector, location, kinectROI, motherFire);
	r.setup();
	Fires.push_back(r);
	FiresToBeDrawn.push_back(r);
	FiresThatCanSpawn.push_back(r);
}

/***
SPREAD FIRE FUNCTION:
	- Considers each fire instance in the vector
	- Checks if it has atleast one non-burning neighbour
	- Creates new instances depending on probabilities
***/
void ofApp::spreadFire() {
	
	// to prevent calling new fires in the same timestep
	vector<Fire> FiresAtThisTime = FiresThatCanSpawn;
	// Clear vectors
	FiresToBeDrawn.clear();
	FiresThatCanSpawn.clear();
	
	//Loop through each fire instance in the vector
	for (auto & r : FiresAtThisTime) {
		
		// runs only if the fire has one unburnt neighbour
		if (r.numberOfBurningNeighbours != 4) {

			// coordinate and elevation of the current fire instance
			float current_x = r.getLocation().x;
			float current_y = r.getLocation().y;
			float elevationAtCurrentCell = kinectProjector->elevationAtKinectCoord(current_x, current_y);
			
			// array containing the coordinate of the four neighbouring cells
			float neighbourhood[4][2] = { { r.getLocation().x - 2,r.getLocation().y },{ r.getLocation().x + 2,r.getLocation().y },{ r.getLocation().x,r.getLocation().y - 2 },{ r.getLocation().x,r.getLocation().y + 2 } };

			for (int i = 0; i < 4; i++) {
				
				// check if the coordinate falls outside the sandbox
				if (neighbourhood[i][0]<kinectROI.getLeft() || neighbourhood[i][0]>kinectROI.getRight() || neighbourhood[i][1]<kinectROI.getTop() || neighbourhood[i][1]>kinectROI.getBottom()) {
					//cout << "\n Point out of bounds! Fire instance not created!";
				}
				
				// run this code when the new cell is inside sandbox
				else {
					// convert to grid coordinates
					int temp_x = (neighbourhood[i][0] - kinectROI.getLeft()) / 2;
					int temp_y = (neighbourhood[i][1] - kinectROI.getTop()) / 2;
					
					// spread fire only if the fire is still burning (extinguished fires cannot spread)
					if ((r.fuel > 0) & (grid[temp_x][temp_y]<2)) {	
						
						// check if the new cell is inside/outside water
						float elevationAtNewCell = kinectProjector->elevationAtKinectCoord(neighbourhood[i][0], neighbourhood[i][1]);
						bool newCellInsideWater = (elevationAtNewCell < 0);
						
						// check if the cell is already burning
						bool alreadyBurning = grid[temp_x][temp_y] == 1;

						// Calculate slope
						// Reference: http://geology.isu.edu/wapi/geostac/Field_Exercise/topomaps/slope_calc.htm
						float elevationChange = elevationAtNewCell - elevationAtCurrentCell; //+ve : uphill; -ve: downhill
						float horizontalDistance = sqrt(pow((neighbourhood[i][0] - current_x), 2) + pow((neighbourhood[i][1] - current_y), 2));
						float slope = (elevationChange / horizontalDistance) * 100;

						// Apply slope, wind and vegetation effects
						bool probToCatchFire = Effects(neighbourhood[i][0], neighbourhood[i][1], current_x, current_y, slope);
						
						// If the cell is on land, not already burning and satisfies all the rules in Effects function, create a new fire instance
						if (!newCellInsideWater & !alreadyBurning & probToCatchFire) {
							// create new fire instance
							ofVec2f newLocation = ofVec2f(neighbourhood[i][0], neighbourhood[i][1]);
							auto newFire = Fire(kinectProjector, newLocation, kinectROI, motherFire);
							newFire.setup();
							
							// update the grid
							grid[temp_x][temp_y] = 1;

							// add the new fire instance to the vectors
							Fires.push_back(newFire);
							FiresToBeDrawn.push_back(newFire);
							FiresThatCanSpawn.push_back(newFire);

							// update the number of burning neighbours of the fire instance
							r.numberOfBurningNeighbours = r.numberOfBurningNeighbours + 1;
						}
					}
					else {
						int temp_x = (current_x - kinectROI.getLeft()) / 2;
						int temp_y = (current_y - kinectROI.getTop()) / 2;
						grid[temp_x][temp_y] = 2;
					}
				}
			}
			//update the number of burning neighbours
			int counter = 0;
			for (int i = 0; i < 4; i++) {
				int temp_x = (neighbourhood[i][0] - kinectROI.getLeft()) / 2;
				int temp_y = (neighbourhood[i][1] - kinectROI.getTop()) / 2;
				if (grid[temp_x][temp_y] == 1)
					counter++;
			}
			r.numberOfBurningNeighbours = counter;
		}

		//check if the Fire still has unburnt neighbours and if it is still burning
		if ((r.numberOfBurningNeighbours != 4) & (r.fuel>0)) {
			FiresThatCanSpawn.push_back(r);
		}
	}
}

/***
PROBABILITIES FOR FIRE SPREAD BASED ON WIND AND SLOPE
Reference:
A Qualitative comparison of fire spread models incorporating wind and slope effects,
Weise and Biging, 
Forest Science 43.2 (1997): 170 - 180
***/
float ofApp::windAndSlopeEffects(float new_x, float new_y, float current_x, float current_y, float slope) {

	float probToCatchFire_WS = 0;

	// Cell in heading direction - wind supports fire spread
	if (headingDirection(new_x, new_y, current_x, current_y)) {
		if (slope > 30) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 100;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 23.32;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 23.32;
		}
		else if ((slope > 15) & (slope <= 30)) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 76.63;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 18.42;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 6.10;
		}
		else if ((slope >= 0) & (slope <= 15)) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 63.6;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 15.6;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 5.26;
		}
		else if ((slope > -15) & (slope < 0)) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 42.0;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 9.9;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 4.9;
		}
		else if ((slope > -30) & (slope < -15)) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 33.4;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 7.1;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 5.4;
		}
		else if (slope < -30) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 36.96;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 8.24;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 2.55;
		}
	}
	// Cell in backing direction - wind doesnt support fire spread
	else if (backingDirection(new_x, new_y, current_x, current_y)) {
		if (slope > 30) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 1.1;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 18.3;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 23.32;
		}
		else if ((slope > 15) & (slope <= 30)) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 2.0;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 5.5;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 6.10;
		}
		else if ((slope >= 0) & (slope <= 15)) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 2.22;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 4.7;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 5.26;
		}
		else if ((slope > -15) & (slope < 0)) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 2.02;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 3.2;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 4.9;
		}
		else if ((slope > -30) & (slope < -15)) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 1.6;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 2.7;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 5.4;
		}
		else if (slope < -30) {
			if (windSpeed == "HighWind")
				probToCatchFire_WS = 2.6;
			else if (windSpeed == "LowWind")
				probToCatchFire_WS = 2.3;
			else if (windSpeed == "NoWind")
				probToCatchFire_WS = 2.55;
		}
	}
	// Cells perpendicular to wind direction - no wind effects
	else if (checkSameDirection(new_x, new_y, current_x, current_y)) {
		if (slope > 30)
			probToCatchFire_WS = 23.32;
		else if ((slope > 15) & (slope <= 30))
			probToCatchFire_WS = 6.10;
		else if ((slope >= 0) & (slope <= 15))
			probToCatchFire_WS = 5.26;
		else if ((slope > -15) & (slope < 0))
			probToCatchFire_WS = 4.9;
		else if ((slope > -30) & (slope < -15))
			probToCatchFire_WS = 5.4;
		else if (slope < -30)
			probToCatchFire_WS = 2.55;
	}

	return probToCatchFire_WS;
}

/***
MULTIPYING FACTORS FOR FIRE SPREAD BASED ON VEGETATION TYPE
Reference:
Chapter 4 - "Fire hazard and flammability of European forest types" in the book "Post-fire management and restoration of southern European forests".
Xanthopoulos, Gavriil, Carlo Calfapietra, and Paulo Fernandes,
Springer Netherlands (2012): 79-92.
***/
float ofApp::vegetationEffect(float probFromSlopeWind) {
	float probFromSlopeWindVeg = 0;
	if (vegType == "Evergreen") {
		probFromSlopeWindVeg = probFromSlopeWind;
	}
	else if (vegType == "Grassland") {
		probFromSlopeWindVeg = probFromSlopeWind * 1.4 ;
	}
	else if (vegType == "Pinus") {
		probFromSlopeWindVeg = probFromSlopeWind * 1.8;
	}
	
	return probFromSlopeWindVeg;
}

// Function that applies wind, slope and vegetation effects
bool ofApp::Effects(float new_x, float new_y, float current_x, float current_y, float slope) {
	float prob = windAndSlopeEffects(new_x, new_y, current_x, current_y, slope);
	prob = vegetationEffect(prob);

	float randomVal = ofRandom(0, 100);
	bool CatchFire = (randomVal < prob);

	return CatchFire;
}

// Returns true if the new cell is in heading direction from the current cell; else false
bool ofApp::headingDirection(float new_x, float new_y, float current_x, float current_y) {
	if (windDirection == "East")
		return (new_x > current_x);
	else if (windDirection == "West")
		return (new_x < current_x);
	else if (windDirection == "North")
		return (new_y < current_y);
	else if (windDirection == "South")
		return (new_y > current_y);
}

// Returns true if the new cell is in backing direction from the current cell; else false
bool ofApp::backingDirection(float new_x, float new_y, float current_x, float current_y) {
	if (windDirection == "East")
		return (new_x < current_x);
	else if (windDirection == "West")
		return (new_x > current_x);
	else if (windDirection == "North")
		return (new_y > current_y);
	else if (windDirection == "South")
		return (new_y < current_y);
}

// Returns true if the new cell is perpendicular to the direction of wind with respect to the current cell; else false
bool ofApp::checkSameDirection(float new_x, float new_y, float current_x, float current_y) {
	if (windDirection == "East" || windDirection == "West") {
		return (new_x == current_x);
	}
	else if (windDirection == "North" || windDirection == "South") {
		return (new_y == current_y);
	}
}

// Marker for showing the staring location on the display
void ofApp::addMarker(int x, int y) {
	//cout << "\nInside ofApp::addMarker function";

	if (setMarkerLocation(kinectROI, false)) {
		addNewMarker(StartXSlider, StartYSlider);
	}
}

// Set the location of the marker and checks if marker is on water
bool ofApp::setMarkerLocation(ofRectangle area, bool liveInWater) {
	//cout << "\nInside ofApp::setMarkerLocation function";

	float x = StartXSlider;
	float y = StartYSlider;
	bool insideWater = kinectProjector->elevationAtKinectCoord(x, y) < 0;
	if ((insideWater && liveInWater) || (!insideWater && !liveInWater)) {
		StartX = x;
		StartY = y;
	}
	else {
		cout << "\nCannot start fire on water!!";
		StartX = kinectROI.getLeft();
		StartY = kinectROI.getTop();
	}
	return true;
}

// add a new marker
void ofApp::addNewMarker(float x, float y) {
	//cout << "\nInside ofApp::addNewMarker function";
	Markers.clear();
	ofVec2f location = ofVec2f(x, y);
	cout << "\n" << (kinectProjector->elevationAtKinectCoord(x, y));
	auto m = Marker(kinectProjector, location, kinectROI, motherFire);
	m.setup();
	Markers.push_back(m);
}

// Adds a house at a random position on land
void ofApp::addHouse() {
	//cout << "\nInside ofApp::addHouse function";
	float gridForHouse[31][31][2] = {0};
	ofVec2f location;
	bool HouseSet = false;
	while (!HouseSet) {
		float x = ofRandom((kinectROI.getLeft() + 10), (kinectROI.getRight() - 10)); // +/-10 so that the house is not put on the borders
		float y = ofRandom((kinectROI.getTop() + 10), (kinectROI.getBottom() - 10));
		bool HouseInWater = false;
		//to create grid of locations surrounding the house
		for (int i = -15; (i <= 15) & (!HouseInWater); i++) {
			for (int j = -15; (j <= 15) & (!HouseInWater); j++) {
					gridForHouse[i+15][j+15][0] = x + i;
					gridForHouse[i+15][j+15][1] = y + j;
					if (kinectProjector->elevationAtKinectCoord(gridForHouse[i+15][j+15][0], gridForHouse[i+15][j+15][1]) < 0)
						HouseInWater = true;
			}
		}
		if (!HouseInWater ) {
			location = ofVec2f(x, y);
			HouseSet = true;
		}
	}
	auto h = House(kinectProjector, location, kinectROI, motherFire);
	h.setup();
	Houses.push_back(h);
	burnHouse = false;
}

// Adds a house with barrier at the location of the house (thus converting house to house with barrier)
bool ofApp::addHouseWithBarrier(float h_x, float h_y) {
	//cout << "\nInside addHouseWithBarrier() function";

	ofVec2f location = ofVec2f(h_x, h_y);
	
	auto h = houseWithBarrier(kinectProjector, location, kinectROI, motherFire);
	h.setup();
	HousesWithBarrier.push_back(h);

	//set value of 3 in the 2D grid for locations of the house with barrier
	int house_x = (h.getLocation().x - kinectROI.getLeft()) / 2;
	int house_y = (h.getLocation().y - kinectROI.getLeft()) / 2;
	for (int i = -15; i <= 15; i++) {
		for (int j = -15; j <= 15; j++) {
			grid[i + house_x][j + house_y] = 3;
		}
	}
	return true;
}

// Adds a fireman at a random location on an unburnt/extinguished cell on land
void ofApp::addFireman() {
	//cout << "\nInside ofApp::addFireman function";
	FiremanSet = false;
	
	// set a random location for fireman
	ofVec2f location;
	while (!FiremanSet) {
		float x = ofRandom((kinectROI.getLeft() + 10), (kinectROI.getRight() - 10)); // +/-10 so that the Fireman is not put on the borders
		float y = ofRandom((kinectROI.getTop() + 10), (kinectROI.getBottom() - 10));
		int temp_x = (x - kinectROI.getLeft()) / 2;
		int temp_y = (y - kinectROI.getLeft()) / 2;

		bool FiremanInWater = false;
		bool FiremanOnFire = false;
		if (kinectProjector->elevationAtKinectCoord(x, y) < 0)
			FiremanInWater = true;
		if (grid[temp_x][temp_y] == 1)
			FiremanOnFire = true;
		if (!FiremanInWater & !FiremanOnFire) {
			location = ofVec2f(x, y);
			FiremanSet = true;
		}
	}
	auto fm = Fireman(kinectProjector, location, kinectROI, motherFire);
	fm.setup();
	Firemen.push_back(fm);
}

void ofApp::onToggleEvent(ofxDatGuiToggleEvent e) {
}

void ofApp::keyReleased(int key) {
	//cout << "\nInside ofApp::keyReleased function";
}

void ofApp::mouseMoved(int x, int y) {
	//cout << "\nInside ofApp::mouseMoved function";
}

void ofApp::mouseDragged(int x, int y, int button) {
	//cout << "\nInside ofApp::mouseDragged function";
}

void ofApp::mousePressed(int x, int y, int button) {
	//cout << "\nInside ofApp::mousePressed function";
}

void ofApp::mouseReleased(int x, int y, int button) {
	//cout << "\nInside ofApp::mouseReleased function";
}

void ofApp::mouseEntered(int x, int y) {
	//cout << "\nInside ofApp::mouseEntered function";
}

void ofApp::mouseExited(int x, int y) {
	//cout << "\nInside ofApp::mouseExited function";
}

void ofApp::windowResized(int w, int h) {
	//cout << "\nInside ofApp::windowResized function";
}

void ofApp::gotMessage(ofMessage msg) {
	//cout << "\nInside ofApp::gotMessage function";
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
	//cout << "\nInside ofApp::dragEvent function";
}