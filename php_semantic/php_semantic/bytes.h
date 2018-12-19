#include <vector>

using namespace std;

//vector<char> get_u2(unsigned short number);
//vector<char> get_u4(unsigned long int number);
//vector<char> get_s2(short number);
//vector<char> get_s4(long int number);

union u2
{
	unsigned short number;
	char bytes[2]; 
};

union u4
{
	unsigned long int number;
	char bytes[4]; 
};

union s2
{
	short number;
	char bytes[2]; 
};

union s4
{
	long int number;
	char bytes[4]; 
};

vector<char> get_u2(unsigned short number) {
	vector<char> result = vector<char>();
	u2 data;
	data.number = number;
	result.push_back(data.bytes[1]);
	result.push_back(data.bytes[0]);
	return result;
}

vector<char> get_u4(unsigned long int number) {
	vector<char> result = vector<char>();
	u4 data;
	data.number = number;
	result.push_back(data.bytes[3]);
	result.push_back(data.bytes[2]);
	result.push_back(data.bytes[1]);
	result.push_back(data.bytes[0]);
	return result;
}

vector<char> get_s2(short number) {
	vector<char> result = vector<char>();
	s2 data;
	data.number = number;
	result.push_back(data.bytes[1]);
	result.push_back(data.bytes[0]);
	return result;
}

vector<char> get_s4(long int number) {
	vector<char> result = vector<char>();
	s4 data;
	data.number = number;
	result.push_back(data.bytes[3]);
	result.push_back(data.bytes[2]);
	result.push_back(data.bytes[1]);
	result.push_back(data.bytes[0]);
	return result;
}