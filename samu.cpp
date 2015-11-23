/**
 * @brief JUDAH - Jacob is equipped with a text-based user interface
 *
 * @file samu.cpp
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

#include <iostream>
#include <string>
#include <sstream>
#include "samu.hpp"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <ncurses.h>


#ifdef NETCHAT

Net Samu::net;

void Samu::NetworkCaregiverShell ( void )
{

  std::string cmd_prefix = "cmd";

  int sleep {0};

  if ( sleep_ )
    sleep = sleep_after_ + 1;

  int prev_sec {0};
  for ( ; run_ ; )
    {

      try
        {

          net.cg_read();

          if ( ++sleep > sleep_after_ )
            {
              if ( state == NETWORK )
                {
                  std::cerr << "Isaac went to sleep." << std::endl;
#ifdef DISP_CURSES
                  disp.log ( "I went to sleep." );
#endif
                  sleep_ = true;
                  state = SLEEP;
                }
              sleep = sleep_after_ + 1;

            }
          else if ( state == NETWORK )
            {
              std::cerr << sleep << " *** " << std::flush;

              int sec = ( sleep * read_usec_ ) / ( 1000*1000 );
              if ( sec != prev_sec )
                {
                  int after = ( sleep_after_ * read_usec_ ) / ( 1000*1000 );

                  std::stringstream sleep_after;

                  sleep_after << "I will go to sleep after ";
                  sleep_after << ( after-sec );
                  sleep_after <<  " seconds";

#ifdef DISP_CURSES
                  disp.log ( sleep_after.str() );
#endif
                  prev_sec = sec;
                }
            }
        }
      catch ( std::string line )
        {

          if ( state != TERMINAL )
            {
              if ( sleep_ )
                {
                  std::cerr << "Isaac is awake now." << std::endl;
#ifdef DISP_CURSES
                  disp.log ( "I am awake now." );
#endif
                }
              sleep_ = false;
              sleep = 0;
              state = NETWORK;

              disp.refresh(0);

              if ( !line.compare ( 0, cmd_prefix.length(), cmd_prefix ) )
                {
                  std::string readCmd {"cmd read"};

                  size_t f = line.find ( readCmd );
                  if ( f != std::string::npos )
                    {
                      f = f+readCmd.length() +1;
                      if ( f < line.length() )
                        {
                          std::string fname = line.substr ( f );
                          set_training_file ( fname );
                        }
                    }
                  else
                    NextCaregiver();
                }
              else
                {
                  std::istringstream iss ( line );
                  SPOTriplet t;
                  iss >> t;

                  SPOTriplets tv;
                  tv.push_back ( t );

                  try
                    {
                      //sentence ( -1, line );
                      triplet ( -1000, tv );
                    }
                  catch ( const char* err )
                    {
                      std::cerr << err << std::endl;
#ifdef DISP_CURSES
                      disp.log ( err );
#endif
                    }
                }
            }
        }

      usleep ( read_usec_ );

    }

  run_ = false;
}

#else

void Samu::NetworkCaregiverShell ( void )
{
  for ( ; run_ ; )
    usleep ( read_usec_ );

  run_ = false;
}

#endif

#ifdef DISP_CURSES
Disp Samu::disp;

void Samu::FamilyCaregiverShell ( void )
{
  std::string cmd_prefix = "cmd";

  int sleep {0};

  if ( sleep_ )
    sleep = sleep_after_ + 1;

  int prev_sec {0};
  for ( ; run_ ; )
    {

      try
        {

          disp.cg_read();

          if ( ++sleep > sleep_after_ )
            {
              if ( state == TERMINAL )
                {
                  std::cerr << "Isaac went to sleep." << std::endl;
                  disp.log ( "I went to sleep." );
                  sleep_ = true;
                  state = SLEEP;
                }
              sleep = sleep_after_ + 1;

            }
          else if ( state == TERMINAL )
            {
              std::cerr << sleep << " +++" << std::flush;

              int sec = ( sleep * read_usec_ ) / ( 1000*1000 );
              if ( sec != prev_sec )
                {
                  int after = ( sleep_after_ * read_usec_ ) / ( 1000*1000 );

                  std::stringstream sleep_after;

                  sleep_after << "I will go to sleep after ";
                  sleep_after << ( after-sec );
                  sleep_after <<  " seconds";

                  disp.log ( sleep_after.str() );
                  prev_sec = sec;
                }
            }
        }
      catch ( std::string line )
        {
          if ( state != NETWORK )
            {

              if ( sleep_ )
                {
                  std::cerr << "Isaac is awake now." << std::endl;
                  disp.log ( "I am awake now." );
                }
              sleep_ = false;
              sleep = 0;
              state = TERMINAL;

              if ( !line.compare ( 0, cmd_prefix.length(), cmd_prefix ) )
                {
                  std::string readCmd {"cmd read"};

                  size_t f = line.find ( readCmd );
                  if ( f != std::string::npos )
                    {
                      f = f+readCmd.length() +1;
                      if ( f < line.length() )
                        {
                          std::string fname = line.substr ( f );
                          set_training_file ( fname );
                        }
                    }
                  else
                    NextCaregiver();
                }
              else
                {
                  try
                    {
                      sentence ( -1, line );
                    }
                  catch ( const char* err )
                    {
                      std::cerr << err << std::endl;
                      disp.log ( err );
                    }
                }
            }
        }
      usleep ( read_usec_ );

    }

  run_ = false;
}

#else

void Samu::FamilyCaregiverShell ( void )
{
  for ( ; run_ ; )
    usleep ( read_usec_ );

  run_ = false;
}

#endif
