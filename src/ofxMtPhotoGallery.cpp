//
//  ofxMtPhotoGallery.cpp
//  multiTouch Gallery
//
//  Created by Patricio González Vivo on 3/2/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com All rights reserved.
//

#include "ofxMtPhotoGallery.h"

ofxMtPhotoGallery::ofxMtPhotoGallery(){
    
    // Link the core events in order to catch mouse and keys
    //
    ofAddListener(ofEvents().mouseMoved, this, &ofxMtPhotoGallery::_mouseMoved);
	ofAddListener(ofEvents().mousePressed, this, &ofxMtPhotoGallery::_mousePressed);
	ofAddListener(ofEvents().mouseDragged, this, &ofxMtPhotoGallery::_mouseDragged);
	ofAddListener(ofEvents().mouseReleased, this, &ofxMtPhotoGallery::_mouseReleased);
    
#ifdef USING_OFXTUIO
    // Add TUIO touch events
    ofAddListener(tuioClient.cursorAdded,this,&ofxMtPhotoGallery::tuioAdded);
    ofAddListener(tuioClient.cursorRemoved,this,&ofxMtPhotoGallery::tuioRemoved);
    ofAddListener(tuioClient.cursorUpdated,this,&ofxMtPhotoGallery::tuioUpdated);
    tuioClient.start(3333);
#endif
        
    // Defaul the position and size of the gallery it´s fixed
    // to the window parameters. Also the size of the thums it´s
    // a 1/3 of the windows.
    //
    params.x = 0;
    params.y = 0;
    params.width = ofGetWindowWidth();
    params.height = ofGetWindowHeight();
    params.thumbWidth = params.width * 0.3;
    params.thumbHeight = params.height * 0.3;
    params.selectedScale = 0.8;
    params.resistence = 0.1;
    params.state = VERTICAL_ALIGN;
    
    
    selected = 0;       
    moveSelection = 0;
    
	bDebug = false;
    bActivity = false;
}

ofxMtPhotoGallery::~ofxMtPhotoGallery(){
    
    // This prevent memory licks deleting stash memory allocation
    // one by one. And the the vector.
    //
	for(int i = 0; i < photos.size() ;i++)
		delete photos[i];
    
	photos.clear();
}

// Set the position of the gallery
//
ofxMtPhotoGallery& ofxMtPhotoGallery::setPosition(int _x, int _y) {
    params.x = _x;
    params.y = _y;
    
    return * this;
}

// Set the size of the gallery
//
ofxMtPhotoGallery& ofxMtPhotoGallery::setSize(int _width, int _height){
    params.width = _width;
    params.height = _height;
    params.thumbWidth = params.width * 0.3;
    params.thumbHeight = params.height * 0.3;
    
    return * this;
}

// Set´s the mode of the gallery. You can choose from
// VERTICAL_ALIGN and HORIZONTAL_ALIGN witch basicaly arrange the
// pictures on a vertical or horizontal sliders. Also the why
// of interaction it´s constrained on going forward or backwards
// On FREE mode acts as a banch of photos over a tables and pictures
// can be rotated, translated and resized
//
ofxMtPhotoGallery& ofxMtPhotoGallery::setMode(State _state){
    params.state = _state;
    
    if (photos.size() > 0){
        if ( params.state == VERTICAL_ALIGN){
            photos[0]->setPos(ofVec2f(params.x + params.width *0.5, params.width + 0));
        } else if (params.state == HORIZONTAL_ALIGN){
            photos[0]->setPos(ofVec2f(params.x+0, params.y+ params.height*0.5));
        }
    
        for(int i = 0; i < photos.size() ; i++){
            photos[i]->setState(_state);
            photos[i]->backLooking = true;
        }
    
        repairLinks();
    
        if ( params.state != FREE )
            setSelected(selected);
    }
}

// Load an entire directory of photos.
//
void ofxMtPhotoGallery::loadDir(string _dirPath){
	ofDirectory dir;
    
    // Allowed extensions
    //
    dir.allowExt("jpg");
    dir.allowExt("JPG");
	dir.allowExt("jpeg");
    dir.allowExt("JPEG");
    dir.allowExt("png");
    dir.allowExt("PNG");
    dir.allowExt("gif");
    dir.allowExt("GIF");
    
    // Open the directoy in order to load the pictures and know how much image have to load
    //
	int nPhotos = dir.listDir(_dirPath);
    
    // One by one declare them inside the array of pointers
    //
	for(int i = 0; i < nPhotos;i++){
        loadPhoto(dir.getPath(i));
    }
}

// Add one photo to the gallery
//
void ofxMtPhotoGallery::loadPhoto(string _filePath){
    ofxMtPhoto * newPhoto = new ofxMtPhoto();

    // Loading the images
    newPhoto->setImage(_filePath);    
    
    // Place it randomly on the gallery
    newPhoto->setPos(ofVec2f(ofRandom(params.x, params.width), ofRandom( params.y, params.height)));
    
    // Set the aligment style
    newPhoto->setState( params.state );
    
    // Link the thumbSize, galleryPos, and gallerySize parametters
    newPhoto->setGalleryParams( &params );
    
    // Link the newPhoto to the previus one and the last of the 
    // array to the newPhoto. This make double chain
    //
    if (photos.size() > 0) {
        newPhoto->prev = photos.back();
        photos.back()->next = newPhoto;   
    } else {
        newPhoto->prev = NULL;
    }
    
    // Then is pushed into the vector
    photos.push_back(newPhoto);
}

void ofxMtPhotoGallery::del( int _photoId){
    
    // Checks that it´s something to do here
    //
    if ( (_photoId >= 0 ) && (_photoId < photos.size() )){
        
        // Any time you need to delete a pointer to something on the 
        // stash, if you delete the pointer you lose te why to 
        // access to it, but the place continue allocated on memory
        // that´s why before deleting the pointer on the vector
        // you need to delete the memory allocated on the stash
        //
        delete photos[_photoId];
        
        photos.erase( photos.begin() + _photoId );
        
        repairLinks();
    }
}

void ofxMtPhotoGallery::pushToFront(int _photoId ){
    
    // Swap a ofxMtPhoto to the fron 
    // and rebuild relation ship between them
    //
	if ( (_photoId >= 0 ) && (_photoId < photos.size() )){
        
        // Store the place on memory where the photo is
        //
		ofxMtPhoto * tmp = photos[ _photoId ];
        
        // Move everthing one step backwards and
        //
		for (int i = _photoId ; i < photos.size()-1 ; i++){
			photos[i] = photos[i+1];
            
            if (i > 0){
                photos[i]->prev = photos[i-1];
                photos[i]->prev->next = photos[i];
            }
            
		}
		
        // Put the selected photo to the back of the array
        // and makes the links
        //
        photos.back() = tmp;
        photos.back()->prev = photos[ photos.size() - 2];
        photos.back()->prev->next = photos.back();
        
        // This fix the first and last elements on the array
        // Both have to be trated differently
        //
        photos[0]->prev = NULL;        
        photos.back()->next = NULL;
        
        //repairLinks();
	}
}

// This is handy if you mess arround with the links
// it runs all arround the photos fixing and settings the
// links between them
//
void ofxMtPhotoGallery::repairLinks(){
    
    // Checks that it´s something to do here
    //
    if ( photos.size() > 0){
        
        for (int i = 0 ; i < photos.size() ; i++){
            if (i > 0){
                photos[i]->prev = photos[i-1];
                photos[i]->prev->next = photos[i];
            }
        }
    
        photos[0]->prev = NULL;
        photos[ photos.size() -1]->next = NULL;
    }
}

void ofxMtPhotoGallery::setSelected(int _photoId){
    
    // First check that the selection it´s in the limits.
    // 
    if ((_photoId >= 0) && ( _photoId < photos.size()) ){
        
        // Make previus photos to look to the next one
        for (int i = 0; i < _photoId; i++ ){
            photos[i]->setState( params.state );
            photos[i]->backLooking = false;
        }

        selected = _photoId;
        photos[_photoId]->setState(CENTER);
        photos[_photoId]->backLooking = false;
    
        // Make next photos to look to the previus one
        for (int i = _photoId+1; i < photos.size() ; i++ ){
            photos[i]->setState( params.state );
            photos[i]->backLooking = true;
        }
    }
}

void ofxMtPhotoGallery::moveFoward(){
    
    // Checks that it´s something to do here
    //
    if (selected < photos.size())
        setSelected( selected+1 );

}

void ofxMtPhotoGallery::moveBackward(){
    
    // Checks that it´s something to do here
    //
    if (selected > 0)
        setSelected( selected-1 );
}


void ofxMtPhotoGallery::update(){    
#ifdef USING_OFXTUIO
    // Update the TUIO Client
    //
    tuioClient.getMessage();
#endif

    // If it´s on VERTICAL or HORIZONTAL align mode means that
    // that it´s working as a slide gallery. The interactions
    // in order to be smooth have to ocurre when the user finish 
    // it´s gesture of going forward or backwards, that´s way
    // use moveSelection in order if it have to move the focus
    // and in what direction
    //
    if (( (params.state == VERTICAL_ALIGN) || (params.state == HORIZONTAL_ALIGN) ) && 
        ( moveSelection != 0) ){
        if ( !bActivity 
#ifdef USING_OFXTUIO           
           && (tuioClient.getTuioCursors().size() == 0)
#endif        
            ){
            if ( moveSelection == -1)
                moveBackward();
            else if ( moveSelection == 1)
                moveFoward();
        
            moveSelection = 0;
        }
	}
    
    // Update the pictures. Basicaly the physics and the movements
    // of them. They are like particles and this update it´s what 
    // make the movements so organic and natural
    //
	for(int i = 0; i < photos.size(); i++){
		photos[i]->update();
	}
}

void ofxMtPhotoGallery::draw(){
    
    // Draw all the photos
    //
	for(int i = 0; i < photos.size(); i++){
		photos[i]->draw();
	}
	

	if (bDebug){
#ifdef USING_OFXTUIO
		tuioClient.drawCursors();
#endif
    }   
}

//Mouse Events
void ofxMtPhotoGallery::_mouseMoved(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);
    mouseLast = mouse;
}

void ofxMtPhotoGallery::_mousePressed(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);
    
    if ( params.state == FREE ){
        for(int i = photos.size() - 1; i >= 0; i--){
            if (photos[i]->isOver(mouse)){
                pushToFront(i);
                break;
            }
        }
    }
    mouseLast = mouse;
    bActivity  = true;
}

void ofxMtPhotoGallery::_mouseDragged(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);
    
    ofVec2f mouseVel = mouse - mouseLast;
    
    // On VERTICAL_ALIGN and HORIZONTAL_ALIGN the user
    // just can go forward or backward with a sliding gesture
    //
    if ( params.state == VERTICAL_ALIGN) {
        
        if ( (mouseVel.y <= -1) && ( moveSelection == 0) ){
            moveSelection = 1;
        } else if (mouseVel.y >= 1){
            moveSelection = -1;
        }
        
    } else if ( params.state == HORIZONTAL_ALIGN){
        
        if ( (mouseVel.x <= -1) && ( moveSelection == 0) ){
            moveSelection = 1;
        } else if ((mouseVel.x >= 1 ) && ( moveSelection == 0)){
            moveSelection = -1;
        }
        
    } else if ( params.state == FREE ){
        
        // FREE mode update it´s handled by each picture
        // there each picture see "if" and "how" to
        // translate, resize and rotate
        //
        for(int i = photos.size() - 1; i >= 0; i--){
            if ( photos[i]->isOver(mouse) ){
                photos[i]->setPos(mouse);
                break;
            }
        }
    }
    bActivity  = true;
}

void ofxMtPhotoGallery::_mouseReleased(ofMouseEventArgs &e){
    bActivity  = false;
}

#ifdef USING_OFXTUIO
// TUIO events
// ------------------------------------------------------
//
void ofxMtPhotoGallery::tuioAdded(ofxTuioCursor &tuioCursor){
    ofVec2f loc = ofVec2f(tuioCursor.getX()*ofGetWidth(),
                          tuioCursor.getY()*ofGetHeight());
    
    // If a picture it´s touch on FREE mode it push it to the
    // top
    //
    if ( params.state == FREE ){
        for(int i = photos.size() - 1; i >= 0; i--){
            if (photos[i]->isOver(loc)){
                photos[i]->tuioAdded(tuioCursor);
                pushToFront(i);
                break;
            }
        }
    }
}

void ofxMtPhotoGallery::tuioUpdated(ofxTuioCursor &tuioCursor){
    ofVec2f loc = ofVec2f(tuioCursor.getX()*ofGetWidth(),
                          tuioCursor.getY()*ofGetHeight());
    
    // On VERTICAL_ALIGN and HORIZONTAL_ALIGN the user
    // just can go forward or backward with a sliding gesture
    //
    if ( params.state == VERTICAL_ALIGN) {
        
        if ( (tuioCursor.getYSpeed() <= -1) && ( moveSelection == 0) ){
            moveSelection = 1;
        } else if (tuioCursor.getYSpeed() >= 1){
            moveSelection = -1;
        }
        
    } else if ( params.state == HORIZONTAL_ALIGN){
        
        if ( (tuioCursor.getXSpeed() <= -1) && ( moveSelection == 0) ){
            moveSelection = 1;
        } else if (( tuioCursor.getXSpeed() >= 1 ) && ( moveSelection == 0)){
            moveSelection = -1;
        }
        
    } else if ( params.state == FREE ){
        
        // FREE mode update it´s handled by each picture
        // there each picture see "if" and "how" to
        // translate, resize and rotate
        //
        for(int i = photos.size() - 1; i >= 0; i--){
            if ( photos[i]->isOver(loc) ){
                photos[i]->tuioUpdated(tuioCursor);
                break;
            }
        }
    }
}

void ofxMtPhotoGallery::tuioRemoved(ofxTuioCursor &tuioCursor){
    ofVec2f loc = ofVec2f( tuioCursor.getX() * ofGetWidth(),
                           tuioCursor.getY() * ofGetHeight());
    
    for(int i = 0; i < photos.size(); i++){
        photos[i]->tuioRemoved(tuioCursor);
    }
}
#endif