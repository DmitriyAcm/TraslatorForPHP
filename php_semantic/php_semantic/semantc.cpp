#if 1
#include <iostream>
#include <string>
#include <map>
#include <string.h>
#include <vector>
#include <set>

using namespace std;

#define pb push_back
#define mp make_pair

const string OBJECT = "rtl/BaseType";
const string FUNCT = "Function Definition";
const string CLAS = "Class Declaration";

const int MAXINT = 65535;

////////
//Parse
struct Node {
	string label;
	vector<Node*> child;
	Node(string label) : label(label) {}
};

class ParseTree {
	map<string, Node*> nodes;
	map<string, string> book;

private:
	bool connect(const string& namefile) {
		return freopen(namefile.c_str(), "r", stdin);
	}

public:
	ParseTree(const string& namefile) {
		if(!connect(namefile)) {
			throw new exception("No such file!");
		}
	}

	Node* parse() {
		Node* root = NULL;
		string s;
		getline(cin, s);
		while(getline(cin, s)) {
			if(s[0] == '}') {
				break;
			}

			int id = s.find("->n");
			if(id == -1) {

				string buf1, buf2;
				for(int i = 0; i < s.size(); ++i) {
					if(s[i] != ' ') {
						buf1.pb(s[i]);
					} else {
						break;
					}
				}

				int i = s.find("\"") + 1;
				while(s[i] != '\"') {
					buf2.pb(s[i++]);
				}

				book[buf1] = buf2;

				if(root == NULL) {
					nodes[buf1] = new Node(buf1);
					root = nodes[buf1];
				}

			} else {

				int i = 0;
				string buf1, buf2;
				while(s[i] != '-') {
					buf1.pb(s[i++]);
				}

				i += 2;

				while(s[i] != ' ') {
					buf2.pb(s[i++]);
				}

				if(nodes[buf1] == NULL) {
					nodes[buf1] = new Node(buf1);
				}
				if(nodes[buf2] == NULL) {
					nodes[buf2] = new Node(buf2);
				}

				nodes[buf1]->child.pb(nodes[buf2]);
			}
		}

		for(auto it = nodes.begin(); it != nodes.end(); ++it) {
			it->second->label = book[it->second->label];
		}

		return root;
	}
};

////////
//Main struct

enum AccessLevel {
	PUBLIC,
	PROTECTED,
	PRIVATE
};

enum ConstantType {
	UTF8, 
	CLASS,
	NAME_AND_TYPE,
	METHOD_REF,
	FIELD_REF,
	STRING,
	INT,
	FLOAT
};

class PHPClass;

struct PHPConstant {
	ConstantType type;
	const void* value;
	PHPConstant(const ConstantType type, const void* value) : type(type), value(value) {}
	friend bool operator<(const PHPConstant this1, const PHPConstant other) {
		if(this1.type < other.type) {
			return true;
		} else if(this1.type == other.type) {
			switch (this1.type) {

			case ConstantType::STRING:
			case ConstantType::UTF8:
				return *((string*)((this1).value)) < *((string*)((other).value));

			case ConstantType::INT:
			case ConstantType::CLASS:
				return *((int*)((this1).value)) < *((int*)((other).value));

			case ConstantType::FLOAT:
				return *((float*)((this1).value)) < *((float*)((other).value));

			default:
				int left1 = ((int*)((this1).value))[0], left2 = ((int*)((this1).value))[1];
				int right1 = ((int*)((other).value))[0], right2 = ((int*)((other).value))[1];
				if(left1 < right1 || left1 == right1 && left2 < right2) {
					return true;
				}
			}
		}

		return false;
	}

public:
	static PHPConstant* getConstant(const string& name, PHPClass* cls);
};

// echo $a, $b
// ->
// IO.echo(a); IO.echo(b);

struct PHPProperty {
	AccessLevel accessLevel;
	bool isStatic;
	bool isConst;
	int fieldrefConstantNumber;
};

struct PHPMethod {
	AccessLevel accessLevel;
	bool isStatic;
	int methodrefConstantNumber;
	map<string, pair<int, int>> localVariables;
	vector<string> sequenceLocalVariables;
	Node* body;
};

class PHPClass {
public:
	string name;
	int parent;
	int classConstantNumber;
	vector<PHPConstant*> constantTable;
	map<string, PHPProperty*> properties;
	map<string, PHPMethod*> methods;

	int pushConst(PHPConstant* cst) {

		if(inTableConst[*cst] == 0) {
			constantTable.pb(cst);
			inTableConst[*cst] = constantTable.size() - 1;
		}

		return inTableConst[*cst];
	}
	//"java/lang/Object"
	int putRef(const string& nameRef, const string& nameClass, ConstantType typeRef, const string nameObject, ConstantType typeObject) {
		int id1 = pushConst(new PHPConstant(ConstantType::UTF8, new string(nameRef)));

		int id2 = pushConst(new PHPConstant(typeObject, new string(nameObject)));

		int *pos = new int(2);
		pos[0] = id1;
		pos[1] = id2;
		int id3 = pushConst(new PHPConstant(ConstantType::NAME_AND_TYPE, pos));

		pos = new int(2);
		pos[0] = pushConst(new PHPConstant(ConstantType::CLASS, new int(pushConst(new PHPConstant(ConstantType::UTF8, new string(nameClass) ) ) ) ) );
		pos[1] = id3;
		int id4 = pushConst(new PHPConstant(typeRef, pos));

		return id4;
	}

private:
	map<PHPConstant, int> inTableConst;
};

PHPConstant* PHPConstant::getConstant(const string& name, PHPClass* cls) {
	ConstantType type;

	if (name[0] == '\'') {		
		cls->putRef("<init>", "rtl/StringType", ConstantType::METHOD_REF, "(Ljava/lang/String;)V", ConstantType::UTF8);
		return new PHPConstant(ConstantType::STRING, new string(name));

	} else if(name.find('.') != string::npos) {
		cls->putRef("<init>", "rtl/FloatType", ConstantType::METHOD_REF, "(D)V", ConstantType::UTF8);
		return new PHPConstant(ConstantType::FLOAT, new float(atof(name.c_str())));

	} else if(name[0] >= '0' && name[0] <= '9') {
		cls->putRef("<init>", "rtl/IntType", ConstantType::METHOD_REF, "(I)V", ConstantType::UTF8);
		return new PHPConstant(ConstantType::INT, new int(atoi(name.c_str())));

	} else {
		return new PHPConstant(ConstantType::UTF8, new string(name));
	}
}

////////
//Fill table

map<string, PHPClass*> phpClasses;
////////

class FinderOper {
	PHPClass* curClass;

	void tryNode(Node* node) {
		if(node->label == CLAS) {
			return;
		}

		if(node->label == "+") {
			curClass->putRef("add", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "-") {
			curClass->putRef("sub", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "*") {
			curClass->putRef("mul", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "/") {
			curClass->putRef("div", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "<") {
			curClass->putRef("lesser", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == ">") {
			curClass->putRef("greater", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == "<=") {
			curClass->putRef("lesserEqual", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == ">=") {
			curClass->putRef("greaterEqual", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == "==") {
			curClass->putRef("equal", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == "!=") {
			curClass->putRef("notEqual", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == ".") {
			curClass->putRef("concat", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "=") {
			//curClass->putRef("concat", "rtl/BaseType;", ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		}

		for(auto it = node->child.begin(); it != node->child.end(); ++it) {
			tryNode(*it);
		}
	}

public:
	FinderOper(PHPClass* cls, Node* node) {
		curClass = cls;
		tryNode(node);
	}
};

class FinderParam {
private:
	vector<string> list;
	set<string> pre;
	// здесь должна быть принадлежность статическому полю
	void DefinitionVar(const string& str) {

	}

	void tryNode(Node* node) {
		if(node->label == FUNCT || node->label == CLAS) {
			return;
		}

		if(node->label == "$" && node->child[0]->label == "ID") {
			if(!pre.count(node->child[0]->child[0]->label)) {
				list.push_back(node->child[0]->child[0]->label);
				pre.insert(node->child[0]->child[0]->label);
			}

			return;
		}

		for(auto it = node->child.begin(); it != node->child.end(); ++it) {
			tryNode(*it);
		}
	}

public:

	static void putField(PHPMethod* mtd, PHPClass* cls, const vector<string>& list) {
		for(auto it = list.begin(); it != list.end(); ++it) {
			int id1 = cls->pushConst(new PHPConstant(ConstantType::UTF8, new string(*it)));
			int id2 = cls->pushConst(new PHPConstant(ConstantType::UTF8, &OBJECT));

			//PHPProperty
			if(mtd->localVariables[*it] == mp(0, 0)) {
				mtd->localVariables[*it] = mp(id1, id2);
				mtd->sequenceLocalVariables.pb(*it);
			}

		}
	}

	FinderParam(Node* node, PHPClass* cls, PHPMethod* mtd) {
		tryNode(node);
		putField(mtd, cls, list);
	}

	string* GetListParam(int num) {
		if(num < list.size()) {
			return &list[num];
		}

		return NULL;
	}
};

// "Function definition"
void ParseAccStat(Node* node, AccessLevel& acc, bool& isStat) {
	if(node->label.size() > 5 && node->label.substr(0, 6) == "public") {
		acc = AccessLevel::PUBLIC;

		if(node->label.size() > 6) {
			isStat = true;
		}
	} else if(node->label.size() > 6 && node->label.substr(0, 7) == "protect") {
		acc = AccessLevel::PROTECTED;

		if(node->label.size() > 7) {
			isStat = true;
		}
	} else if(node->label.size() > 6 && node->label.substr(0, 7) == "private") {
		acc = AccessLevel::PRIVATE;

		if(node->label.size() > 7) {
			isStat = true;
		}
	}
}

class FinderFunc {
private:
	AccessLevel acc;
	bool isStat;

	vector<string> list;
	set<string> pre;

	PHPClass* curClass;
	bool haveConstruct;

	void fillArguments(Node* node, vector<string>& args) {
		for(auto it = node->child.begin(); it != node->child.end(); ++it) {
			args.pb((*it)->child[0]->label);
		}
	}

	void tryNode(Node* node) {
		if(node->label == CLAS) {
			return;
		}

		if(node->label == FUNCT) {
			PHPMethod* mtd = new PHPMethod();
			FinderOper(curClass, node);

			mtd->body = node->child[1];
			mtd->isStatic = isStat;
			mtd->accessLevel = acc;

			if(node->child[0]->child[0]->label == "__construct") {
				node->child[0]->child[0]->label = "<init>";
				haveConstruct = true;
			}

			string lab = node->child[0]->child[0]->label;
			curClass->methods[node->child[0]->child[0]->label] = mtd;
			if (!mtd->isStatic) {
				mtd->localVariables["this"] = mp(curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string("this"))), curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(curClass->name))));
				mtd->sequenceLocalVariables.pb("this");
			}

			vector<string> args;
			fillArguments(node->child[0]->child[1], args);

			mtd->methodrefConstantNumber = curClass->putRef(lab, curClass->name, ConstantType::METHOD_REF, getConstName(args.size()), ConstantType::UTF8);

			//Def fields
			//arg
			FinderParam::putField(mtd, curClass, args);

			//param
			FinderParam fParam(mtd->body, curClass, mtd);

			return;
		}

		AccessLevel lastAcc = acc;
		bool lastStat = isStat;

		ParseAccStat(node, acc, isStat);

		for(auto it = node->child.begin(); it != node->child.end(); ++it) {
			tryNode(*it);
		}

		acc = lastAcc;
		isStat = lastStat;
	}

public:
	static string getConstName(int col) {
		string res;

		res.pb('(');

		while(col-- > 0) {
			res += "Lrtl/BaseType;";
		}
		res.pb(')');

		res += "Ljava/lang/Object;";

		return res;
	}

	FinderFunc(Node* node, const string& nameClass) {
		acc = AccessLevel::PUBLIC;
		isStat = false;
		haveConstruct = true;
		curClass = phpClasses[nameClass];
		tryNode(node);
	}

	bool haveConstructor() {
		return haveConstruct;
	}
};

void FillListConstantTable(Node* node, PHPClass* cls) {
	if(node->label == CLAS) {
		return;
	}

	if(node->child.empty()) {
		if(node->label == "Expression List" || node->label == "Statement List" 
			|| node->label == "Variable List" || node->label == "Class Member List") {
				return;
		}
		
		PHPConstant* buf = PHPConstant::getConstant(node->label, cls);
		if(buf->type != ConstantType::INT || *((int*)(buf->value)) > MAXINT) {
			cls->pushConst(buf);
		}
	}

	for(auto it = node->child.begin(); it != node->child.end(); ++it) {
		FillListConstantTable(*it, cls);
	}
}

void GenDefaultConstructor(PHPClass* cls) {
	PHPMethod* meth = new PHPMethod();
	string name = "<init>";
	meth->methodrefConstantNumber = cls->putRef(name, "java/lang/Object", ConstantType::METHOD_REF, "()V", ConstantType::UTF8);
	meth->accessLevel = AccessLevel::PUBLIC;
	meth->isStatic = false;
	meth->body = NULL;
	cls->methods[name] = meth;
	meth->localVariables["this"] = mp(cls->pushConst(new PHPConstant(ConstantType::UTF8, new string("this"))), cls->pushConst(new PHPConstant(ConstantType::UTF8, new string(cls->name))));
	meth->sequenceLocalVariables.pb("this");
}

void FillTables(Node* node) {
	for(auto it = node->child.begin(); it != node->child.end(); ++it) {
		Node* curNode = (*it);
		if((*it)->label == CLAS) {

			string name = curNode->child[0]->label;

			PHPClass* curClass = new PHPClass();

			phpClasses[name] = curClass;

			curClass->name = name;
			curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string("")));
			curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string("Code")));
			curClass->classConstantNumber = curClass->pushConst(new PHPConstant(ConstantType::CLASS, new int(curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(name))))));

			FillListConstantTable(*it, curClass);

			curClass->parent = curClass->pushConst(new PHPConstant(ConstantType::CLASS, new int(curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string("java/lang/Object"))))));

			if(curNode->child.size() == 1) {
				return;
			}

			Node* nodeMem = curNode->child[1];
			// Extend
			if(curNode->child[1]->label != "Class Member List") {
				curClass->parent = curClass->pushConst(new PHPConstant(ConstantType::CLASS, new int(curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(curNode->child[1]->label))))));
				if(curNode->child.size() > 2) {
					nodeMem = curNode->child[2];
				} else {
					nodeMem = NULL;
				}
			}

			// members
			if(nodeMem == NULL) {
				break;
			}

			FillListConstantTable(nodeMem, curClass);

			FinderFunc(nodeMem, name);

			if(curClass->methods["<init>"] == NULL) {
				GenDefaultConstructor(curClass);
			}

			for(auto it1 = nodeMem->child.begin(); it1 != nodeMem->child.end(); ++it1) {
				Node* csNd = *it1;
				if(csNd->label == "Class Property" || csNd->label == "Class Const") {
					bool isConst = csNd->label == "Class Const";
					csNd = csNd->child[0];
					AccessLevel acc = AccessLevel::PUBLIC;
					bool isStat = false;

					ParseAccStat(csNd, acc, isStat);

					csNd = csNd->child[0];

					for(auto it2 = csNd->child.begin(); it2 != csNd->child.end(); ++it2) {
						Node* var = *it2;

						PHPProperty* prop = new PHPProperty();

						prop->accessLevel = acc;
						prop->isStatic = isStat;
						prop->isConst = isConst;

						prop->fieldrefConstantNumber = curClass->putRef(var->child[0]->label, curClass->name, ConstantType::FIELD_REF, OBJECT, ConstantType::UTF8);

						curClass->properties[var->child[0]->label] = prop;
					}
				}
			}
		}
	}
}

void prints();

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////	Code Generation	  //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


union u2
{
	unsigned short number;
	char bytes[2]; 
};

union u4
{
	unsigned long int number;
	char bytes[4]; 
};

union s2
{
	short number;
	char bytes[2]; 
};

union s4
{
	long int number;
	char bytes[4]; 
};

vector <char> get_u2(unsigned short number) {
	vector <char> result = vector<char>();
	u2 data;
	data.number = number;
	result.push_back(data.bytes[1]);
	result.push_back(data.bytes[0]);
	return result;
}

vector <char> get_u4(unsigned long int number) {
	vector <char> result = vector<char>();
	u4 data;
	data.number = number;
	result.push_back(data.bytes[3]);
	result.push_back(data.bytes[2]);
	result.push_back(data.bytes[1]);
	result.push_back(data.bytes[0]);
	return result;
}

vector <char> get_s2(short number) {
	vector <char> result = vector<char>();
	s2 data;
	data.number = number;
	result.push_back(data.bytes[1]);
	result.push_back(data.bytes[0]);
	return result;
}

vector <char> get_s4(long int number) {
	vector <char> result = vector<char>();
	s4 data;
	data.number = number;
	result.push_back(data.bytes[3]);
	result.push_back(data.bytes[2]);
	result.push_back(data.bytes[1]);
	result.push_back(data.bytes[0]);
	return result;
}

void printString(string& str) {
	const char* s = str.c_str();
	int length = str.length();
	for (int i = 0; i < length; ++i) {
		cout << s[i];
	}
}

void printMagic() {
	printf("%c", 0xCA);
	printf("%c", 0xFE);
	printf("%c", 0xBA);
	printf("%c", 0xBE);
}

void printBytes(vector<char> bytes) {
	for (auto it = bytes.begin(); it != bytes.end(); ++it) {
		printf("%c", (*it));
	}
}

void printFlags() {
	printf("%c", 0x00); 
	printf("%c", 0x21); //PUBLIC //SUPER
}

void printTableConstant(vector<PHPConstant*>& tc) {
	for (auto it = tc.begin(); it != tc.end(); ++it) {
		int** data;
		string str;
		switch ((*it)->type) {
		case ConstantType::UTF8:
				printf("%c", 0x01);
				str = *((string*)(*it)->value);
				printBytes(get_u2(str.length()));
				printString(str);
				break;
			case ConstantType::CLASS:
				printf("%c", 0x07);
				printBytes(get_u2(*((int*)(*it)->value)));
				break;
			case ConstantType::NAME_AND_TYPE:
				printf("%c", 0x0C);
				data = (int**)(*it)->value;
				printBytes(get_u2((int)(*data)));
				printBytes(get_u2((int)(*data+1)));
				break;
			case ConstantType::METHOD_REF:
				printf("%c", 0x0A);
				data = (int**)(*it)->value;
				printBytes(get_u2((int)(*data)));
				printBytes(get_u2((int)(*data+1)));
				break;
			case ConstantType::FIELD_REF:
				printf("%c", 0x09);
				data = (int**)(*it)->value;
				printBytes(get_u2((int)(*data)));
				printBytes(get_u2((int)(*data+1)));
				break;
		}
	}
}

void printAccessFlags(PHPMethod* method) {
	int res;
	method->isStatic ? res = 0x08 : res = 0x00;
	switch (method->accessLevel) {
		case AccessLevel::PRIVATE:
			res |= 0x02;
			break;
		case AccessLevel::PROTECTED:
			res |= 0x04;
			break;
		case AccessLevel::PUBLIC:
			res |= 0x01;
			break;
	}
	printf("%c", 0x00);
	printf("%c", res);
}

void printMethods(PHPClass* phpClass, map<string, PHPMethod*>& methods) {
	for (auto it = methods.begin(); it != methods.end(); ++it) {
		printAccessFlags(it->second);
		int i = it->second->methodrefConstantNumber;
		PHPConstant* mrconst = phpClass->constantTable.at(i);
		int** datamr = (int**)mrconst->value;
		PHPConstant* ntconst = phpClass->constantTable.at((int)*(datamr+1));
		int** datant = (int**)ntconst->value;
		printBytes(get_u2((int)*(datant)));
		printBytes(get_u2((int)*(datant+1)));
		printBytes(get_u2(1));
	}
}

void codeGeneration() {
	
	for (auto it = phpClasses.begin(); it != phpClasses.end(); ++it) {
		freopen((it->first + ".class").c_str(), "w", stdout);
		printMagic();
		printBytes(get_u2(0));
		printBytes(get_u2(52));
		printBytes(get_u2(phpClasses.size() + 1));
		printTableConstant(it->second->constantTable);
		printFlags();
		printBytes(get_u2(it->second->classConstantNumber)); // this class
		printBytes(get_u2(it->second->parent)); // parent class
		printBytes(get_u2(0)); //interfaces
		printBytes(get_u2(it->second->properties.size())); //fields
		printBytes(get_u2(it->second->methods.size())); //methods
		printMethods(it->second, it->second->methods);
		printBytes(get_u2(0)); //attributes
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////	Code Generation	  //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void main() {
	ParseTree tree("finalTree.dot");
	Node* root = tree.parse();

	root = root->child[0];

	string nameClass = "___Base___";
	phpClasses[nameClass] = (new PHPClass());

	auto it = --phpClasses.end();
	it->second->name = nameClass;
	
	it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("")));
	it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("Code")));
	it->second->classConstantNumber = it->second->pushConst(new PHPConstant(ConstantType::CLASS, new int(it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string(it->second->name))))));

	it->second->parent = it->second->pushConst(new PHPConstant(ConstantType::CLASS, new int(it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("java/lang/Object"))))));

	// init
	GenDefaultConstructor(it->second);

	FinderOper(it->second, root);

	// main
	{
		PHPMethod* meth = new PHPMethod();
		string name = "main";
		meth->methodrefConstantNumber = it->second->putRef(name, it->second->name, ConstantType::METHOD_REF, "([Ljava/lang/String;)V", ConstantType::UTF8);
		meth->accessLevel = AccessLevel::PUBLIC;
		meth->isStatic = true;
		meth->body = root;
		it->second->methods[name] = meth;
		meth->localVariables["args"] = mp(it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("args"))), it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("[Ljava/lang/String;"))));
		meth->sequenceLocalVariables.pb("args");
	}

	FillListConstantTable(root, it->second);
	FinderParam s(root, phpClasses["___Base___"], it->second->methods["main"]);
	FinderFunc fs(root, "___Base___");

	FillTables(root);

	prints();

	codeGeneration();
}

string toStr[] = { 
	"UTF8", 
	"CLASS",
	"NAME_AND_TYPE",
	"METHOD_REF",
	"FIELD_REF",
	"STRING",
	"INT",
	"FLOAT"
};

void prints() {
	freopen("res.txt", "w", stdout);
	for(auto it = phpClasses.begin(); it != phpClasses.end(); ++it) {
		cout << it->first << endl;

		int cnt = 0;
		for(auto it1 = (it->second)->constantTable.begin(); it1 != (it->second)->constantTable.end(); ++it1) {
			if(cnt == 0) {
				cnt++;
				continue;
			}

			cout << cnt++ << ' ' << toStr[(*it1)->type] << " --- ";

			switch ((*it1)->type) {

			case ConstantType::STRING:
			case ConstantType::UTF8:
				cout << *((string*)((*it1)->value)) << endl;
				break;

			case ConstantType::INT:
			case ConstantType::CLASS:
				cout << *((int*)((*it1)->value)) << endl;
				break;

			case ConstantType::FLOAT:
				cout << *((float*)((*it1)->value)) << endl;
				break;

			default:
				cout << ((int*)((*it1)->value))[0] << ',' << ((int*)((*it1)->value))[1] << endl;
				break;
			}
		}
	}

	cout << endl;

	for(auto it = phpClasses.begin(); it != phpClasses.end(); ++it) {
		cout << it->first << endl << endl;
		for(auto it1 = (*it).second->methods.begin(); it1 != (*it).second->methods.end(); ++it1) {
			cout << it1->first << endl << endl;
			int cnt = 0;
			for(auto it2 = it1->second->sequenceLocalVariables.begin(); it2 != it1->second->sequenceLocalVariables.end(); ++it2) {
				cout << cnt++ << ' ' << *it2 << ' ' << it1->second->localVariables[*it2].first << ' ' << it1->second->localVariables[*it2].second <<endl;
			}
			cout << endl;
		}
	}
}
#endif