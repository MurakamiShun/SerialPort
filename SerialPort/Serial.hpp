#pragma once
#include <string>
#include <vector>
#include <initializer_list>

class SerialPort {
public:
	//設定
	struct State {
		unsigned int baudRate;
		unsigned int byteSize;
		enum class Parity {
			NO,//パリティなし
			EVEN,//偶数パリティ
			ODD//奇数パリティ
		} parity;
		enum class StopBits {
			//1ビット
			ONE,
			//1.5ビット
			ONE5,
			//2ビット
			TWO
		} stopBits;
		bool useParity;
	};
private:
	//ポート番号
	const unsigned int num;
	//ポート名
	const std::string name;
	//フルネーム
	const std::string fullname;
	bool isOpened;

	State state;

	void* handle;

	SerialPort() = delete;
	SerialPort(const unsigned int num, const std::string name, const std::string fullname);
public:
	SerialPort(const SerialPort&);
	~SerialPort();
	//デフォルト設定
	bool open();
	//設定
	bool open(const State&);
	void close();
	State& getState();
	void setState();

	std::vector<char> receive();
	void send(std::initializer_list<char>);

	const unsigned int getNumber() const;
	const std::string& getPortName() const;
	const std::string& getFullName() const;
	static std::vector<SerialPort> getList();
};