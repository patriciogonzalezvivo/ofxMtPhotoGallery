//
//  ofxMtPhotoGallery.h
//  multiTouch Gallery
//
//  Created by Patricio González Vivo on 3/2/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com  All rights reserved.
//

#define USING_OFXTUIO
// Get it at https://github.com/arturoc/ofxTuioWrapper
// or you can use it just with the mouse
//
#ifdef USING_OFXTUIO
#include "ofxTuio.h"
#endif

#ifndef OFXMTPHOTOGALLERY
#define OFXMTPHOTOGALLERY

#include "ofxMtPhoto.h"

class ofxMtPhotoGallery{
public:
	ofxMtPhotoGallery();
    ~ofxMtPhotoGallery();
    
    ofxMtPhotoGallery& setPosition( int _x, int _y );
    ofxMtPhotoGallery& setSize(int _width, int _height);
    ofxMtPhotoGallery& setMode(State _state);  // Modes could be: VERTICAL_ALIGN / HORIZONTAL_ALIGN / FREE
    
    void loadDir(string _dirPath);
    void loadPhoto(string _filePath);
    void del( int _photo );
    
    void setSelected(int _photo);
    void pushToFront(int _photo);
    void moveFoward();
    void moveBackward();
    
    int  getSelected(){ return selected;};
    
    void repairLinks();
    void update();
    void draw();

    bool bDebug;
    
private:
    // Mouse Events from the testApp
    //
	void _mouseMoved(ofMouseEventArgs &e);
    void _mousePressed(ofMouseEventArgs &e);
    void _mouseDragged(ofMouseEventArgs &e);
    void _mouseReleased(ofMouseEventArgs &e);
    
#ifdef USING_OFXTUIO
	void tuioAdded(ofxTuioCursor & tuioCursor);
    void tuioRemoved(ofxTuioCursor & tuioCursor);
    void tuioUpdated(ofxTuioCursor & tuioCursor);
    myTuioClient tuioClient;
#endif
    
    gParams     params;
    
	vector<ofxMtPhoto*> photos;
    ofVec2f     mouseLast;
    
    float       speedThreshold;
    
    int         selected;       // Hold the selected photo 
    int         moveSelection;  // This is for the slider in order to know if a moving have to be done.
    
    bool        bActivity;
};
#endif
