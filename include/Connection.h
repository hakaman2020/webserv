#ifndef CONNECTION_H
# define CONNECTION_H

# include "Core.h"

# include "Request.h"
# include "Response.h"
# include "Server.h"

namespace webserv {

#define HTTP_HEADER_BUFFER_SIZE 8192

class Socket;

class Connection
{
	public:
	enum State
	{
		READY_TO_READ = 0,
		READING,
		READY_TO_WRITE,
		WRITING,
		CGI,
		CLOSE
	};

	public:
	Connection(sockfd_t connection_fd, addr_t address);
	~Connection();

	Request const& get_last_request(void) const;
	Response const& get_last_response(void) const;

	std::string get_ip(void) const;

	State get_state(void) const;

	void on_pollin(Socket& socket, sockfd_t fd);
	void on_pollout(Socket& socket, sockfd_t fd);

	private:
	// unused constructors
	Connection();
	Connection(Connection const& other);
	
	Connection& operator=(Connection const& other);

	// functions
	private:

	void new_request(void);
	void new_request_post(void);
	void continue_request(void);

	void new_response(Socket& socket);
	void new_response_get(Server const& server, Location const& loc);
	void new_response_post(Server const& server, Location const& loc);
	void new_response_delete(Server const& server, Location const& loc);
	void cgi_pollin(void);
	void cgi_pollout(void);
	void continue_response(void);

	Request build_request(std::string buffer);
	void build_request_get(Request& request, std::stringstream& buffer);
	void build_request_post(Request& request, std::stringstream& buffer);
	void build_request_delete(Request& request, std::stringstream& buffer);

	private:
	sockfd_t socket_fd;
	addr_t address;

	Request last_request;
	Response last_response;

	State state;

	struct HandlerData
	{
		Request current_request;
		Response current_response;
		std::string custom_page;
		std::vector<char> buffer;
		std::vector<char> cgi_buffer;
		size_t content_size;
		size_t received_size;
		std::ifstream file;
		int pid;
		struct s_pipes
		{
			int in[2];
			int out[2];
		} pipes;
	} handler_data;

	public:
	bool busy;
};

} // namespace webserv

#endif // CONNECTION_H
