/*
 *  ofxFlickr.h
 *  openFrameworks
 *
 *  Created by Todd Vanderlin on 11/22/09.
 *  Copyright 2009 Interactive Design. All rights reserved.
 *
 */

#pragma once
#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxFlickrTypes.h"
#include "hashlibpp.h"

// ------------------------------------------------
const string FLICKR_BASE_URL				 = "http://api.flickr.com/services/rest/?method=";
const string FLICKR_GET_RECENT				 = "flickr.photos.getRecent";
const string FLICKR_PHOTO_SEARCH			 = "flickr.photos.search";
const string FLICKR_PHOTOS_GET_WITH_GEO_DATA = "flickr.photos.getWithGeoData";


const string FLICKR_API_KEY					 = "api_key=";
const string FLICKR_API_SIG					 = "api_sig=";

const string FLICKR_PER_PAGE				 = "per_page=";
const string FLICKR_PAGE					 = "page=";
const string FLICKR_TAGS					 = "tags=";

// Sorting
const string FLICKR_SORT_DATE_DESC			 = "date-posted-desc";
const string FLICKR_SORT_DATE_ASC			 = "date-taken-asc";
const string FLICKR_SORT_TAKEN_DESC			 = "date-taken-desc";
const string FLICKR_SORT_TAKEN_ASC			 = "date-posted-asc";
const string FLICKR_SORT_INTERESTING_DESC    = "interestingness-desc";
const string FLICKR_SORT_INTERESTING_ASC     = "interestingness-asc";
const string FLICKR_SORT_RELEVANCE			 = "relevance";

const string FLICKR_SEARCH_ADD				 = "AND";
const string FLICKR_SEARCH_OR				 = "OR";


// Auth
const string FLICKR_AUTH_GET_FROB			 = "flickr.auth.getFrob";
const string FLICKR_AUTH_GET_TOKEN			 = "flickr.auth.getToken";

enum {
	FLICKR_METHOD_GET_RECENT_PHOTOS,	// get most recent public photos
	FLICKR_METHOD_PHOTO_SEARCH,			// basic flickr search
	FLICKR_METHOD_AUTH_FROB,			// get frob
	FLICKR_METHOD_AUTH_GET_TOKEN,		// login for authentication
	FLICKR_METHOD_PHOTO_UPLOAD			// upload photo
};


struct KeyValuePair {
    string key, value;
    KeyValuePair(string k="", string v="") {
        key = k; value = v;
    }
};

// ------------------------------------------------
class ofxFlickr {

protected:
	
	ofxXmlSettings	xml;
	bool			bAllocated;
	string			API_KEY;
	string			API_SECRET;
	string			API_SIG;
	
    // uploading
	string			AUTH_FROB;
	string			AUTH_TOKEN;
	
	void parseXML(ofxFlickrResponse &res, int method);
	void loadURL(string url, int method);
    bool loading;
    
public:
	
	ofxFlickr();
	~ofxFlickr();

	// ----------------------------
	void flickrResponse(ofxFlickrResponse &response);
    bool isLoading() { return loading; }
	// ----------------------------
	void setup(string key, string secret);
	void setApiKey(string key);
	
	ofEvent <ofxFlickrResponse> FLICKR_RESPONSE;
	
	// ----------------------------
	void getRecentPhotos(int num, int page=1);
	
	void tagSearch(string tags, string tag_mode, string sort_mode, int num=10, int page=1);
	void photoSearch(string tags, 
					 string tag_mode	= FLICKR_SEARCH_ADD, 
					 string text		= "", 
					 string sort_mode   = FLICKR_SORT_RELEVANCE,
					 int num			= 10, 
					 int page			= 1,
					 string extra		= "");
	
    // utils
    string convertStringToMD5(string str);
    string getSignedRequest(string method, string params);
    
	// authorizing
	void getFrob();
	void requestToken();
	void launchLoginLink();
	void authorize();
	void enableAuthoriztion();
	void saveToken();
    void loadToken();
    void checkToken();
    
    
    
	// uploading
	void uploadPhoto();
	string getAuthToken() { return AUTH_TOKEN; }
    string getAPIKey() { return API_KEY; }
    string getSecret() { return API_SECRET; }
};


