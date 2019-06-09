#include "utils.h"

const string get_current_username()
{
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	return pw ? pw->pw_name : "";
}

const string get_home_directory()
{
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	return pw ? pw->pw_dir : "";
}

// Stolen from: https://stackoverflow.com/a/2072890/192001
bool ends_with(std::string const &value, std::string const &ending)
{
	if (ending.size() > value.size())
	{
		return false;
	}
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// Stolen from: https://stackoverflow.com/a/12468109
string random_string(size_t length)
{
	auto randchar = []() -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	string str(length, 0);
	generate_n(str.begin(), length, randchar);
	return str;
}

// Stolen from: https://github.com/ilvn/aes256ctr
void hexdump(char *s, uint8_t *buf, size_t sz)
{
	size_t i;
	printf("%s\n\t", s);
	for (i = 0; i < sz; i++)
		printf("%02x%s", buf[i], ((i % 16 == 15) && (i < sz - 1)) ? "\n\t" : " ");
	printf("\n");
}