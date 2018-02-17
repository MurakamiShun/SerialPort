#include "Serial.hpp"

#include <Windows.h>
#include <SetupAPI.h>
#pragma comment(lib, "setupapi.lib")

SerialInfo::SerialInfo() {
	port = 0;
	name = "no device";
	dev_name = "no device";
}

SerialInfo::SerialInfo(const unsigned int _port, const std::string _name, const std::string dev_name):
	port(_port),
	name(_name),
	dev_name(dev_name)
{
}

SerialInfo::SerialInfo(const unsigned int n){
	port = n;
	auto list = serialList();
	for (auto dev:list) {
		if (dev.port == n) {
			name = dev.name;
			dev_name = dev.dev_name;
			return;
		}
	}
	name = "no device";
	dev_name = "no device";
}


const Serial::Config defconf = {
	9600,
	8,
	Serial::Config::Parity::NO,
	Serial::Config::StopBits::ONE,
	true
};

void Serial::setBuffSize(size_t read, size_t write){
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

bool Serial::open(unsigned int port, unsigned int baudRate){
	if (opened) {
		return true;
	}

	info = SerialInfo(port);
	if (info.name == "no device") {
		return false;
	}
	//オープン
	std::string path = "\\\\.\\" + info.name;
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
	dcb.ByteSize = cfg.byteSize;
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
	dcb.fParity = cfg.useParity;
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

size_t Serial::available() const{
	unsigned long error;
	COMSTAT stat;
	ClearCommError(handle, &error, &stat);
	return stat.cbInQue;
}

bool Serial::read(unsigned char* data, size_t size) {
	unsigned long readSize;
	bool rtn = true;
	size_t read_size = available();
	if (read_size == 0) {
		return false;
	}
	else if (size < read_size) {
		read_size = size;
		rtn = false;
	}
	ReadFile(handle, data, read_size, &readSize, NULL);
	return rtn;
}

unsigned char Serial::read1byte(){
	unsigned char data;
	while (available() == 0);
	read(&data, 1);
	return data;
}

std::vector<unsigned char> Serial::read(){
	unsigned long readSize;
	size_t read_size = available();
	unsigned char* data = new unsigned char[read_size];
	ReadFile(handle, data, read_size, &readSize, NULL);
	std::vector<unsigned char> rtn;
	for (size_t i = 0; i < read_size; i++) {
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

void Serial::write(unsigned char* data, size_t size){
	unsigned long writtenSize;
	WriteFile(handle, data, size, &writtenSize, NULL);
}

void Serial::write(const std::vector<unsigned char>& data){
	unsigned long writtenSize;
	size_t size = data.size();
	unsigned char* buff = new unsigned char[size];
	for (size_t i = 0; i < size; i++) {
		buff[i] = data[i];
	}

	WriteFile(handle, buff, size, &writtenSize, NULL);
}

std::vector<SerialInfo> serialList() {
	std::vector<SerialInfo> list;
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


	char buff[MAX_PATH];
	std::string name;
	std::string fullname;
	unsigned int num;
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
			unsigned long temp_type = REG_SZ;
			//ポート名取得
			RegQueryValueEx(hkey, "PortName", 0, &type, (LPBYTE)buff, &size);
			//クローズ
			RegCloseKey(hkey);
			name = buff;
			//COM"number"
			num = atoi(&buff[3]);
		}
		list.push_back(SerialInfo(num, name, fullname));
		index++;
	}
	SetupDiDestroyDeviceInfoList(hinfo);
	return list;

}