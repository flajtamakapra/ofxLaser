#include "testApp.h"
#include "ofxIldaFrame.h"


//--------------------------------------------------------------
void testApp::setup(){
    ofBackground(100);

    // OSC receiver - listening port
    cout << "listening for osc messages on port " << PORT << "\n";
    receiver.setup(PORT);

    // Chargement du fichier SVG
    svg.load("foot_small.svg");
    
    etherdream.setup();
    etherdream.setPPS(20000);
}

//--------------------------------------------------------------
void testApp::update(){

    oscRCV();

    // Si le laser est innactif, on remet tout à zero
    if(!laserEstActif){
        posX = posY = angle = 0;
    }
    // Sinon, les traces se font seulement si toutes les coordonnées sont présentes. 
    else if(posX && posY && angle){
        ildaFrame.clear();      // Effacer le cadre
        polySVG.clear();        // Effacer le SVG
        ildaFrame.params.output.transform.offset.x = posX;  // positions x,y
        ildaFrame.params.output.transform.offset.y = posY;
        svgReload(angle);       // Charger le SVG avec l'angle courant  
        ildaFrame.addPoly(polySVG); // Charger le ofPolyline
    }
}



//--------------------------------------------------------------
void testApp::draw() {
    // do your thang
    ildaFrame.update();

    // draw to the screen
    ildaFrame.draw(0, 0, ofGetWidth(), ofGetHeight());
    
    // send points to the etherdream, seulement si une entree est disponible
    if(laserEstActif) {
        etherdream.setPoints(ildaFrame);
    }
    
    ofSetColor(255);
    ofDrawBitmapString(ildaFrame.getString(), 10, 30);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key) {
        case 'f': ofToggleFullscreen(); break;
            
            
            // clear the frame
        case 'c': ildaFrame.clear(); break;
            
            // draw rectangle
        case 'r': {
            ildaFrame.clear();
            polySVG.clear();
            float angle = ofRandom(0, 180);
            ildaFrame.params.output.transform.offset.x = ofRandom(0.2, 0.8); 
            ildaFrame.params.output.transform.offset.y = ofRandom(0.2, 0.8);
            svgReload(angle);
            ildaFrame.addPoly(polySVG);
        }
            break;


            // toggle draw lines (on screen only)
        case 'l': ildaFrame.params.draw.lines ^= true; break;
            
            // toggle loop for last poly
        case 'o': ildaFrame.getLastPoly().setClosed(ildaFrame.getLastPoly().isClosed()); break;
            
            // toggle draw points (on screen only)
        case 'p': ildaFrame.params.draw.points ^= true; break;
            
            // adjust point count
        case '.': ildaFrame.polyProcessor.params.targetPointCount++; break;
        case ',': if(ildaFrame.polyProcessor.params.targetPointCount > 10) ildaFrame.polyProcessor.params.targetPointCount--; break;

            // adjust point count quicker
        case '>': ildaFrame.polyProcessor.params.targetPointCount += 10; break;
        case '<': if(ildaFrame.polyProcessor.params.targetPointCount > 20) ildaFrame.polyProcessor.params.targetPointCount -= 10; break;
            
            // flip image
        case 'x': ildaFrame.params.output.transform.doFlipX ^= true; break;
        case 'y': ildaFrame.params.output.transform.doFlipY ^= true; break;

            // cap image
        case 'X': ildaFrame.params.output.doCapX ^= true; break;
        case 'Y': ildaFrame.params.output.doCapY ^= true; break;

            // move output around
        case OF_KEY_UP: ildaFrame.params.output.transform.offset.y -= 0.05; break;
        case OF_KEY_DOWN: ildaFrame.params.output.transform.offset.y += 0.05; break;
        case OF_KEY_LEFT: ildaFrame.params.output.transform.offset.x -= 0.05; break;
        case OF_KEY_RIGHT: ildaFrame.params.output.transform.offset.x += 0.05; break;

            
            // scale output
        case 'w': ildaFrame.params.output.transform.scale.y += 0.05; break;
        case 's': ildaFrame.params.output.transform.scale.y -= 0.05; break;
        case 'a': ildaFrame.params.output.transform.scale.x -= 0.05; break;
        case 'd': ildaFrame.params.output.transform.scale.x += 0.05; break;
            
        case 'C': ildaFrame.drawCalibration(); break;
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
    //ildaFrame.params.output.transform.offset.y = mouseY; 
    //ildaFrame.params.output.transform.offset.x = mouseX;
    ildaFrame.addPoly();
}

void testApp::oscRCV(){
    // ---------------------------------------------------------
    // OSC Receiver
    // hide old messages
    for(int i = 0; i < NUM_MSG_STRINGS; i++){
        if(timers[i] < ofGetElapsedTimef()){
            msg_strings[i] = "";
        }
    }

    // check for waiting messages
    while(receiver.hasWaitingMessages()){

        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        // Check for x coord message
        if(m.getAddress() == "/xCoord"){
            posX = m.getArgAsFloat(0);
        }
        // Check for y coord message
        else if(m.getAddress() == "/yCoord"){
            posY = m.getArgAsFloat(0);         
        }
        // Check for angle message
        else if(m.getAddress() == "/angle" ){
            angle = m.getArgAsFloat(0);    
        }
        // Check for gaucheDroite message
        else if(m.getAddress() == "/gaucheDroite" ){
            gaucheDroite = m.getArgAsInt32(0);    
        }
        // Check for onOff message
        else if(m.getAddress() == "/laserEstActif" ){
            laserEstActif = m.getArgAsInt32(0);    
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
    // End OSC receiver
    // ---------------------------------------------------------
}
void testApp::svgReload(float angle){

    for (int i = 0;i < svg.getNumPath();i++){  
           ofPath p = svg.getPathAt(i);
           p.scale(0.1, 0.1);
           p.setPolyWindingMode(OF_POLY_WINDING_ODD);  
           p.rotate(angle, ofVec3f(0,0,1));
           vector<ofPolyline> vpl = p.getOutline(); // Here!  
           // And if you want vertices:  
           for(int z = 0; z < vpl.size(); z++) {

                    ofPolyline pl = vpl[z];
                    vector<ofPoint> vp = pl.getVertices();
                    polySVG.addVertices(vp);
           }
    }
}
