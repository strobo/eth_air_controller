#ifndef HTML_DATA_H
#define HTML_DATA_H

#include <avr/pgmspace.h>
const  prog_char pc_html[] = "\
<html><head>\
<meta charset='utf-8'>\
<title>AirConditionerController</title>\
<script src='http://ajax.googleapis.com/ajax/libs/jquery/1.6.2/jquery.min.js'></script>\
<link rel='stylesheet'href='http://twitter.github.com/bootstrap/1.3.0/bootstrap.min.css'>\
<script>\
state={a:'off',b:30};state.send=function(){$.get('/ir',{power:state.a,temp:state.b},function(a){console.log(a)})};$(function(){$('#power').text(state.a==='off'?'ON':'OFF').toggle(function(){$(this).text('OFF').toggleClass('primary',!1);state.a='on';state.b=$('#temp').val()-0;state.send()},function(){$(this).text('ON').toggleClass('primary',!0);state.a='off';state.b=$('#temp').val()-0;state.send()});$('#temp').change(function(){state.b=$('#temp').val()-0;state.a==='on'&&state.send()})});\
</script>\
</head><body>\
<div class='container'>\
<h1>RemoteAirController</h1>\
<p><select id='temp'name='temp'style='font-size:27px;width:100px;height:54px'>\
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
</select></p>\
<a href='#'id='power'class='btn large primary'style='width:100px;text-align:center'></a>\
</div>\
</body></html>";

const  prog_char mobile_html[] = "\
<html><head>\
<title>RemoteAirConditionerController</title>\
</head><body>\n\
<form action='/ir'method='GET'>\
<select name='power'>\
<option value='on'>ON</option>\
<option value='off'>OFF</option>\
</select>\n\
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
<input type='submit'value='OK!'>\
</form>\
<p>RemoteIR for mobile</p>\
</body></html>";
#endif
