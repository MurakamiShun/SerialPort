#include "Serial.hpp"
#include <iostream>
#include <string>
using namespace std;


int main() {
	auto list = SerialPort::getList();
	int num;
	for (int i = 0; i < list.size(); i++) {
		cout << list[i].getPortName() << ":" << list[i].getFullName() << endl;
		if (list[i].getFullName().find("Arduino") != string::npos) {
			num = i;
		}
	}

	SerialPort usb(list[num]);
	if (usb.open()) {
		while (true) {
			auto data = usb.receive();
			for (auto d:data) {
				cout << d;
			}
		}
	}
	usb.close();
	return 0;
}