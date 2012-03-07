/*
 *  ofxMtPhoto.cpp
 *  multiTouch
 *
 *  Created by Patricio González Vivo on 21/09/11.
 *  Copyright 2011 http://www.PatricioGonzalezVivo.com  All rights reserved.
 *
 */

#include "ofxMtPhoto.h"

ofxMtPhoto::ofxMtPhoto(){
	angle = 0;
	pos.set(0,0);
	imgSize.set(0,0);
    
	scale	= 1;
    
    state = FREE;
    
    backLooking = true;
    
    prev = NULL;
    next = NULL;
    gallery = NULL;
}

void ofxMtPhoto::resize(float _resize){
	scale *= _resize;
}

ofxMtPhoto& ofxMtPhoto::setImage(string _imageFile){
	img.loadImage(_imageFile);
    imgSize.set( img.getWidth(), img.getHeight() );
}

void ofxMtPhoto::update(){
    
	if (state == FREE){
        
        // Just take care that pictures don´t go aways from the gallery
        //
        //pos.x = ofClamp(pos.x, galleryPos->x, gallerySize->x);
        //pos.y = ofClamp(pos.y, galleryPos->y, gallerySize->y);
        //
        if  (( pos.x < gallery->x ) ||
             ( pos.y < gallery->y ) ||
             ( pos.x > (gallery->x + gallery->width) ) ||
             ( pos.y > (gallery->y + gallery->height) )   ){
                 ofVec2f direction = ofVec2f( gallery->x, gallery->y) + 
                                     ofVec2f( gallery->width, gallery->height)*0.5 + 
                                     ofVec2f( ofRandom(-gallery->width*0.3,gallery->width*0.3), 
                                              ofRandom(-gallery->height*0.3,gallery->height*0.3) );
                 direction -= pos;
                 move( direction );
            cout << "move inside " << endl;
             }
    } else if (state == CENTER){
        
        // Scale the picture until it get it original size or achive the size of the gallery
        // 
        if ((getScaledHeight() < img.getHeight()) && 
            (getScaledHeight() < gallery->height * gallery->selectedScale ) && 
            (getScaledWidth() < gallery->width * gallery->selectedScale ) )
            scale += 0.01;
        
        // Adds a force to the center of the gallery in order to get centered
        //
        ofVec2f direction = ofVec2f(gallery->x, gallery->y) + 
                            ofVec2f(gallery->width, gallery->height) * 0.5;
        direction -= pos;
        move( direction );
        
    } else if (state == HORIZONTAL_ALIGN){
        
        // Scales the picture to thumbSize 
        // 
        if ( getScaledHeight() >= gallery->thumbHeight )
            scale -= 0.01;
        
        // If it have a previus photo it auto align with it. 
        // So you just need to move the first picture in order to move them all
        //
        if ( backLooking ) {
            if (prev != NULL){
                ofVec2f direction = prev->pos;
                direction.x += prev->getScaledWidth() * 0.5 + 10 + getScaledWidth() * 0.5;
                direction.y = gallery->y + gallery->height *0.5;
                direction -= pos;
                move(direction);
            }
        } else { 
            if (next != NULL){
                ofVec2f direction = next->pos;
                direction.x -= next->getScaledWidth() * 0.5 + 10 + getScaledWidth() * 0.5;
                direction.y = gallery->y + gallery->height *0.5;
                direction -= pos;
                move(direction);
            }
        }
        
    } else if ( state == VERTICAL_ALIGN){
        
        // Scales the picture to thumbSize 
        // 
        if ( getScaledWidth() >= gallery->thumbWidth )
            scale -= 0.01;
        
        // If it have a previus photo it auto align with it. 
        // So you just need to move the first picture in order to move them all
        //
        if ( backLooking ){ 
            if (prev != NULL){
                ofVec2f direction = prev->pos;
                direction.x = gallery->x + gallery->width *0.5;
                direction.y += prev->getScaledHeight() * 0.5 + 10 + getScaledHeight() * 0.5;
                direction -= pos;
                move(direction);
            }
        } else {
            if (next != NULL){
                ofVec2f direction = next->pos;
                direction.x = gallery->x + gallery->width *0.5;
                direction.y -= next->getScaledHeight() * 0.5 + 10 + getScaledHeight() * 0.5;
                direction -= pos;
                move(direction);
            }
        }
    }
    
    // Do the physics
    //
	vel += acc;
	vel *= gallery->resistence;
	pos += vel;
	acc *= 0;
}

void ofxMtPhoto::draw(){
	ofEnableAlphaBlending();
	
	ofPushMatrix();
	ofTranslate(pos.x, pos.y);
	ofScale(scale, scale, 1);
	ofRotateZ(ofRadToDeg(angle));
		
	ofSetColor(255);
	ofFill();
	
	img.draw(-imgSize.x*0.5,-imgSize.y*0.5,imgSize.x,imgSize.y);
		
	ofPopMatrix();
	ofDisableAlphaBlending();
}

// Chech if something is over a picture
//
bool ofxMtPhoto::isOver(ofVec2f _loc){
	bool over;
	
	ofVec2f dirToCenter = pos - _loc;
	float theta = atan2(dirToCenter.x,dirToCenter.y)-(PI/2);
	float r = dirToCenter.length();
	float x = r * cos(theta + angle);
	float y = r * sin(theta + angle);
	
	if ((x <=  imgSize.x * 0.5 * scale ) && 
        (x >= -imgSize.x * 0.5 * scale ) && 
        (y <=  imgSize.y * 0.5 * scale ) && 
        (y >= -imgSize.y * 0.5 * scale ) ) 
		over = true;
	else 
		over = false;
	
	return over;
}

#ifdef USING_OFXTUIO
// TUIO Events
// -------------------------------------------
//
void ofxMtPhoto::tuioAdded(ofxTuioCursor &tuioCursor){
    // Map the normalized cursor position to one that fits into the screen
    //
    ofVec2f loc = ofVec2f(tuioCursor.getX() * ofGetWidth(),
                          tuioCursor.getY() * ofGetHeight());
    
    if (isOver(loc)){
        tCursor c;
        c.idN = tuioCursor.getSessionId();
        c.loc = loc;
        
        // First finger over the photo (first trigger)
        //
        if (cursorsOnBorder.size() == 0){   
            cursorsOnBorder.push_back(c);
            oldLoc[0] = loc;
        } 
        // Second finger over the photo (second and finall trigger)
        //
        else if (cursorsOnBorder.size() == 1){
            cursorsOnBorder.push_back(c);
            oldLoc[0] = cursorsOnBorder[0].loc;
            oldLoc[1] = cursorsOnBorder[1].loc;
        }
        
    }
}

void ofxMtPhoto::tuioUpdated(ofxTuioCursor &tuioCursor){
    // Map the normalized cursor position to one that fits into the screen
    //
    ofVec2f loc = ofVec2f( tuioCursor.getX() * ofGetWidth(),
                          tuioCursor.getY() * ofGetHeight() );
    
    if ( isOver(loc) ){
        
        // Store the position of the fingers over the photo
        //
        for ( int i = 0; i < cursorsOnBorder.size(); i++)
            if (cursorsOnBorder[i].idN == tuioCursor.getSessionId())
                cursorsOnBorder[i].loc = loc;
        
        // GESTURES ON FREE MODE
        //
        if (state == FREE ){
            // If there is just one finger over an image...
            // ...it moves it
            //
            if (cursorsOnBorder.size() == 1 ){
                ofVec2f oldStaCursorToCenter = pos - oldLoc[0];
                pos = loc + oldStaCursorToCenter;
                oldLoc[0] = loc;
            
            } 
            
            // If there is two fingers over the image... 
            // ... lot´s of transformations will happend ( translate, rotate and scale )
            //
            else if (cursorsOnBorder.size() == 2){
                
                // First reduce the velocity that it have
                vel *= 0.5;
            
                // One of the two fingers it´s going to be the Static ( sta ) 
                // and the other the dinamic ( din )
                // Knowing witch is moving and witch not, it´s more easy to calculate 
                // the rotation.
                //
                int sta = -1;
                int din = -1;
            
                // So, first search for the static by looking to it position.
                // The other one it´s going to be the dinamic
                for ( int i = 0; i < cursorsOnBorder.size(); i++){
                    if (cursorsOnBorder[i].loc == oldLoc[i]) {
                        sta = i;
                    } else { 
                        din = i;
                    }
                }
            
        
                // Now it´s ready for making some calculations
                //
                if ((sta != -1) && (din != -1) && (din != sta)){
                    
                    // Remember the OLD direction, distance and rotation
                    // 
                    ofVec2f Di = oldLoc[sta] - oldLoc[din];
                    float di = Di.length();
                    float ri = -1*atan2(Di.x,Di.y)+(PI/2);
                
                    // Calculates the NEW direction, distance and rotation
                    //
                    ofVec2f Dii = oldLoc[sta] - cursorsOnBorder[din].loc;
                    float dii = Dii.length();
                    float rii = -1*atan2(Dii.x,Dii.y)+(PI/2);
                
                    // Calculates the amount (0.0 - 1.0) of re-scale between the OLD and NEW positions
                    //
                    float scaleF = dii / di;
                
                    // Calculates the rotation degree between the OLD and the NEW positions
                    //
                    float rotateF = rii - ri;
                
                    // Calculates the translation amount on polar coordenates ( this allow rotations at the same time )
                    //
                    ofVec2f oldStaCursorToCenter = pos - oldLoc[sta];
                    float oldAngleToCenter = -1*atan2(oldStaCursorToCenter.x,oldStaCursorToCenter.y)+(PI/2);
                    float oldRadioToCenter = oldStaCursorToCenter.length(); 
                
                    float newRadioToCenter = oldRadioToCenter * scaleF;
                    float newAngleToCenter = oldAngleToCenter + rotateF;
                    ofVec2f newStaCursorToCenter = ofVec2f(newRadioToCenter*cos(newAngleToCenter),newRadioToCenter*sin(newAngleToCenter));
                
                    // Update the values
                    //
                    resize(scaleF);                         
                    rotate(rotateF);
                    pos = oldLoc[sta] + newStaCursorToCenter;
                }
                
                // Store the finger positions for future calculations
                // 
                oldLoc[0] = cursorsOnBorder[0].loc;
                oldLoc[1] = cursorsOnBorder[1].loc;
            }
            
            
        }
    }
}
 

void ofxMtPhoto::tuioRemoved(ofxTuioCursor &tuioCursor){
    
    // Map the normalized cursor position to one that fits into the screen
    //
    ofVec2f loc = ofVec2f(tuioCursor.getX() * ofGetWidth(),
                          tuioCursor.getY() * ofGetHeight());
    
    // Removes the stored fingers positions over the photo
    //
    for (int i = 0; i < cursorsOnBorder.size(); i++ ){
        if (cursorsOnBorder[i].idN == tuioCursor.getSessionId()){
            /*
            if (state == FREE ){
                ofVec2f oldStaCursorToCenter = pos - oldLoc[0];
                pos = loc + oldStaCursorToCenter;
            
                if (cursorsOnBorder.size() == 1 ){
                    ofVec2f newForce = cursorsOnBorder[0].loc - oldLoc[0];
                    //newForce.normalize();
                    acc += newForce;
                }
            }*/
            cursorsOnBorder.erase(cursorsOnBorder.begin()+i);
        }
    }
}
#endif