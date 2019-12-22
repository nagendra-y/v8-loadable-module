//mesibo_test.js

const MESIBO_RESULT_OK          = 0
const MESIBO_RESULT_FAIL        = -1

const MESIBO_RESULT_CONSUMED    = 1 ;
const MESIBO_RESULT_PASS        = 0 ;

mesibo_log("Initializing V8");
mesibo_log(1/0);
mesibo_log(NaN);
//var p = {"aid":6320,"id":713364881,"refid":0,"groupid":0,"flags":1,"type":0,"expiry":3600,"to_online":0,"to":"test_user_demo","from":"js_user"};
//p.id = parseInt(Math.floor(2147483647*Math.random())); 
//mesibo_message(p,"Hi", 2);
// Example http response callback
function Mesibo_onHttpResponse(cbdata, response, datalen) {
        var rp_log = " ----- Recieved http response ----";
        mesibo_log(rp_log);
	mesibo_log(JSON.stringify(cbdata));
	
	mesibo_log(response);
	var p = JSON.parse(response);
	mesibo_log(p);
	mesibo_log(p.data.translations[0].translatedText);
	
	mesibo_log(datalen);

        return MESIBO_RESULT_OK;
}


function Mesibo_onMessage(params, m, len){
	mesibo_log("Mesibo_onMessage called");
	
	mesibo_log(JSON.stringify(params));
        mesibo_log(m);
	mesibo_log(len);
	
	var p = params;
	var temp = p['from'];
 	p['id'] = parseInt(Math.floor(2147483647*Math.random()));	
	p['from'] = p['to'];
	p['to'] = temp;
	//mesibo_message(p, "Hi From V8", NaN);

	mesibo_log(JSON.stringify(p));
	mesibo_log(mesibo_http);

	mesibo_http("https://translation.googleapis.com/language/translate/v2", "{\"q\":\"Who am I. Are you from India. Or Germany? Mr. Adolf\", \"target\":\"de\"}", Mesibo_onHttpResponse, {'a':'a'}, {'extra_header':"Authorization: Bearer ya29.c.Kl62B2vuYbpv-sqL9AuEbHapTnFScgenYQ-t5atHBUKTWPy7oFnEyJbKKN36PVOeTB0ixPHQ3XBSU3JMQ77joMBYcUr6QrDn71B6QguX6fmFsJeQAZykP-RqpkH4rmTL",content_type:'application/json'});
	
	//mesibo_http("https://app.mesibo.com/api.php", "op=test",
	//	null, null, null);
	
	return MESIBO_RESULT_OK;
}

function Mesibo_onMessageStatus(p, s){
	mesibo_log("Mesibo_onMessageStatus called");
	mesibo_log(JSON.stringify(p));
	mesibo_log(s);

	return MESIBO_RESULT_OK;
}
