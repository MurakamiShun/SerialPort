#if defined(_WIN32) || defined(_WIN64)

#include "Serial.hpp"

#include <Windows.h>
#include <SetupAPI.h>
#pragma comment(lib, "setupapi.lib")

#ifdef _MBCS
#define NO_DEVICE "no device"
#define PATH "\\\\.\\"
#define PORTS "Ports"
#define PORTNAME "PortName"
#endif
#ifdef _UNICODE
#define NO_DEVICE L"no device"
#define PATH L"\\\\.\\"
#define PORTS L"Ports"
#define PORTNAME L"PortName"
#endif

const Tstring & SerialInfo::port() const{
	return port_name;
}

const Tstring & SerialInfo::device_name() const{
	return device;
}

SerialInfo::SerialInfo() {
	port_name = NO_DEVICE;
	device = NO_DEVICE;
}

SerialInfo::SerialInfo(const SerialInfo& info):
	port_name(info.port_name),
	device(info.device){
}

SerialInfo::SerialInfo(const Tstring& _port):
	port_name(_port){
	auto list = getSerialList();
	for (auto dev : list) {
		if (dev.port() == port_name) {
			device = dev.device;
			return;
		}
	}
	port_name = NO_DEVICE;
	device = NO_DEVICE;
}

SerialInfo::SerialInfo(const Tstring & _port, const Tstring & _device_name):
	port_name(_port),
	device(_device_name) {
}


const Serial::Config defconf = {
	9600,
	8,
	Serial::Config::Parity::NO,
	Serial::Config::StopBits::ONE,
};

void Serial::setBuffSize(unsigned long read, unsigned long write){
	SetupComm(handle, read, write);
}

Serial::Serial() {
	conf = defconf;
	opened = false;
	handle = nullptr;
}

Serial::~Serial(){
	close();
}

bool Serial::open(const Tstring& port, unsigned int baudRate){
	if (opened)
		return true;

	info = SerialInfo(port);
	return open(info, baudRate);
}

bool Serial::open(const SerialInfo & port_info, unsigned int baudRate){
	info = port_info;
	if (info.port() == NO_DEVICE)
		return false;

	//オープン
	Tstring path = PATH + info.port();
	handle = CreateFile(
		path.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		opened = false;
		return false;
	}
	conf.baudRate = baudRate;
	//設定を反映
	setConfig(conf);
	//バッファサイズの決定
	setBuffSize(1024, 1024);

	opened = true;
	return true;
}

void Serial::close(){
	if (opened) {
		CloseHandle(handle);
	}
	opened = false;
}

const Serial::Config & Serial::getConfig() const{
	return conf;
}

const SerialInfo& Serial::getInfo() const{
	return info;
}

bool Serial::isOpened() const{
	return opened;
}

void Serial::setConfig(const Config& cfg){
	//設定を取得
	DCB dcb;
	GetCommState(handle, &dcb);
	//設定を変更
	dcb.BaudRate = cfg.baudRate;
	dcb.ByteSize = (BYTE)cfg.byteSize;
	switch (cfg.parity) {
	case Config::Parity::NO:
		dcb.Parity = NOPARITY;
		break;
	case Config::Parity::EVEN:
		dcb.Parity = EVENPARITY;
		break;
	case Config::Parity::ODD:
		dcb.Parity = ODDPARITY;
		break;
	}
	dcb.fParity = (conf.parity != Config::Parity::NO);
	switch (cfg.stopBits) {
	case Config::StopBits::ONE:
		dcb.StopBits = ONESTOPBIT;
		break;
	case Config::StopBits::ONE5:
		dcb.StopBits = ONE5STOPBITS;
		break;
	case Config::StopBits::TWO:
		dcb.StopBits = TWOSTOPBITS;
		break;
	}
	//セット
	SetCommState(handle, &dcb);
}

int available(void* handle) {
	unsigned long error;
	COMSTAT stat;
	ClearCommError(handle, &error, &stat);
	return stat.cbInQue;
}

int Serial::read(unsigned char* data, int size) {
	unsigned long readSize;
	bool rtn = true;
	int read_size = available(handle);
	if (read_size == 0) {
		return 0;
	}
	else if (size < read_size) {
		read_size = size;
		rtn = false;
	}
	ReadFile(handle, data, read_size, &readSize, NULL);
	return readSize;
}

unsigned char Serial::read1byte(){
	unsigned char data;
	read(&data, 1);
	return data;
}

std::vector<unsigned char> Serial::read(){
	unsigned long readSize;
	int read_size = available(handle);
	if (read_size == 0) {
		read_size = 1;
	}
	unsigned char* data = new unsigned char[read_size];
	ReadFile(handle, data, read_size, &readSize, NULL);
	std::vector<unsigned char> rtn;
	for (int i = 0; i < read_size; i++) {
		rtn.push_back(data[i]);
	}
	delete[] data;
	return rtn;
}

void Serial::clear(){
	PurgeComm(handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}

void Serial::clearWrite(){
	PurgeComm(handle, PURGE_TXABORT | PURGE_TXCLEAR);
}

void Serial::clearRead(){
	PurgeComm(handle, PURGE_RXABORT | PURGE_RXCLEAR);
}

int Serial::write(unsigned char* data, int size){
	unsigned long writtenSize;
	WriteFile(handle, data, size, &writtenSize, NULL);
	return writtenSize;
}

int Serial::write(const std::vector<unsigned char>& data){
	unsigned long writtenSize;
	int size = data.size();
	char* buff = new char[size];
	for (int i = 0; i < size; i++) {
		buff[i] = data[i];
	}

	WriteFile(handle, buff, size, &writtenSize, NULL);
	return writtenSize;
}

std::vector<SerialInfo> getSerialList() {
	std::vector<SerialInfo> list;
	HDEVINFO hinfo = NULL;
	SP_DEVINFO_DATA info_data = { 0 };
	info_data.cbSize = sizeof(SP_DEVINFO_DATA);

	GUID guid;
	unsigned long guid_size = 0;
	if (SetupDiClassGuidsFromName(PORTS, &guid, 1, &guid_size) == FALSE)
		//SetupDiDestroyDeviceInfoList(hinfo);
		return list;

	hinfo = SetupDiGetClassDevs(&guid, 0, 0, DIGCF_PRESENT | DIGCF_PROFILE);
	if (hinfo == INVALID_HANDLE_VALUE)
		return list;


	Tchar buff[MAX_PATH];
	Tstring name;
	Tstring fullname;
	unsigned int index = 0;
	while (SetupDiEnumDeviceInfo(hinfo, index, &info_data)) {
		unsigned long type;
		unsigned long size;


		//フレンドリーネーム(fullname)
		if (SetupDiGetDeviceRegistryProperty(
			hinfo, &info_data, SPDRP_DEVICEDESC, &type, (BYTE*)buff, MAX_PATH, &size)
			== TRUE) {
			fullname = buff;
		}

		//とりあえず開く
		HKEY hkey = SetupDiOpenDevRegKey(hinfo, &info_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
		if (hkey) {
			//ポート名取得
			RegQueryValueEx(hkey, PORTNAME, 0, &type, (LPBYTE)buff, &size);
			//クローズ
			RegCloseKey(hkey);
			name = buff;
		}
		list.push_back(SerialInfo(name, fullname));
		index++;
	}
	SetupDiDestroyDeviceInfoList(hinfo);
	return list;

}

#endif