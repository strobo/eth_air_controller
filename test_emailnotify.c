/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher
 * Copyright: GPL V2
 * See http://www.gnu.org/licenses/gpl.html
 *
 * Chip type           : Atmega168 or Atmega328 with ENC28J60
 *********************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "ip_arp_udp_tcp.h"
#include "websrv_help_functions.h"
#include "enc28j60.h"
#include "timeout.h"
#include "net.h"

// Note: This software implements a web server and a web browser.
// The web server is at "myip" and the browser tries to access "websrvip".
// We do not use DNS lookup here. If you want to add it then look at
// the example test_web_client.c
// 
// Please modify the following lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x29};
// how did I get the mac addr? Translate the first 3 numbers into ascii is: TUX
static uint8_t myip[4] = {10,0,0,29};
//static uint8_t myip[4] = {192,168,255,100};
// IP address of the web server to contact (IP of the first portion of the URL):
static uint8_t websrvip[4] = {77,37,2,152};
// The name of the virtual host which you want to contact at websrvip (hostname of the first portion of the URL):
#define WEBSERVER_VHOST "tuxgraphics.org"
//static uint8_t websrvip[4] = {10,0,0,1};
// Default gateway. The ip address of your DSL router. It can be set to the same as
// websrvip the case where there is no default GW to access the 
// web server (=web server is on the same lan as this host) 
static uint8_t gwip[4] = {10,0,0,2};
//
// You get the account ID when you purchase a tuxgraphics microcontroller mail account.
// Put your own ID in the string below in the program (search for MMaccountID)
// You can administrate your mail account at http://tuxgraphics.org/cgi-bin/mm
//
// A non static portion of the URL, e.g for measurement results
static char urlvarstr[31];
// listen port for tcp/www:
#define MYWWWPORT 80
//
// the password string, (only a-z,0-9,_ characters):
static char password[]="secret"; 

#define BUFFER_SIZE 590
#define DATE_BUFFER_SIZE 30
static char datebuf[DATE_BUFFER_SIZE]="none";
static uint8_t buf[BUFFER_SIZE+1];
static volatile uint8_t start_web_client=0;  // 0=off but enabled, 1=send mail, 2=main sending initiated, 3=email was sent OK, 4=diable mail notify
static uint8_t contact_onoff_cnt=0;
static uint8_t contact_debounce=0;
static uint8_t web_client_attempts=0;
static uint8_t web_client_sendok=0;
static uint8_t resend=0;
static volatile uint8_t sec=0;
static volatile uint8_t cnt2step=0;

// set output to VCC, red LED off
#define LEDOFF PORTB|=(1<<PORTB1)
// set output to GND, red LED on
#define LEDON PORTB&=~(1<<PORTB1)
// to test the state of the LED
#define LEDISOFF PORTB&(1<<PORTB1)
// 

uint8_t verify_password(char *str)
{
        // the first characters of the received string are
        // a simple password/cookie:
        if (strncmp(password,str,strlen(password))==0){
                return(1);
        }
        return(0);
}

// analyse the url given
// return values: -1 invalid 
//
//                The string passed to this function will look like this:
//                /?mn=1&pw=secret HTTP/1.....
//                / HTTP/1.....
int8_t analyse_get_url(char *str)
{
        uint8_t mn=0;
        char kvalstrbuf[10];
        // the first slash:
        if (str[0] == '/' && str[1] == ' '){
                // end of url, display just the web page
                return(2);
        }
        // str is now something like ?pw=secret&mn=0 or just end of url
        if (find_key_val(str,kvalstrbuf,10,"mn")){
                if (kvalstrbuf[0]=='1'){
                        mn=1;
                }
                // to change the mail notification one needs also a valid passw:
                if (find_key_val(str,kvalstrbuf,10,"pw")){
                        if (verify_password(kvalstrbuf)){
                                return(mn);
                        }else{
                                return(-1);
                        }
                }
        }
        // browsers looking for /favion.ico, non existing pages etc...
        return(-1);
}
uint16_t http200ok(void)
{
        return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n")));
}


// prepare the webpage by writing the data to the tcp send buffer
uint16_t print_webpage(uint8_t *buf)
{
        uint16_t plen;
        char vstr[5];
        plen=http200ok();
        plen=fill_tcp_data_p(buf,plen,PSTR("<h2>Email notification status</h2>\n<pre>\n"));
        plen=fill_tcp_data_p(buf,plen,PSTR("Number of PD6 to GND connections: "));
        // convert number to string:
        itoa(contact_onoff_cnt,vstr,10);
        plen=fill_tcp_data(buf,plen,vstr);
        plen=fill_tcp_data_p(buf,plen,PSTR("\nNumber of email attempts: "));
        // convert number to string:
        itoa(web_client_attempts,vstr,10);
        plen=fill_tcp_data(buf,plen,vstr);
        plen=fill_tcp_data_p(buf,plen,PSTR("\nNumber successful emails: "));
        // convert number to string:
        itoa(web_client_sendok,vstr,10);
        plen=fill_tcp_data(buf,plen,vstr);
        plen=fill_tcp_data_p(buf,plen,PSTR("\nDate of last mail: "));
        plen=fill_tcp_data(buf,plen,datebuf);
        plen=fill_tcp_data_p(buf,plen,PSTR("\nEmail notify is: "));
        if (start_web_client==4){
                plen=fill_tcp_data_p(buf,plen,PSTR("OFF"));
        }else{
                plen=fill_tcp_data_p(buf,plen,PSTR("ON"));
        }
        plen=fill_tcp_data_p(buf,plen,PSTR("\n<form action=/ method=get>\npassw: <input type=password size=8 name=pw><input type=hidden name=mn "));
        if (start_web_client==4){
                plen=fill_tcp_data_p(buf,plen,PSTR("value=1><input type=submit value=\"enable email\">"));
        }else{
                plen=fill_tcp_data_p(buf,plen,PSTR("value=0><input type=submit value=\"disable email\">"));
        }
        plen=fill_tcp_data_p(buf,plen,PSTR("</form>\n</pre><br><hr>tuxgraphics.org"));
        return(plen);
}

/* setup timer T2 as an interrupt generating time base.
* You must call once sei() in the main program */
void init_cnt2(void)
{
	cnt2step=0;
	PRR&=~(1<<PRTIM2); // write power reduction register to zero
	TIMSK2=(1<<OCIE2A); // compare match on OCR2A
	TCNT2=0;  // init counter
	OCR2A=244; // value to compare against
	TCCR2A=(1<<WGM21); // do not change any output pin, clear at compare match
	// divide clock by 1024: 12.5MHz/128=12207 Hz
	TCCR2B=(1<<CS22)|(1<<CS21)|(1<<CS20); // clock divider, start counter
	// 12207/244=50Hz
}

// called when TCNT2==OCR2A
// that is in 50Hz intervals
ISR(TIMER2_COMPA_vect){
	cnt2step++;
	if (cnt2step>50){
                cnt2step=0;
                sec++; // stepped every second
                contact_debounce=0;
	}
}


void store_date_if_found(char *str)
{
        uint8_t i=100; // search the first 100 char
        uint8_t j=0;
        char datekeyword[]="Date: "; // not any repeating characters, search does not need to resume at partial match
        while(i && *str){
                if (j && datekeyword[j]=='\0'){
                        // found date
                        i=0;
                        while(*str && *str!='\r' && *str!='\n' && i< DATE_BUFFER_SIZE-1){
                                datebuf[i]=*str;
                                str++;
                                i++;
                        }
                        datebuf[i]='\0';
                        return;
                }
                if (*str==datekeyword[j]){
                        j++;
                }else{
                        j=0;
                }
                str++;
                i--;
        }
}

void browserresult_callback(uint8_t statuscode,uint16_t datapos, uint16_t len){
        if (statuscode==0){
                len=0; // avoid warning about unused variable
                web_client_sendok++;
                LEDOFF;
                // copy the "Date: ...." as returned by the server
                store_date_if_found((char *)&(buf[datapos]));
        }
        // clear pending state at sucessful contact with the
        // web server even if account is expired:
        if (start_web_client==2) start_web_client=3;
}

int main(void){

        
        uint16_t dat_p;
        int8_t cmd;
        
        // Set the clock speed to "no pre-scaler" (8MHz with internal osc or 
        // full external speed)
        // set the clock prescaler. First write CLKPCE to enable setting 
        // of clock the next four instructions.
        // Note that the CKDIV8 Fuse determines the initial
        // value of the CKKPS bits.
        CLKPR=(1<<CLKPCE); // change enable
        CLKPR=0; // "no pre-scaler"
        _delay_loop_1(0); // 60us


        /*initialize enc28j60*/
        enc28j60Init(mymac);
        enc28j60clkout(2); // change clkout from 6.25MHz to 12.5MHz
        _delay_loop_1(0); // 60us
        
        /* Magjack leds configuration, see enc28j60 datasheet, page 11 */
        // LEDB=yellow LEDA=green
        //
        // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
        // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
        enc28j60PhyWrite(PHLCON,0x476);
        
        DDRB|= (1<<DDB1); // LED, enable PB1, LED as output
        LEDOFF;

        DDRD&= ~(1<<DDD6); // enable PB6, as input
        PORTD|= (1<<PIND6); // internal pullup resistor on

        init_cnt2();
        sei();

        //init the web server ethernet/ip layer:
        init_ip_arp_udp_tcp(mymac,myip,MYWWWPORT);
        // init the web client:
        client_set_gwip(gwip);  // e.g internal IP of dsl router
        client_set_wwwip(websrvip);
        //
        while(1){
                // handle ping and wait for a tcp packet
                dat_p=packetloop_icmp_tcp(buf,enc28j60PacketReceive(BUFFER_SIZE, buf));
                if(dat_p==0){
                        if (start_web_client==1){
                                LEDON;
                                start_web_client=2;
                                sec=0;
                                web_client_attempts++;
                                // the text to send in the body of the mail:
                                urlencode("The door is open",urlvarstr);
                                // The string "test+emailnotify" is the subject line
                                // of the email. A plus sign translates into a space character.
                                // The MMaccountID is gus123, edit and replace by your own.
                                client_browse_url(PSTR("/cgi-bin/mm?a=gus123&s=test+emailnotify&d="),urlvarstr,PSTR(WEBSERVER_VHOST),&browserresult_callback);
                        }
                        // count how often the switch was triggered at all:
                        if (contact_debounce==0 && bit_is_clear(PIND,PIND6)){
                                contact_onoff_cnt++;
                                contact_debounce=1;
                        }
                        // we send only an email if we are in the right state (do
                        // not flood with email):
                        if (start_web_client==0 && bit_is_clear(PIND,PIND6)){
                                resend=1; // resend once if it failed
                                start_web_client=1;
                        }
                        // Reset after a delay of 3 min to prevent email sending.
                        // We reset only if the contact on PD6 was released.
                        if (start_web_client<=3 && sec==180 && !bit_is_clear(PIND,PIND6)){
                                start_web_client=0;
                        }
                        // Resend the email if it failed:
                        if (start_web_client==2 && sec==7 && resend){
                                start_web_client=1;
                                resend--;
                        }
                        continue;
                }
                        
                if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
                        // head, post and other methods:
                        //
                        // for possible status codes see:
                        // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
                        dat_p=http200ok();
                        dat_p=fill_tcp_data_p(buf,dat_p,PSTR("<h1>200 OK</h1>"));
                        goto SENDTCP;
                }
                cmd=analyse_get_url((char *)&(buf[dat_p+4]));
                // for possible status codes see:
                // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
                if (cmd==-1){
                        dat_p=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
                        goto SENDTCP;
                }
                if (cmd==1 && start_web_client==4){
                        // email was off, switch on
                        start_web_client=0;
                }
                if (cmd==0 ){
                        start_web_client=4; // email off
                }
                dat_p=http200ok();
                dat_p=print_webpage(buf);
                //
SENDTCP:
                www_server_reply(buf,dat_p); // send data

        }
        return (0);
}
