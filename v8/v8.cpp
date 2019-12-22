/** 
 * File: v8.cpp 
 *
 * */

/** Copyright (c) 2019 Mesibo
 * https://mesibo.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the terms and condition mentioned
 * on https://mesibo.com as well as following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions, the following disclaimer and links to documentation and
 * source code repository.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of Mesibo nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific prior
 * written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Documentation
 * https://mesibo.com/documentation/on-premise/loadable-modules/ 
 *
 * Source Code Repository
 * http://github.com/mesibo/onpremise-loadable-modules
 * 
 * Skeleton Module
 * https://github.com/mesibo/onpremise-loadable-modules/skeleton
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "module.h"
#include <assert.h>
#include <iostream>
#include <stdarg.h>
#include <v8.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <include/libplatform/libplatform.h>

#include <map>
#include <string>

using std::map;
using std::pair;
using std::string;

using v8::Context;
using v8::EscapableHandleScope;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Global;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Name;
using v8::NamedPropertyHandlerConfiguration;
using v8::NewStringType;
using v8::Object;
using v8::ObjectTemplate;
using v8::PropertyCallbackInfo;
using v8::Script;
using v8::String;
using v8::TryCatch;
using v8::Value;

using std::cout;
using std::endl;

#define HTTP_BUFFER_LEN (64 * 1024)
#define MODULE_LOG_LEVEL_0VERRIDE 0

/** Message Parmeters **/
#define MESSAGE_AID 				"aid"
#define MESSAGE_ID 				"id"
#define MESSAGE_REFID 				"refid"
#define MESSAGE_GROUPID 			"groupid"
#define MESSAGE_FLAGS 				"flags"
#define MESSAGE_TYPE 				"type"
#define MESSAGE_EXPIRY 				"expiry"
#define MESSAGE_TO_ONLINE 			"to_online"
#define MESSAGE_TO 				"to"
#define MESSAGE_FROM 				"from"

/** HTTP options **/
#define HTTP_PROXY 				"proxy"
#define HTTP_CONTENT_TYPE 			"content_type"
#define HTTP_EXTRA_HEADER 			"extra_header"
#define HTTP_USER_AGENT 			"user_agent"
#define HTTP_REFERRER 				"referrer"
#define HTTP_ORIGIN 				"origin"
#define HTTP_COOKIE 				"cookie"
#define HTTP_ENCODING 				"encoding"
#define HTTP_CACHE_CONTROL 			"cache_control"
#define HTTP_ACCEPT 				"accept"
#define HTTP_ETAG 				"etag"
#define HTTP_IMS 				"ims"
#define HTTP_MAXREDIRECTS 			"maxredirects"
#define HTTP_CONN_TIMEOUT 			"conn_timeout"
#define HTTP_HEADER_TIMEOUT 			"header_timeout"
#define HTTP_BODY_TIMEOUT 			"body_timeout"
#define HTTP_TOTAL_TIMEOUT 			"total_timeout"
#define HTTP_RETRIES 				"retries"
#define HTTP_SYNCHRONOUS 			"synchronous"

/** Listener functions **/
#define MESIBO_LISTENER_ON_MESSAGE 		"Mesibo_onMessage"
#define MESIBO_LISTENER_ON_MESSAGE_STATUS 	"Mesibo_onMessageStatus"
#define MESIBO_LISTENER_ON_HTTP_RESPONSE 	"Mesibo_onHttpResponse"

/** Mesibo module Helper functions **/
#define MESIBO_MODULE_MESSAGE                   "mesibo_message"
#define MESIBO_MODULE_HTTP                      "mesibo_http"
#define MESIBO_MODULE_LOG 			"mesibo_log"  

typedef struct v8_config_s v8_config_t;
typedef struct http_context_s http_context_t;

class MesiboJsProcessor {
	public:
		MesiboJsProcessor(mesibo_module_t* mod, Isolate* isolate, const char* script, int log_level)
			:mod_(mod), isolate_(isolate), script_(script), log_(log_level), lastchanged_(0) {}
		virtual int Initialize();
		virtual ~MesiboJsProcessor() { };
		int ExecuteJsFunction(Local<Context>& context,
				const char* func_name, int argc, Local<Value> argv[]);

		//Callbacks to Javascript
		mesibo_int_t OnMessage(mesibo_message_params_t p, const char* message,
				mesibo_uint_t len);
		mesibo_int_t OnMessageStatus(mesibo_message_params_t p, mesibo_uint_t status);

		//Callables from Javscript
		static void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args); 
		static void MessageCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void HttpCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

		void SetCallables(Local<ObjectTemplate> & global);
		
		//Debug-Interface	
		int Log(const char* format, ...);

		//Module Configuration
		mesibo_module_t* mod_;
		const char* script_;//The /full/path/to/script
		int log_; //log-level

		//V8 Isolate & Context
		Isolate* GetIsolate() { return isolate_; }
		Local<Context> GetContext(); //Will return allocated context from base context 

	private:
		int ExecuteScript(Local<String> script, Local<ObjectTemplate> global);
		MaybeLocal<String> ReadFile(Isolate* isolate, const string& name);

		Isolate* isolate_;
		Global<Context> context_; //Load base context from here 
		int lastchanged_;

		
		//Utility methods for wrapping C++ objects as JavaScript objects,
		// and going back again.

		/*Messaging*/
		Local<Object> WrapMessageParams(Local<Context>& context , 
				mesibo_message_params_t* p);
		static mesibo_message_params_t UnwrapMessageParams(Isolate* isolate, 
				Local<Context> context, Local<Value> arg_params);

		/*Http*/
		static mesibo_http_option_t* UnwrapHttpOptions(Isolate* isolate, 
				Local<Context> context, Local<Value> arg_options);
		static http_context_t* BundleHttpCallback(Isolate* isolate, 
				Local<Context> context, Local<Value> js_cb, 
				Local<Value> js_cbdata, mesibo_module_t* mod);	

		int WrapParamUint(Local<Context>& context, Local<Object> &js_params, 
				const char* key, mesibo_uint_t value);
		static mesibo_uint_t UnwrapParamUint(Isolate* isolate, 
				Local<Context> context, const char* param_name, 
				Local<Object>params);

		int WrapParamString(Local<Context>& context, Local<Object> &js_params, 
				const char* key, const char* value);
		static std::string UnwrapParamString(Isolate* isolate, 
				Local<Context> context, const char* param_name,
				Local<Object>params);

		static Local<Value> GetParamValue(Isolate* isolate, 
				Local<Context> context, const char* param_name,
				Local<Object>params);


		static mesibo_uint_t UnwrapUint(Isolate* isolate,
				Local<Context>context, Local<Value>integer); 

		static std::string UnwrapString(Isolate* isolate, 
				Local<Context> context, Local<Value> byte_string);


};

/**
 * Sample V8 Module Configuration
 * Refer sample.conf
 */
struct v8_config_s{
	const char* script;
	int log; //log level
	long last_changed; // Time Stamp
	MesiboJsProcessor* ctx; // v8 context
	std::unique_ptr<v8::Platform> platform; //v8 Platform
};

struct http_context_s {
	char buffer[HTTP_BUFFER_LEN];
	int datalen;	
	MesiboJsProcessor* ctx_;
	mesibo_module_t* mod;

	Isolate* isolate;
	Local<Context> context;

	Local<Value> http_cb; //Js HTTP Callback function
	Local<Value> http_cbdata; 
};

int MesiboJsProcessor::Log(const char* format, ...) {
	if(log_>= 0){ //Log-Level Specified in Configuration
		va_list args;
		va_start(args, format);
		return mesibo_log(mod_, log_, format , args);
	}
	else
		return 0;
}

mesibo_uint_t MesiboJsProcessor::UnwrapUint(Isolate* isolate, Local<Context>context, 
		Local<Value>integer){ 

	double value;

	if(integer->IsNumber()){
		value = integer->NumberValue(context).ToChecked();
		return 	value;
	}
	return 0;
}

std::string MesiboJsProcessor::UnwrapString(Isolate* isolate, Local<Context> context, 
		Local<Value> byte_string ){

	if(byte_string->IsString() && !byte_string->IsNullOrUndefined()){
		String::Utf8Value value(isolate, byte_string);
		std::string str_value(*value);
		return str_value;
	}

	return NULL;
}

Local<Value> MesiboJsProcessor::GetParamValue(Isolate* isolate, Local<Context> context, 
		const char* param_name, Local<Object>params){

	// Create a handle scope to keep the temporary object references.
	EscapableHandleScope handle_scope(isolate);
	Context::Scope context_scope(context);

	Local<Value> p_key;
	Local<Value> p_value;

	p_key = String::NewFromUtf8(isolate, param_name).ToLocalChecked();
	p_value = params->Get(context, p_key).ToLocalChecked();

	return  handle_scope.Escape(p_value);	
}

mesibo_uint_t MesiboJsProcessor::UnwrapParamUint(Isolate* isolate, Local<Context> context,     
		const char* param_name, Local<Object>params){

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(isolate);

	Local<Value> value = GetParamValue(isolate, context, param_name, params);
	mesibo_uint_t uint_val;
	uint_val =  value->IsNullOrUndefined()? 0 : UnwrapUint(isolate, context, value);		

	return uint_val;
}

std::string  MesiboJsProcessor::UnwrapParamString(Isolate* isolate, Local<Context> context,     
		const char* param_name, Local<Object>params){

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(isolate);

	Local<Value> value = GetParamValue(isolate, context, param_name, params);
	std::string str_val;

	str_val = value->IsNullOrUndefined()? "": UnwrapString(isolate, context, value);		

	return str_val;

}

static char* GetCString(std::string s){
	if(!s.empty())
		return strdup(s.c_str());
	else
		return NULL;	
}

mesibo_message_params_t MesiboJsProcessor::UnwrapMessageParams(Isolate* isolate, Local<Context> context,
		Local<Value> arg_params){

	assert(arg_params->IsObject());

	HandleScope scope(isolate);
	Local<Object> params = arg_params->ToObject(context).ToLocalChecked();

	mesibo_message_params_t mp ; 

	mp.aid = UnwrapParamUint(isolate, context, MESSAGE_AID, params);	
	mp.id = UnwrapParamUint(isolate, context, MESSAGE_ID, params);	
	mp.refid = UnwrapParamUint(isolate, context, MESSAGE_REFID, params);	
	mp.groupid = UnwrapParamUint(isolate, context, MESSAGE_GROUPID, params);	
	mp.flags = UnwrapParamUint(isolate, context, MESSAGE_FLAGS, params);	
	mp.type = UnwrapParamUint(isolate, context, MESSAGE_TYPE, params);	
	mp.expiry = UnwrapParamUint(isolate, context, MESSAGE_EXPIRY, params);	
	mp.to_online = UnwrapParamUint(isolate, context, MESSAGE_TO_ONLINE, params);	
	mp.to = GetCString(UnwrapParamString(isolate, context, MESSAGE_TO, params));	
	mp.from = GetCString(UnwrapParamString(isolate, context, MESSAGE_FROM, params));	

	return mp;
}

mesibo_http_option_t* MesiboJsProcessor::UnwrapHttpOptions(Isolate* isolate, Local<Context> context,
		Local<Value> arg_options){

	HandleScope scope(isolate);
	Local<Object> options = arg_options->ToObject(context).ToLocalChecked();

	mesibo_http_option_t* opt = (mesibo_http_option_t*) calloc(1, sizeof(mesibo_http_option_t)); 

	//Suggestion: Collect the given keys. Check if option is present, then unwrap	
	//The string reference is allocated by strdup, needs to be freed 
	opt->proxy = GetCString(UnwrapParamString(isolate, context, HTTP_PROXY, options));
	opt->content_type = GetCString(UnwrapParamString(isolate, context, HTTP_CONTENT_TYPE, options));
	opt->extra_header = GetCString(UnwrapParamString(isolate, context, HTTP_EXTRA_HEADER, options));
	opt->referrer = GetCString(UnwrapParamString(isolate, context, HTTP_REFERRER, options));
	opt->origin = GetCString(UnwrapParamString(isolate, context, HTTP_ORIGIN, options));
	opt->cookie = GetCString(UnwrapParamString(isolate, context, HTTP_COOKIE, options));
	opt->encoding = GetCString(UnwrapParamString(isolate, context, HTTP_ENCODING, options));
	opt->cache_control = GetCString(UnwrapParamString(isolate, context, HTTP_CACHE_CONTROL, options));
	opt->accept = GetCString(UnwrapParamString(isolate, context, HTTP_ACCEPT, options));
	opt->etag = GetCString(UnwrapParamString(isolate, context, HTTP_ETAG, options));

	opt->ims = UnwrapParamUint(isolate, context, HTTP_IMS, options);
	opt->maxredirects = UnwrapParamUint(isolate, context, HTTP_MAXREDIRECTS, options);
	opt->conn_timeout = UnwrapParamUint(isolate, context, HTTP_CONN_TIMEOUT, options);
	opt->header_timeout = UnwrapParamUint(isolate, context, HTTP_HEADER_TIMEOUT, options);
	opt->body_timeout = UnwrapParamUint(isolate, context, HTTP_BODY_TIMEOUT, options);
	opt->total_timeout= UnwrapParamUint(isolate, context, HTTP_TOTAL_TIMEOUT, options);
	opt->retries = UnwrapParamUint(isolate, context, HTTP_RETRIES, options);

	return opt;
}


http_context_t* MesiboJsProcessor::BundleHttpCallback(Isolate* isolate, 
		Local<Context> context, Local<Value> js_cb, 
		Local<Value> js_cbdata, mesibo_module_t* mod){

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(isolate);

	if((!js_cb->IsFunction()) || js_cb->IsNullOrUndefined()){
		mesibo_log(mod, MODULE_LOG_LEVEL_0VERRIDE, "Invalid HTTP Callback");
		return NULL;
	}
	http_context_t* hc = (http_context_t*)calloc(1, sizeof(http_context_t));

	//V8 Context	
	hc->isolate = isolate;
	hc->context = context;	

	hc->http_cb = js_cb;
	hc->http_cbdata = js_cbdata;

	hc->mod = mod;

	return hc; //The caller is responsible for freeing this pointer
}

void MesiboJsProcessor::LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
	if (args.Length() < 1) return;
	Isolate* isolate = args.GetIsolate();
	HandleScope scope(isolate);
	Local<Value> arg = args[0];
	Local<External> mod_cb = args.Data().As<External>();

	String::Utf8Value value(isolate, arg);

	mesibo_module_t* mod =  static_cast<mesibo_module_t*>(mod_cb->Value()); 

	mesibo_log(mod, MODULE_LOG_LEVEL_0VERRIDE , "Logged: %s\n", *value);
}

void MesiboJsProcessor::MessageCallback(const v8::FunctionCallbackInfo<v8::Value>& args){

	if (args.Length() < 1) return;
	Isolate* isolate = args.GetIsolate();
	HandleScope scope(isolate);

	v8::Local<v8::Context> context = Context::New(isolate);
	Context::Scope context_scope(context);

	Local<External> mod_cb = args.Data().As<External>();
	Local<Value> arg_params = args[0];
	Local<Value> arg_message = args[1];
	Local<Value> arg_len = args[2];

	mesibo_module_t* mod;
	mesibo_message_params_t p;
	std::string message;
	mesibo_uint_t len;

	mod = static_cast<mesibo_module_t*>(mod_cb->Value()); 

	p = UnwrapMessageParams(isolate, context, arg_params);
	message = UnwrapString(isolate, context, arg_message); //Pass len, and get raw bytes
	len = UnwrapUint(isolate, context, arg_len);

	mesibo_message(mod, &p, message.c_str(), strlen(message.c_str()));

}

void mesibo_js_destroy_http_context(http_context_t* mc){
	free(mc);
}


/**
 *
 */

int mesibo_module_http_data_callback(void *cbdata, mesibo_int_t state, 
		mesibo_int_t progress, const char *buffer, mesibo_int_t size) {
	http_context_t *b = (http_context_t *)cbdata;
	mesibo_module_t* mod = b->mod;

	MesiboJsProcessor* mp = ((v8_config_t*)mod->ctx)->ctx;
	int log = mp->log_; //GetLogLevel

	if (progress < 0) {
		mesibo_log(mod, MODULE_LOG_LEVEL_0VERRIDE, " Error in http callback \n");
		mesibo_js_destroy_http_context(b);
		return MESIBO_RESULT_FAIL;
	}

	if ((0 < progress) && (MODULE_HTTP_STATE_RESPBODY == state)) {
		if(HTTP_BUFFER_LEN < (b->datalen + size )){
			mesibo_log(mod, MODULE_LOG_LEVEL_0VERRIDE,
					"Error http callback :Buffer overflow detected \n");
			mesibo_js_destroy_http_context(b);
			return MESIBO_RESULT_FAIL;
		}
	}
	memcpy(b->buffer + b->datalen, buffer, size);
	b->datalen += size;

	if ( 100 == progress ) {
		mesibo_log(mod, log,"HTTP response complete\n %.*s\n", b->datalen, b->buffer);
		
		Isolate* isolate = mp->GetIsolate();
		v8::Locker locker(isolate);
		v8::HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = mp->GetContext(); 

		int argc = 3;
		Local<Value> argv[3];
		argv[0] = b->http_cbdata;
		argv[1] = v8::String::NewFromUtf8(isolate, b->buffer, NewStringType::kNormal, b->datalen)
			.ToLocalChecked();
		argv[2] = v8::Integer::NewFromUnsigned(isolate, b->datalen); 

		int rv = mp->ExecuteJsFunction(context,MESIBO_LISTENER_ON_HTTP_RESPONSE, argc, argv);

		if(MESIBO_RESULT_FAIL == rv){	//Blocking Call
			mesibo_log(mod, MODULE_LOG_LEVEL_0VERRIDE, "Error calling HTTP callback");
			return MESIBO_RESULT_FAIL;		
		}
		mesibo_js_destroy_http_context(b);
	}

	return MESIBO_RESULT_OK;
}


void MesiboJsProcessor::HttpCallback(const v8::FunctionCallbackInfo<v8::Value>& args){

	if (args.Length() < 1) return;
	Isolate* isolate = args.GetIsolate();
	HandleScope scope(isolate);

	v8::Local<v8::Context> context = Context::New(isolate);
	Context::Scope context_scope(context);

	Local<External> mod_cb = args.Data().As<External>();

	mesibo_module_t* mod = static_cast<mesibo_module_t*>(mod_cb->Value()); 
	std::string url = UnwrapString(isolate, context, args[0]); 
	std::string post = UnwrapString(isolate, context, args[1]); 

	//Bundle the Js function http callback data 
	Local<Value> js_cb = args[2];//The Js callback function 
	Local<Value> js_cbdata = args[3]; //The Js callback data (arbitrary)	

	http_context_t* hc =  BundleHttpCallback(isolate, context, js_cb, js_cbdata, mod); 
	if(hc) //Valid http callback parameters 
	{
		void* cbdata = (void*)hc;
		//Clenup http context once done

		mesibo_http_option_t* opt;	
		opt = args[4]->IsObject() ? UnwrapHttpOptions(isolate, context, args[4]): NULL;

		mesibo_util_http(url.c_str(), post.c_str(), 
				mesibo_module_http_data_callback, cbdata, opt);
	}
	else
		mesibo_log(mod, MODULE_LOG_LEVEL_0VERRIDE,"Invalid HTTP Callback\n");
}

//Get the timestamp of the last made change in script
long GetLastChanged(const char* script){
	struct stat attr;
	if(!stat(script, &attr) != 0){
		return attr.st_mtime;
	}
	return MESIBO_RESULT_FAIL;
}

/**
 * Recompile if the script is changed.
 * Returns newly allocated context object derived from base context
 * 
 **/
Local<Context> MesiboJsProcessor::GetContext() {

	EscapableHandleScope handle_scope(GetIsolate());
	
	long latestchange = GetLastChanged(script_);
	if(MESIBO_RESULT_FAIL == latestchange)
		mesibo_log(mod_, MODULE_LOG_LEVEL_0VERRIDE,"Error stating file %s \n", script_);

	//lastchanged will be zero if file is loaded for the first time 
	bool isFileChange = (lastchanged_!= 0) && (lastchanged_ != latestchange); 
	lastchanged_ = latestchange;
	
	int init_status = 0; 
	if(isFileChange){ 
		mesibo_log(mod_, log_, "File has changed \n");
		init_status = Initialize(); // Recompile & Reinitialize 
	}

	if(MESIBO_RESULT_FAIL == init_status)
		mesibo_log(mod_, MODULE_LOG_LEVEL_0VERRIDE, "Error initializing context\n");

	return handle_scope.Escape(Local<Context>::New(GetIsolate(), context_));
}

int MesiboJsProcessor::ExecuteScript(Local<String> script, Local<ObjectTemplate> global) {

	HandleScope handle_scope(GetIsolate());

	// We're just about to compile the script; set up an error handler to
	// catch any exceptions the script might throw.
	TryCatch try_catch(GetIsolate());

	// Each module script gets its own context so different script calls don't
	// affect each other. Context::New returns a persistent handle which
	// is what we need for the reference to remain after we return from
	// this method. 

	Local<Context> context = Context::New(GetIsolate(), NULL, global);
	context_.Reset(GetIsolate(), context);
	
	// Enter the new context so all the following operations take place within it.
	Context::Scope context_scope(context);
	
	// Compile the script and check for errors.
	Local<Script> compiled_script;
	if (!Script::Compile(context, script).ToLocal(&compiled_script)) {
		String::Utf8Value error(GetIsolate(), try_catch.Exception());
		mesibo_log(mod_, MODULE_LOG_LEVEL_0VERRIDE,"%s\n", *error);
		// The script failed to compile; bail out.
		return MESIBO_RESULT_FAIL;
	}

	// Run the script
	Local<Value> result;
	if (!compiled_script->Run(context).ToLocal(&result)) {
		// The TryCatch above is still in effect and will have caught the error.
		String::Utf8Value error(GetIsolate(), try_catch.Exception());
		mesibo_log(mod_,MODULE_LOG_LEVEL_0VERRIDE, "%s\n", *error);
		// Running the script failed; bail out.
		return MESIBO_RESULT_FAIL;
	}

	return MESIBO_RESULT_OK;
}

void MesiboJsProcessor::SetCallables(Local<ObjectTemplate> & global){

	global->Set(String::NewFromUtf8(GetIsolate(), MESIBO_MODULE_LOG, NewStringType::kNormal)
			.ToLocalChecked(),
			FunctionTemplate::New(GetIsolate(), 
				LogCallback, External::New(GetIsolate(), (void*)mod_)));

	global->Set(String::NewFromUtf8(GetIsolate(), MESIBO_MODULE_MESSAGE, NewStringType::kNormal)
			.ToLocalChecked(),
			FunctionTemplate::New(GetIsolate(),
				MessageCallback, External::New(GetIsolate(), (void*)mod_)));

	global->Set(String::NewFromUtf8(GetIsolate(), MESIBO_MODULE_HTTP, NewStringType::kNormal)
			.ToLocalChecked(),
			FunctionTemplate::New(GetIsolate(),
				HttpCallback, External::New(GetIsolate(), (void*)mod_)));


}


int MesiboJsProcessor::Initialize(){
	// Create a handle scope to hold the temporary references.
	HandleScope handle_scope(GetIsolate());

	// Create a template for the global object where we set the
	// built-in global functions.

	Local<ObjectTemplate> global = ObjectTemplate::New(GetIsolate());
	SetCallables(global);

	Local<String> source = ReadFile(GetIsolate(), script_).ToLocalChecked();
	int rv = ExecuteScript(source, global);
	if(MESIBO_RESULT_FAIL == rv){
		mesibo_log(mod_, MODULE_LOG_LEVEL_0VERRIDE, "Error executing script %s\n", script_);
		return MESIBO_RESULT_FAIL;
	}

	return MESIBO_RESULT_OK;

}

int MesiboJsProcessor::WrapParamUint(Local<Context>& context, Local<Object> &js_params, 
		const char* key, mesibo_uint_t value){

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(GetIsolate());

	Local<Value> p_key;
	Local<Value> p_value;

	p_key = String::NewFromUtf8(GetIsolate(), key).ToLocalChecked();
	p_value = v8::Integer::NewFromUnsigned(GetIsolate(), value);

	Local<Value> set_param ; //Check return value of set
	js_params->Set(context, p_key, p_value).FromMaybe(&set_param);

	return MESIBO_RESULT_OK;
}


int MesiboJsProcessor::WrapParamString(Local<Context>& context, Local<Object> &js_params, 
		const char* key, const char* value){

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(GetIsolate());

	Local<Value> p_key;
	Local<Value> p_value;

	p_key = String::NewFromUtf8(GetIsolate(), key).ToLocalChecked();
	p_value = String::NewFromUtf8(GetIsolate(), value).ToLocalChecked();

	Local<Value> set_param ; //Check return value of set
	js_params->Set(context, p_key, p_value).FromMaybe(&set_param);

	return MESIBO_RESULT_OK;
}

Local<Object> MesiboJsProcessor::WrapMessageParams(Local<Context>& context, mesibo_message_params_t* p){

	// Local scope for temporary handles.
	EscapableHandleScope handle_scope(GetIsolate());

	Local<Object> js_params = v8::Object::New(GetIsolate());

	WrapParamUint(context, js_params, MESSAGE_AID,       p->aid);
	WrapParamUint(context, js_params, MESSAGE_ID,        p->id);
	WrapParamUint(context, js_params, MESSAGE_REFID,     p->refid);
	WrapParamUint(context, js_params, MESSAGE_GROUPID,   p->groupid);
	WrapParamUint(context, js_params, MESSAGE_FLAGS,     p->flags);
	WrapParamUint(context, js_params, MESSAGE_TYPE,      p->type);
	WrapParamUint(context, js_params, MESSAGE_EXPIRY,    p->expiry);
	WrapParamUint(context, js_params, MESSAGE_TO_ONLINE, p->to_online);
	WrapParamString(context, js_params, MESSAGE_TO,      p->to);
	WrapParamString(context, js_params, MESSAGE_FROM,    p->from);

	// Return the result through the current handle scope.  Since each
	// of these handles will go away when the handle scope is deleted
	// we need to call Close to let one, the result, escape into the
	// outer handle scope.
	return handle_scope.Escape(js_params);
}


//Modify to take v8 function reference directly .
//Otherwise it has to fetch reference with every call

int MesiboJsProcessor::ExecuteJsFunction(Local<Context>& context,
		const char* func_name, int argc, Local<Value> argv[]){

	mesibo_log(mod_, log_, "Executing function %s \n", func_name);

	v8::Isolate::Scope isolateScope(GetIsolate());

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(GetIsolate());

	//Enter the context
	Context::Scope context_scope(context);

	v8::Local<v8::String>js_func_name =
		v8::String::NewFromUtf8(GetIsolate(), func_name).ToLocalChecked();
	v8::Local<v8::Value> js_ref;

	//Note: Set up exception handler
	if (!context->Global()->Get(context, js_func_name).ToLocal(&js_ref) ||
			!js_ref->IsFunction()) {
		return MESIBO_RESULT_FAIL;
	}

	//Note: We can store all the function references, validate them and store it inside V8 object
	//for Mesibo_onMessage, Mesibo_onMessageStatus, Mesibo_onLogin
	if (js_ref->IsFunction()) {
		v8::Local<v8::Function> js_fun = v8::Local<v8::Function>::Cast(js_ref);
		v8::Local<v8::Value> js_result ;

		v8::TryCatch tryCatch(GetIsolate());
		if(!js_fun->Call(context, context->Global(), argc, argv).ToLocal(&js_result)){
			mesibo_log(mod_, log_, "%s call failed", func_name);
			return MESIBO_RESULT_FAIL;
		}

		mesibo_log(mod_, log_, "%s returned %s \n", func_name, 
				*(v8::String::Utf8Value(GetIsolate(), js_result)));

	} else {
		mesibo_log(mod_, log_, "%s is not a function \n", func_name); 
		return MESIBO_RESULT_FAIL;
	}
	return MESIBO_RESULT_OK;
}

// Reads a file into a v8 string.
MaybeLocal<String> MesiboJsProcessor::ReadFile(Isolate* isolate, const string& name) {
	FILE* file = fopen(name.c_str(), "rb");
	if (file == NULL) return MaybeLocal<String>(); //Error reading file. Return MESIBO_RESULT_FAIL	

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	std::unique_ptr<char> chars(new char[size + 1]);
	chars.get()[size] = '\0';
	for (size_t i = 0; i < size;) {
		i += fread(&chars.get()[i], 1, size - i, file);
		if (ferror(file)) {
			fclose(file);
			return MaybeLocal<String>();
		}
	}
	fclose(file);
	MaybeLocal<String> result = String::NewFromUtf8(
			isolate, chars.get(), NewStringType::kNormal, static_cast<int>(size));
	return result;
}


MesiboJsProcessor* mesibo_v8_init(mesibo_module_t* mod, v8_config_t* vc){
	
	vc->platform = v8::platform::NewDefaultPlatform();

	v8::V8::InitializePlatform(vc->platform.get());
	v8::V8::Initialize();

	v8::Isolate::CreateParams createParams;
	createParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate* isolate = v8::Isolate::New(createParams);

	mesibo_log(mod, vc->log,"Running V8 version %s\n", v8::V8::GetVersion());

	const char* script_path = strdup(vc->script);
	int log_level = vc->log;

	MesiboJsProcessor* mesibo_js = new MesiboJsProcessor(mod, isolate, script_path, log_level);

	mesibo_js->Initialize();

	return mesibo_js;
}

/**
 * Notify JS Callback function Mesibo_onMessage
 **/
mesibo_int_t MesiboJsProcessor::OnMessage(mesibo_message_params_t p, const char* message,
		mesibo_uint_t len){
	mesibo_log(mod_, log_, "OnMessage called\n");	

	v8::Locker locker(GetIsolate());
	v8::Isolate::Scope isolateScope(GetIsolate());

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(GetIsolate());

	//To each it's own context
	v8::Local<v8::Context> context = GetContext(); 
	
	Context::Scope context_scope(context);

	v8::Handle<v8::Value> args_bundle[3];
	v8::Local<v8::Object> params = WrapMessageParams(context, &p); // Don't pass context
	//Replace with something like GetCurrentContext

	args_bundle[0] = params; 
	args_bundle[1] = v8::String::NewFromUtf8(GetIsolate(), message).ToLocalChecked();
	args_bundle[2] = v8::Integer::NewFromUnsigned(GetIsolate(), len); //use utils to check for errors

	ExecuteJsFunction(context, MESIBO_LISTENER_ON_MESSAGE , 3, args_bundle);

	//Return whatever JS Callback is returning, But that would mean blocking until return value
	return MESIBO_RESULT_OK;

}

/**
 * Notify JS Callback function Mesibo_onMessageStatus
 **/
mesibo_int_t MesiboJsProcessor::OnMessageStatus(mesibo_message_params_t p, mesibo_uint_t status){
	mesibo_log(mod_, log_, "OnMessageStatus called\n");	

	v8::Locker locker(GetIsolate());

	v8::Isolate::Scope isolateScope(GetIsolate());

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(GetIsolate());

	//To each it's own context
	v8::Local<v8::Context> context = GetContext(); 

	Context::Scope context_scope(context);

	v8::Handle<v8::Value> args_bundle[2];
	v8::Local<v8::Object> params = WrapMessageParams(context, &p); // Don't pass context

	args_bundle[0] = params; 
	args_bundle[1] = v8::Integer::NewFromUnsigned(GetIsolate(), status); //use utils to check for errors

	ExecuteJsFunction(context, MESIBO_LISTENER_ON_MESSAGE_STATUS, 2, args_bundle);

	//Return whatever JS Callback is returning, But that would mean blocking until return value
	return MESIBO_RESULT_OK;

}

static mesibo_int_t v8_on_message(mesibo_module_t *mod, mesibo_message_params_t *p, const char *message,
		mesibo_uint_t len) {

	v8_config_t* vc = (v8_config_t*)mod->ctx;
	MesiboJsProcessor* mesibo_js = vc->ctx;

	mesibo_log(mod, vc->log,  "================> %s on_message called\n", mod->name);
	mesibo_log(mod, vc->log, " from %s to %s id %u message %s\n", 
			p->from, p->to, (uint32_t) p->id, message);

	// Clone Parameters
	mesibo_message_params_t mp;
	memset(&mp, 0, sizeof(mesibo_message_params_t));
	memcpy(&mp, p, sizeof(mesibo_message_params_t));
	mp.to = p->to ? strdup(p->to): strdup("");
	mp.from = strdup(p->from);

	mesibo_js->OnMessage(mp, message, len);	

	return MESIBO_RESULT_PASS; 
}

static mesibo_int_t v8_on_message_status(mesibo_module_t *mod, mesibo_message_params_t *p, mesibo_uint_t status) {
	v8_config_t* vc = (v8_config_t*)mod->ctx;
	MesiboJsProcessor* mesibo_js = vc->ctx;

	mesibo_log(mod, vc->log, "================>%s on_message_status called\n", mod->name);
	mesibo_log(mod, vc->log, "to %s from %s id %u status %d\n", p->to, p->from, (uint32_t)p->id, status);

	// Clone Parameters
	mesibo_message_params_t mp;
	memset(&mp, 0, sizeof(mesibo_message_params_t));
	memcpy(&mp, p, sizeof(mesibo_message_params_t));
	mp.to = p->to ? strdup(p->to): strdup("");
	mp.from = strdup(p->from);

	mesibo_js->OnMessageStatus(mp, status);	

	return 0;
}

/**
 * Helper function for getting v8 module configuration
 * Gets /path/to/script which contains Javascript code
 **/
static v8_config_t* get_config_v8(mesibo_module_t* mod){

	v8_config_t* vc = (v8_config_t*)calloc(1, sizeof(v8_config_t));
	vc->script = mesibo_util_getconfig(mod, "script");
	vc->log = atoi(mesibo_util_getconfig(mod, "log"));
	vc->last_changed = 0; //Initialize TS to zero
	vc->ctx = NULL;
	mesibo_log(mod, vc->log, "V8 Module Configured : script %s log %d\n", vc->script, vc->log);

	return vc;
}


MESIBO_EXPORT int mesibo_module_v8_init(mesibo_int_t version, mesibo_module_t *m, mesibo_uint_t len) {
	MESIBO_MODULE_SANITY_CHECK(m, version, len);

	if(m->config) {
		v8_config_t* vc = get_config_v8(m);
		if(NULL == vc){
			mesibo_log(m, MODULE_LOG_LEVEL_0VERRIDE, "%s : Missing Configuration\n", m->name);
			return MESIBO_RESULT_FAIL;
		}

		MesiboJsProcessor* mesibo_js = mesibo_v8_init(m, vc);
		vc->ctx = mesibo_js;

		if (!vc->ctx) {
			mesibo_log(m, MODULE_LOG_LEVEL_0VERRIDE, "Loading context failed for script %s", 
					vc->script);
			return MESIBO_RESULT_FAIL;
		}
		m->ctx = (void*)vc;
	}

	else {
		mesibo_log(m, MODULE_LOG_LEVEL_0VERRIDE, "%s : Missing Configuration\n", m->name);
		return MESIBO_RESULT_FAIL;
	}


	m->flags = 0;
	m->description = strdup("Sample V8 Module");
	m->on_message= v8_on_message;
	m->on_message_status = v8_on_message_status;

	return MESIBO_RESULT_OK;
}
