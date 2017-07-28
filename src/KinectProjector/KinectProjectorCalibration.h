/***********************************************************************
KinectProjectorCalibration.cpp - KinectProjectorCalibration compute
the calibration of the kinect and projector.

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

#ifndef __Magic_Sand__Calibration__
#define __Magic_Sand__Calibration__

#include <iostream>
#include "ofMain.h"
#include "libs/dlib/matrix.h"
#include "libs/dlib/matrix/matrix_qr.h"


class ofxKinectProjectorToolkit
{
public:
    ofxKinectProjectorToolkit(ofVec2f projRes, ofVec2f kinectRes);
    
    void calibrate(vector<ofVec3f> pairsKinect,
                   vector<ofVec2f> pairsProjector);
    
    ofVec2f getProjectedPoint(ofVec3f worldPoint);
    ofMatrix4x4 getProjectionMatrix();
    vector<ofVec2f> getProjectedContour(vector<ofVec3f> *worldPoints);
    
    vector<double> getCalibration();
    
    bool loadCalibration(string path);
    bool saveCalibration(string path);
    
    bool isCalibrated() {return calibrated;}
    
private:
    
    dlib::matrix<double, 0, 11> A;
    dlib::matrix<double, 0, 1> y;
    dlib::matrix<double, 11, 1> x;
    
    ofMatrix4x4 projMatrice;
    
    bool calibrated;
	ofVec2f projRes;
	ofVec2f kinectRes;
};

#endif /* defined(__Magic_Sand__Calibration__) */

