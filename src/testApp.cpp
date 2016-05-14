#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){
    ofBackground(100);
    //ofRotate(-45,1,0,0);

    // OSC receiver - listening port
    cout << "listening for osc messages on port " << PORT << "\n";
    receiver.setup(PORT);

    // Chargement du fichier SVG
    svgPiedGauche.load("piedGauche.svg");
    svgPiedDroite.load("piedDroite.svg");

    // Chargement des parametres du Etherdream
    etherdream.setup();
    etherdream.setPPS(40000);

    // VideoGrabber
    vidGrabber.setVerbose(true);
    vidGrabber.setup(640,480);

    colorImg.allocate(640,480);
    grayImage.allocate(640,480);
    grayBg.allocate(640,480);
    grayDiff.allocate(640,480);

    bLearnBakground = true;
    threshold = 80;

}

//--------------------------------------------------------------
void testApp::update(){

    bool bNewFrame = false;
    
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();

    if (bNewFrame){


        colorImg.setFromPixels(vidGrabber.getPixels());

        grayImage = colorImg;
        if (bLearnBakground == true){
            grayBg = grayImage;     // the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }

        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);

        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinder.findContours(grayDiff, 300, 100000, 10, false);    // find holes

        for(int i = 0; i < contourFinder.nBlobs; i++){
            contourFinder.blobs[i].boundingRect.getCenter().x;
            contourFinder.blobs[i].boundingRect.getCenter().y;
        }

    }


    oscRCV();

    // Si le laser est innactif, on remet tout à zero
    if(!laserEstActif){
        posX = posY = angle = 0;
    }
    // Sinon, les traces se font seulement si toutes les coordonnées sont présentes. 
    else if(posX && posY && angle){

        ildaFrame.clear();                                  // Effacer le cadre
        polySVG.clear();                                    // Effacer les residuts
        // Pied gauche ou pied droit
        switch(gaucheDroite){
            case 0:svgReload(svgPiedDroite); break;
            case 1:svgReload(svgPiedGauche); break;
        }  
        ildaFrame.addPoly(polySVG);                         // Charger le ofPolyline
    }
}



//--------------------------------------------------------------
void testApp::draw() {

    // Afficher la capture
    ofSetHexColor(0xffffff);
    grayDiff.draw(0,0);
    stringstream blobInfo;


    for(int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(0,0);
        ofDrawBitmapString(i, 
        contourFinder.blobs[i].boundingRect.getCenter().x,
        contourFinder.blobs[i].boundingRect.getCenter().y);

    }

    // finally, a report:
    ofSetHexColor(0x333333);
    stringstream reportStr;
    reportStr << "bg subtraction and blob detection" << endl
              << "press ' ' to capture bg" << endl
              << "threshold " << threshold << " (press: +/-)" << endl
              << "num blobs found " << contourFinder.nBlobs << ", fps: " << ofGetFrameRate();
    ofDrawBitmapString(reportStr.str(), 20, 200);
    
    // do your thang
    ildaFrame.update();

    // draw to the screen
    ildaFrame.draw(0, 0, ofGetWidth(), ofGetHeight());
    
    // Envoyer les coordonnes au EtherDream, seulement si le laser est actif
    if(laserEstActif) {
        etherdream.setPoints(ildaFrame);
    }

    ofSetColor(255);
    //ofDrawBitmapString(ildaFrame.getString(), 0, 30);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key) {
            // clear the frame
        case 'c': ildaFrame.clear(); break;
            // Calibration
        case 'C': ildaFrame.drawCalibration(); break;

            // adjust point count
        case '.': ildaFrame.polyProcessor.params.targetPointCount++; break;
        case ',': if(ildaFrame.polyProcessor.params.targetPointCount > 10) ildaFrame.polyProcessor.params.targetPointCount--; break;

        case ' ': bLearnBakground = true; break;
        case '+': threshold ++;
            if (threshold > 255) threshold = 255;
            break;
        case '-': threshold --;
            if (threshold < 0) threshold = 0;
            break;
        case 'm': modeBlobDetect ^= true;
    }
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    // draw a line to the mouse cursor (normalized coordinates) in the last poly created
    ildaFrame.getLastPoly().lineTo(x / (float)ofGetWidth(), y / (float)ofGetHeight());
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    // create a new poly in the ILDA frame
    ildaFrame.addPoly();
}

// ---------------------------------------------------------
// OSC Receiver
void testApp::oscRCV(){


    // hide old messages
    for(int i = 0; i < NUM_MSG_STRINGS; i++){
        if(timers[i] < ofGetElapsedTimef()){
            msg_strings[i] = "";
        }
    }

    // Tant qu'il y a des messages en attente
    while(receiver.hasWaitingMessages()){

        // Prochain message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        // Coordonnees X
        if(m.getAddress() == "/xCoord"){
            if(checkMargin(m.getArgAsFloat(0), 0.05,0.95)){
                posX = m.getArgAsFloat(0);                
            }
        }
        // Coordonnees Y
        else if(m.getAddress() == "/yCoord"){
            if(checkMargin(m.getArgAsFloat(0), 0.05,0.95)){
                posY = m.getArgAsFloat(0);                
            }

        }
        // Angle
        else if(m.getAddress() == "/angle" ){
            angle = m.getArgAsFloat(0);    
        }
        // Pied gauche, pied droite
        else if(m.getAddress() == "/gaucheDroite" ){
            gaucheDroite = m.getArgAsInt32(0);    
        }
        // GaucheDroite a partir de touchOSC pour tests sur android - ignorer
        else if(m.getAddress() == "/gaucheDroiteMobile" ){
            gaucheDroite = m.getArgAsFloat(0);    
        }
        // Laser est actif?
        else if(m.getAddress() == "/laserEstActif" ){
            laserEstActif = m.getArgAsInt32(0);    
        }
        // LaserEstActif a partir de touchOSC pour tests sur android - ignorer
        else if(m.getAddress() == "/laserEstActifMobile" ){
            laserEstActif = m.getArgAsFloat(0);    
        }
        else{
            // unrecognized message: display on the bottom of the screen
            string msg_string;
            msg_string = m.getAddress();
            msg_string += ": ";
            for(int i = 0; i < m.getNumArgs(); i++){
                // get the argument type
                msg_string += m.getArgTypeName(i);
                msg_string += ":";
                // display the argument - make sure we get the right type
                if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                    msg_string += ofToString(m.getArgAsInt32(i));
                }
                else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                    msg_string += ofToString(m.getArgAsFloat(i));
                }
                else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                    msg_string += m.getArgAsString(i);
                }
                else{
                    msg_string += "unknown";
                }
            }
            // add to the list of strings to display
            msg_strings[current_msg_string] = msg_string;
            timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
            current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
            // clear the next line
            msg_strings[current_msg_string] = "";
        }
    }
}
bool testApp::checkMargin(float val, float min, float max){

    return val>=min && val<=max;
}
void testApp::svgReload(ofxSVG svg){

    for (int i = 0;i < svg.getNumPath();i++){  
           
           ofPath p = svg.getPathAt(i);
           ofPoint points(posX, posY);                  // Position du svg dans la matrice

           // Charger les parametres du SVG
           p.scale(0.03, 0.05);                           // Taille
           p.rotate(angle, ofVec3f(0,0,1));             // Angle
           p.translate(points);                         // Position
           
           p.setPolyWindingMode(OF_POLY_WINDING_ODD);   // Euh'l sait pas
           vector<ofPolyline> vpl = p.getOutline();     // Generer les outlines

           for(int z = 0; z < vpl.size(); z++) {
                    ofPolyline pl = vpl[z];
                    vector<ofPoint> vp = pl.getVertices();
                    polySVG.addVertices(vp);
           }
    }
}
