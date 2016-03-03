#ifndef H
#define H
#include <SPI.h>

//enum _CP_HTTP_RESULT {
#define CP_HTTP_INVALID 0
#define CP_HTTP_OK 200
#define CP_HTTP_CREATED 201
#define CP_HTTP_MULTI_STATUS   207
#define CP_HTTP_BAD_REQUEST 400
#define CP_HTTP_UNAUTHORIZED 401
#define CP_HTTP_PAYMENT_REQUIRED 402
#define CP_HTTP_FORBIDDEN 403
#define CP_HTTP_NOT_FOUND 404
#define CP_HTTP_NOT_ALLOWED 405
#define CP_HTTP_NOT_ACCEPTABLE 406
#define CP_HTTP_SERVER_ERROR 500
#define CP_HTTP_NOT_IMPLEMENTED 501
#define CP_HTTP_BAD_GATEWAY 502
#define CP_HTTP_SERVICE_UNAVAILABLE 503
//typedef enum _CP_HTTP_RESULT CP_HTTP_RESULT;

enum _CP_ERR_CODE { CP_ERR_INTERNAL_ERROR = -1,
                    CP_ERR_OUT_OF_MEMORY = -2,
                    CP_ERR_NO_CONNECTION = -3,
                    CP_ERR_INVALID_PARAMETER = -4,
                    CP_ERR_INVALID_LOGIN = -5,
                    CP_ERR_HTTP = -6,
                    CP_ERR_RESPONSE_FAILED = -7,
                    CP_ERR_SKIP_HEADERS = -8
                  };
typedef enum _CP_ERR_CODE CP_ERR_CODE;

class HttpClient;
class Client;

class CloudPlugs {
  protected:
      Client& client;
      HttpClient* httpClient;
      String host;
      String id;
      String auth;
      boolean is_master;
      int http_res;
      CP_ERR_CODE err;
      
  public:
      CloudPlugs(Client& aClient, const char* host = "http://api.cloudplugs.com/iot/");
      ~CloudPlugs();
      
      /**
       Change the default base url.
      
       @param host Http or https url string.
       @return \c true if the new base url is set correctly, \c false otherwise.
      */
      boolean setHost(const char* host);
      
      /**
       Get the current base url.
      
       @return The base url.
      */
      const char* getHost();
      
      /**
       Return a human-readable string that describes the last error.
      
       @return A human-readable string that describes the last error.
      */
      int getLastHttpResult();      
      
      /**
       Return a human-readable string that describes the last error.
      
       @return A human-readable string that describes the last error.
      */
      CP_ERR_CODE getLastErrCode();
      
#ifdef CP_DEBUG
      /**
       Return a human-readable string that describes the last error.
      
       @return A human-readable string that describes the last error.
      */
      void getLastErrString(String& err);
      
      /**
       Return a human-readable string that describes the last error.
      
       @return A human-readable string that describes the last error.
      */
      void getLastHttpResultString(String& http);
#endif //CP_DEBUG
      
      /**
       Set the session authentication credentials.
      
       @param id A string containing the @ref details_PLUG_ID or the master email.
       @param pass A string containing the authentication code.
       @param is_master \c true for using master authentication in the session; \c false for using regular authentication.
       @return \c true if the authentication is set correctly, \c false otherwise.
      */
      boolean setAuth(const char* id, const char* pass, boolean is_master);
      
      /**
       Get the authentication id (@ref details_PLUG_ID or email) of the session.
      
       @param id The destination String where the authentication id will be written.
       @return \c true if the buffer was correctly written, \c false otherwise.
      */
      boolean getAuthId(String& id);
      
      /**
       Get the authentication password of the session.
      
       @param password The destination String where the authentication password will be written.
       @return \c true if the buffer was correctly written, \c false otherwise.
      */
      boolean getAuthPass(String& password);
      
      /**
       Return the authentication mode in the session.
      
       @return \c true if using master authentication in the session; \c false if using regular authentication.
      */
      boolean isAuthMaster();
      
      /**
       Request for enrolling a new production device and place the response in *result and *result_length.
      
       @param body A buffer containing a json object like this:\n
        {\n
        \b "model" : @ref details_PLUG_ID,	// the model of this device\n
        \b "hwid"  : @ref details_HWID,		// the serial number\n
        \b "pass"  : String,		\n
        \b "props" : JSON		// optional, to initialize the custom properties\n
        }
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean enrollProduct(const char* body,  String& result);
      
      /**
       Request for enrolling a prototype and place the response in *result and *result_length.
      
       @param body A buffer containing a json object like this:\n
        {\n
        \b "hwid"  : @ref details_HWID,		// optional, if absent it will be set as a random unique string\n
        \b "pass"  : String,		// optional, if absent set as the X-Plug-Master of the company\n
        \b "name"  : String,\n
        \b "perm"  : @ref details_PERM_FILTER,	// optional, if absent permit all\n
        \b "props" : JSON		// optional, to initialize the custom properties\n
        }
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean enrollPrototype(const char* body, String& result);
      
      /**
       Request for enrolling a new or already existent controller device and place the response in *result and *result_length.
      
       @param body A buffer containing a json object like this:\n
       {\n
        \b "model" : @ref details_PLUG_ID,	// model id of the device to control\n
        \b "ctrl"  : @ref details_HWID,	// serial number (hwid) of the device to control\n
        \b "pass"  : String,\n
        \b "hwid"  : @ref details_HWID,	// unique string to identify this controller device\n
        \b "name"  : String	//  the name of this device\n
        }
       @param result If not NULL, then / A pointer such that - *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean enrollCtrl(const char* body, String& result);
      
      /**
       Request for controlling a device and place the response in *result and *result_length.
      
       @param body A buffer containing a json object like this:\n
      {\n
        \b "model" : @ref details_PLUG_ID,	// model id of the device to control\n
        \b "ctrl"  : @ref details_HWID,	// serial number (hwid) of the device to control\n
        \b "pass"  : String,\n
        }
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean controlDevice(const char* body, String& result);
      
      /**
       Request for uncontrol a device and [optionally] place the response in *result and *result_length.
      

       @param plugid If NULL, then is the @ref details_PLUG_ID in the session.
       @param plugid_controlled If not NULL, then the device(s) to uncontroll (default all associated devices).
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean uncontrolDevice(const char* plugid, const char* plugid_controlled, String& result);
      
      /**
       Request for reading a device and place the response in *result and *result_length.
      
       @param plugid The @ref details_PLUG_ID of the device.
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean getDevice(const char* plugid, String& result);
      
      /**
       Request for reading the device properties  and place the response in *result and *result_length.
      
       @param plugid The @ref details_PLUG_ID of the device.
       @param prop If NULL, then all properties value; otherwise the single property value.
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean getDeviceProp(const char* plugid, const char* prop, String& result);
      
      /**
       Request for writing or deleting device properties and [optionally] place the response in *result and *result_length.
      
       @param plugid The @ref details_PLUG_ID of the device.
       @param prop If NULL, then value must be an object; otherwise the single property value is written.
       @param value A json value, use null to delete one or all device properties.
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean setDeviceProp(const char* plugid, const char* prop, const char* value, String& result);

       /**
       Request for deleting device property and [optionally] place the response in *result and *result_length.
      
       @param plugid The @ref details_PLUG_ID of the device.
       @param prop The single property value to be remove.
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean removeDeviceProp(const char* plugid, const char* prop, String& result);
            
      /**
       Request for modifying device and [optionally] place the response in *result and *result_length.
      
       @param plugid The @ref details_PLUG_ID of the device.
       @param value A json object like this:\n
        {\n
        \b "perm"   : @ref details_PERM_FILTER,	// optional, it contains just the sharing filters to modify\n
        \b "name"   : String,		// optional\n
        \b "status" : @ref details_STATUS,		// optional\n
        \b "props"  : Object		// optional, it contains just the properties to modify\n
        }
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean setDevice(const char* plugid, const char* value, String& result);
      
      /**
       Request for removing any device (development, product or controller) and [optionally] place the response in *result and *result_length.
      
       @param plugid The @ref details_PLUG_ID_CSV of the device(s) to remove; if NULL then remove the device referenced in the session.
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean unenroll(const char* plugid, String& result);

      /**
       Request for retrieving list channels/channels about already published data and place the response in *result and *result_length.

       @param channel_mask The @ref details_CHMASK 
       @param query If not NULL, must be a url-encode string containing those values:\n
        before: Optional, @ref details_TIMESTAMP or @ref details_OBJECT_ID timestamp valid if greater than zero\n
        after: Optional, @ref details_TIMESTAMP or @ref details_OBJECT_ID timestamp valid if greater than zero\n
        at: Optional, @ref details_TIMESTAMP_CSV timestamp valid if greater than zero\n
        of: Optional, @ref details_PLUG_ID_CSV \n
        offset: Optional Number: positive integer (including 0) \n
        limit: Optional Number: positive integer (including 0)\n
       @param result A pointer such that *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return CP_SUCCESS if the request succeeds, CP_FAILED otherwise.
      */
      boolean getChannel(const char* channel_mask, const char* query, String& result);

      /**
       Request for retrieving already published data and place the response in *result and *result_length.
      
       @param channel_mask The @ref details_CHMASK 
       @param query If not NULL, must be a url-encode string containing those values:\n
        before: Optional, @ref details_TIMESTAMP or @ref details_OBJECT_ID timestamp valid if greater than zero\n
        after: Optional, @ref details_TIMESTAMP or @ref details_OBJECT_ID timestamp valid if greater than zero\n
        at: Optional, @ref details_TIMESTAMP_CSV timestamp valid if greater than zero\n
        of: Optional,@ref details_PLUG_ID_CSV \n
        offset: Optional Number: positive integer (including 0)  \n
        limit: Optional Number: positive integer (including 0)  \n
       @param result A pointer such that *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean retrieveData(const char* channel_mask, const char* query, String& result);
      
      /**
       Request for publishing data and [optionally] place the response in *result and *result_length.
      
       @param channel A optional @ref details_CHANNEL , if NULL data need to contain a couple "channel":"channel"
       @param body A json object or an array of objects like this:\n
        {\n
        \b "id"        : “PLUG_ID”,\n
        \b "channel"     : @ref details_CHANNEL,	// optional, to override the channel in the url\n
        \b "data"      : JSON,\n
        \b "at"        : @ref details_TIMESTAMP,\n
        \b "of"        : @ref details_PLUG_ID,	// optional, check if the X-Plug-Id is authorized for setting this field\n
        \b "is_priv"   : Boolean,	// optional, default false\n
        \b "expire_at" : @ref details_TIMESTAMP,	// optional, expire date of this data entry\n
        \b "ttl"       : Number		// optional, how many *seconds* this data entry will live (if "expire_at" is present, then this field is ignored)\n
        } 
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean publishData(const char* channel, const char* body, String& result);
      
      /**
       Request for deleting already published data and [optionally] place the response in *result and *result_length.
      
       @param channel_mask The @ref details_CHMASK
       @param body A buffer containing a json object like this (at least one param is required: id, before, after or at):\n
        {\n
            \b "id"     : @ref details_OBJECT_ID_CSV ,\n
            \b "before" : @ref details_TIMESTAMP or @ref details_PLUG_ID,\n
            \b "after"  : @ref details_TIMESTAMP or @ref details_PLUG_ID,\n
            \b "at"     : @ref details_TIMESTAMP_CSV ,\n
            \b "of"     : @ref details_PLUG_ID_CSV \n
        }
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean removeData(const char* channel_mask, const char* body, String& result);
      
      /**
       Request for writing or deleting device location and [optionally] place the response in *result and *result_length.
      
       @param plugid If not NULL, then the plug-id of the device, otherwise the device referenced in the session.
       @param value A json object like this: {\n
            "x" : LONGITUDE\n
            "y" : LATITUDE\n
            "z" : ALTITUDE //optional\n
            "r" : ACCURACY //optional\n
            "t" : TIMESTAMP\n
        }\n
        Use null to delete a location property.
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean setDeviceLocation(const char* plugid, const char* value, String& result);

      /**
       Request for writing or deleting device location and place the response in *result and *result_length.
      
       @param plugid If not NULL, then the @ref details_PLUG_ID  of the device, otherwise the device referenced in the session.
       @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
       @return \c true if the request succeeds, \c false otherwise.
      */
      boolean getDeviceLocation(const char* plugid, String& result);
      
  private:
  
      boolean request_exec(boolean send_auth, const char*  http_method, const char* path, const char* body, String* result);
};
#endif	// H


