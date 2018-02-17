#include "Serial.hpp"
#include <iostream>
#include <string>
using namespace std;


int main() {
	auto list = serialList();
	for (const auto info : list) {
		cout << "num:" << info.port << endl;
		cout << "device name:" << info.dev_name << endl;
		cout << "name:" << info.name << "\n" << endl;
	}
	Serial serial;
	if (!serial.open(list[1].port))
		return -1;
	SerialInfo info = serial.getInfo();
	cout << "open success" << endl;
	cout << "num:" << info.port << endl;
	cout << "device name:" << info.dev_name << endl;
	cout << "name:" << info.name << "\n" << endl;
	while (true) {
		auto data = serial.read();
		std::string str;
		for (auto d:data) {
			str.push_back(d);
		}
		cout << str;
	}
	system("pause");
	return 0;
}