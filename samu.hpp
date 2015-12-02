#ifndef SAMU_HPP
#define SAMU_HPP

/**
 * @brief JUDAH - Jacob is equipped with a text-based user interface
 *
 * @file samu.hpp
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
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "nlp.hpp"

#include <queue>
#include <cstdio>
#include <cstring>

#include "nlp.hpp"
#include "ql.hpp"
#include "net.hpp"

#ifndef CHARACTER_CONSOLE
#include <pngwriter.h>
#endif
#include <chrono>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>

#ifdef DISP_CURSES
#include "disp.hpp"
#endif

#include <cctype>
#include <cmath>

/**
 * @brief This enum represents the communication states of Samu.
 *
 * aaa
 *
 * @author Norbert Bátfai
 * @date Dec. 1, 2015
 */
enum SamuState {SLEEP, TERMINAL, NETWORK};


/**
 * @brief This class represents the Nahshon version of Samu.
 *
 * aaa
 *
 * @author Norbert Bátfai
 * @date Dec. 1, 2015
 */
class Samu
{
public:

  Samu ( const char* name, const char* soul ) : name ( name ), soul ( soul )
  {
#ifndef Q_LOOKUP_TABLE

    std::fstream samuFile ( soul,  std::ios_base::in );
    if ( samuFile )
      load ( samuFile );
#endif

    std::unique_lock<std::mutex> lk ( mutex_ );
    cv_.notify_all();
  }

  ~Samu()
  {
#ifndef Q_LOOKUP_TABLE
    {
      save ( soul );
    }
#endif

    run_ = false;
    terminal_thread_.join();
    network_thread_.join();
  }

  bool run ( void ) const
  {
    return run_;
  }

  bool halt ( void )
  {
    run_ = false;
  }


  bool sleep ( void ) const
  {
    return sleep_;
  }

  bool sleep_after ( void ) const
  {
    return sleep_after_;
  }

  void clear_vi ( void )
  {
    vi.clear();
  }

  SPOTriplets str2triplets ( std::string str )
  {
    SPOTriplets tv;

    std::istringstream iss ( str );
    SPOTriplet t;
    iss >> t;

    tv.push_back ( t );

    return tv;
  }

  void FamilyCaregiverShell ( void );
  void NetworkCaregiverShell ( void );
  void terminal ( void )
  {

    {
      std::unique_lock<std::mutex> lk ( mutex_ );
      cv_.wait ( lk );
    }

    FamilyCaregiverShell();
  }
  void network ( void )
  {

    {
      std::unique_lock<std::mutex> lk ( mutex_ );
      cv_.wait ( lk );
    }

#ifdef NETCHAT
    //net.start_server ( 2008 );
    net.start_server ( 2006 );
#endif
    NetworkCaregiverShell();
  }

  void client ( void )
  {
    sleep_ = false;

    net.start_client ( "localhost", 2006 );

    for ( ;; )
      {

        disp.refresh ( 1 );

        std::string msg;
        bool eof = false;

        for ( ; !eof; )
          {
            msg.clear();

            try
              {
                msg = net.client_msg();
		if(msg.size() > 0)
		  break;
              }
            catch ( boost::system::system_error e )
              {
		boost::system::error_code error = e.code();
                std::cerr << e.what() << std::endl;
#ifdef DISP_CURSES
                disp.log ( e.what() );
#endif

                if ( error == boost::asio::error::interrupted )
                  {
                    eof = false;
                  }
                else if ( error == boost::asio::error::eof )
                  {
                    eof = true;
                  }

              }

          }

        if ( msg.size() == 0 || eof )
          break;

        SPOTriplets tv = str2triplets ( msg );

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

  void sentence ( int id, std::string & sentence, std::string & file )
  {
    if ( msg_mutex.try_lock() )
      {

        if ( id != old_talk_id )
          clear_vi();

        old_talk_id = id;

        SPOTriplets tv = nlp.sentence2triplets ( sentence.c_str() );

#ifndef INCREMENTAL_CACHE
        vi << tv;
#endif

        if ( tv.size() )
          {
            std::fstream cache ( file,  std::ios_base::out|std::ios_base::app );
            if ( cache )
              {
                for ( auto t : tv )
                  cache << t << std::endl;

                cache.close();
              }
          }

        msg_mutex.unlock();

      }
    else
      {
        throw "My attention diverted elsewhere.";
      }

  }

  void sentence ( int id, std::string & sentence )
  {
    if ( msg_mutex.try_lock() )
      {

        if ( id != old_talk_id )
          clear_vi();

        old_talk_id = id;

        vi << nlp.sentence2triplets ( sentence.c_str() );

        msg_mutex.unlock();

      }
    else
      {
        throw "My attention diverted elsewhere.";
      }

  }

  void triplet ( int id, SPOTriplets & triplets )
  {
    if ( msg_mutex.try_lock() )
      {

        if ( id != old_talk_id )
          clear_vi();

        old_talk_id = id;

        SPOTriplet response = ( vi << triplets );

        msg_mutex.unlock();



#ifdef NETCHAT
        std::string nr = response.s + ' ' + response.p + ' ' + response.o;

        if ( net.get_role() == SERVER )
          {
            if ( net.has_session() && !sleep_ )
              net.write_session ( nr );
          }
        else
          {
            if ( net.has_session2() && !sleep_ )
              net.write_msg ( nr );
          }

#endif



      }
    else
      {
        throw "My attention diverted elsewhere.";
      }



  }


  std::string Caregiver()
  {
    if ( caregiver_name_.size() > 0 )
      return caregiver_name_[caregiver_idx_];
    else
      return "Undefined";
  }

  void NextCaregiver()
  {
    caregiver_idx_ = ( caregiver_idx_ + 1 ) % caregiver_name_.size();
  }

  double reward ( void )
  {
    return vi.reward();
  }

  void save ( )
  {
    save ( soul );
  }

  void save ( std::string & fname )
  {
#ifdef DISP_CURSES
    disp.log ( "Saving Samu..." );
#else
    std::cerr << "Saving Samu..." << std::endl;
#endif
    vi.save ( fname );
  }

  void load ( std::fstream & file )
  {
#ifdef DISP_CURSES
    disp.log ( "Loading Samu..." );
#else
    std::cerr << "Loading Samu..." << std::endl;
#endif
    vi.load ( file );
  }

  std::string get_training_file() const
  {
    return training_file;
  }

  std::string get_name() const
  {
    return name;
  }

  void set_training_file ( const std::string& filename )
  {
    training_file = filename;
  }

  void set_N_e ( int N_e )
  {
    vi.set_N_e ( N_e );
  }

  void clear_N_e ( void )
  {
    vi.clearn();
  }

  void scale_N_e ( void )
  {
    vi.scalen ( .75 );
  }

  void scale_N_e ( double s )
  {
    vi.scalen ( s );
  }

  int get_brel ( void )
  {
    return vi.brel();
  }

  double get_max_reward ( void ) const
  {
    return vi.get_max_reward();
  }

  double get_min_reward ( void ) const
  {
    return vi.get_min_reward();
  }

  void debug_tree ( void )
  {
    vi.debug_tree ( );
  }

private:

  class VisualImagery
  {

  public:

    VisualImagery ( Samu & samu ) :samu ( samu )
    {}

    ~VisualImagery()
    {}

//#ifdef PLACE_VALUE
    double w2d ( std::string w )
    {
      double base = 'z'-'a';
      double d {0.0};
      int exp {1};

      for ( char& c : w )
        {
          char lc = std::tolower ( c );
          d += ( lc-'a' ) *std::pow ( base, -exp++ );
        }
      return d;
    }
//#endif


    SPOTriplet operator<< ( std::vector<SPOTriplet> triplets ) throw ( const char * )
    {

      if ( !triplets.size() )
        throw "Empty triplet vector";

      for ( auto triplet : triplets )
        {
          if ( program.size() >= stmt_max )
            program.pop();

          program.push ( triplet );
        }

#ifdef FEELINGS
      if ( feelings.size() >= stmt_max )
        feelings.pop();

      feelings.push ( ql.feeling() );
#endif

      boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();

#ifndef CHARACTER_CONSOLE
      std::string image_file = "samu_vi_"+boost::posix_time::to_simple_string ( now ) +".png";
      char * image_file_p = strdup ( image_file.c_str() );
      pngwriter image ( 256, 256, 65535, image_file_p );
      free ( image_file_p );
#else
      char console[nrows][ncols];
      std::memset ( console, 0, nrows*ncols );
#endif

      char stmt_buffer[1024];
      char *stmt_buffer_p = stmt_buffer;

      std::queue<SPOTriplet> run = program;
#ifdef FEELINGS
      std::queue<Feeling> feels = feelings;
#endif

#ifndef Q_LOOKUP_TABLE

      std::string prg;
      stmt_counter = 0;
#ifdef PYRAMID_VI
      SPOTriplets pyramid;
#endif

#ifdef PLACE_VALUE
      double wbuf[nrows][3];
#endif

      while ( !run.empty() )
        {
          auto triplet = run.front();

          prg += triplet.s.c_str();
          prg += triplet.p.c_str();
          prg += triplet.o.c_str();

#ifdef PLACE_VALUE

          // std::cerr << "iter " << triplet.s << "iter " << triplet.p<< "iter " << triplet.o<< std::endl;

          wbuf[stmt_counter][0] = w2d ( triplet.s );
          wbuf[stmt_counter][1] = w2d ( triplet.p );
          wbuf[stmt_counter][2] = w2d ( triplet.o );
#endif

#ifdef PYRAMID_VI
          pyramid.push_back ( triplet );
          SPOTriplets reverse_pyramid ( pyramid.size() );
          std::reverse_copy ( pyramid.begin(), pyramid.end(), reverse_pyramid.begin() );

          int cnt {0};
          for ( SPOTriplets::iterator it=reverse_pyramid.begin(); it!=reverse_pyramid.end() && cnt < 80; ++it )
            //while ( cnt < 80 )
            //cnt += std::snprintf ( stmt_buffer+cnt, 1024-cnt, "%s.%s(%s);", triplet.s.c_str(), triplet.p.c_str(), triplet.o.c_str() );
            cnt += std::snprintf ( stmt_buffer+cnt, 1024-cnt, "%s.%s(%s);", ( *it ).s.c_str(), ( *it ).p.c_str(), ( *it ).o.c_str() );
#elif JUSTIFY_VI
          int cnt {0};
          while ( cnt < mcols )
            cnt += std::snprintf ( stmt_buffer+cnt, 1024-cnt, "%s.%s(%s);", triplet.s.c_str(), triplet.p.c_str(), triplet.o.c_str() );
#else

#ifndef FEELINGS
#ifdef DRAW_WNUM
          std::snprintf ( stmt_buffer, 1024, "%s.%s(%s); %f.%f(%f)",
                          triplet.s.c_str(), triplet.p.c_str(), triplet.o.c_str(),
                          w2d ( triplet.s ), w2d ( triplet.p ), w2d ( triplet.o ) );
#else
          std::snprintf ( stmt_buffer, 1024, "%s.%s(%s);", triplet.s.c_str(), triplet.p.c_str(), triplet.o.c_str() );
#endif

#else
          if ( !feels.empty() )
            {
              auto s = feels.front();
              std::stringstream ss;
#ifndef FEELINGS
              ss << triplet.s << "." << triplet.p << "(" << triplet.o << ");";
#else
              ss << triplet.s << "." << triplet.p << "(" << triplet.o << "); "
                 << w2d ( triplet.s ) << "." << w2d ( triplet.p ) << "(" << w2d ( triplet.o ) << "); ";
#endif
              std::string spo = ss.str();
              std::snprintf ( stmt_buffer, 1024, "%-30s %s", spo.c_str(), s.c_str() );
            }
#endif
#endif


#ifndef CHARACTER_CONSOLE
          char font[] = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf";
          char *font_p = font;

          image.plot_text_utf8 ( font_p,
                                 11,
                                 5,
                                 256- ( ++stmt_counter ) *28,
                                 0.0,
                                 stmt_buffer_p, 0, 0, 0 );
#else

          std::strncpy ( console[stmt_counter++], stmt_buffer, 80 );

#endif

          run.pop();
#ifdef FEELINGS
          feels.pop();
#endif
        }


#ifdef CELL_AUTOMATA
      char console2[nrows][ncols];
      for ( int i {1}; i<nrows-1; ++i )
        for ( int j {1}; j<ncols-1; ++j )
          console2[i][j] = console[i-1][j]+console[i][j-1]+console[i+1][j]+console[i][j+1];


      for ( int i {1}; i<nrows-1; ++i )
        for ( int j {1}; j<ncols-1; ++j )
          console[i][j] = console2[i][j];
#endif

#ifdef PLACE_VALUE

      double *img_input = new double[nrows*3];
      for ( int i {0}; i<nrows; ++i )
        {
          for ( int j {0}; j<3; ++j )
            {
              img_input[i*3+j] = wbuf[i][j];
              //std::cerr << "iter " << img_input[i*3+j] << " | ";

            }
          //std::cerr << "iter " << std::endl;
        }
#elif CHARACTER_CONSOLE

#ifdef FOUR_TIMES
      double *img_input = new double[2*nrows*2*ncols];
#else
      double *img_input = new double[nrows*ncols];
#endif

#ifdef DISP_CURSES
      std::stringstream con;
#endif

      for ( int i {0}; i<nrows; ++i )
        {
#ifdef DISP_CURSES
          std::string ci;
#endif
          for ( int j {0}; j<ncols; ++j )
            {
#ifdef FOUR_TIMES
              img_input[2*i*ncols+2*j] = ( ( double ) console[i][j] ) / 255.0;
              img_input[2*i*ncols+2*j+1] = ( ( double ) console[i][j] ) / 255.0;
              img_input[2* ( i+1 ) *ncols+2*j] = ( ( double ) console[i][j] ) / 255.0;
              img_input[2* ( i+1 ) *ncols+2*j+1] = ( ( double ) console[i][j] ) / 255.0;
#else
              img_input[i*ncols+j] = ( ( double ) console[i][j] ) / 255.0;
#endif

#ifdef DISP_CURSES
              //if ( isgraph ( console[i][j] ) )
              if ( isprint ( console[i][j] ) )
                ci += console[i][j];
#endif
            }

#ifdef DISP_CURSES
          con << " " << i << ". " << ( ( ci.length() <75 ) ?ci:ci.substr ( 0, 75 ) ) << std::endl;

#endif
        }

#ifdef DISP_CURSES

#ifndef PRINTING_CHARBYCHAR
      samu.disp.vi ( con.str() );
#else
      samu.disp.vi ( &console[0][0] );
#endif

#endif

#else

      double *img_input = new double[256*256];

      for ( int i {0}; i<256; ++i )
        for ( int j {0}; j<256; ++j )
          {
            img_input[i*256+j] = image.dread ( i, j );
          }

#endif

#else
      std::string prg;
      while ( !run.empty() )
        {
          auto triplet = run.front();

          prg += triplet.s.c_str();
          prg += triplet.p.c_str();
          prg += triplet.o.c_str();

          run.pop();
        }
#endif

      auto start = std::chrono::high_resolution_clock::now();

      std::cerr << "QL start... ";

#ifndef Q_LOOKUP_TABLE

      SPOTriplet response = ql ( triplets[0], prg, img_input );

      std::stringstream resp;

      resp << samu.name
#ifdef QNN_DEBUG
           << "@"
           << ( samu.sleep_?"sleep":"awake" )
           << "."
           << ql.get_action_count()
           << "."
           << ql.get_action_relevance()
           << "%"
#endif
           <<"> "
           << response;

      std::string r = resp.str();

      std::cerr << r << std::endl;

      /* még ennek a fgv.-nek a vége előtt jön a read e miatt, ebből fagyó szinkronizációs probléme lesz
      #ifdef NETCHAT
            std::string nr = response.s + ' ' + response.p + ' ' + response.o;

      if( samu.net.get_role() == SERVER)
      {
            if ( samu.net.has_session() && !samu.sleep_ )
              samu.net.write_session ( nr );
      }
      else
      {
            if ( samu.net.has_session2() && !samu.sleep_ )
              samu.net.write_msg ( nr );
      }

      #endif
      */

#ifdef DISP_CURSES
      samu.disp.log ( r );
#endif

#else

      std::cerr << ql ( triplets[0], prg ) << std::endl;

#endif

      std::cerr << std::chrono::duration_cast<std::chrono::milliseconds> ( std::chrono::high_resolution_clock::now() - start ).count()
                << " ms ---"
                <<  std::endl;

#ifndef CHARACTER_CONSOLE

#ifndef Q_LOOKUP_TABLE
      delete[] img_input;
      image.close();
#endif

#else

#ifndef Q_LOOKUP_TABLE
      delete[] img_input;
#endif

#endif

      return response;

    }

    double reward ( void )
    {
      return ql.reward();
    }

    void debug_tree ( void )
    {
      ql.debug_tree ( );
    }

    void save ( std::string &fname )
    {
      ql.save ( fname );
    }

    void load ( std::fstream & file )
    {
      ql.load ( file );
    }

    void clear ( void )
    {
      while ( !program.empty() )
        {
          program.pop();
        }

      ql.clear();
    }

    void set_N_e ( int N_e )
    {
      ql.set_N_e ( N_e );
    }

    void clearn ( void )
    {
      ql.clearn();
    }

    void scalen ( double s )
    {
      ql.scalen ( s );
    }

    int brel ( void )
    {
      return ql.get_action_relevance();
    }

    double get_max_reward ( void ) const
    {
      return ql.get_max_reward();
    }

    double get_min_reward ( void ) const
    {
      return ql.get_min_reward();
    }

  private:

    int nrows = 10;
    int ncols = 80;
    Samu &samu;
    QL ql {nrows};
    std::queue<SPOTriplet> program;
#ifdef FEELINGS
    std::queue<Feeling> feelings;
#endif
    int stmt_counter {0};
    static const int stmt_max = 10;

  };

  std::string soul;

#ifdef DISP_CURSES
  static Disp disp;
#endif

#ifdef NETCHAT
  static Net net;
#endif

  std::string name;

  SamuState state = SLEEP;

  bool run_ {true};
  bool sleep_ {true};
  int sleep_after_ {160};
  unsigned int read_usec_ {50*1000};
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread terminal_thread_ {&Samu::terminal, this};
  std::thread network_thread_ {&Samu::network, this};

  NLP nlp;
  VisualImagery vi {*this};

  int caregiver_idx_ {0};
  std::vector<std::string> caregiver_name_ {"Norbi", "Nandi", "Matyi", "Greta"};

  std::mutex msg_mutex;
  int old_talk_id {-std::numeric_limits<int>::max() };

  std::string training_file;
};

#endif


