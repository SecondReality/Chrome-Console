#include <string>
#include <vector>

class ConsoleAccess
{
public:
	typedef std::vector<char> ConsoleString;
	typedef void(*outputHandler)(const ConsoleString&);
	ConsoleAccess(outputHandler);
	~ConsoleAccess();

	// This is a blocking function
	void run();

	void sendString(const ConsoleString&);

private:
	void ReadAndHandleOutput(HANDLE hPipeRead);

	outputHandler callback_;
	HANDLE writeHandle_;
};