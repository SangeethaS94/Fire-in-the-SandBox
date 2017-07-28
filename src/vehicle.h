/***********************************************************************
vehicle.h - vehicle class

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
#include "ofxOpenCv.h"
#include "ofxCv.h"

#include "KinectProjector/KinectProjector.h"

//Parent class for all the objects used in the application
class Vehicle{

public:
    Vehicle(std::shared_ptr<KinectProjector> const& k, ofPoint slocation, ofRectangle sborders, bool sliveInWater, ofVec2f motherLocation);
    
    // Virtual functions
    virtual void setup() = 0;
    virtual void draw() = 0;
    
    void update();
    
    const ofPoint& getLocation() const {
        return location;
    }
    
protected:    
    std::shared_ptr<KinectProjector> kinectProjector;

    ofPoint location; //location of the vehicle
    float angle; // direction of the drawing
 
    bool mother;
    ofVec2f motherLocation;
    
    bool liveInWater; // false for Fires which must stay on the ground
    
    ofVec2f projectorCoord;
    ofRectangle borders, internalBorders;
    int r, minborderDist;
};

class Fire : public Vehicle {
public:
    Fire(std::shared_ptr<KinectProjector> const& k, ofPoint slocation, ofRectangle sborders, ofVec2f motherLocation) : Vehicle(k, slocation, sborders, false, motherLocation){}
    
    void setup();
    void draw();
	int fuel;
	int numberOfBurningNeighbours;	
};

class Marker : public Vehicle {
public:
	Marker(std::shared_ptr<KinectProjector> const& k, ofPoint slocation, ofRectangle sborders, ofVec2f motherLocation) : Vehicle(k, slocation, sborders, false, motherLocation) {}

	void setup();
	void draw();
};

class House : public Vehicle {
public:
	House(std::shared_ptr<KinectProjector> const& k, ofPoint slocation, ofRectangle sborders, ofVec2f motherLocation) : Vehicle(k, slocation, sborders, false, motherLocation) {}

	void setup();
	void draw();
	ofImage image;
	bool burningState;
};

class houseWithBarrier : public Vehicle {
public:
	houseWithBarrier(std::shared_ptr<KinectProjector> const& k, ofPoint slocation, ofRectangle sborders, ofVec2f motherLocation) : Vehicle(k, slocation, sborders, false, motherLocation) {}

	void setup();
	void draw();
	ofImage image;
};

class Fireman : public Vehicle {
public:
	Fireman(std::shared_ptr<KinectProjector> const& k, ofPoint slocation, ofRectangle sborders, ofVec2f motherLocation) : Vehicle(k, slocation, sborders, false, motherLocation) {}

	void setup();
	void draw();
	void moveFireman(int moveDirection);
	ofImage image;
	string lookDirection;
};