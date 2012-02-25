verification(&im_ref, &bioHash_ref, &bioHash_cur, FLASH_START_ADR_REF, hdtresh){
...
	struct DpyLine line2;
	struct DpyRectangle verRec;

	verRec.posX = 14*CHAR_WIDTH-2;
	verRec.posY = 1*CHAR_HEIGHT-1;
	verRec.width= 20*CHAR_WIDTH+2;
	verRec.height=6*CHAR_HEIGHT+1;
	
	line2.posX1 = 14*CHAR_WIDTH; 
	line2.posY1 = 3*CHAR_HEIGHT;
 	line2.posX2 = 34*CHAR_WIDTH;
	line2.posY2 = 3*CHAR_HEIGHT;
...
 if(hd<hdtresh){
 	dpy_draw_rect_struct(verRec);	
 	dpy_draw_line_struct(line2);
 	dpy_set_cursor(17*CHAR_WIDTH, 2*CHAR_HEIGHT);
 	dpy_draw_string(" Verification!", strlen(" Verification!"), DPY_DRAW_SOLID);
 	dpy_set_cursor(18*CHAR_WIDTH, 5*CHAR_HEIGHT);
 	dpy_draw_string(" successful!", strlen(" successful!"), DPY_DRAW_SOLID);	
 }
 else{
 	dpy_draw_rect_struct(verRec);	
 	dpy_draw_line_struct(line2);
 	dpy_set_cursor(17*CHAR_WIDTH, 2*CHAR_HEIGHT);
 	dpy_draw_string(" Verification!", strlen(" Verification!"), DPY_DRAW_SOLID);
 	dpy_set_cursor(20*CHAR_WIDTH, 5*CHAR_HEIGHT);
 	dpy_draw_string(" failed!", strlen(" failed!"), DPY_DRAW_SOLID);	
 }
 ...
 }