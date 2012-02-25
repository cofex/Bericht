//////////////////////////////////////////////////////////////////////
// $Id: main.c 155 2009-10-23 13:10:11Z student $
//
// $Author: student $
// Copyright by Simple Solutions, Stephan Schirrmann, Martin Miedreich, Andre Niemann
//
// $Project$ 
//
// File Description:   
//
//////////////////////////////////////////////////////////////////////

#include <LPC23xx.H>
#include <stdio.h>
#include "type.h"			                                         
#include "config.h"
#include "hiduser.h"
#include "ports.h"
#include "touch.h"
#include "spi.h"
#include "display.h"
#include "rtc_nvram.h"
#include "timer.h"
#include "serial.h"
#include "usbcmd.h"
#include "pll.h"
#include "ethernet.h"
#include "flash.h"
#include "dc.h"
#include "gui.h"
#include "authentication.h"
#include "main.h"

#define	DATA_BUF_SIZE 43

// BioHash
#include "biohash.h"

//////////////////////////////////////////////////////////////////////
// FIFO variables
//////////////////////////////////////////////////////////////////////

tSampleData sample_fifo[SAMPLE_FIFO_SIZE];
uint8_t fifo_wr = 0;
uint8_t fifo_rd = 0;

//////////////////////////////////////////////////////////////////////
// USB
//////////////////////////////////////////////////////////////////////

BYTE InReport[HID_IN_REPORT_LENGTH]; // HID IN  Report => Touch data
BYTE OutReport[HID_OUT_REPORT_LENGTH]; // HID OUT Report => only for tests


//////////////////////////////////////////////////////////////////////
// Misc. variables
//////////////////////////////////////////////////////////////////////

BOOL power_up_done = 0;

unsigned char pkt_counter;
unsigned char line_num;
unsigned char last_z;
unsigned char drawing_enabled;
unsigned char check_rotate_button;
unsigned char restart_touch_read; // This is set from usb_set/get_report to initiate reading the ADC


uint8_t dpy_x;
uint8_t dpy_y;

uint8_t fcount;

// Globale Variablen (nicht mit extern in main.h gekennzeichnet), nur fuer die main sichbar
struct IntervalMatrix im_ref;
struct BioHash bioHash_ref;
struct BioHash bioHash_cur;
uint16_t tv[ANZ_MERKMALE];


uint8_t main_cmd = 0;
uint16_t FrmBaseAddr = 0;




// Funktion "pausiert" das System um x 
void delay_ms(long time) {
	unsigned long inner, outer;

	for (outer = 0; outer < time; outer++) {
		// seed the ndrng
		for (inner = 0; inner < 10000; inner++)
			;
	}

}

///////////////////////////////////////////////////////
int main(void) {
	uint8_t i; //Zaehlvariable
	uint8_t j = 0;
	uint8_t hdtresh = 10;
	
	// GUI Komponents //

	struct DpyButton button2; 
	struct DpyButton button3; 
	struct DpyMnuButton m_button1;
	struct DpyMnuButton m_button2;
	struct DpyMnuButton m_button3;
	struct DpyLine line2;
	struct DpyLine line3;
	struct DpyLine line4;
	struct DpyFrame1 mainframe;
	struct DpyFrame1 confframe;

	struct DpyRectangle welcRec;

	struct DpyButton u_button1; 
	struct DpyButton u_button2; 
	struct DpyButton u_button3; 
	struct DpyButton u_button4; 
	struct DpyButton u_button5; 
	struct DpyButton u_button6; 
	struct DpyButton u_button7; 
	struct DpyButton u_button8; 

	struct DpyButton conf_button1; 
	struct DpyButton conf_button2; 
	struct DpyButton conf_button3; 
	struct DpyButton conf_button4; 
	
	DpyButton conf_buttons[4];
	DpyButton buttons[8];
	DpyMnuButton m_buttons[3];
	
	
	// Positionierung und Groesseneinteilung der Elemente		
	welcRec.posX = 14*CHAR_WIDTH-2;
	welcRec.posY = 1*CHAR_HEIGHT-1;
	welcRec.width= 20*CHAR_WIDTH+2;
	welcRec.height=6*CHAR_HEIGHT+1;
	
	line2.posX1 = 14*CHAR_WIDTH; 
	line2.posY1 = 3*CHAR_HEIGHT;
 	line2.posX2 = 34*CHAR_WIDTH;
	line2.posY2 = 3*CHAR_HEIGHT;

	line3.posX1 = 7*CHAR_WIDTH+2; //Verti. Linie fuer das Menue
	line3.posY1 = 0;
 	line3.posX2 = 7*CHAR_WIDTH+2;
	line3.posY2 = 8*CHAR_HEIGHT;

	line4.posX1 = 0;//hori. Linie fuer das Menue
	line4.posY1 = 1*CHAR_HEIGHT;
 	line4.posX2 = 7*CHAR_WIDTH+2;
	line4.posY2 = 1*CHAR_HEIGHT;

	u_button1.posX = 11*CHAR_WIDTH;
	u_button1.posY = 2*CHAR_HEIGHT;
	u_button1.name = "User1";
	u_button1.clicked = FALSE;

	u_button2.posX = 18*CHAR_WIDTH;
	u_button2.posY = 2*CHAR_HEIGHT;
	u_button2.name = "User2";
	u_button2.clicked = FALSE;

	u_button3.posX = 25*CHAR_WIDTH;
	u_button3.posY = 2*CHAR_HEIGHT;
	u_button3.name = "User3";
	u_button3.clicked = FALSE;

	u_button4.posX = 32*CHAR_WIDTH;
	u_button4.posY = 2*CHAR_HEIGHT;
	u_button4.name = "User4";
	u_button4.clicked = FALSE;

	u_button5.posX = 11*CHAR_WIDTH;
	u_button5.posY = 5*CHAR_HEIGHT;
	u_button5.name = "User5";
	u_button5.clicked = FALSE;

	u_button6.posX = 18*CHAR_WIDTH;
	u_button6.posY = 5*CHAR_HEIGHT;
	u_button6.name = "User6";
	u_button6.clicked = FALSE;

	u_button7.posX = 25*CHAR_WIDTH;
	u_button7.posY = 5*CHAR_HEIGHT;
	u_button7.name = "User7";
	u_button7.clicked = FALSE;

	u_button8.posX = 32*CHAR_WIDTH;
	u_button8.posY = 5*CHAR_HEIGHT;
	u_button8.name = "User8";
	u_button8.clicked = FALSE;

	button2.posX = 27*CHAR_WIDTH;
	button2.posY = 7*CHAR_HEIGHT;
	button2.name = "Cancel";
	button2.clicked = FALSE;

	button3.posX = 14*CHAR_WIDTH;
	button3.posY = 7*CHAR_HEIGHT;
	button3.name = "Ok";
	button3.clicked = FALSE;


	m_button1.posX = 1*CHAR_WIDTH;
	m_button1.posY = 2*CHAR_HEIGHT;
	m_button1.name = "Verif.";
	m_button1.clicked = FALSE;

	m_button2.posX = 1*CHAR_WIDTH;
	m_button2.posY = 3*CHAR_HEIGHT;
	m_button2.name = "Enrol.";
	m_button2.clicked = FALSE;

	m_button3.posX = 1*CHAR_WIDTH;
	m_button3.posY = 4*CHAR_HEIGHT;
	m_button3.name = "Config";
	m_button3.clicked = FALSE;

	conf_button1.posX = 20*CHAR_WIDTH;
	conf_button1.posY = 1*CHAR_HEIGHT;
	conf_button1.name = "05";
	conf_button1.clicked = FALSE;

	conf_button2.posX = 24*CHAR_WIDTH;
	conf_button2.posY = 1*CHAR_HEIGHT;
	conf_button2.name = "10";
	conf_button2.clicked = TRUE;

	conf_button3.posX = 28*CHAR_WIDTH;
	conf_button3.posY = 1*CHAR_HEIGHT;
	conf_button3.name = "15";
	conf_button3.clicked = FALSE;

	conf_button4.posX = 32*CHAR_WIDTH;
	conf_button4.posY = 1*CHAR_HEIGHT;
	conf_button4.name = "20";
	conf_button4.clicked = FALSE;

	m_buttons[0] = m_button1;
	m_buttons[1] = m_button2;
	m_buttons[2] = m_button3;
	
	// hinzufuegen der Userbuttons in das array welches spaeter in den frame eingebunden wird
	buttons[0] = u_button1;
	buttons[1] = u_button2;
	buttons[2] = u_button3;
	buttons[3] = u_button4;
	buttons[4] = u_button5;
	buttons[5] = u_button6;
	buttons[6] = u_button7;
	buttons[7] = u_button8;

	// hinzufuegen der Configbuttons in das array welches spaeter in den frame eingebunden wird
	conf_buttons[0] = conf_button1;
	conf_buttons[1] = conf_button2;
	conf_buttons[2] = conf_button3;
	conf_buttons[3] = conf_button4;


	mainframe.buttonLst = buttons;

	mainframe.buttonLstLen = 8;

	confframe.buttonLst = conf_buttons;

	confframe.buttonLstLen = 4;



	ConfigurePLL();

 	// ETM OFF!
	PINSEL10 = 0;

	init_ports();
	init_serial();
	// Test: disable ADC
	FIO0SET = SPI_TOUCH_CSN; // P0.16 = SS_TOUCH = 1
	FIO0DIR |= SPI_TOUCH_CSN; // P0.16 = SS_TOUCH

	SPI_Init();
	rtc_init();
	touch_init();
	init_display();
	init_usb_serial_number();
	dpy_send_cmd8(DPY_CTRL0 | DPY_CTRL1, DPY_CMD_DISPLAY_REV);

	// draw version number
	dpy_set_cursor(200, 0);
	dpy_draw_char('V', DPY_DRAW_INVERTED);
	dpy_draw_num8(USB_DeviceDescriptor[13], DPY_DRAW_INVERTED);
	dpy_draw_char('.', DPY_DRAW_INVERTED);
	dpy_draw_num8(USB_DeviceDescriptor[12], DPY_DRAW_INVERTED);


	// Delay for Logo Display
	delay_ms(800);
	dpy_send_cmd8(DPY_CTRL0 | DPY_CTRL1, DPY_CMD_DISPLAY_NORMAL);
 	delay_ms(800);

	power_up_done = 1;
	timer_init();
	touch_calibrate(0);

	drawing_enabled = true;
	LED_ORANGE_OFF
	LED_GREEN_ON

#define TOUCH_RX_PLATE  1500.0
#define TOUCH_RY_PLATE   150.0
	
    dpy_clear();
	dpy_set_cursor(0, 0);


	
	set_flash_cur_adr(FLASH_START_ADR_REF); //Schreibadresse der Referenzdaten im Flash ROM festlegen

	// Anzeige nach der Neuinitialisierung
	dpy_draw_rect_struct(welcRec);	
	dpy_draw_line_struct(line2);
	dpy_set_cursor(18*CHAR_WIDTH, 2*CHAR_HEIGHT);
	dpy_draw_string(" Welcome!", strlen(" Welcome!"), DPY_DRAW_SOLID);
	dpy_set_cursor(16*CHAR_WIDTH, 5*CHAR_HEIGHT);
	dpy_draw_string(" NaturaSign...", strlen(" NaturaSign..."), DPY_DRAW_SOLID);


	// Dauerschleife
	while(1){
		dpy_draw_line_struct(line3); //Menue zeichnen
		dpy_draw_line_struct(line4); //Menue zeichnen
		// Setzt den Cursor
		dpy_set_cursor(1*CHAR_WIDTH, 0*CHAR_HEIGHT);
		// Schreibt einen String an Cursorposition
		dpy_draw_string(" MENU", strlen(" MENU"), DPY_DRAW_SOLID);
		// Setzt den Cursor
		dpy_set_cursor(1*CHAR_WIDTH, 2*CHAR_HEIGHT);
		dpy_draw_mnuButton_struct(m_buttons[0]);
		dpy_draw_mnuButton_struct(m_buttons[1]);
		dpy_draw_mnuButton_struct(m_buttons[2]);

		//Fragt Sensor ab

		get_next_touch();
		//if(is_touched())
		//{
			LED_GREEN_ON
			LED_ORANGE_OFF
			touch_normalize_xy();						   
			touch_calc_z();
			
			dpy_set_cursor(34*CHAR_WIDTH, 4*CHAR_HEIGHT);
			
			//Displaykoordinaten berechnen basierend auf Sensordaten
			dpy_x = touch_x	* ((float) DPY_XSIZE / (float) TOUCH_OUTPUT_RANGE_X);
			dpy_y = touch_y	* ((float) DPY_YSIZE / (float) TOUCH_OUTPUT_RANGE_Y);
		
			
			//If Button area is touched, change status 
			//wenn einer der beiden Menupunkte zum Enrollment oder Verifikation gewaehlt worden ist
			if(!m_buttons[2].clicked && (m_buttons[0].clicked || m_buttons[1].clicked)){
			// ueberpruefe, dass nich 2 Buttons gleichzeitig aktiv sind
			for(i=0;i<mainframe.buttonLstLen;i++){
				if(is_button_touched(mainframe.buttonLst[i], dpy_x, dpy_y)){
				    for(j=0; j<mainframe.buttonLstLen; j++){
					 	if(buttons[i].name!=buttons[j].name)
					 	buttons[j].clicked = FALSE;
					}
					//aktiviere Button falls er vorher inaktiv war und zeichne frame neu
					if(!mainframe.buttonLst[i].clicked){
						mainframe.buttonLst[i].clicked = TRUE;
						dpy_draw_multiframe_struct(mainframe);
					// deaktiviere Button falls er vorher aktiv war  und zeichne frame neu
					}else
					{
					 	mainframe.buttonLst[i].clicked = FALSE;
						dpy_draw_multiframe_struct(mainframe);
					}
				}
			}
			}

			//If Button area is touched, change status
			// wenn das Konfigurationsmenu angewaehlt worden ist
			if(m_buttons[2].clicked){
			for(i=0;i<confframe.buttonLstLen;i++){
				// ueberpruefe, dass nich 2 Buttons gleichzeitig aktiv sind
				if(is_button_touched(confframe.buttonLst[i], dpy_x, dpy_y)){
				    for(j=0; j<mainframe.buttonLstLen; j++){
					 	if(conf_buttons[i].name!=conf_buttons[j].name)
					 	conf_buttons[j].clicked = FALSE;
					}
					//aktiviere Button falls er vorher inaktiv war, weise Wert zu und zeichne Frame neu
					if(!confframe.buttonLst[i].clicked){
						confframe.buttonLst[i].clicked = TRUE;
						// jenachdem welcher Button angeklickt worden ist weise dem Treshold den zugehoerigen Wert zu
						switch(i)
						{
  						 case 0:  hdtresh = 5; break;
  						 case 1:  hdtresh = 10; break;
   						 case 2:  hdtresh = 15; break;
   						 case 3:  hdtresh = 20; break;
						 default:  hdtresh = 20; break;
						}
						dpy_set_cursor(9*CHAR_WIDTH, 1*CHAR_HEIGHT);
						dpy_draw_string("Treshold:", strlen("Treshold:"), DPY_DRAW_SOLID);
						dpy_draw_multiframe_struct(confframe);
					//deaktiviere Button falls er vorher inaktiv war und zeichne Frame neu
					}else
					{
					 	confframe.buttonLst[i].clicked = FALSE;
						dpy_set_cursor(9*CHAR_WIDTH, 1*CHAR_HEIGHT);
						dpy_draw_string("Treshold:", strlen("Treshold:"), DPY_DRAW_SOLID);
						dpy_draw_multiframe_struct(confframe);
					}
				}
			}
			}

			//If m_Button area is touched, change status
			for(i=0; i<3; i++){
				if(is_mnuButton_touched(m_buttons[i], dpy_x, dpy_y)){
					if(!m_buttons[i].clicked){
						m_buttons[i].clicked = TRUE;
					}else
					{
					 	m_buttons[i].clicked = FALSE;
					}
				    for(j=0; j<3; j++){
					 	if(m_buttons[i].name!=m_buttons[j].name)
					 	m_buttons[j].clicked = FALSE;
					}
					// zeige Userframe falls Enr. oder Ver. Menu ausgewaehlt wurde
					if(!m_buttons[2].clicked && (m_buttons[0].clicked || m_buttons[1].clicked)){
						dpy_clear();
					   	dpy_draw_multiframe_struct(mainframe);
					}
					//ansonsten zeige das Konfigurationsmenu an
					else{
						dpy_clear();
						dpy_set_cursor(9*CHAR_WIDTH, 1*CHAR_HEIGHT);
						dpy_draw_string("Treshold:", strlen("Treshold:"), DPY_DRAW_SOLID);
						dpy_draw_multiframe_struct(confframe);
					}
				}
			}

			// sollte keiner der der Menupunkte gewaehlt worden sein zeige den Willkommensbildschirm an
			if(!m_buttons[0].clicked && !m_buttons[1].clicked && !m_buttons[2].clicked){
				dpy_clear();
				dpy_draw_rect_struct(welcRec);	
				dpy_draw_line_struct(line2);
				dpy_set_cursor(18*CHAR_WIDTH, 2*CHAR_HEIGHT);
				dpy_draw_string(" Welcome!", strlen(" Welcome!"), DPY_DRAW_SOLID);
				dpy_set_cursor(16*CHAR_WIDTH, 5*CHAR_HEIGHT);
				dpy_draw_string(" NaturaSign...", strlen(" NaturaSign..."), DPY_DRAW_SOLID);	
			}



// ------------ ENROLLMENT ------------------------//
// sollte der Nutzer sich in Menupunkt Enr. befinden und der Button fuer User1 geklickt worden sein
		if(buttons[0].clicked && m_buttons[1].clicked){ // Enrollment
			dpy_clear();
			delay_ms(180); // Fuer 180 ms den gedrueckten Button darstellen
			buttons[0].clicked = FALSE;
			m_buttons[1].clicked  = FALSE;
			// rufe funktion aus authentication.c auf
			enrollment(&im_ref, &bioHash_ref, FLASH_START_ADR_REF);		
		}
		
// ------------------- Verifikation -------------------//
// sollte der Nutzer sich in Menupunkt Ver. befinden und der Button fuer User1 geklickt worden sein
		if(buttons[0].clicked && m_buttons[0].clicked ){ // Verifikation
			dpy_clear();
			delay_ms(180); // Fuer 180 ms den gedrueckten Button darstellen
			buttons[0].clicked = FALSE;
			m_buttons[0].clicked  = FALSE;
			// rufe funktion aus authentication.c auf
			verification(&im_ref, &bioHash_ref, &bioHash_cur, FLASH_START_ADR_REF, hdtresh);
		}
	}
	
}