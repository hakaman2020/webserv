#include "Request.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace webserv {

Request::Request() : validity(INVALID) {}

RequestType get_request_type(std::string const& word)
{
	if (word == "GET")		return GET;
	if (word == "POST")		return POST;
	if (word == "DELETE")	return DELETE;
	return  UNKNOWN;
}

char const* get_request_string(RequestType type)
{
	switch (type)
	{
		case GET: return "GET";
		case POST: return "POST";
		case DELETE: return "DELETE";
		default: break;
	}
	return "UNKNOWN";
}

void request_print(Request const& request, std::ostream& out)
{
	out << "REQUEST:\n";
	if (request.validity == INVALID)
	{
		out << "INVALID" << std::endl;
		return ;
	}
	
	out << get_request_string(request.type);

	out << ' ' << request.path << ' ' << request.http_version << '\n';

	for (const auto& pair : request.fields)
	{
		out << pair.first << ": " << pair.second << '\n';
	}

	out << std::endl;
}

void parse_header_fields(std::unordered_map<std::string, std::string>& fields, std::vector<char>& buffer, std::stringstream& buffer_stream)
{
	std::string word;
	while (!buffer_stream.eof())
	{
		if (buffer_stream.peek() == '\r' || buffer_stream.peek() == '\n')
			break ;
		buffer_stream >> word;
		if (word.length() == 0)
			break ;
		// remove last character from word (the ':')
		word.erase(word.length() - 1);

		// Field case insensitive
		std::transform(word.begin(), word.end(), word.begin(),
			[](unsigned char c) { return std::tolower(c); });

		std::string key = word;
		std::string value;
		buffer_stream.get(); // skip the space
		std::getline(buffer_stream, value);

		// Field case insensitive
		std::transform(value.begin(), value.end(), value.begin(),
			[](unsigned char c) { return std::tolower(c); });

		if (value.empty())
		{
			// Unknown key
			std::getline(buffer_stream, word);
			continue ;
		}

		if (value.length() > 0 && value.back() == '\r')
			value.erase(value.end() - 1);

		// Add to fields
		fields.emplace(key, value);

	}
	std::getline(buffer_stream, word);
	
	// consume the read part of the buffer
	ssize_t tg = buffer_stream.tellg();
	if (tg > 0)
		buffer.erase(buffer.begin(), buffer.begin() + tg);
	else
		std::cout << "tg == " << tg << std::endl;;
}

// This consumes the part of the buffer that's used
Request request_build(std::vector<char>& buffer)
{
	Request request;
	if (buffer.empty())
		return (request);

	// Create a string-stream from the data
	buffer.push_back(0); // null-termination
	std::stringstream buffer_stream(buffer.data());
	std::string word;

	// First line of REQUEST
	buffer_stream >> word;
	request.type = get_request_type(word);
	if (request.type == UNKNOWN) return (request); // Unsupported request
	
	buffer_stream >> request.path;
	if (request.path.length() == 0) return (request); // No path

	size_t pos = request.path.find_first_of('?');
	if (pos != std::string::npos && pos > 0 && pos < request.path.length())
	{
		request.path_arguments = request.path.substr(pos + 1);
		request.path = request.path.substr(0, pos);
	}

	if (request.path.find("..") != std::string::npos) return (request); // Highly illegal use of ".."
	
	buffer_stream >> request.http_version;
	if (request.http_version.length() == 0) return (request); // No HTTP version
	
	std::getline(buffer_stream, word); // skip line
	parse_header_fields(request.fields, buffer, buffer_stream);

	request.validity = VALID;
	return (request);
}

} // namespace webserv
