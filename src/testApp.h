#pragma once

#include "ofMain.h"
#include "ofxEtherdream.h"
#include "ofxGui.h"
#include "ofxSvg.h"
#include "ofxOsc.h"
#include "ofxIldaFrame.h"
#include "ofxOpenCv.h"

// OSC listen
#define PORT 12345
#define NUM_MSG_STRINGS 20

class testApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed  (int key);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void oscRCV();
    void svgReload(ofxSVG svg);
    bool checkMargin(float, float, float);

    // Video
    ofVideoGrabber          vidGrabber;
    ofxCvColorImage         colorImg;

    ofxCvGrayscaleImage     grayImage;
    ofxCvGrayscaleImage     grayBg;
    ofxCvGrayscaleImage     grayPrevFrame;  // Test frame precedent
    ofxCvGrayscaleImage     grayDiff;

    ofxCvContourFinder      contourFinder;

    int     threshold;
    bool    bLearnBakground;
    bool    modeBlobDetect;                 // false:background subtraction, true: Movement Detection


    // SVG
	ofxSVG svgPiedGauche;
    ofxSVG svgPiedDroite;
    //ofxSVG svg;

    vector<ofPolyline> outlines;
    ofPolyline polySVG;

    //Osc Receiver
    ofxOscReceiver receiver;
    int current_msg_string;
    string msg_strings[NUM_MSG_STRINGS];
    float timers[NUM_MSG_STRINGS];

    // Parametres de pas
    float posX, posY, angle;
    int gaucheDroite;
    int laserEstActif = 0;

    // OfxEtherdream    
    ofxIlda::Frame ildaFrame;   // stores and manages ILDA frame drawings
    ofxEtherdream etherdream;   // interface to the etherdream device
};
