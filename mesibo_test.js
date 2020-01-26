//mesibo_test.js

initialize();

function initialize(){
	mesibo.onmessage = Mesibo_onMessage;
	mesibo.onmessagestatus = Mesibo_onMessageStatus;
	mesibo.onlogin = Mesibo_onLogin;
}

function test_auto_reply(message){
	var tmp = message.to;
	message.to = message.from;
	message.from = tmp;
	message.message = "This is an automated response";
	message.send();

}

function test_message(){
	var message = new Message();
	message.id = parseInt(Math.floor(2147483647*Math.random()));
	message.aid = 6323;
	message.refid = 0;
	message.gid = 0;
	message.enableReadReceipt(true);
	message.enableDeliveryReceipt(true);
	message.expiry = 3600;
	message.sendIfOnline(true);
	message.when = +new Date(); 
	message.to = "test_user_demo";
	message.from = "js_user";
	message.message = "testMessage";
	
	message.send();
}

function Mesibo_onMessage(message) {
	message.when //Timestamp
	message.data; //raw bytes
	message.message; //string
	
	return mesibo.RESULT_OK; 
}

function Mesibo_onMessageStatus(message) {
	return mesibo.RESULT_OK; 
}


function test_log(){}

function Mesibo_onHttpResponse(http) {
	http.response;
	http.responseType;
	http.responseText;
	http.responseJSON;
	http.result;
}

function Mesibo_onTranslate(http) {
	var response  = http.responseJSON;
	var responseText = response.data.translations[0].translatedText;
}

function test_http(){
	var http = new Http();
	http.url = "https://app.mesibo.com/api.php";
	http.post = "op=test";	
	http.contentType = 'application/xml';
	http.header;
	http.extraHeaders;
	http.userAgent;
	http.referrer;
	http.origin;
	http.cookie;
	http.encoding = 'gzip';
	http.cacheControl;
	http.accept;
	http.etag;
	http.ims;
	
	http.connTimeout;
	http.headerTimeout;
	http.bodyTimeout;
	http.totalTimeout;
	
	http.ondata = function(http) {}
	
	http.cbdata = "cbdata"
	http.send();
	
	var tHttp = new Http();
	tHttp.url = "https://translation.googleapis.com/language/translate/v2";
	tHttp.post = "{\"q\":\"Who are you\", \"target\":\"de\"}";
	tHttp.ondata = Mesibo_onTranslate;
	tHttp.cbdata = "Google Translate";
	tHttp.header = "Authorization: Bearer ya29.c.Kl66B3ReS77KYYMi1vG_-gQa0fnN9vlJB_rfdKLrYD1aJCYSsiLnmOpbM8gQYqpEqXigz26It6n04r0yfv7mHUdhpgATBFbqeA63qr3yRNFTzNV9nqtl3fA7AMLzWYqW";
	tHttp.contentType = 'application/json';
	tHttp.send();	
}


function Socket_onConnect(socket){
	socket.write("SOCKET DATA");
}

function Socket_onData(socket){
	socket.stringData;
	socket.data;
	socket.length;
}

function test_socket(){
	var sock = new Socket();
	mesibo.log(sock);
	sock.host = '127.0.0.1';
	sock.port = 65432;
	sock.keepAlive;
	sock.verifyHost = true;
	sock.enableSsl = true;
	sock.onconnect = Socket_onConnect;
	sock.ondata = Socket_onData;
	sock.connect();
	return mesibo.RESULT_OK;
}

function Mesibo_onLogin(user){
	user.online;
	user.address;
}

