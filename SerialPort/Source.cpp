#include "Serial.hpp"
#include <vector>
#include <string>
#include <iostream>

using namespace std;

int main() {
	auto list = getSerialList();
	for (const auto info : list) {
		cout << "device name:" << info.device_name() << endl;
		cout << "name:" << info.port() << "\n" << endl;
	}
	Serial serial;
	int port;
	cin >> port;
	if (!serial.open(list[port], 9600))
		return -1;
	SerialInfo info = serial.getInfo();
	cout << "open success" << endl;
	cout << "device name:" << info.device_name() << endl;
	cout << "name:" << info.port() << "\n" << endl;
	while (true) {
		auto v = serial.read();
		for (auto c : v) {
			cout << c;
		}
	}
	return 0;
}