#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <map>
#include <vector>
#include <memory>

class Metadata {
public:
	Metadata() {}
	virtual ~Metadata() {}
	virtual void print(std::string tabs="") const {}
};

class MetaStr : public Metadata {
public:
	MetaStr(std::string&& s) { str = std::move(s); }
	bool operator<(const MetaStr& other) const { return str < other.str; }
	std::string str;
	void print(std::string tabs="") const { std::cout << tabs << str; }
};

class MetaDict : public Metadata {
public:
	std::map<MetaStr, std::shared_ptr<Metadata>> dict;
	void print(std::string tabs="") const
	{
		std::cout << tabs << "{\n";
		std::string newtabs = tabs + '\t';
		for (auto itr = dict.cbegin(); itr != dict.cend(); ++itr) {
			itr->first.print(newtabs);
			std::cout << ",\n";
			itr->second->print(newtabs);
			std::cout << "\n\n";
		}
		std::cout << tabs << '}';
	}
};

class MetaList : public Metadata {
public:
	std::vector<std::shared_ptr<Metadata>> list;
	void print(std::string tabs="") const
	{
		std::cout << tabs << "[\n";
		std::string newtabs = tabs + '\t';
		for (const auto& x : list) {
			x->print(newtabs);
			std::cout << ",\n";
		}
		std::cout << tabs << ']';
	}
};

class MetaInt : public Metadata {
public:
	MetaInt(long long i) { integer = i; }
	long long integer;
	void print(std::string tabs="") const { std::cout << tabs << integer; }
};

class Parser {
public:
	Parser(const std::string&);
	std::shared_ptr<Metadata> parse() { return elem(); }
	int count();

	struct Exception {
		std::string msg;
		Exception(std::string&& m) { msg = std::move(m); }
	};

private:
	std::ifstream infile;
	int charcount = 1;

	int getchar();
	std::string read(long long);
	long long getint();
	void expect(char);
	std::shared_ptr<Metadata> elem();
	MetaList list();
	void list_elems(MetaList*);
	MetaDict dict();
	void dict_elems(MetaDict*);
	MetaStr string();
	MetaInt integer();
};

Parser::Parser(const std::string& fname)
{
	infile.open(fname);
}

int Parser::count()
{
	return charcount;
}

int Parser::getchar()
{
	++charcount;
	return infile.get();
}

void Parser::expect(char c)
{
	if (this->getchar() != c)
		throw Exception("expected '" + std::string(1, c) + "'");
}

std::shared_ptr<Metadata> Parser::elem()
{
	int c = infile.peek();

	if (c == 'd') 
		return std::make_unique<MetaDict>(dict());
	else if (c == 'l')
		return std::make_unique<MetaList>(list());
	else if (c == 'i')
		return std::make_unique<MetaInt>(integer());
	else if (isdigit(c))
		return std::make_unique<MetaStr>(string());
	else
		throw Exception("expected element");
}

MetaInt Parser::integer()
{
	expect('i');
	long long i = getint();
	expect('e');
	return MetaInt(i);
}

void Parser::list_elems(MetaList *ml)
{
	ml->list.push_back(elem());
	if (infile.peek() == 'e')
		return;
	list_elems(ml);
}

MetaList Parser::list()
{
	MetaList ml;
	expect('l');
	list_elems(&ml);
	expect('e');
	return ml;
}

void Parser::dict_elems(MetaDict *md)
{
	MetaStr s = string();
	md->dict[std::move(s)] = elem();
	if (infile.peek() == 'e')
		return;
	dict_elems(md);
}

MetaDict Parser::dict()
{
	MetaDict md;
	expect('d');
	dict_elems(&md);
	expect('e');
	return md;
}

long long Parser::getint()
{
	long long i;
	if (!(infile >> i))
		throw Exception("expected number");
	charcount += infile.gcount();
	return i;
}

std::string Parser::read(long long len)
{
	char buf[len+1];
	buf[len] = '\0';
	if (!infile.read(buf, len))
		throw Exception("expected string");
	charcount += infile.gcount();
	return std::string(buf);
}

MetaStr Parser::string()
{
	long long numchars = getint();
	expect(':');
	return MetaStr(read(numchars));
}

/*
int main(int argc, char **argv)
{
	Parser p("test.torrent");
	try {
		std::shared_ptr<Metadata> m = p.parse();
		m->print();
	} catch (Parser::Exception e) {
		std::cout << "Error at " << p.count() << ": " << e.msg << "\n";
		return 1;
	}
	return 0;
}
*/
