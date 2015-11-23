#ifndef DISP_HPP
#define DISP_HPP

/**
 * @brief JUDAH - Jacob is equipped with a text-based user interface
 *
 * @file disp.hpp
 * @author  Norbert Bátfai <nbatfai@gmail.com>
 * @version 0.0.1
 *
 * @section LICENSE
 *
 * Copyright (C) 2015 Norbert Bátfai, batfai.norbert@inf.unideb.hu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * JACOB, https://github.com/nbatfai/jacob
 *
 * "The son of Isaac is Jacob." The project called Jacob is an experiment
 * to replace Isaac's (GUI based) visual imagination with a character console.
 *
 * ISAAC, https://github.com/nbatfai/isaac
 *
 * "The son of Samu is Isaac." The project called Isaac is a case study
 * of using deep Q learning with neural networks for predicting the next
 * sentence of a conversation.
 *
 * SAMU, https://github.com/nbatfai/samu
 *
 * The main purpose of this project is to allow the evaluation and
 * verification of the results of the paper entitled "A disembodied
 * developmental robotic agent called Samu Bátfai". It is our hope
 * that Samu will be the ancestor of developmental robotics chatter
 * bots that will be able to chat in natural language like humans do.
 *
 */

#include <cstring>
#include <sstream>

#include <ncurses.h>
#include <mutex>

class Disp
{
public:

  Disp()
  {
    initscr();

    cbreak();
    noecho();
    timeout ( 0 );
    curs_set ( FALSE );

    clear();

    int  max_x, max_y;
    getmaxyx ( stdscr, max_y, max_x );

    mx = 0;
    my = 0;

    vi_w = newwin ( 10+2, max_x, 0, 0 );
    log_w = newwin ( max_y- ( 10+2 ) - 3, max_x, 10+2, 0 );
    log_iw = newwin ( max_y- ( 10+2 ) - 3 -2, max_x-2, 10+2+1, 1 );
    shell_w = newwin ( 3, max_x, 10+2+max_y- ( 10+2 ) - 3, 0 );

    start_color();
    /*
    init_pair ( 1,COLOR_WHITE, COLOR_BLUE );
    init_pair ( 2, COLOR_WHITE, COLOR_YELLOW );
    init_pair ( 3, COLOR_YELLOW, COLOR_BLUE );
    */
    /*
    init_pair ( 1, COLOR_BLACK, COLOR_WHITE );
    init_pair ( 2, COLOR_WHITE, COLOR_YELLOW );
    init_pair ( 3, COLOR_WHITE, COLOR_RED );
    */
    init_pair ( 1, COLOR_BLACK, COLOR_WHITE );
    init_pair ( 2, COLOR_WHITE, COLOR_MAGENTA );
    init_pair ( 3, COLOR_WHITE, COLOR_RED );
    
    init_pair ( 4, COLOR_BLACK, COLOR_CYAN );
    init_pair ( 5, COLOR_BLACK, COLOR_GREEN );
    init_pair ( 6, COLOR_BLUE, COLOR_YELLOW );
    init_pair ( 7, COLOR_BLACK, COLOR_MAGENTA );
    init_pair ( 8, COLOR_CYAN, COLOR_RED );
    init_pair ( 9, COLOR_WHITE, COLOR_BLACK );
    init_pair ( 10, COLOR_MAGENTA, COLOR_BLACK );
    init_pair ( 11, COLOR_GREEN, COLOR_MAGENTA );

    wbkgd ( vi_w, COLOR_PAIR ( 1 ) );
    wbkgd ( log_w, COLOR_PAIR ( 2 ) );
    wbkgd ( log_iw, COLOR_PAIR ( 2 ) );
    wbkgd ( shell_w, COLOR_PAIR ( 1 ) );

    nodelay ( shell_w, TRUE );
    keypad ( shell_w, TRUE );
    scrollok ( log_iw, TRUE );

    ui( );

  }

  ~Disp()
  {
    delwin ( vi_w );
    delwin ( log_w );
    delwin ( log_iw );
    delwin ( shell_w );
    endwin();
  }

  void shell ( std::string msg )
  {
    ncurses_mutex.lock();
    ui();
    werase ( shell_w );
    box ( shell_w, 0, 0 );
    mvwprintw ( shell_w, 0, 1, " Caregiver shell " );
    mvwprintw ( shell_w, 1, 1, "Norbi> " );
    waddstr ( shell_w, msg.c_str() );
    wrefresh ( shell_w );
    ncurses_mutex.unlock();
  }

  void vi ( std::string msg )
  {
    if ( ncurses_mutex.try_lock() )
      {
        ui();
        werase ( vi_w );
        wmove ( vi_w, 1, 0 );
        waddstr ( vi_w, msg.c_str() );
        box ( vi_w, 0, 0 );
        mvwprintw ( vi_w, 0, 1, " Samu's visual imagery " );
        wrefresh ( vi_w );
        ncurses_mutex.unlock();
      }
  }

  void vi ( char* vi_console )
  {
    if ( ncurses_mutex.try_lock() )
      {
	
        ui();
        werase ( vi_w );

        for ( int i {0}; i<10; ++i )
          {
            wmove ( vi_w, i+1, 1 );
            for ( int j {0}; j<80; ++j )
              {
                char c = vi_console[i*80+j];

                if ( isprint ( c ) )
                  {
                    if ( isdigit ( c ) )
                      {
                        wattron ( vi_w,COLOR_PAIR ( c-'0'+2 ) );
                        waddch ( vi_w, c );
                        wattroff ( vi_w,COLOR_PAIR ( c-'0'+2 ) );
                      }
                    else
                      waddch ( vi_w, c );

                  }
              }
          }

        box ( vi_w, 0, 0 );
        mvwprintw ( vi_w, 0, 1, " Samu's visual imagery " );
        wrefresh ( vi_w );
        ncurses_mutex.unlock();
      }
  }

  void log ( std::string msg )
  {
    ncurses_mutex.lock();
    ui();
    msg =  msg + "\n";
    waddstr ( log_iw, msg.c_str() );
    box ( log_w, 0, 0 );
    mvwprintw ( log_w, 0, 1, " Samu's answers " );
    wrefresh ( log_iw );
    ncurses_mutex.unlock();
  }

  void cg_read()
  {
    int ch;
    if ( ( ch = wgetch ( shell_w ) ) != ERR )
      {

        if ( ch == '\n' )
          {
            std::string ret ( buf );
            buf.clear();
            shell ( buf );
            throw ret;
          }
        else if ( ch == KEY_BACKSPACE )
          {
            if ( buf.size() >= 1 )
              {
                buf.pop_back();
                shell ( buf );
              }
          }
        else
          {
            if ( isalnum ( ch ) || isspace ( ch ) )
              {
                if ( buf.length() < 78 )
                  {
                    buf += ch;
                    shell ( buf );
                  }
              }
          }
      }
  }

  int role{0};

  void refresh ( int role )
  {
   // ncurses_mutex.lock();
   // this->role = role;
   // ui();
   // ncurses_mutex.unlock();
  }

  
private:

  void ui ( void )
  {
    
    /*
#ifdef NETCHAT

if(!role) 
{
    wbkgd ( log_w, COLOR_PAIR ( 2 ) );
    wbkgd ( log_iw, COLOR_PAIR ( 2 ) );
}
else
{
    wbkgd ( log_w, COLOR_PAIR ( 3 ) );
    wbkgd ( log_iw, COLOR_PAIR ( 3 ) );  
}
        wrefresh ( log_w );
        wrefresh ( log_iw );

#endif    
    */
    
    int  max_x, max_y;

    getmaxyx ( stdscr, max_y, max_x );

    if ( mx != max_x || my != max_y )
      {
        mx = max_x;
        my = max_y;

        wresize ( vi_w, 10+2, mx );
        mvwin ( vi_w, 0, 0 );
        werase ( vi_w );

        wresize ( log_w, my- ( 10+2 ) - 3, mx );
        mvwin ( log_w, 10+2, 0 );
        werase ( log_w );

        wresize ( log_iw, my- ( 10+2 ) - 3-2, mx-2 );
        mvwin ( log_iw, 10+2+1, 1 );
        werase ( log_iw );

        wresize ( shell_w, 3, mx );
        mvwin ( shell_w, 10+2+my- ( 10+2 ) - 3, 0 );
        werase ( shell_w );

        box ( vi_w, 0, 0 );
        mvwprintw ( vi_w, 0, 1, " Samu's visual imagery " );

        box ( log_w, 0, 0 );
        mvwprintw ( log_w, 0, 1, " Samu's answers " );

        box ( shell_w, 0, 0 );
        mvwprintw ( shell_w, 0, 1, " Caregiver shell " );
        mvwprintw ( shell_w, 1, 1, "Norbi> Type your sentence and press [ENTER]" );

        wrefresh ( vi_w );
        wrefresh ( log_w );
        wrefresh ( log_iw );
        wrefresh ( shell_w );
      }
  }

  
  std::mutex ncurses_mutex;
  std::string buf;
  WINDOW *vi_w;
  WINDOW *log_w, *log_iw;
  WINDOW *shell_w;
  int mx {0}, my {0};
};

#endif
