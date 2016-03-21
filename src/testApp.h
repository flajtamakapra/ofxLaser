#pragma once

#include "ofMain.h"
#include "ofxEtherdream.h"
#include "ofxGui.h"
#include "ofxSvg.h"
#include "ofxOsc.h"

// listen on port 12345
#define PORT 5050
#define NUM_MSG_STRINGS 20

class testApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed  (int key);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);

    // GUI
	ofParameter<float> radius;
	ofParameter<ofColor> color;
	ofParameter<ofVec2f> center;
	ofParameter<int> circleResolution;
	ofParameter<bool> filled;
	ofxButton twoCircles;
	ofxButton ringButton;
	ofParameter<string> screenSize;

	ofxPanel gui;

	// SVG
	ofxSVG svg;
    float step;
    vector<ofPolyline> outlines;
    ofPolyline polySVG;

    //Osc Receiver
    ofxOscReceiver receiver;
    int current_msg_string;
    string msg_strings[NUM_MSG_STRINGS];
    float timers[NUM_MSG_STRINGS];
    float posX, posY, angle;
    int gaucheDroite, laserEstActif;


    // OfxEtherdream    
    ofxIlda::Frame ildaFrame;   // stores and manages ILDA frame drawings

    ofxEtherdream etherdream;   // interface to the etherdream device
};
