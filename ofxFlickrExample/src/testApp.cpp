#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

    
	ofBackground(0, 0, 0);
	ofSetLogLevel(OF_LOG_ERROR);
	
    ofxXmlSettings apiSettings;
    apiSettings.loadFile("api.xml");
    string key = apiSettings.getValue("key", "no key");
    string secret = apiSettings.getValue("secret", "no secret");
    
	flickr.setup(key, secret);
	flickr.loadToken();
    ofAddListener(flickr.FLICKR_RESPONSE, this, &testApp::flickrResponse);
  
    
    searchStr = "openframeworks";
    flickr.photoSearch(searchStr, FLICKR_SEARCH_ADD, searchStr, FLICKR_SORT_RELEVANCE, 117, 1);

}


//--------------------------------------------------------------
void testApp::flickrResponse(ofxFlickrResponse &e) {

    if(e.type==FLICKR_METHOD_PHOTO_UPLOAD) {
        printf("Photo Uploaded %s\n", e.content.c_str());
    }
    else {
        for(int i=0; i<e.photos.size(); i++) {
            
            // Make a photo to load
            Photo p;
            p.img.setUseTexture(false);
            p.url = e.photos[i].squareURL;
            p.name = e.photos[i].title;
            p.bLoaded = false;
            p.bStarted = false;
            p.bTextureUploaded = false;
            p.bReady = false;
            photos.push_back(p);
            
        }
        
        if(isThreadRunning() == false) startThread(true, false);
    }
}



//-------------------------------------------------------------- 
void testApp::threadedFunction() {
	if (isThreadRunning() == true) {
		if(lock()) {
			
			bool bStopThread = true;
			
			for(int i=0; i<photos.size(); i++) {
				if(photos[i].bStarted == false) {
					printf("-- loading photo %s ---\n", photos[i].name.c_str());
					bool loaded = photos[i].img.loadImage(photos[i].url);
					photos[i].img.update();
					photos[i].bLoaded = loaded;
					photos[i].bStarted = true;
					bStopThread = false;
				}
			}
			
			if(bStopThread) {
                stopThread();	
			}
			
			ofSleepMillis(100);
			unlock();
		}
	
	}
	
}

//--------------------------------------------------------------
void testApp::update() {
	
	for(int i=0; i<photos.size(); i++) {
		if(photos[i].bTextureUploaded == false && photos[i].bLoaded == true) {
			
			photos[i].texture.allocate(photos[i].img.getWidth(), 
									   photos[i].img.getHeight(),
									   GL_RGB);
			photos[i].texture.loadData(photos[i].img.getPixels(),
									   photos[i].img.getWidth(), 
									   photos[i].img.getHeight(),
									   GL_RGB);
			photos[i].bTextureUploaded = true;
			photos[i].bReady           = true;
		}
	}

}

//--------------------------------------------------------------
void testApp::draw() {
	
    ofEnableAlphaBlending();

	float x  = 0;
	float y  = 70;
	int   mx = ofGetWidth()/75;
    ofPushMatrix();
    ofTranslate((ofGetWidth()-(mx*75))/2, 0);
	for(int i=0; i<photos.size(); i++) {
		
		if(photos[i].bReady) {
			ofSetColor(0xffffff);
			photos[i].texture.draw(x, y);
		}
		else {
            ofFill();
            ofSetColor(90, 10);
            ofRect(x, y, 75, 75);
            ofNoFill();
            ofSetColor(150, 100);
            ofRect(x, y, 75, 75);
            
            ofPushStyle();
            ofSetRectMode(OF_RECTMODE_CENTER);
            ofPushMatrix();
            ofTranslate(x+75/2, y+75/2);
            ofRotate(ofGetElapsedTimef()*100.0);
            ofSetColor(255);
            ofRect(0, 0, 15, 15);
            ofPopMatrix();
            ofPopStyle();
        }
		x += 75;
		if(i % mx == mx-1) {
			x = 0;
			y += 75;
		}
	}
    ofPopMatrix();
	
	ofSetColor(20, 20, 20, 150);
	ofFill();
	ofRect(0, 0, ofGetWidth(), 70);
	
    
    
	ofFill();
	ofSetColor(255);
	string info = "OF + FLickr Searching for: "+searchStr;
	if(flickr.isLoading()) {
		int c = (ofGetFrameNum()%2) * 255;
		ofSetColor(255-c, 255-c, 255-c);
		info += "\nLoading Flickr...";
	}
	ofDrawBitmapString(info, 50, 30);
	
	
}

//--------------------------------------------------------------
void testApp::keyPressed(int key) {
    
    // hit enter
    if(key == 13) {
        printf("Searching for: %s\n", searchStr.c_str());
        photos.clear();
        stopThread();
        photos.clear();
        flickr.photoSearch(searchStr, FLICKR_SEARCH_ADD, searchStr, FLICKR_SORT_RELEVANCE, 117, 1);
    }
    else if(key == OF_KEY_BACKSPACE||key == OF_KEY_DEL) {
        if(searchStr.size()>0)searchStr.erase(searchStr.end()-1);
    }
    else if(key >= 32 || key <= 126) {
        searchStr += (char)key;
    }
    /*
	if(key == '5') flickr.getRecentPhotos(117, 1);
	if(key == '6') flickr.photoSearch("", FLICKR_SEARCH_ADD, "green", FLICKR_SORT_RELEVANCE, 3, 1);
	if(key == '1') flickr.authorize();
	if(key == '2') flickr.enableAuthoriztion();
	if(key == '3') flickr.uploadPhoto();
    */
}

//--------------------------------------------------------------
void testApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

