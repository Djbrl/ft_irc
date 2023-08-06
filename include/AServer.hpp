#ifndef __ASERVER_HPP__
#define __ASERVER_HPP__

#include "_defines.hpp"

// ASERVER CLASS
// AServer is a pure abstract class that carries all the exceptions needed for our IRC Server
class IrcServerException : public std::exception
{
public:
	IrcServerException();
	virtual ~IrcServerException() throw();
	virtual const char* what() const throw()
	{
		return "IrcServerException: An unknown error occurred.";
	}
};

class AServer
{
public:
	AServer();
	virtual ~AServer();
	virtual void run() = 0;

	// EXCEPTIONS
	class InvalidPortException : public IrcServerException
	{
		public:
			virtual const char* what() const throw()
			{
				return "InvalidPortException: Invalid or occupied port number provided.";
			}
	};

	class SocketCreationException : public IrcServerException
	{
		public:
			virtual const char* what() const throw()
			{
				return "SocketCreationException: Socket creation failed.";
			}
	};

	class BindException : public IrcServerException
	{
		public:
			virtual const char* what() const throw()
			{
				return "BindException: Bind() failed.";
			}
	};

	class ListenException : public IrcServerException
	{
		public:
			virtual const char* what() const throw()
			{
				return "ListenException: Listen() failed.";
			}
	};

	class AcceptException : public IrcServerException
	{
		public:
			virtual const char* what() const throw()
			{
				return "AcceptException: Accept() failed.";
			}
	};

	class ReadSocketException : public IrcServerException
	{
		public:
			virtual const char* what() const throw()
			{
				return "ReadSocketException: Failed to read client socket.";
			}
	};

	class SelectHandlerException : public IrcServerException
	{
		public:
			virtual const char* what() const throw()
			{
				return "SelectHandlerException: Failed to handle socket in select().";
			}
	};

};

#endif
