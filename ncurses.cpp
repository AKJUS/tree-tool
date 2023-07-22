// ncurses.cpp

/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE                          
*               National Center for Biotechnology Information
*                                                                          
*  This software/database is a "United States Government Work" under the   
*  terms of the United States Copyright Act.  It was written as part of    
*  the author's official duties as a United States Government employee and 
*  thus cannot be copyrighted.  This software/database is freely available 
*  to the public for use. The National Library of Medicine and the U.S.    
*  Government have not placed any restriction on its use or reproduction.  
*                                                                          
*  Although all reasonable efforts have been taken to ensure the accuracy  
*  and reliability of the software and data, the NLM and the U.S.          
*  Government do not and cannot warrant the performance or results that    
*  may be obtained by using this software or data. The NLM and the U.S.    
*  Government disclaim all warranties, express or implied, including       
*  warranties of performance, merchantability or fitness for any particular
*  purpose.                                                                
*                                                                          
*  Please cite the author in any work or product based on this material.   
*
* ===========================================================================
*
* Author: Vyacheslav Brover
*
* File Description:
*   ncurses classes
*
*/


#undef NDEBUG
#include "common.inc"

#include "ncurses.hpp"
using namespace Common_sp;



#define CTRL(x)     ((x) & 0x1f)



namespace NCurses_sp
{
  
  
int getKey ()
{ 
  // Use wgetch(win) ?? 
  const int escdelay_old = set_escdelay (0);  // PAR
	const int key = getch ();  
	set_escdelay (escdelay_old);
	return key;
}




// NCurses

NCurses::NCurses (bool hideCursor)
{ 
  initscr (); 
  cbreak ();
  noecho ();
  keypad (stdscr, TRUE);
  if (hideCursor)
    curs_set (0);  
  hasColors = has_colors ();
  if (hasColors)
    { EXEC_ASSERT (start_color () == OK); }
  resize ();
  constexpr short bkgdColor = COLOR_BLACK;
  init_pair (1, COLOR_WHITE,   bkgdColor);  // colorNone
  init_pair (2, COLOR_RED,     bkgdColor);
  init_pair (3, COLOR_GREEN,   bkgdColor);
  init_pair (4, COLOR_YELLOW,  bkgdColor);
  init_pair (5, COLOR_BLUE,    bkgdColor);
  init_pair (6, COLOR_MAGENTA, bkgdColor);
  init_pair (7, COLOR_CYAN,    bkgdColor);
  init_pair (8, COLOR_WHITE,   bkgdColor);  
  background = COLOR_PAIR (1);
  bkgdset (background);
  attron (COLOR_PAIR (1)); 
  wclear (stdscr);
}



void NCurses::resize ()
{ 
  int row_max_, col_max_;
  getmaxyx (stdscr, row_max_, col_max_); 
  QC_ASSERT (row_max_ >= 0);
  QC_ASSERT (col_max_ >= 0);
  row_max = (size_t) row_max_;
  col_max = (size_t) col_max_;
}

	
	

// AttrColor

AttrColor::AttrColor (NCurses::Color color,
                      bool active_arg)
: Attr ((attr_t) COLOR_PAIR (color + 1), active_arg)
{}




// Window

Window::Window (size_t global_x_arg,
	              size_t global_y_arg,
	              size_t width_arg,
	              size_t height_arg)
: global_x (global_x_arg)
, global_y (global_y_arg)
, width (width_arg)
, height (height_arg)
, win (newwin ((int) height, (int) width, (int) global_y, (int) global_x))
{
	ASSERT (height);
	ASSERT (width);
	ASSERT (win);
	box (win, 0, 0);  // default characters for the vertical and horizontal lines	  
}




// Field

Field::Field (Window &win_arg,
 			        size_t x_arg,
		          size_t y_arg,
			        size_t width_arg,
			        bool upper_arg,
			        const StringVector &val_arg)
: win (win_arg)
, x (x_arg)
, y (y_arg)
, width (width_arg)
, upper (upper_arg) 
, val (val_arg)
{
  ASSERT (x + width <= win. width);
  ASSERT (y < win. height);
	ASSERT (width > 1);

 	if (upper)
		for (string& s : val)
			if (s. size () == 1)
		  	strUpper (s);
}



void Field::print () const
{ 
	string s;
	FOR_START (size_t, i, val_start, val_start + width)
	  if (i < val. size ())
	    s += val [i];
	  else
	  	s += ' ';
	win. print (x, y, s); 
}



Field::Exit Field::run ()
{
	Exit ex = fieldCancel;
 	const int prev_visibility = curs_set (1);
 	bool done = false;
  string utf8;  // One character
  while (! done)
  {
  	if (val. empty ())
  		{ ASSERT (! val_start); }
  	else
  	{
  	  ASSERT (val_start <= val. size ());
  	  if (val_start == val. size ())
  	  {
  	  	ASSERT (val_start);
  	  	val_start--;
  	  }
  	}
  	ASSERT (pos < width);
		
    if (utf8. empty ())
    {
	  	print ();
			win. cursor (x + pos, y);
		}
		
		const size_t real_pos = val_start + pos;

    const int key = getKey ();  
    if (between<int> (key, ' ', 256) && key != 127)
    {
	    if (key >= 128)  // Non-ASCII UTF-8
	    	utf8 += char (key);    
	    else
	    {
	    	ASSERT (utf8. empty ());
	    	char c = (char) key;
	    	if (upper)
	    	  c = toUpper (c);
	    	utf8 += c;	    		
	    }	    
	    ASSERT (! utf8. empty ());
  		const size_t len = utf8_len (utf8 [0]);
  		ASSERT (len != 1);
  		if (! len || utf8. size () == len)
  		{
  		  auto it = val. begin ();
  		  advance (it, real_pos);
  			val. insert (it, utf8);
  			utf8. clear ();    			
	    	if (pos + 1 < width)
	    	  pos++;
	    	else
	    		val_start++;
	    }
	  }
    else
    {
      ASSERT (utf8. empty ());
	    switch (key)
	    {
	    	case 27:  // ESC
	    		done = true;
		    	break;
	    	case 10:
	    	case KEY_ENTER:	
	    		done = true;
	    		ex = fieldDone;
	    		break;
	    	case KEY_UP:   
	    	case KEY_BTAB:
	    		done = true;
	    		ex = fieldPrev;
	    		break;
	    	case KEY_DOWN:  
	    	case '\t':
	    		done = true;
	    		ex = fieldNext;
	    		break;
	    	case KEY_LEFT:
	    		if (pos)
	    			pos--;
	    		else if (val_start)
	    			val_start--;
	    		else
	    			beep ();
	    		break;
	    	case KEY_RIGHT:
	    		if (real_pos < val. size ())
	    		{
	    			if (pos + 1 < width)
	    			  pos++;
	    			else
	    				val_start++;
	    		}
	    		else
	    			beep ();
	    		break;
	    	// ^left, ^right, clipboard ??
	    	case KEY_HOME:
	    		val_start = 0;
	    		pos = 0;
	    		break;
	    	case KEY_END:
	    		if (val. size () < width)
	    		{
	    			val_start = 0;
	    			pos = val. size ();
	    		}
	    		else
	    		{
	    			val_start = val. size () - width + 1;
	    		  pos = width - 1;
	    		}
	    		break;
	    	case CTRL('k'):
	    		val. eraseMany (real_pos, val. size ());
	    		break;
	    	case KEY_DC:
	    		if (real_pos < val. size ())
	    			val. eraseAt (real_pos);
	    		else
	    			beep ();
	    		break;
	    	case 127:
	    	case KEY_BACKSPACE:
	    		if (real_pos)
	    		{
	    			val. eraseAt (real_pos - 1);
	    			if (pos)
	    			  pos--;
	    			else
	    			{
	    				ASSERT (val_start);
	    				val_start--;
	    			}
	    		}
	    		else
	    			beep ();
	    		break;	    		
	    	default: 
	    		beep ();
	    }
	  }
  }
 	curs_set (prev_visibility);

  return ex;
}




// Form

bool Form::run ()
{
	ASSERT (! fields. empty ());
	
	for (const Field* f : fields)
	{
		ASSERT (f);
		f->print ();
	}
	
	size_t i = 0;
	for (;;)
	{
		const Field* f = fields [i];
		switch (var_cast (f) -> run ())
		{
			case Field::fieldDone:   return true;
			case Field::fieldCancel: return false;
			case Field::fieldNext: 
				i++; 
				if (i == fields. size ())
					i = 0;
				break;
			case Field::fieldPrev: 
				if (i)
					i--;
				else
					i = fields. size () - 1;
				break;
		}
	}
	NEVER_CALL;
}



}  // namespace


