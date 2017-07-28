/***********************************************************************
Main.cpp

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

#include "ofMain.h"
#include "ofApp.h"

bool setSecondWindowDimensions(ofGLFWWindowSettings& settings) {
	//cout << "\nInside SecondWindowDimensions function";
	// Check screens size and location
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	cout << "Number of screens found: " << count << endl;
	if (count>1) {
		int xM; int yM;
		glfwGetMonitorPos(monitors[1], &xM, &yM); // We take the second monitor
		const GLFWvidmode * desktopMode = glfwGetVideoMode(monitors[1]);

		settings.width = desktopMode->width;
		settings.height = desktopMode->height;
		settings.setPosition(ofVec2f(xM, yM));
		return true;
	} else {
		settings.width = 800; // Default settings if there is only one screen
		settings.height = 600;
		settings.setPosition(ofVec2f(0, 0));
		return false;
	}
}

//========================================================================
int main() {
	//cout << "\nInside main function";
	ofGLFWWindowSettings settings;
	settings.width = 1200;
	settings.height = 600;
	settings.resizable = true;
	settings.decorated = true;
	settings.title = "Magic Sand";
	shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
	mainWindow->setWindowPosition(ofGetScreenWidth() / 2 - settings.width / 2, ofGetScreenHeight() / 2 - settings.height / 2);

	setSecondWindowDimensions(settings);
	settings.resizable = false;
	settings.decorated = false;
	settings.shareContextWith = mainWindow;
	shared_ptr<ofAppBaseWindow> secondWindow = ofCreateWindow(settings);
	secondWindow->setVerticalSync(false);

	shared_ptr<ofApp> mainApp(new ofApp);
	ofAddListener(secondWindow->events().draw, mainApp.get(), &ofApp::drawProjWindow);
	mainApp->projWindow = secondWindow;
		
	ofRunApp(mainWindow, mainApp);
	ofRunMainLoop();
}
