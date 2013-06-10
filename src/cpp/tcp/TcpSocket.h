#ifndef TCPSOCKET
#define TCPSOCKET

#include <sys/socket.h>
#include <string>

/**
 * API for tcp-socket server and client
 *
 * The implementation is based on getaddrinfo() manpage's example
 * server and client
 */
class TcpSocket
{
	public:
		/**
		 * Default constructor.
		 */
		TcpSocket();

		/**
		 * Creates a tcp server and starts listening
		 *
		 * @param	port[]	Port on which to listen
		 */
		TcpSocket(char port[]);

		/**
		 * Connects to a tcp server
		 *
		 * @param	port[]	Port on which to connect
		 * @param	host[]	Address of host to connect to
		 */
		TcpSocket(char port[], char host[]);

		/// @overload
		TcpSocket( std::string port, std::string host );

		/**
		 * Destructor, tidies up (call close())
		 */
		~TcpSocket();

		/**
		 * Waits for and accepts incoming connection to server
		 * instance
		 *
		 * @return	Success when an accept has been tried
		 */
		bool accept();

		/**
		 * Read from incoming buffer into string
		 *
		 * @param	recepticle	By reference string to which
		 * 						incoming data will be written
		 * @param	endTag	If given, function will read
		 * 					on port till endTag	is recieved
		 * 
		 * @return	Success of recieving, returns false if
		 * 			the incoming buffer was empty
		 */
		bool read(std::string& recepticle, std::string endTag = "");

		/**
		 * Write string to outgoing buffer, splitting message
		 * as neccesary
		 *
		 * @param	message	String to be sent
		 *
		 * @return	Success of sending
		 */
		bool write(std::string& message);

		/**
		 * Close connection
		 *
		 * @return	Success of closing
		 */
		bool close();

		bool isConnected();
	
	private:
		bool
			isServer,
			_isConnected;

		int
			socketFD,
			connectionFD;

		void
			* peer_addr;

		char
			host[],
			port[];

		socklen_t peer_addr_len;

		/**
		 * Creates a tcp client and connects to host
		 *
		 * @param	port[]	Port on host
		 * @param	host[]	Host which to connect
		 */
		void client( char port[], char host[] );
			
		/**
		 * Function for easily displaying debug messages
		 *
		 * @param	message	Message to be displayed
		 * @param	output	Optional variable to output
		 */
		void debug(const char message[], std::string output = "");
};

#endif
