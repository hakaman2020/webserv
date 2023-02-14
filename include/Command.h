#ifndef COMMAND_H
# define COMMAND_H

# include "Core.h"
# include "Socket.h"

# include <functional>

namespace webserv {

class Command
{
	public:
	using function_t = std::function<void(std::ostream&, std::string)>;
	using pointer = std::unique_ptr<Command>;

	Command(std::string const& cmd);
	Command(std::string const& cmd, function_t func);
	Command(Command&& cmd);

	~Command();
	Command& add_subcommand(Command* scmd);

	static Command* find(std::string& str);
	void run(std::ostream& out, std::string arguments);

	static void add_command(Command* cmd);
	static void run_thread(void);

	private:
	Command* find_impl(std::stringstream& line_stream, std::stringstream& consumed_stream);

	public:
	std::string name;

	static std::unordered_map<std::string, pointer> s_commands;

	function_t func;
	std::unordered_map<std::string, pointer> subcommands;
};

void terminal_setup(void);
void command_init(std::unordered_map<sockfd_t, Socket*>& fd_map, bool& run);

} // webserv

#endif // COMMAND_H