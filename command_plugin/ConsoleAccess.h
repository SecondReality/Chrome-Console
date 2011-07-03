#include <string>
#include <vector>

// This is a blocking function
// The function will return when the command ends.
class ConsoleAccess
{
public:
	typedef std::vector<char> ConsoleString;
	typedef void(*outputHandler)(const ConsoleString&);
	ConsoleAccess(outputHandler);
	~ConsoleAccess();

	void run();

	void sendString(const ConsoleString&);

private:
	void ReadAndHandleOutput(HANDLE hPipeRead);

	outputHandler callback_;
	HANDLE writeHandle_;
};