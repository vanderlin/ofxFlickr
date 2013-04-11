/*
 *  ofxFlickrTypes.h
 *  openFrameworks
 *
 *  Created by Todd Vanderlin on 11/23/09.
 *  Copyright 2009 Interactive Design. All rights reserved.
 *
 */

#pragma once
#include "ofMain.h"


// ------------------------------------------------
class ofxFlickrPhoto {

	public:
	
	ofxFlickrPhoto() {
	}
	
	string squareURL;	// 75 x 75
	string thumbURL;	// 100 x X
	string mediumURL;	// 240 x X
	string largeURL;	// 500 x X
	string originalURL;	// orginal size
	
	string title;
	string owner;
};


// ------------------------------------------------
class ofxFlickrResponse : public ofEventArgs {
public:
	
    int                     type;
	string                  content;
	vector <ofxFlickrPhoto> photos;
};
