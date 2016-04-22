#include "testApp.h"
#include "ofxIldaFrame.h"


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

        ildaFrame.clear();                                  // Effacer le cadre
        polySVG.clear();                                    // Effacer les residuts
        svgReload();                                        // Charger le SVG avec l'angle courant  
        ildaFrame.addPoly(polySVG);                         // Charger le ofPolyline
    }
}



//--------------------------------------------------------------
void testApp::draw() {
    // do your thang
    ildaFrame.update();

    // draw to the screen
    ildaFrame.draw(0, 0, ofGetWidth(), ofGetHeight());
    
    // Envoyer les coordonnes au EtherDream, seulement si le laser est actif
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
void testApp::svgReload(){

    // Pied gauche ou pied droit
    switch(gaucheDroite){
        case 0:svg = svgPiedDroite; break;
        case 1:svg = svgPiedGauche; break;
    }

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
