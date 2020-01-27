//dialogflow.js

const MESIBO_RESULT_OK          = 0 
const MESIBO_RESULT_FAIL        = -1 
 
const MESIBO_RESULT_CONSUMED    = 1 ;  
const MESIBO_RESULT_PASS        = 0 ;

function Mesibo_onHttpResponse(cbdata, response, datalen) {
	mesibo_log(cbdata);
	mesibo_log(response);
	mesibo_log(datalen);

	var p = cbdata;
	var tmp = p.to;
	p.to = p.from;
	p.from = tmp;

	rp_log = " ---Sending response to :" + p.to + " from :" + p.from;
	mesibo_log(rp_log);
	var rp = JSON.parse(response); 
	var fulfillment = rp.queryResult.fulfillmentText;
	
	mesibo_log(fulfillment);
	
	mesibo_message(p, fulfillment, fulfillment.length);

	return MESIBO_RESULT_OK;
}

function Mesibo_onMessage(p, message, len) {

	if (1){
		p.refid = p.id;

		var base_url = 
			"https://dialogflow.googleapis.com/v2/projects/mesibo-dialogflow/agent/sessions/"
			+ p.id + ":detectIntent";
		var request_body = {
			"queryInput": {
				"text": {
					"text": message, 
					"languageCode" : "en" 
				}   
			}
		};

		var request_options = {
			"extra_header": "Authorization: Bearer ya29.c.Kl64B6-kkKnQebpSkK9v7EDaEl0L0esmhWf9WjC1UR4AV_Ac2fQG3Sa2vVihUqqXB0SxfrBFfstElVb1QVzJrfgQ0aordgLLp9VOg3YvPaS3RHkmmABKdsRgjqUkR1hc",
			"content_type": "application/json"
		};

		mesibo_http(base_url, JSON.stringify(request_body), Mesibo_onHttpResponse, p, request_options);
		return MESIBO_RESULT_CONSUMED;
	}

	return MESIBO_RESULT_PASS;
}

function Mesibo_onMessageStatus(p, pStatus) {
	return MESIBO_RESULT_PASS;
}
