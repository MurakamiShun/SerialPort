#pragma once
#include <string>
#include <vector>
#include <initializer_list>

struct SerialInfo {
	//ポート番号
	unsigned int port;
	//ポート名
	std::string name;
	//フルネーム
	std::string dev_name;
	SerialInfo();
	SerialInfo(const unsigned int, const std::string name, const std::string dev_name);
	SerialInfo(const unsigned int);
};

std::vector<SerialInfo> serialList();

class Serial {
public:
	//設定
	struct Config {
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
	//ポート情報
	SerialInfo info;

	//オープンしてるか
	bool opened;

	//設定
	Config conf;

	void* handle;
	void setBuffSize(size_t read, size_t write);

public:
	Serial();
	Serial(const Serial&) = delete;
	~Serial();

	//<sammary>
	//デバイスをオープン
	//</sammary>
	bool open(unsigned int port, unsigned int baudRate = 9600);
	//<sammary>
	//デバイスをクローズ
	//</sammary>
	void close();

	//<sammary>
	//ポート情報の取得
	//</sammary>
	const Config& getConfig() const;
	//<sammary>
	//ポート情報を設定
	//</sammary>
	void setConfig(const Config&);
	//<sammary>
	//デバイス情報の取得
	//</sammary>
	const SerialInfo& getInfo() const;
	//<sammary>
	//デバイスがオープンしているか
	//</sammary>
	bool isOpened() const;

	//<sammary>
	//受信バッファのバイト数
	//</sammary>
	size_t available() const;
	//<sammary>
	//受信(非推奨)
	//</sammary>
	bool read(unsigned char* data, size_t size);
	//<sammary>
	//1バイト受信
	//</sammary>
	unsigned char read1byte();
	//<sammary>
	//バッファすべて受信
	//</sammary>
	std::vector<unsigned char> read();


	//<sammary>
	//バッファをクリア
	//</sammary>
	void clear();
	//<sammary>
	//出力バッファをクリア
	//</sammary>
	void clearWrite();
	//<sammary>
	//入力バッファをクリア
	//</sammary>
	void clearRead();

	//<sammary>
	//送信
	//</sammary>
	void write(unsigned char* data, size_t size);
	//<sammary>
	//送信
	//</sammary>
	void write(const std::vector<unsigned char>& data);
};