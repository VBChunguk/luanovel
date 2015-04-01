#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

void write_varint(size_t value, fstream& out)
{
	vector<char> ret;
	int c;
	for (c = 0; value > 0; c++) {
		ret.push_back(value & 0x7f);
		value >>= 7;
	}
	for (int i = c - 1; i >= 0; i--) {
		unsigned char byte = ret[i];
		if (i) byte |= 0x80;
		out.write((char*)&byte, 1);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2) return 0;

	fstream inputfile(argv[1]);

	int langcnt;
	inputfile >> langcnt;
	map<string, fstream> langfiles;
	map<string, unsigned int*> langheader;
	for (int i = 0; i < langcnt; i++) {
		string langid;
		inputfile >> langid;
		langfiles[langid] = fstream(langid, fstream::out | fstream::binary);
	}

	int filecnt;
	inputfile >> filecnt;
	for (auto i = langfiles.begin(); i != langfiles.end(); i++) {
		unsigned int* temp;
		langheader[i->first] = temp = new unsigned int[filecnt * sizeof(unsigned int)];
		memset(temp, 0xff, filecnt * sizeof(unsigned int));
		i->second.write((char*)temp, filecnt * sizeof(unsigned int));
	}

	fstream index("index.txt", fstream::out);
	char* buf = new char[2048];
	for (int i = 0; i < filecnt; i++) {
		string entry;
		inputfile >> entry;
		cout << entry << " mapped at " << hex << i * 4 << endl;
		index << entry << " " << i * 4 << endl;
		for (int j = 0; j < langcnt; j++) {
			string lang;
			string path;
			inputfile >> lang >> path;
			if (path == "x") continue;

			fstream& langfile = langfiles[lang];
			unsigned int pos = (unsigned int)langfile.tellg();
			langheader[lang][i] = pos;

			fstream target(path, fstream::in | fstream::binary);
			target.seekg(0, fstream::end);
			size_t len = (size_t)target.tellg();
			target.seekg(0);
			write_varint(len, langfile);

			while (!target.eof()) {
				target.read(buf, 2048);				

				int n_read = (int)target.gcount();
				langfile.write(buf, n_read);
			}
			langfile.flush();

			cout << lang << "\\" << path << " stored at " << hex << pos << endl;
		}
	}

	for (auto i = langfiles.begin(); i != langfiles.end(); i++) {
		fstream& langfile = i->second;
		langfile.seekg(0);
		langfile.write((char*)langheader[i->first], filecnt * sizeof(unsigned int));
		langfile.close();
		delete[] langheader[i->first];
	}
	delete[] buf;
	return 0;
}
