# Fire in the SandBox
**Fire in the SandBox** is a software for operating an augmented reality sandbox. This was developed as a result of the study project **Fire in the SandBox** at the Institute for Geoinformatics (IFGI), University of Muenster, Germany by Master of Geoinformatics students Charu Manivannan, Mina Karamesouti, Sangeetha Shankar and Zhihao Liu guided by Jun. Prof. Dr. Judith Verstegen. It is based on the [Magic-Sand](https://github.com/thomwolf/Magic-Sand) software by Thomas Wolf. The aim of the project was to reuse the Magic Sand code by Thomas Wolf to simulate Forest Fire Spread.

*Last modified on July 28, 2017.*

## Main Features

The software runs on a computer connected to a home cinema projector and a kinect sensor. The software reads the elevation of the sand in the SandBox using kinect and obtains the Digital Evelation Model (DEM). It projects the output back onto the Sand using the home cinema projector.

The **Fire Spread Model** implemented in the software is a basic model that incorporates the effect of topography, wind speed, wind direction and selected vegetation types (Evergreen forest, Grassland, Pinus Forest) on forest fire spread. The topography can be varied by reshaping the sand in the SandBox. Other parameters can be chosen in the user interface. The primary use of the model is to educate students on the effect of various parameters on fire spread.

A simple game is also implemented in the software. The user is a Fireman. The goal of the game is to save a house places randomly in the forest before it gets burnt by the forest fire. The fireman can be controlled using the arrow keys. Once the fireman reaches the house, the house turns into a house with barrier, which means that the firemen are protecting the house and the area around it and hence fire cannot enter the zone protected by the fireman. (Fireman does not extinguish active fires in the current version). Alternately, the house can also be saved by digging trenches in the sand so that the fire is not able to cross the trench.

## Main differences with Magic Sand software by Thomas Wolf:
- The rabbit-fish model in the Magic Sand has been replaced with a fire spread model
- The game of finding the mother rabbit/fish has been replaced with a game of saving a house from the fire.

## Dependencies
Fire in the SandBox is based on [openframeworks](openframeworks.cc/) release 0.9.8 and makes use of the following addons:
- official addons (included in openframeworks 0.9.8)
  * ofxOpenCv
  * ofxKinect
  * ofxXmlSettings
- community addons:
  * [ofxCv - stable branch](https://github.com/kylemcdonald/ofxCv)
  * [ofxParagraph](https://github.com/braitsch/ofxParagraph)
  * [ofxDatGui](https://github.com/thomwolf/ofxDatGui)
  * [ofxModal](https://github.com/braitsch/ofxModal)

  
## Running the software
To run the Fire in the SandBox software, connect your computer to kinect and home cinema projector. 
Run the file *bin/Fire-in-the-SandBox_debug.exe*. 
HAVE FUN!! :+1:

On the bottom right of the application is the main control options for the model.
- Set FrameRate: This slider (varies from 5 to 20) controls the speed of model execution, i.e., the number of timesteps that run in each second.
- Options for Wind: There are three options for wind
	* No Wind
	* Low Wind Speed
	* High Wind Speed
- Wind Direction: There are four options for wind direction
	* East to West
	* West to East
	* North to South
	* South to North
- Vegetation Type: There are three vegetation types
	* Evergreen Forest
	* Grassland
	* Pinus Forest
- Starting Point of Fire: The starting point of fire can be defined manually using the two sliders (X Coordinate and Y Coordinate) provided in the interface.
- Start: Button to start the running of the fire spread model
- Add a House: Button to add a house at a random location on the land
- Add a Fireman: Button to add a fireman at a random location on the land
- Reset: This button clears the display and resets the model

(Note: The buttons *Add a House* and *Add a Fireman* clicked several times to change the position of the house or fireman)
(Rest of the interface is the same as Magic Sand software)


## Quick start for editing the source code
- Download [openframeworks](http://openframeworks.cc/download/) for your OS.
- Install all the addons listed above.
- Install the drivers needed for the kinect and the projector. The easiest way is to use [Zadig](http://zadig.akeo.ie/) 

### A quick look into the code
All other files except **ofApp.h**, **ofApp.cpp**, **vehicle.h** and **vehicle.cpp** remain the same as in Magic SandBox

vehicle.cpp consists of a parent class `Vehicle` that contains all common attributes and methods for all the other vehicles (agents) in the software.
Four classes are derived from `Vehicle`: `Fire`, `House`, `HouseWithBarrier`, `Fireman`. The images for the agents are stored in the folder *bin/data*. These images are read into the code as instances of the class `ofImage` and drawn when the model runs.

The fire spread model is an agent-based model. Several fire agents (instances of the class `Fire`) are created as the model runs, based on the topography, wind speed, wind direction and vegetation type. The spreading of fire is done by the `spreadFire()` function in ofApp.h. The function `windAndSlopeEffects()` and `vegetationEffects()` give the probability of a fire instance being created on an empty location. The 2D array variable `grid` in ofApp.h keeps track of the locations where fire instances have already been created. This prevents the creation of duplicates on the same location and improves performance.

For the game, the function `addHouse()` adds a house at a random location on land and the function `addFireman()` adds a fireman at a random location. In the `update()` function in ofApp.h, the position of the fireman and house are constantly checked. When the location of the fireman and the house coincide, the function `addHouseWithBarrier()` runs and puts a HouseWithBarrier in the same location as the house. For the fireman, the function `moveFireman()` in vehicle.h controls the movement of the fireman on the sandbox.

## Connection to reality
We have attempted to scale the model to reality. The estimated scales are as follows:
- Horizontal Scale: 1:2550
- Vertical Scale: 1:1040
- Time Scale: 1:1800 (1 second = 30 minutes) (This scale applies when FrameRate is 1. Scale to be adjusted based on the chosed FrameRate)

However, these values are approximate and hence should be used with caution.

## References:
- Source for Slope and Wind Effects:
	**A Qualitative comparison of fire spread models incorporating wind and slope effects**, Weise and Biging, *Forest Science 43.2* (1997): 170 - 180
- Source for Vegetation Effects:
	**Chapter 4 - "Fire hazard and flammability of European forest types" in the book "Post-fire management and restoration of southern European forests".** Xanthopoulos, Gavriil, Carlo Calfapietra, and Paulo Fernandes, *Springer Netherlands* (2012): 79-92.
