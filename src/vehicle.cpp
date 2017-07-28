/***********************************************************************
vehicle.cpp - vehicle class (Fire in the sandbox)

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

#include "vehicle.h"

//==============================================================
//Parent class Vehicle
//==============================================================

Vehicle::Vehicle(std::shared_ptr<KinectProjector> const& k, ofPoint slocation, ofRectangle sborders, bool sliveInWater, ofVec2f smotherLocation) {
	//cout << "\nInside vehicle constructor";
	kinectProjector = k;
    liveInWater = sliveInWater;
    location = slocation;
    borders = sborders;
    angle = 0;
    mother = false;
    motherLocation = smotherLocation;
}

//Converts Kinect Coordinate to Projector Coordinate
void Vehicle::update(){
	//cout << "\nInside Vehicle::update function";
    projectorCoord = kinectProjector->kinectCoordToProjCoord(location.x, location.y);
}

//==============================================================
// Derived class Fire
//==============================================================

void Fire::setup(){
	//cout << "\nInside Fire:setup function";
	fuel = 50;						//number of timesteps the fire would burn
	numberOfBurningNeighbours = 0;	//counter to keep track of the number of burning neighbours
}

void Fire::draw()
{
	//cout << "\nInside Fire:draw function";

	ofPushMatrix();
	ofTranslate(projectorCoord);
	ofRotate(angle);

    // Fire scale
    float sc = 1;
    
    ofFill();
    ofSetLineWidth(1.0);  // Line widths apply to polylines
	
	ofColor c1;
	//the fire instances are made semi-transparent so that the DEM below is also visible
	if(fuel<=0){
		c1 = ofColor(0, 0, 0,200);	//Black color for extinguished fires
	}
	else{
		c1 = ofColor(255, 0, 0, 175);	//Orange color for burning fires
	}
    
    ofPath body;
    body.curveTo( ofPoint(-2*sc, 2*sc));
    body.curveTo( ofPoint(-2*sc, 2*sc));
    body.curveTo( ofPoint(2*sc, 2*sc));
    body.curveTo( ofPoint(2*sc, -2*sc));
    body.curveTo( ofPoint(-2*sc, -2*sc));
    body.curveTo( ofPoint(-2*sc, -2*sc));
    body.close();
    ofSetColor(c1);
    body.setFillColor(c1);
    body.draw();
    
    ofSetColor(255);
    ofNoFill();
	
    ofPopMatrix();
}

//==============================================================
// Derived class Marker
// (Marker indicates the starting point of fire)
//==============================================================

void Marker::setup() {
	//cout << "\nInside Marker:setup function";
}

void Marker::draw()
{
	ofPushMatrix();
	ofTranslate(projectorCoord);
	ofRotate(angle);
	
	// Fire scale
	float sc = 1;

	ofFill();
	ofSetLineWidth(1.0);  // Line widths apply to polylines

	ofColor c1;
	c1 = ofColor(255, 0, 0);	//Red color

	ofPath body;
	body.curveTo(ofPoint(-5 * sc, 5 * sc));
	body.curveTo(ofPoint(-5 * sc, 5 * sc));
	body.curveTo(ofPoint(5 * sc, 5 * sc));
	body.curveTo(ofPoint(5 * sc, -5 * sc));
	body.curveTo(ofPoint(-5 * sc, -5 * sc));
	body.curveTo(ofPoint(-5 * sc, -5 * sc));
	body.close();
	ofSetColor(c1);
	body.setFillColor(c1);
	body.draw();

	ofSetColor(255);
	ofNoFill();
	
	ofPopMatrix();
}

//==============================================================
// Derived class House 
// (an ordinary house that burns when the fire reaches it)
//==============================================================

void House::setup() {
	//cout << "\nInside House:setup function";
	image.load("house.png");	//the image is vertically flipped so that it looks correct in the Sandbox
	burningState = false;		//indicates whether the house has caught fire or not
}

void House::draw()
{	
	ofPushMatrix();
	ofTranslate(projectorCoord);

	ofSetColor(255, 255, 255);
	image.draw(-15, -15, 30, 30);
	//x,y = -half(w,h) => So that the center of the image corresponds to the location of the House
	ofPopMatrix();
}

//==============================================================
// Derived class House With Barrier
// (A house with barrier is protected by Firemen and hence the house and the area around it are protected from burning)
//==============================================================

void houseWithBarrier::setup() {
	//cout << "\nInside housewithBarrier:setup function";
	image.load("houseWithBarrier.png");	//the image is vertically flipped so that it looks correct in the Sandbox
}

void houseWithBarrier::draw()
{
	ofPushMatrix();
	ofTranslate(projectorCoord);

	ofSetColor(255, 255, 255);
	image.draw(-15, -15, 30, 30);
	//x,y = -half(w,h) => So that the center of the image corresponds to the location of the House
	ofPopMatrix();
}

//==============================================================
// Derived class Fireman
//==============================================================

void Fireman::setup() {
	//cout << "\nInside Marker:setup function";
	image.load("fireman_right.png"); //the image is vertically flipped so that it looks correct in the Sandbox
}

void Fireman::draw()//, std::vector<ofVec2f> forces)
{
	ofPushMatrix();
	ofTranslate(projectorCoord);

	ofSetColor(255, 255, 255);
	image.draw(-20, -10, 40, 20);
	//x,y = -half(w,h) => So that the center of the image corresponds to the location of the House
	ofPopMatrix();
}

/*****
Fireman moves faster on flat and downhill areas and slower on uphill areas
Fireman can swim
The multiplication factors for elevation change (10) and location increment/decrement (2) are set by trial and error
*****/
void Fireman::moveFireman(int moveDirection) {
	float elevationAtCurrentCell = kinectProjector->elevationAtKinectCoord(location.x,location.y);

	if (moveDirection == OF_KEY_UP) {
		float elevationAtNewCell = kinectProjector->elevationAtKinectCoord(location.x, (location.y)+1);
		float elevationChange = (elevationAtNewCell - elevationAtCurrentCell)*10; //+ve : uphill; -ve: downhill
		if(elevationChange>1)
			location.y = location.y + (1/elevationChange)*2;
		else
			location.y = location.y + 1;
	}
	else if (moveDirection == OF_KEY_DOWN) {
		float elevationAtNewCell = kinectProjector->elevationAtKinectCoord(location.x, (location.y) - 1);
		float elevationChange = (elevationAtNewCell - elevationAtCurrentCell)*10; //+ve : uphill; -ve: downhill
		if (elevationChange>1)
			location.y = location.y - (1 / elevationChange)*2;
		else
			location.y = location.y - 1;
	}
	else if (moveDirection == OF_KEY_LEFT) {
		if (lookDirection != "left") {
			image.load("fireman_left.png");
			lookDirection == "left";
		}
		float elevationAtNewCell = kinectProjector->elevationAtKinectCoord((location.x)+1, location.y);
		float elevationChange = (elevationAtNewCell - elevationAtCurrentCell)*10; //+ve : uphill; -ve: downhill
		if (elevationChange>1)
			location.x = location.x + (1 / elevationChange)*2;
		else
			location.x = location.x+ 1;
	}
	else if (moveDirection == OF_KEY_RIGHT) {
		if (lookDirection != "right") {
			image.load("fireman_right.png");
			lookDirection == "right";
		}
		float elevationAtNewCell = kinectProjector->elevationAtKinectCoord((location.x) - 1, location.y);
		float elevationChange = (elevationAtNewCell - elevationAtCurrentCell)*10; //+ve : uphill; -ve: downhill
		if (elevationChange>1)
			location.x = location.x - (1 / elevationChange)*2;
		else
			location.x = location.x - 1;
	}
	//left and right are inverted on the sandbox
}