/*
 *  ofxMtPhoto.h
 *  multiTouch
 *
 *  Created by Patricio González Vivo on 21/09/11.
 *  Copyright 2011 http://www.PatricioGonzalezVivo.com All rights reserved.
 *
 * 
 */

#define USING_OFXTUIO
// If you are not planing to use this addon with ofxTuio 
// you can comment this and use it with the mouse
#ifdef USING_OFXTUIO
#include "ofxTuio.h"
#endif

#ifndef OFXMTPHOTO
#define OFXMTPHOTO

#include "ofMain.h"
#include <vector>

// tCursor´s are a local way of remebering where fingers are
// for each photo in order to make the proper calculations from
// the movements of them
//
struct tCursor {
	int	idN;
	ofVec2f loc;
};

enum State {
	FREE,
    VERTICAL_ALIGN,
    HORIZONTAL_ALIGN,
    CENTER
};

struct gParams {
	float   x,y,width,height, thumbWidth, thumbHeight, resistence, selectedScale;
    State   state;
};

class ofxMtPhoto{
public:
	ofxMtPhoto();
	
	ofxMtPhoto& setPos(ofVec2f _pos){pos = _pos; return * this; };
	ofxMtPhoto& setImage(string _imageFile);
    ofxMtPhoto& setState(State _state){state = _state;};
    
    ofxMtPhoto& setGalleryParams(gParams *_gallery){ gallery = _gallery; return * this;};
    
    void    rotate(float _angle){ angle += _angle;};
	void    resize(float _resize);
    void    move(ofVec2f _force){ acc += _force; };
    
    ofVec2f getPos(){ return pos; };
    float   getWidth(){ return imgSize.x; };
    float   getHeight(){ return imgSize.y; };
    float   getScaledWidth(){ return imgSize.x * scale; };
    float   getScaledHeight(){ return imgSize.y * scale; };
    
	void    update();
	void    draw();
	
	bool    isOver(ofVec2f _loc);
	bool    isOver(int _x, int _y){ return isOver(ofVec2f(_x,_y));};

#ifdef USING_OFXTUIO
	void tuioAdded(ofxTuioCursor & tuioCursor);
    void tuioRemoved(ofxTuioCursor & tuioCursor);
    void tuioUpdated(ofxTuioCursor & tuioCursor);
#endif
    
    ofxMtPhoto      *prev;
    ofxMtPhoto      *next;
    
    bool            backLooking;
    
private:
    ofImage         img;
    
    gParams         *gallery;
    
	vector<tCursor>	cursorsOnBorder;
	ofVec2f         oldLoc[3];
    
    ofVec2f         pos,vel,acc;
    ofVec2f         imgSize;
    
	float           angle, scale;
    
    State           state;
};

#endif