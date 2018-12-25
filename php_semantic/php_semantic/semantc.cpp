#if 1
#include <iostream>
#include <string>
#include <map>
#include <string.h>
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>
#include <process.h>

#include "bytecodeInstructions.h"

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

				int j = s.find("\"") + 1;

				while(1) {
					while(j < s.size() && s[j] != '\"') {
						buf2.pb(s[j++]);
					}

					if(j == s.size()) {
						buf2.pb('\n');
						getline(cin, s);
						continue;
					}

					break;
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
	INTERFACE_METHOD_REF,
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


			case ConstantType::UTF8:
				return *((string*)((this1).value)) < *((string*)((other).value));

			case ConstantType::STRING:
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
	map<string, int> operators;

	map<string, int> liters;
	map<string, int> interegs;
	map<string, int> floats;

	int BaseType;
	int pushConst(PHPConstant* cst) {
		string cur = "";
		if(cst->type == ConstantType::UTF8) {
			cur = *((string*)cst->value);
		}
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
		cls->operators["S<init>"] = cls->putRef("<init>", "rtl/StringType", ConstantType::METHOD_REF, "(Ljava/lang/String;)V", ConstantType::UTF8);

		string cur(name);
		cur.pop_back();
		cur.erase(0,1);
		return new PHPConstant(ConstantType::STRING, new int(cls->pushConst(new PHPConstant(ConstantType::UTF8, new string(cur)))));

	} else if(name.find('.') != string::npos) {
		cls->operators["D<init>"] = cls->putRef("<init>", "rtl/FloatType", ConstantType::METHOD_REF, "(D)V", ConstantType::UTF8);
		return new PHPConstant(ConstantType::FLOAT, new float(atof(name.c_str())));

	} else if(name[0] >= '0' && name[0] <= '9') {
		cls->operators["I<init>"] = cls->putRef("<init>", "rtl/IntType", ConstantType::METHOD_REF, "(I)V", ConstantType::UTF8);
		return new PHPConstant(ConstantType::INT, new int(atoi(name.c_str())));

	} else {
		return new PHPConstant(ConstantType::UTF8, new string(name));
	}
}

////////
//Fill table

map<string, PHPClass*> phpClasses;
////////

class FinderFunc;

string getConstName(int col) {
	string res;

	res.pb('(');

	while(col-- > 0) {
		res += "Lrtl/BaseType;";
	}
	res.pb(')');

	res += "Lrtl/BaseType;";

	return res;
}

class FinderOper {
	PHPClass* curClass;

	void tryNode(Node* node) {
		if(node->label == CLAS) {
			return;
		}

		int id = -1;
		string name;

		if(node->label == "+") {
			name = "___add___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "-") {
			name = "___sub___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "*") {
			name = "___mul___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "/") {
			name = "___div___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "<") {
			name = "___lesser___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == ">") {
			name = "___greater___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == "<=") {
			name = "___lesserEqual___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == ">=") {
			name = "___greaterEqual___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == "==") {
			name = "___equal___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == "!=") {
			name = "___notEqual___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Z", ConstantType::UTF8);

		} else if (node->label == ".") {
			name = "___concat___";
			id = curClass->putRef(name, OBJECT, ConstantType::METHOD_REF, "(Ljava/lang/Object;)Lrtl/BaseType;", ConstantType::UTF8);

		} else if (node->label == "Foreach Statement") {
			name = "___iterator___";
			id = curClass->putRef(name, "rtl/Array", ConstantType::METHOD_REF, "()Ljava/util/Iterator;", ConstantType::UTF8);
			curClass->operators[name] = id;

			name = "___hasNext___";
			id = curClass->putRef(name, "java/util/Iterator", ConstantType::INTERFACE_METHOD_REF, "()Z", ConstantType::UTF8);
			curClass->operators[name] = id;

			name = "___next___";
			id = curClass->putRef(name, "java/util/Iterator", ConstantType::INTERFACE_METHOD_REF, "()Ljava/lang/Object;", ConstantType::UTF8);
			curClass->operators[name] = id;
		}
		if(id != -1) {
			curClass->operators[name] = id;
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
	PHPClass* curClass;

	void genArray(Node* exprList) {
		curClass->operators["___putkey___"] = curClass->putRef("___put___", "rtl/Array", ConstantType::METHOD_REF, "(Lrtl/BaseType;Lrtl/BaseType;)V", ConstantType::UTF8);
		curClass->operators["___put___"] = curClass->putRef("___put___", "rtl/Array", ConstantType::METHOD_REF, "(Lrtl/BaseType;)V", ConstantType::UTF8);
		curClass->operators["___getkey___"] = curClass->putRef("___get___", "rtl/Array", ConstantType::METHOD_REF, "(Lrtl/BaseType;)Lrtl/BaseType;", ConstantType::UTF8);
		curClass->operators["___get___"] = curClass->putRef("___get___", "rtl/Array", ConstantType::METHOD_REF, "()Lrtl/BaseType;", ConstantType::UTF8);
		curClass->operators["Array<init>"] = curClass->putRef("<init>", "rtl/Array", ConstantType::METHOD_REF, "([Lrtl/Pair;)V", ConstantType::UTF8);
		curClass->operators["Pair2<init>"] = curClass->putRef("<init>", "rtl/Pair", ConstantType::METHOD_REF, "(Lrtl/BaseType;Lrtl/BaseType;)V", ConstantType::UTF8);
		curClass->operators["Pair1<init>"] = curClass->putRef("<init>", "rtl/Pair", ConstantType::METHOD_REF, "(Lrtl/BaseType;)V", ConstantType::UTF8);
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

		if(node->label == "Echo") {
			curClass->operators["___echo___"] = curClass->putRef("___echo___", "rtl/IO", ConstantType::METHOD_REF, "(Ljava/lang/Object;)V", ConstantType::UTF8);
		}

		if(node->label == "Function Call") {
			string nameFunc = node->child[0]->child[0]->label;

			if(nameFunc == "array" || nameFunc == "Array Element") {
				genArray(node->child[1]);
			} else if(nameFunc == "fgetc"){
				curClass->operators["___getChar___"] = curClass->putRef("___getChar___", "rtl/IO", ConstantType::METHOD_REF, "()Lrtl/BaseType;", ConstantType::UTF8);
			} else if(nameFunc == "fgets") {
				curClass->operators["___getString___"] = curClass->putRef("___getString___", "rtl/IO", ConstantType::METHOD_REF, "()Lrtl/BaseType;", ConstantType::UTF8);
			} else if(nameFunc == "count") {
				curClass->operators["___count___"] = curClass->putRef("___count___", "rtl/BaseType", ConstantType::METHOD_REF, "()Lrtl/BaseType;", ConstantType::UTF8);
			} else {
				// TODO надо исправить ___Base___ на класс, к которому относится данная функция.
				curClass->operators[nameFunc] = curClass->putRef(nameFunc, "___Base___", ConstantType::METHOD_REF, getConstName(node->child[1]->child.size()), ConstantType::UTF8);
			}
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
		curClass = cls;
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

int CreateLocalVariableForForeach(Node* node) {
	if(node->label == CLAS || node->label == FUNCT) {
		return 0;
	}

	int res = 0;

	for(auto it = node->child.begin(); it != node->child.end(); ++it) {
		res = max(res, CreateLocalVariableForForeach(*it));
	}

	return (res + (node->label == "Foreach Statement"));
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

			if(curClass->name == "___Base___") {
				mtd->isStatic = true;
			}

			string lab = node->child[0]->child[0]->label;
			curClass->methods[node->child[0]->child[0]->label] = mtd;
			if (!mtd->isStatic) {
				mtd->localVariables["this"] = mp(curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string("this"))), curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(curClass->name))));
				mtd->sequenceLocalVariables.pb("this");
			}

			int deep = CreateLocalVariableForForeach(node->child[1]);
			for(int i = 1; i <= deep; ++i) {
				string name = "___ITER" + to_string((long long)i) + "___";
				mtd->localVariables[name] = mp(curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(name))), curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(curClass->name))));
				mtd->sequenceLocalVariables.pb(name);
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
			|| node->label == "Variable List" || node->label == "Class Member List"
			|| node->label == "Return") {
				return;
		}

		PHPConstant* buf = PHPConstant::getConstant(node->label, cls);
		if(buf->type != ConstantType::INT || *((int*)(buf->value)) > MAXINT) {
			int id = cls->pushConst(buf);

			switch (buf->type) {
			case ConstantType::INT:
				cls->interegs[node->label] = id;
				break;
			case ConstantType::FLOAT:
				cls->floats[node->label] = id;
				break;
			case ConstantType::STRING:
				cls->liters[node->label] = id;
				break;
			}
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

void FindBaseType(PHPClass* curClass) {
	curClass->BaseType = curClass->pushConst(new PHPConstant(ConstantType::CLASS, new int(curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(OBJECT) ) ) ) ) );
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

			FindBaseType(curClass);
		}
	}
}

void prints();

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////	Code Generation	  //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

//	BYTECODE	//
ByteCode getBytecodeNewArray(PHPClass* phpClass, PHPMethod* method, Node* body);
ByteCode getBytecodeArrayElement(PHPClass* phpClass, PHPMethod* method, Node* body, int depth);
//	BYTECODE	//



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
		char a = *it;
		printf("%c", (*it));
	}
}

void printFlags() {
	printf("%c", 0x00); 
	printf("%c", 0x21); //PUBLIC //SUPER
}

void printTableConstant(vector<PHPConstant*>& tc) {
	for (int i = 1; i < tc.size(); ++i) {
		int** data;
		string str;
		switch (tc[i]->type) {
		case ConstantType::UTF8:
			printf("%c", 0x01);
			str = *((string*)(tc[i]->value));
			printBytes(get_u2((unsigned short)str.length()));
			printString(str);
			break;
		case ConstantType::CLASS:
			printf("%c", 0x07);
			printBytes(get_u2(*((int*)tc[i]->value)));
			break;
		case ConstantType::INT:
			printf("%c", 0x03);
			printBytes(get_s4(*((long int*)tc[i]->value)));
			break;
		case ConstantType::FLOAT:
			printf("%c", 0x04);
			printBytes(get_s4(*((long int*)tc[i]->value)));
			break;
		case ConstantType::STRING:
			printf("%c", 0x08);
			printBytes(get_u2(*((int*)tc[i]->value)));
			break;
		case ConstantType::NAME_AND_TYPE:
			printf("%c", 0x0C);
			data = (int**)tc[i]->value;
			printBytes(get_u2((int)(*data)));
			printBytes(get_u2((int)(*(data+1))));
			break;
		case ConstantType::METHOD_REF:
			printf("%c", 0x0A);
			data = (int**)tc[i]->value;
			printBytes(get_u2((int)(*data)));
			printBytes(get_u2((int)(*(data+1))));
			break;
		case ConstantType::FIELD_REF:
			printf("%c", 0x09);
			data = (int**)tc[i]->value;
			printBytes(get_u2((int)(*data)));
			printBytes(get_u2((int)(*(data+1))));
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

ByteCode getDefaultConstructor() {
	vector<char> bytecode;
	bytecode.push_back(0x2A);
	bytecode.push_back(0xB7);
	vector<char> invoke = get_u2(9);
	bytecode = append(bytecode, invoke);
	bytecode.push_back(0xB1);
	return ByteCode(bytecode);
}

ByteCode getBytecode(PHPClass* phpClass, PHPMethod* method, Node* body) {
	/* */
	ByteCode bytecode;
	if (body == NULL) {
		return getDefaultConstructor();
	}
	if (body->label == "Program List" || body->label == "Statement List") {
		for (auto it = body->child.begin(); it != body->child.end(); ++it) {
			ByteCode op(getBytecode(phpClass, method, (*it)));
			bytecode = append(bytecode, op);
		}
		if (body->label == "Program List") {
			bytecode = append(bytecode, returnVoid());
		}
		return bytecode;
	}
	if (body->label == "While Statement") {
		ByteCode ifblock = getBytecode(phpClass, method, body->child[0]);
		ifblock = append(ifblock, ifeq());

		ByteCode body1 = getBytecode(phpClass, method, body->child[1]);

		body1 = append(body1, _goto());

		int shift = -(int)(body1.size() + ifblock.size() + 1);
		body1 = append(body1, get_s2(shift));

		shift = body1.size() + 3;
		ifblock = append(ifblock, get_s2(shift));

		bytecode = append(bytecode, ifblock);
		bytecode = append(bytecode, body1);

		return bytecode;
	}
	if (body->label == "Do-While Statement") {
		ByteCode ifblock = getBytecode(phpClass, method, body->child[0]);
		ifblock = append(ifblock, ifne());

		ByteCode body1 = getBytecode(phpClass, method, body->child[1]);

		int shift = -(int)(body1.size() + ifblock.size() - 1);
		ifblock = append(ifblock, get_s2(shift));

		bytecode = append(bytecode, body1);
		bytecode = append(bytecode, ifblock);

		return bytecode;
	}
	if (body->label == "For Statement") {
		for(int i = 0; i < body->child[0]->child.size(); ++i) {
			bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]->child[i]));
		}

		ByteCode ifblock = getBytecode(phpClass, method, body->child[1]->child[0]);
		ifblock = append(ifblock, ifeq());

		ByteCode body1 = getBytecode(phpClass, method, body->child[3]);

		for(int i = 0; i < body->child[2]->child.size(); ++i) {
			body1 = append(body1, getBytecode(phpClass, method, body->child[2]->child[i]));
		}

		body1 = append(body1, _goto());

		int shift = -(int)(body1.size() + ifblock.size() + 1);
		body1 = append(body1, get_s2(shift));

		shift = body1.size() + 3;
		ifblock = append(ifblock, get_s2(shift));

		bytecode = append(bytecode, ifblock);
		bytecode = append(bytecode, body1);

		return bytecode;
	}
	if (body->label == "If Statement") {
		vector<ByteCode> blocks;

		{
			ByteCode mainIfCode;
			mainIfCode = append(mainIfCode, getBytecode(phpClass, method, body->child[0]));
			ByteCode ieb = ifeq();
			mainIfCode = append(mainIfCode, ieb);

			ByteCode ifblockcode = getBytecode(phpClass, method, body->child[1]);
			ifblockcode = append(ifblockcode, _goto());

			int shift = ifblockcode.size() + 2 + 3;
			mainIfCode = append(mainIfCode, get_s2(shift));

			mainIfCode = append(mainIfCode, ifblockcode);

			blocks.push_back(mainIfCode);
		}

		int allshift = blocks.back().size() + 2;

		if(body->child.size() > 2 && body->child[2]->label == "Else If Statement List") {
			for(int i = 0; i < body->child[2]->child.size(); ++i) {
				ByteCode mainIfCode;
				mainIfCode = append(mainIfCode, getBytecode(phpClass, method, body->child[2]->child[i]->child[0]));
				ByteCode ieb = ifeq();
				mainIfCode = append(mainIfCode, ieb);

				ByteCode ifblockcode = getBytecode(phpClass, method, body->child[2]->child[i]->child[1]);
				ifblockcode = append(ifblockcode, _goto());

				int shift = ifblockcode.size() + 2 + 3;
				mainIfCode = append(mainIfCode, get_s2(shift));

				mainIfCode = append(mainIfCode, ifblockcode);

				blocks.push_back(mainIfCode);

				allshift += blocks.back().size() + 2;
			}	
		}

		if(body->child.size() > 3 || body->child.size() > 2 && body->child[2]->label == "Statement List") {
			ByteCode mainIfCode;

			const int SIZE = body->child.size() - 1;

			mainIfCode = append(mainIfCode, getBytecode(phpClass, method, body->child[SIZE]->child[0]));
			mainIfCode = append(mainIfCode, _goto());

			blocks.push_back(mainIfCode);

			allshift += blocks.back().size() + 2;
		}

		for(int i = 0; i < blocks.size(); ++i) {
			allshift -= (int)(blocks[i].size()) + 2;
			blocks[i] = append(blocks[i], get_s2(allshift + 3));
			bytecode = append(bytecode, blocks[i]);
		}

		return bytecode;
	}
	if (body->label == "=") {
		if (body->child[0]->label == "Array Element") {
			bytecode = append(bytecode, getBytecodeArrayElement(phpClass, method, body->child[0]->child[0], 1));
			if (body->child[0]->child.size() > 1) {
				bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]->child[1]));
				bytecode = append(bytecode, getBytecode(phpClass, method, body->child[1]));
				bytecode = append(bytecode, invokevirtual(phpClass->operators["___putkey___"]));
			} else {
				bytecode = append(bytecode, getBytecode(phpClass, method, body->child[1]));
				bytecode = append(bytecode, invokevirtual(phpClass->operators["___put___"]));
			}
			return bytecode;
		} else {
			auto it = find(method->sequenceLocalVariables.begin(), method->sequenceLocalVariables.end(), body->child[0]->child[0]->child[0]->label);
			int pos = it - method->sequenceLocalVariables.begin();
			if (body->child[1]->label == "Function Call" && body->child[1]->child[0]->label == "ID" && body->child[1]->child[0]->child[0]->label == "array") {
				/*int mrarrayvalue = phpClass->operators["Array<init>"];
				PHPConstant* mrarray = phpClass->constantTable[mrarrayvalue];
				int classConstArray = (int)(*(int**)mrarray->value);
				bytecode = append(bytecode, _new(classConstArray));
				bytecode = append(bytecode, dup());
				bytecode = append(bytecode, invokespecial(mrarrayvalue));
				bytecode = append(bytecode, astore(pos));
				bytecode = append(bytecode, getBytecodeNewArray(phpClass, method, body->child[1]->child[1], classConstArray, pos, 0));
				return bytecode;*/
				bytecode = append(bytecode, getBytecodeNewArray(phpClass, method, body->child[1]->child[1]));
				bytecode = append(bytecode, astore(pos));
				return bytecode;
			} else {
				bytecode = append(bytecode, getBytecode(phpClass, method, body->child[1]));
				bytecode = append(bytecode, astore(pos));
			}
			//bytecode = append(bytecode, astore(pos));
			//return bytecode;
		}

		return bytecode;
	}
	if (body->label == "+") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___add___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == ".") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___concat___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == "-") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___sub___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == "*") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___mul___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == "/") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___div___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == "<") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___lesser___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == ">") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___greater___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == "<=") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___lesserEqual___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == ">=") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___greaterEqual___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == "==") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___equal___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == "!=") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		ByteCode right = getBytecode(phpClass, method, body->child[1]);
		bytecode = append(bytecode, right);
		ByteCode iv = invokevirtual(phpClass->operators["___notEqual___"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	}
	if (body->label == "Return") {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[0]));
		bytecode = append(bytecode, areturn());
		return bytecode;
	}
	if (body->label == "Echo") {
		for (auto it = body->child[0]->child.begin(); it != body->child[0]->child.end(); ++it) {
			bytecode = append(bytecode, getBytecode(phpClass, method, (*it)));
			bytecode = append(bytecode, invokestatic(phpClass->operators["___echo___"]));
		}
		return bytecode;
	}
	if (body->label == "Array Element") {
		bytecode = append(bytecode, getBytecodeArrayElement(phpClass, method, body, 0));
		return bytecode;
	}
	if (body->label == "Function Call") {
		string funcName = body->child[0]->child[0]->label;
		if (funcName == "fgets") {
			bytecode = append(bytecode, invokestatic(phpClass->operators["___getString___"]));
		} else if (funcName == "fgetc") {
			bytecode = append(bytecode, invokestatic(phpClass->operators["___getChar___"]));
		} else if (funcName == "array") {
			bytecode = append(bytecode, getBytecodeNewArray(phpClass, method, body->child[1]));
		} else if (funcName == "count") {
			bytecode = append(bytecode, getBytecode(phpClass, method, body->child[1]->child[0]));
			bytecode = append(bytecode, invokevirtual(phpClass->operators["___count___"]));
		} else {
			for (auto itf = body->child[1]->child.begin(); itf != body->child[1]->child.end(); ++itf) {
				bytecode = append(bytecode, getBytecode(phpClass, method, (*itf)));
			}
			bytecode = append(bytecode, invokestatic(phpClass->methods[funcName]->methodrefConstantNumber));
		}
		return bytecode;
	}
	if (body->label.find("\'") != string::npos) {
		PHPConstant* mr = phpClass->constantTable[phpClass->operators["S<init>"]];
		int classConst = (int)(*(int**)mr->value);
		bytecode = _new(classConst);
		ByteCode d = dup();
		bytecode = append(bytecode, d);
		ByteCode ldcv = ldc(phpClass->liters[body->label]);
		bytecode = append(bytecode, ldcv);
		ByteCode iv = invokespecial(phpClass->operators["S<init>"]);
		bytecode = append(bytecode, iv);
		return bytecode;
	} else if (body->label.find(".") != string::npos) {
		PHPConstant* mr = phpClass->constantTable[phpClass->operators["D<init>"]];
		int classConst = (int)(*(int**)mr->value);
		bytecode = _new(classConst);
		ByteCode d = dup();
		bytecode = append(bytecode, d);
		ByteCode ldcv = ldc(phpClass->floats[body->label]);
		bytecode = append(bytecode, ldcv);
		ByteCode iv = invokespecial(phpClass->operators["D<init>"]);
		bytecode = append(bytecode, iv);
	} else if (body->label[0] >= '0' && body->label[0] <= '9') {
		PHPConstant* mr = phpClass->constantTable[phpClass->operators["I<init>"]];
		int classConst = (int)(*(int**)mr->value);
		bytecode = _new(classConst);
		ByteCode d = dup();
		bytecode = append(bytecode, d);
		int number = stoi(body->label);
		ByteCode pint;
		if (number >= -32768 && number <= 32767) {
			pint = pushIntConstant(number);
		} else {
			pint = ldc(phpClass->interegs[body->label]);
		}
		bytecode = append(bytecode, pint);
		ByteCode iv = invokespecial(phpClass->operators["I<init>"]);
		bytecode = append(bytecode, iv);
	} else if (body->label == "$") {
		auto it = find(method->sequenceLocalVariables.begin(), method->sequenceLocalVariables.end(), body->child[0]->child[0]->label);
		int pos = it - method->sequenceLocalVariables.begin();
		ByteCode aloadByte = aload(pos);
		bytecode = append(bytecode, aloadByte);
		return bytecode;
	} else if (body->label == "NULL") {
		bytecode = append(bytecode, aconst_null());
		return bytecode;
	} 
	return bytecode;
	/* */
}

ByteCode getBytecodeNewArray(PHPClass* phpClass, PHPMethod* method, Node* body) {
	ByteCode bytecode;
	PHPConstant* mrarray = phpClass->constantTable[phpClass->operators["Array<init>"]];
	int classConstArray = (int)(*(int**)mrarray->value);
	PHPConstant* mrpair1 = phpClass->constantTable[phpClass->operators["Pair1<init>"]];
	int classConstPair = (int)(*(int**)mrpair1->value);
	int countInits = body->child.size();
	bytecode = append(bytecode, _new(classConstArray));
	bytecode = append(bytecode, dup());
	bytecode = append(bytecode, pushIntConstant(countInits));
	bytecode = append(bytecode, anewarray(classConstPair));
	for (int i = 0; i < body->child.size(); ++i) {
		bytecode = append(bytecode, dup());
		bytecode = append(bytecode, pushIntConstant(i));
		bytecode = append(bytecode, _new(classConstPair));
		bytecode = append(bytecode, dup());
		if (body->child[i]->label == "=>") {
			bytecode = append(bytecode, getBytecode(phpClass, method, body->child[i]->child[0]));
			bytecode = append(bytecode, getBytecode(phpClass, method, body->child[i]->child[1]));
			bytecode = append(bytecode, invokespecial(phpClass->operators["Pair2<init>"]));
		} else {
			bytecode = append(bytecode, getBytecode(phpClass, method, body->child[i]));
			bytecode = append(bytecode, invokespecial(phpClass->operators["Pair1<init>"]));
		}
		bytecode = append(bytecode, aastore());
	}
	bytecode = append(bytecode, invokespecial(phpClass->operators["Array<init>"]));
	/*for (auto it = body->child.begin(); it != body->child.end(); ++it) {
	bytecode = append(bytecode, aload(varPos));
	if (depth > 0) {
	for (int curDepth = 1; curDepth <= depth; ++curDepth) {
	bytecode = append(bytecode, invokevirtual(phpClass->operators["___get___"]));
	bytecode = append(bytecode, checkcast(classConst));
	}
	}
	if ((*it)->label == "=>") {
	bytecode = append(bytecode, getBytecode(phpClass, method, (*it)->child[0]));
	if ((*it)->child[1]->label == "Function Call" && (*it)->child[1]->child[0]->label == "ID" && (*it)->child[1]->child[0]->child[0]->label == "array") {
	bytecode = append(bytecode, _new(classConst));
	bytecode = append(bytecode, dup());
	bytecode = append(bytecode, invokespecial(phpClass->operators["Array<init>"]));
	bytecode = append(bytecode, invokevirtual(phpClass->operators["___putkey___"]));
	bytecode = append(bytecode, getBytecodeNewArray(phpClass, method, (*it)->child[1]->child[1], classConst, varPos, depth + 1));
	} else {
	bytecode = append(bytecode, getBytecode(phpClass, method, (*it)->child[1]));
	bytecode = append(bytecode, invokevirtual(phpClass->operators["___putkey___"]));
	}
	} else if ((*it)->label == "Function Call" && (*it)->child[0]->label == "ID" && (*it)->child[0]->child[0]->label == "array") {
	bytecode = append(bytecode, _new(classConst));
	bytecode = append(bytecode, dup());
	bytecode = append(bytecode, invokespecial(phpClass->operators["Array<init>"]));
	bytecode = append(bytecode, invokevirtual(phpClass->operators["___put___"]));
	bytecode = append(bytecode, getBytecodeNewArray(phpClass, method, (*it)->child[1], classConst, varPos, depth + 1));
	} else {
	bytecode = append(bytecode, getBytecode(phpClass, method, (*it)));
	bytecode = append(bytecode, invokevirtual(phpClass->operators["___put___"]));
	}
	}*/
	return bytecode;
}

ByteCode getBytecodeArrayElement(PHPClass* phpClass, PHPMethod* method, Node* body, int depth) {
	ByteCode bytecode;
	if (body->child[0]->label == "Array Element") {
		bytecode = append(bytecode, getBytecodeArrayElement(phpClass, method, body->child[0], depth + 1));
	} else {
		int pos;
		if (body->label == "$") {
			auto it = find(method->sequenceLocalVariables.begin(), method->sequenceLocalVariables.end(), body->child[0]->child[0]->label);
			pos = it - method->sequenceLocalVariables.begin();
			bytecode = append(bytecode, aload(pos));
			return bytecode;
		} else {
			auto it = find(method->sequenceLocalVariables.begin(), method->sequenceLocalVariables.end(), body->child[0]->child[0]->child[0]->label);
			pos = it - method->sequenceLocalVariables.begin();
			bytecode = append(bytecode, aload(pos));
		}
		//return bytecode;
	}
	if (body->child.size() > 1) {
		bytecode = append(bytecode, getBytecode(phpClass, method, body->child[1]));
		bytecode = append(bytecode, invokevirtual(phpClass->operators["___getkey___"]));
	} else {
		bytecode = append(bytecode, invokevirtual(phpClass->operators["___get___"]));
	}
	/*if (body->child.size() > 1) {
	bytecode = append(bytecode, getBytecode(phpClass, method, body->child[1]));
	bytecode = append(bytecode, invokevirtual(phpClass->operators["___getkey___"]));
	} else {
	bytecode = append(bytecode, invokevirtual(phpClass->operators["___get___"]));
	}*/
	if (depth != 0) {
		PHPConstant* mr = phpClass->constantTable[phpClass->operators["Array<init>"]];
		int classConst = (int)(*(int**)mr->value);
		bytecode = append(bytecode, checkcast(classConst));
	}
	return bytecode;
}

void printCode(PHPClass* phpClass, PHPMethod* method) {
	printBytes(get_u2(1)); //"Code" const
	ByteCode bc = getBytecode(phpClass, method, method->body);
	//TODO ����� ��������
	printBytes(get_u4(2 + 2 + 4 + bc.size() + 2 + 2));
	printBytes(get_u2(1000)); //stack_size
	printBytes(get_u2(method->localVariables.size())); // local_vars_size
	printBytes(get_u4(bc.size())); // local_vars_size
	printBytes(bc.code); //bytecode
	printBytes(get_u2(0)); //exceptions
	printBytes(get_u2(0)); //attrs
}

void printMethods(PHPClass* phpClass, map<string, PHPMethod*>& methods) {
	for (auto it = methods.begin(); it != methods.end(); ++it) {
		printAccessFlags(it->second); 
		int i = it->second->methodrefConstantNumber;
		PHPConstant* mrconst = phpClass->constantTable.at(i);
		int** datamr = (int**)mrconst->value;
		PHPConstant* ntconst = phpClass->constantTable.at((int)*(datamr+1));
		int** datant = (int**)ntconst->value;
		printBytes(get_u2((int)*(datant))); //name_index
		printBytes(get_u2((int)*(datant+1))); //descriptor_index
		printBytes(get_u2(1)); //attributes_count
		printCode(phpClass, it->second);
	}
}

void codeGeneration() {
	for (auto it = phpClasses.begin(); it != phpClasses.end(); ++it) {
		freopen((it->first + ".class").c_str(), "wb", stdout);
		printMagic();
		printBytes(get_u2(0));
		printBytes(get_u2(52));
		printBytes(get_u2(it->second->constantTable.size()));
		printTableConstant(it->second->constantTable);
		printFlags();
		printBytes(get_u2(it->second->classConstantNumber)); // this class
		printBytes(get_u2(it->second->parent)); // parent class
		printBytes(get_u2(0)); //interfaces
		printBytes(get_u2(0)); //fields
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

	PHPClass* curClass = it->second; 
	PHPMethod* mtd = curClass->methods["main"];
	int deep = CreateLocalVariableForForeach(root);
	for(int i = 1; i <= deep; ++i) {
		string name = "___ITER" + to_string((long long)i) + "___";
		mtd->localVariables[name] = mp(curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(name))), curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(curClass->name))));
		mtd->sequenceLocalVariables.pb(name);
	}

	FillListConstantTable(root, it->second);
	FinderParam s(root, phpClasses["___Base___"], it->second->methods["main"]);
	FinderFunc fs(root, "___Base___");
	FindBaseType(it->second);

	FillTables(root);

	prints();

	codeGeneration();
}

string toStr[] = { 
	"UTF8", 
	"CLASS",
	"NAME_AND_TYPE",
	"METHOD_REF",
	"INTERFACE_METHOD_REF"
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

			case ConstantType::UTF8:
				cout << *((string*)((*it1)->value)) << endl;
				break;

			case ConstantType::STRING:
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


	cout << "////////////////////////////////////////////////////////////////////////////////////OPERATORS////////////////////////////////////////////////////////////////////////////////////" << endl;
	for(auto it = phpClasses.begin(); it != phpClasses.end(); ++it) {
		cout << it->first << endl << endl;
		for(auto it1 = (*it).second->operators.begin(); it1 != (*it).second->operators.end(); ++it1) {
			cout << it1->first  << " " << it1->second << endl;
		}
		cout << endl;
	}
}
#endif