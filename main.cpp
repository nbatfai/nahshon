/**
 * @brief JUDAH - Jacob is equipped with a text-based user interface
 *
 * @file main.hpp
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
 * [1] Norbert Bátfai, "A disembodied developmental robotic agent
 * called Samu Bátfai" (submitted manuscript)
 */

#include <iostream>
#include <string>
#include <sstream>
#include <signal.h>
#include "samu.hpp"

Samu samu {"Nahshon", "samu.soul.txt"};

bool halted {false};

void save_samu ( int sig )
{

  if ( halted )
    return;
  halted = true;

#ifndef Q_LOOKUP_TABLE
  samu.save ();
#endif

  samu.halt();
  exit ( 0 );
}

double to_samu ( int channel, SPOTriplets &tv )
{
  double r {0.0};

  try
    {
      samu.triplet ( channel, tv );
      r = samu.reward();
    }
  catch ( const char* err )
    {
      std::cerr << err << std::endl;
    }
  return r;
}


double to_samu ( int channel, std::string &msg )
{
  double r {0.0};

  try
    {
      samu.sentence ( channel, msg );
      r = samu.reward();
    }
  catch ( const char* err )
    {
      std::cerr << err << std::endl;
    }
  return r;
}

double to_samu ( int channel, std::string &msg, std::string &key )
{
  double r {0.0};

  try
    {
      samu.sentence ( channel, msg, key );
      r = samu.reward();
    }
  catch ( const char* err )
    {
      std::cerr << err << std::endl;
    }
  return r;
}

std::map<std::string, SPOTriplets> cache;

int samuHasAlreadyLearned {7};
int reinforcement {0};

double read_cache ( std::string & key, int &cnt, int &brel )
{
  double sum {0.0};
  int count {0};
  for ( auto const & t: cache[key] )
    {
      if ( !samu.sleep() )
        break;

      if ( count++ >= samuHasAlreadyLearned )
        break;

      SPOTriplets tv;
      tv.push_back ( t );
      sum += to_samu ( 12, tv );
      ++cnt;
      brel += samu.get_brel();
    }

  return sum;
}

int main ( int argc, char **argv )
{
  /*
  #ifndef Q_LOOKUP_TABLE
    std::string samuImage {"samu.soul.txt"};

    std::fstream samuFile ( samuImage,  std::ios_base::in );
    if ( samuFile )
      samu.load ( samuFile );
  #endif
  */

  struct sigaction sa;
  sa.sa_handler = save_samu;
  sigemptyset ( &sa.sa_mask );
  sa.sa_flags = SA_RESTART;

  sigaction ( SIGINT, &sa, NULL );
  sigaction ( SIGTERM, &sa, NULL );
  sigaction ( SIGKILL, &sa, NULL );
  sigaction ( SIGHUP, &sa, NULL );

  // Do not remove this copyright notice!
  std::cerr << "This program is "
            << samu.get_name()
            << ", the son of Samu Bátfai."
            << std::endl
            << "Copyright (C) 2015 Norbert Bátfai"
            << std::endl
            << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>"
            << std::endl
            << "This is free software: you are free to change and redistribute it."
            << std::endl
            << "There is NO WARRANTY, to the extent permitted by law."
            << std::endl
            << std::endl;


  std::map<std::string, std::vector<std::string>> tests
  {
    {
      "sentences of the paper [1]",
      {
        "A rare black squirrel has become a regular visitor to a suburban garden",
        "This is a car",
        "This car is mine",
        "I have a little car",
        "The sky is blue",
        "The little brown bear has eaten all of the honey",
        "I love Samu"
      }
    },
    {
      "introduce myself",
      {
        "Who are you",
        "I am a robot",
        "What is your name",
        "My name is Judah",
        "Where do you live",
        "I live in Debrecen",
        "How old are you",
        "I am one year old",
        "Where were you born"
        "I was born is Debrecen"
        "What is your favourite colour"
        "My favourite colours are red, white and green"
      }
    }
  };

  std::map<std::string, SPOTriplets> test_triplets
  {
    {
      "introduce myself",
      {
        { "who", "are", "you" },
        { "i", "am", "robot"  },
        {"what",  "is",  "name"},
        {"name", "is", "Judah"},
        {"you", "live", "where"},
        {"I", "live", "Debrecen"},
        {"you", "are", "how"},
        {"i", "am", "one"},
        {"you", "born", "where"},
        {"I", "born", "Debrecen"}
      }
    }
  };

  int j {0};
  int N_e {20};
  std::string training_file = samu.get_training_file();

  samu.set_training_file ( "bbe" );

  double prev_mbrel {0};
  int mbrelc {0};
  int mbrelc2 {0};
  int prev_bad {0};


#ifdef SUPER_OR_REMOTE_COMP
  //for ( int ii {0}; samu.run() && ii < 1000 + 4000 + 5000 + 4000 + 1000; ++ii )
  for ( int ii {0}; samu.run() /*&& ii < 50000*/; ++ii )
#else
  for ( ; samu.run(); )
    //for ( int ii {0}; samu.run() && ii < 50; ++ii )
#endif
    {
      auto start = std::chrono::high_resolution_clock::now();
      double sum {0.0};
      int cnt {0};
      int brel {0};
//      if ( false /* samu.sleep() */ )
//      if (  samu.sleep() )      
      if ( false )
      
        {

#ifdef SUPER_OR_REMOTE_COMP
          /*
                    if ( ii == 1 )
                      {
                        std::cerr << " iter, training file changed " << std::endl;
                        samu.set_training_file ( "bbe" );
                      }
                    else if ( ii == 3 )
                      {
                        std::cerr << " iter, training file changed " << std::endl;
                        training_file = "bbe";
                      }
          */
          /*
                if ( ii == 1000 )
                  {
                    std::cerr << " iter, training file changed " << std::endl;
                    samu.set_training_file ( "bbe" );
                  }
                else if ( ii == 1000 + 4000 )
                  {
                    std::cerr << " iter, training file changed " << std::endl;
                    training_file = "none";
                    samu.set_training_file ( training_file );
                  }
                else if ( ii == 1000 + 4000 + 5000 )
                  {
                    std::cerr << " iter, training file changed " << std::endl;
                    samu.set_training_file ( "bbe" );
                  }
                else if ( ii == 1000 + 4000 + 5000 + 4000 )
                  {
                    std::cerr << " iter, training file changed " << std::endl;
                    training_file = "none";
                    samu.set_training_file ( training_file );
                  }
                  */
#endif

          samu.clear_vi();
          if ( samu.get_training_file() == training_file )
            {
              samu.set_N_e ( N_e );
              for ( int i {0}; i<test_triplets["introduce myself"].size() && samu.sleep(); ++i )
                {
                  SPOTriplets tv;
                  tv.push_back ( test_triplets["introduce myself"][i] );
                  sum += to_samu ( 11, tv );
                  ++cnt;
                  brel += samu.get_brel();
                }
            }
          else
            {
              samu.set_N_e ( N_e );
              std::string key = samu.get_training_file();

              if ( cache.find ( key ) == cache.end() )
                {

                  std::fstream triplet_train ( key+".triplets",  std::ios_base::in );
                  if ( triplet_train )
                    {

                      do
                        {

                          SPOTriplet t;
                          triplet_train >> t;

                          if ( !t.empty() )
                            cache[key].push_back ( t );

                        }
                      while ( !triplet_train.eof() && samu.sleep() );

                      triplet_train.close();

                      sum = read_cache ( key, cnt, brel );

                    }
                  else
                    {

                      std::fstream train ( samu.get_training_file(),  std::ios_base::in );
                      if ( train )
                        {
                          std::string file = key+".triplets";
                          for ( std::string line; std::getline ( train, line ) && samu.sleep(); )
                            {

#ifndef TRIPLET_CACHE
                              sum += to_samu ( 12, line );
#else
                              sum += to_samu ( 12, line, file );
#endif
                              ++cnt;
                              brel += samu.get_brel();

                            }
                          train.close();
                        }

                    }

                }
              else
                {

                  sum = read_cache ( key, cnt, brel );

                }

            }

          //std::cerr << "###### " << ++j << "-th iter " << sum << std::endl;

          double mbrel = ( double ) brel/ ( double ) cnt;
          int bad = ( sum - samu.get_max_reward() * cnt ) / ( samu.get_min_reward() - samu.get_max_reward() );
          std::cerr << ++j
                    << "-th iter, err: "
                    << cnt*samu.get_max_reward() - sum
                    << " ("
                    << sum
                    << ", good: "
                    << ( cnt-bad )
                    << ", bad: "
                    << bad
                    << "), brel%: "
                    << mbrel
                    << ", "
                    << std::chrono::duration_cast<std::chrono::milliseconds> ( std::chrono::high_resolution_clock::now() - start ).count()
                    << " ms "
                    << N_e
                    << std::endl;

          /*
            //if ( fabs ( prev_mbrel - mbrel ) < 1.2 )
            if ( mbrel  < 0.0 )
              ++mbrelc;
            else
              mbrelc = 0;

            if ( fabs ( prev_mbrel - mbrel ) < 2.0 )
              ++mbrelc2;
            else
              mbrelc2 = 0;
          */

          if ( fabs ( prev_mbrel - mbrel ) < 1.0 && prev_bad == bad )
            ++mbrelc;
          else
            mbrelc = 0;

          prev_bad = bad;

          /*
                    if ( fabs ( prev_mbrel - mbrel ) < .2 )
                      ++mbrelc2;
                    else
                      mbrelc2 = 0;
          */
          /*
                    if ( bad <= 3 )
                      {

                        if ( ++reinforcement == 20 )
                          {
                            samuHasAlreadyLearned += 7;
                            reinforcement = 0;
                          }
                      }
                    else if ( mbrelc2 >= 2*samuHasAlreadyLearned )
                      {

                        if ( N_e < samuHasAlreadyLearned )
                          N_e +=1;
                        samu.scale_N_e ( .75 );
                        mbrelc2 = 0;
                        std::cerr << "(mbrelc2) iter, N structure rescaled " << std::endl;

                      }
                    else if ( mbrelc > 2*samuHasAlreadyLearned && bad > 3 )
                      {

                        if ( N_e >samuHasAlreadyLearned )
                          N_e -=1;

                        samu.scale_N_e ( .82 );
                        //N_e += 5;
                        mbrelc = 0;
                        std::cerr << " iter, N structure rescaled " << std::endl;

                        //} else if(!bad)
                        //} else if(bad <cnt/10)
                      }
          */

          if ( /*mbrel > 35.0 &&*/ mbrelc > 20 && bad >=2 )
            {
              samu.scale_N_e();
              //N_e += 5;
              mbrelc = 0;
              std::cerr << " iter, N structure rescaled " << std::endl;

              //} else if(!bad)
            }
          else if ( bad <2 )
            {
              if ( ++reinforcement == 10 )
                {
                  samuHasAlreadyLearned += 7;
                  reinforcement = 0;
                  samu.debug_tree();
                }
            }


          //if ( /*mbrel > 35.0 &&*/ mbrelc > 50 && cnt-bad <= cnt- ( cnt/10 ) )
          /*if ( mbrelc > samuHasAlreadyLearned && bad >= 3 )
            {
              //samu.scale_N_e(.92);


          //N_e += 1;

              //samu.scale_N_e(.6);
              //mbrelc = 0;
              //std::cerr << " iter, N structure rescaled " << std::endl;

              //} else if(!bad)
              //} else if(bad <cnt/10)
            }
          else */
          /*if ( mbrelc2 >= samuHasAlreadyLearned )
                  {

              	                  if(N_e < samuHasAlreadyLearned)
              N_e +=1;
                    samu.scale_N_e(.67);
              mbrelc2 = 0;
                    std::cerr << "(mbrelc2) iter, N structure rescaled " << std::endl;

                  }
                else if ( bad < 3 )
                  {

                    if ( ++reinforcement == 10 )
                      {
                        samuHasAlreadyLearned += 7;
                        reinforcement = 0;
                      }
                  }
          */
          prev_mbrel = mbrel;

        }

      /*
      Author: Norbert Bátfai <batfai.norbert@inf.unideb.hu>
      Date:   Sat Nov 21 11:22:24 2015 +0100

          After the work on the networked chat server for Samu is done, now I have started to work on a network chat client.

          The two typical use cases may be the following:

          - a Samu program talks to another Samu program
          - a Samu program talks to himself

          What I'd really like, of course, is not to have to code this as follows:

          main()
          {
          ...
          if(random() < a_magic_number)
           Samu_starts_talk();
          ...
          }

          but he should do it of his own free will :) A similar suggestion has been already made in paper http://arxiv.org/abs/1511.02889:

          "This is also reinforced by the fact that the incremental learning was implemented in the main.cpp and not in a class of Samu. From a point of view of a programmer, it means that the incremental approach is not yet really part of Samu."

          In the case of starting conversations, it means Samu's free will must be based on his learning algorithm. We are going to focus on trying to find a solution to this challenge in the next edition of Samu.

       */


      else if ( ( ( double ) rand() ) / ( 1.0+RAND_MAX ) < .5 )
        samu.client ();
      
      
      else
        sleep ( 1 );
    }

  return 0;
}
