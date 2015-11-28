#ifndef NET_HPP
#define NET_HPP

/**
 * @brief Amminadab - "The son of Ram is Amminadab."
 *
 * @file net.hpp
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

#include <boost/asio.hpp>

enum SamuRole {SERVER, CLIENT};

class Net
{
public:

  Net()
  {
  }

  ~Net()
  {
  }

  void write_session ( std::string msg )
  {

    std::cerr << "SW write_session "<< msg << std::endl;

    char data[4096];

    int length = std::sprintf ( data, "%s", msg.c_str() );
    try
      {
        boost::asio::write ( client_socket_, boost::asio::buffer ( data, length ) );
      }
    catch ( std::exception& e )
      {
        std::cerr << "Ooops: " << e.what() << std::endl;
      }
  }

  void write_msg ( std::string msg )
  {

    std::cerr << "CW write_msg "<< msg << std::endl;

    char data[4096];

    int length = std::sprintf ( data, "%s", msg.c_str() );
    try
      {
        boost::asio::write ( socket_, boost::asio::buffer ( data, length ) );
      }
    catch ( std::exception& e )
      {
        std::cerr << "Ooops: " << e.what() << std::endl;
      }
  }


  void read_session ( void )
  {
    char data[4096];

    try
      {
        for ( ;; )
          {
	    // only for testing network latency
	    //sleep(2);
	    
            boost::system::error_code error;
            size_t length = client_socket_.read_some ( boost::asio::buffer ( data ), error );

            if ( ! error )
              {
                role = SERVER;

                if ( length )
                  {
                    std::string s ( data, data+length );
                    buf = s;
                    already_read_ = false;

                    std::cerr << "SR read_session "<< buf << std::endl;

                  }

              }
            else if ( error == boost::asio::error::eof )
              {
                std::cerr << "read_some error: " << error << std::endl;
                hasSession = false;
                break;
              }
              /*
            else if ( error )
              {
                std::cerr << "read_some error: " << error << std::endl;
                throw boost::system::system_error ( error );
              }
*/
          }
      }
    catch ( std::exception& e )
      {
        std::cerr << "Ooooops: " << e.what() << std::endl;
      }
  }

  void start_server ( unsigned short port )
  {
    std::thread t {&Net::server, this, port };
    t.detach();
  }

  void server ( unsigned short port )
  {
    boost::asio::ip::tcp::acceptor acceptor ( io_service_,
        boost::asio::ip::tcp::endpoint ( boost::asio::ip::tcp::v4(), port ) );

    char data[4096];

    for ( ;; )
      {
        boost::asio::ip::tcp::socket socket ( io_service_ );
        acceptor.accept ( socket );

        if ( !hasSession )
          {
            hasSession = true;
            client_socket_ = std::move ( socket );
            std::thread t {&Net::read_session, this };
            t.detach();
          }
        else
          {
            int length = std::sprintf ( data, "Samu is talking with somebody else" );
            try
              {
                boost::asio::write ( socket, boost::asio::buffer ( data, length ) );
              }
            catch ( std::exception& e )
              {
                std::cerr << "Ooooops: " << e.what() << std::endl;
              }

          }
      }
  }

  void start_client ( const char * host, unsigned short port )
  {

    boost::asio::ip::tcp::resolver resolver ( io_service_ );
    boost::asio::ip::tcp::resolver::query query ( boost::asio::ip::tcp::v4(), host, "2006" );
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve ( query );

    boost::asio::ip::tcp::socket socket ( io_service_ );
    boost::asio::connect ( socket, iterator );

    socket_ = std::move ( socket );

    hasSession2 = true;

    char data[4096];

    size_t length = std::sprintf ( data, "I am Samu" );
    socket_.send ( boost::asio::buffer ( data, length ) );

  }

  std::string client_msg ( void )
  {
    char data[4096];

    boost::system::error_code error;
    size_t length = socket_.read_some ( boost::asio::buffer ( data ), error );

    if ( !error )
      {
        role = CLIENT;

        if ( length )
          {
            std::string s ( data, data+length );
            buf2_ = s;
            already_read2_ = false;

            std::cerr << "CR client_msg "<< buf2_ << std::endl;

            return buf2_;
          }

      }
    else if ( error == boost::asio::error::interrupted )
      {
        // resizing ncurses window
        std::cerr << "read_some error: " << error << std::endl;
        throw boost::system::system_error ( error );

      }
    else if ( error == boost::asio::error::eof )
      {

        std::cerr << "read_some error: " << error << std::endl;
        throw boost::system::system_error ( error );

      }

    return "";
  }


  void cg_read ( void )
  {
    if ( buf.size() > 0 &&  !already_read_ /*( ch = wgetch ( shell_w ) ) != ERR*/ )
      {
        already_read_ = true;

        std::string ret ( buf );
        throw ret;

      }
  }

  /*
  void cg_read2 ( void )
  {
    if ( buf2_.size() > 0 &&  !already_read2_  )
      {
        already_read2_ = true;

        std::string ret ( buf2_ );
        throw ret;

      }
  }
  */

  bool has_session ( void ) const
  {
    return hasSession;
  }
  bool has_session2 ( void ) const
  {
    return hasSession2;
  }

  SamuRole get_role ( void ) const
  {
    return role;
  }


private:
  SamuRole role = SERVER;
  bool hasSession {false};
  bool already_read_ {false};
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::socket client_socket_ {io_service_};
  boost::asio::ip::tcp::socket socket_ {io_service_};
  std::string buf;

  bool already_read2_ {false};
  std::string buf2_;
  bool hasSession2 {false};


};

#endif
