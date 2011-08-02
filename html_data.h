#ifndef HTML_DATA_H
#define HTML_DATA_H

#include <avr/pgmspace.h>
const  prog_char pc_html[] = "\
<html><head>\
<title>RemoteAirConditionerController</title>\
<script type='text/javascript' src='http://ajax.googleapis.com/ajax/libs/jquery/1.6.2/jquery.min.js'></script>\
<script>\n\
state = {};\n\
state.power = 'off';\n\
state.temp = 30;\n\
state.send = function(){\n\
		$.get('/ir',{power: state.power, temp: state.temp}, function(data){\
					consolo.log(data);\n\
							});\n\
};\n\
$(function(){\
	$('#on').click( function(){\
			state.power = 'on';\
				state.temp = $('#selectTemp').val()-0;\
					state.send();\
						});\
	$('#off').click( function(){\
			state.power = 'off';\
				state.temp = $('#selectTemp').val()-0;\
					state.send();\
						});\
	$('#selectTemp').change( function(){\
			state.temp = $('#selectTemp').val()-0;\
				if(state.power === 'on') state.send();\
					});\
});\n\
</script>\n\
</head><body>\n\
<div>\n\
<a href='#' id='on'>ON</a>\
<a href='#' id='off'>OFF</a>\
<select id='selectTemp'name='temp'>\
<option value='30'>30</option>\
<option value='29'>29</option>\
<option value='28'>28</option>\
<option value='27'>27</option>\
<option value='26'>26</option>\
<option value='25'>25</option>\
<option value='24'>24</option>\
<option value='23'>23</option>\
<option value='22'>22</option>\
<option value='21'>21</option>\
<option value='20'>20</option>\
</select>\
<p>RemoteIR for PC</p>\
</body></html>";

const  prog_char mobile_html[] = "\
<html><head>\
<title>RemoteAirConditionerController</title>\
</head><body>\n\
<div>\n\
<a href='#' id='on'>ON</a>\
<a href='#' id='off'>OFF</a>\
<select id='selectTemp'name='temp'>\
<option value='30'>30</option>\
<option value='29'>29</option>\
<option value='28'>28</option>\
<option value='27'>27</option>\
<option value='26'>26</option>\
<option value='25'>25</option>\
<option value='24'>24</option>\
<option value='23'>23</option>\
<option value='22'>22</option>\
<option value='21'>21</option>\
<option value='20'>20</option>\
</select>\
<p>RemoteIR for mobile</p>\
</body></html>";
#endif
