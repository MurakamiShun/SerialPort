#include <Serial.hpp>
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

/*
UNICODE

int main() {
	std::wcout.imbue(std::locale(""));
	auto list = getSerialList();
	for (const auto info : list) {
		wcout << L"device name:" << info.device_name() << endl;
		wcout << L"name:" << info.port() << L"\n" << endl;
	}
	Serial serial;
	int port;
	cin >> port;
	if (!serial.open(list[port], 9600))
		return -1;
	SerialInfo info = serial.getInfo();
	wcout << L"open success" << endl;
	wcout << L"device name:" << info.device_name() << endl;
	wcout << L"name:" << info.port() << L"\n" << endl;
	while (true) {
		auto v = serial.read();
		for (auto c : v) {
			cout << c;
		}
	}
	return 0;
}
*/
