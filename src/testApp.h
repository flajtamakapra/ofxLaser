#pragma once

#include "ofMain.h"
#include "ofxEtherdream.h"
#include "ofxGui.h"

class testApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed  (int key);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);


	ofParameter<float> radius;
	ofParameter<ofColor> color;
	ofParameter<ofVec2f> center;
	ofParameter<int> circleResolution;
	ofParameter<bool> filled;
	ofxButton twoCircles;
	ofxButton ringButton;
	ofParameter<string> screenSize;

	ofxPanel gui;
    
    
    ofxIlda::Frame ildaFrame;   // stores and manages ILDA frame drawings

    ofxEtherdream etherdream;   // interface to the etherdream device
};
