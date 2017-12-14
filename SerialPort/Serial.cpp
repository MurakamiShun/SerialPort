#include "Serial.hpp"

#include <Windows.h>
#include <SetupAPI.h>
#pragma comment(lib, "setupapi.lib")

SerialPort::SerialPort(const unsigned int _num, const std::string _name, const std::string _fullname):
	num(_num),
	name(_name),
	fullname(_fullname)
{
	handle = nullptr;
	isOpened = false;
}
SerialPort::SerialPort(const SerialPort& port):
	num(port.num),
	name(port.name),
	fullname(port.fullname),
	isOpened(port.isOpened),
	state(port.state),
	handle(handle)
{

}

SerialPort::~SerialPort(){
	if (isOpened) {
		close();
	}
}

bool SerialPort::open(){
	State state;
	state.baudRate = 9600;
	state.byteSize = 8;
	state.parity = State::Parity::NO;
	state.stopBits = State::StopBits::ONE;
	state.useParity = true;
	return open(state);
}

bool SerialPort::open(const State & _state){
	if (isOpened) {
		return true;
	}
	//オープン
	std::string path = "\\\\.\\" + name;
	handle = CreateFile(
		path.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	unsigned long error = GetLastError();
	if (handle == INVALID_HANDLE_VALUE) {
		isOpened = false;
		return false;
	}

	//設定を取得
	DCB dcb;
	GetCommState(handle, &dcb);
	//設定を変更
	state = _state;
	dcb.BaudRate = state.baudRate;
	dcb.ByteSize = state.byteSize;
	switch (state.parity){
	case State::Parity::NO:
		dcb.Parity = NOPARITY;
		break;
	case State::Parity::EVEN:
		dcb.Parity = EVENPARITY;
		break;
	case State::Parity::ODD:
		dcb.Parity = ODDPARITY;
		break;
	}
	dcb.fParity = state.useParity;
	switch (state.stopBits){
	case State::StopBits::ONE:
		dcb.StopBits = ONESTOPBIT;
		break;
	case State::StopBits::ONE5:
		dcb.StopBits = ONE5STOPBITS;
		break;
	case State::StopBits::TWO:
		dcb.StopBits = TWOSTOPBITS;
		break;
	}
	//セット
	SetCommState(handle, &dcb);

	isOpened = true;
	return true;
}

void SerialPort::close(){
	if (isOpened) {
		CloseHandle(handle);
	}
	isOpened = false;
}

SerialPort::State& SerialPort::getState()
{
	return state;
}

void SerialPort::setState(){
	//設定を取得
	DCB dcb;
	GetCommState(handle, &dcb);
	//設定を変更
	dcb.BaudRate = state.baudRate;
	dcb.ByteSize = state.byteSize;
	switch (state.parity) {
	case State::Parity::NO:
		dcb.Parity = NOPARITY;
		break;
	case State::Parity::EVEN:
		dcb.Parity = EVENPARITY;
		break;
	case State::Parity::ODD:
		dcb.Parity = ODDPARITY;
		break;
	}
	dcb.fParity = state.useParity;
	switch (state.stopBits) {
	case State::StopBits::ONE:
		dcb.StopBits = ONESTOPBIT;
		break;
	case State::StopBits::ONE5:
		dcb.StopBits = ONE5STOPBITS;
		break;
	case State::StopBits::TWO:
		dcb.StopBits = TWOSTOPBITS;
		break;
	}
	//セット
	SetCommState(handle, &dcb);
}

std::vector<char> SerialPort::receive(){
	unsigned long readSize;
	unsigned long error;
	COMSTAT stat;
	std::vector<char> data;

	ClearCommError(handle, &error, &stat);
	if (stat.cbInQue == 0) {
		return data;
	}
	char* buff = new char[stat.cbInQue];
	ReadFile(handle, buff, stat.cbInQue, &readSize, NULL);
	for (unsigned int i = 0; i < readSize; i++) {
		data.push_back(buff[i]);
	}
	delete[] buff;
	return data;
}

void SerialPort::send(std::initializer_list<char> list){
	char* buff = new char[list.size()];
	unsigned int i = 0;
	for (char c:list) {
		buff[i] = c;
		i++;
	}
	unsigned long writtenSize;
	WriteFile(handle, buff, list.size(), &writtenSize, NULL);

	delete[] buff;
}

const unsigned int SerialPort::getNumber() const{
	return num;
}

const std::string & SerialPort::getPortName() const{
	return name;
}

const std::string & SerialPort::getFullName() const{
	return fullname;
}

std::vector<SerialPort> SerialPort::getList(){
	std::vector<SerialPort> list;
	HDEVINFO hinfo = NULL;
	SP_DEVINFO_DATA info_data = { 0 };
	info_data.cbSize = sizeof(SP_DEVINFO_DATA);

	GUID guid;
	unsigned long guid_size = 0;
	if (SetupDiClassGuidsFromName("Ports", &guid, 1, &guid_size) == FALSE)
		//SetupDiDestroyDeviceInfoList(hinfo);
		return list;

	hinfo = SetupDiGetClassDevs(&guid, 0, 0, DIGCF_PRESENT | DIGCF_PROFILE);
	if (hinfo == INVALID_HANDLE_VALUE)
		return list;

	unsigned int index = 0;

	while (SetupDiEnumDeviceInfo(hinfo, index, &info_data)) {
		index++;
		unsigned long type;
		unsigned long size;
		char name_buff[MAX_PATH];
		std::string name;
		std::string fullname;
		unsigned int num;

		//フレンドリーネーム
		if (SetupDiGetDeviceRegistryProperty(
			hinfo, &info_data, SPDRP_DEVICEDESC, &type, (BYTE*)name_buff, MAX_PATH, &size)
			== TRUE) {
			fullname = name_buff;
		}

		//とりあえず開く
		HKEY hkey = SetupDiOpenDevRegKey(hinfo, &info_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
		if (hkey) {
			unsigned long temp_type = REG_SZ;
			//ポート名取得
			RegQueryValueEx(hkey, "PortName", 0, &type, (LPBYTE)name_buff, &size);
			//クローズ
			RegCloseKey(hkey);
			name = name_buff;
			num = atoi(&name_buff[3]);
		}
		list.push_back(SerialPort(num, name, fullname));
	}
	SetupDiDestroyDeviceInfoList(hinfo);
	return list;
}


/*
vector<string> listup() {
HMODULE h = { 0 };
char* buff = new char[65535];
vector<string> list;


//関数が使用可能か判断
if ((h = GetModuleHandle("kernel32.dll")) == NULL || GetProcAddress(h, "QueryDosDeviceA") == NULL) {
return list;
}
//とりあえずデバイスの一覧取得
if (QueryDosDevice(NULL, buff, 65535)) {
//選別
char* p = buff;
while (*p != '\0') {
string device = p;
p += device.length() + 1;
if (device.find("COM") == 0) {
list.push_back(device);
char b[1024];
QueryDosDevice(device.c_str(), b, 1024);
cout << b << endl;
}
}
}
delete[] buff;
return list;
}
*/