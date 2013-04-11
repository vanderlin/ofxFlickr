#pragma once
#include "ofMain.h"
#include "ofxFlickr.h"

typedef struct {
    ofImage img;
	ofTexture texture;
	bool bLoaded;
	bool bStarted;
	bool bTextureUploaded;
	bool bReady;
	string name;
	string url;
} Photo;

class testApp : public ofBaseApp, public ofThread {
	
public:
	
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	void flickrResponse(ofxFlickrResponse &e);
	void threadedFunction();
    string          searchStr;
	vector <Photo>	photos;
	ofxFlickr		flickr;
	
};
