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

const string OBJECT = "<Object;>";
const string FUNCT = "Function Definition";
const string CLAS = "Class Declaration";


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
	FIELD_REF
};

struct PHPConstant {
	ConstantType type;
	const void* value;
	PHPConstant(const ConstantType type, const void* value) : type(type), value(value) {}
	friend bool operator<(const PHPConstant this1, const PHPConstant other) {
		if(this1.type < other.type) {
			return true;
		} else if(this1.type == other.type) {
			if((this1).type == ConstantType::UTF8) {
				return *((string*)((this1).value)) < *((string*)((other).value));
			} else if((this1).type == ConstantType::CLASS) {
				return *((int*)((this1).value)) < *((int*)((other).value));
			} else {
				int left1 = ((int*)((this1).value))[0], left2 = ((int*)((this1).value))[1];
				int right1 = ((int*)((other).value))[0], right2 = ((int*)((other).value))[1];
				if(left1 < right1 || left1 == right1 && left2 < right2) {
					return true;
				}
			}
		}

		return false;
	}
};

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

struct PHPClass {
public:
	string name;
	string parent;
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
	int putRef(const string& label, const string& nameClass, ConstantType type, string typeObject = "") {
		int id1 = pushConst(new PHPConstant(ConstantType::UTF8, new string(label)));
		
		int id2 = 0;
		if(typeObject == "") {
			id2 = pushConst(new PHPConstant(ConstantType::UTF8, &OBJECT));
		} else {
			id2 = pushConst(new PHPConstant(ConstantType::UTF8, new string(typeObject)));
		}

		int *pos = new int(2);
		pos[0] = id1;
		pos[1] = id2;
		int id3 = pushConst(new PHPConstant(ConstantType::NAME_AND_TYPE, pos));

		pos = new int(2);
		pos[0] = pushConst(new PHPConstant(ConstantType::CLASS, new int(pushConst(new PHPConstant(ConstantType::UTF8, new string(nameClass) ) ) ) ) );
		pos[1] = id3;
		int id4 = pushConst(new PHPConstant(type, pos));

		return id4;
	}

private:
	map<PHPConstant, int> inTableConst;

};

////////
//Fill table

map<string, PHPClass*> phpClasses;

////////

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

	void fillArguments(Node* node, vector<string>& args) {
		for(auto it = node->child.begin(); it != node->child.end(); ++it) {
			args.pb((*it)->child[0]->label);
		}
	}

public:
	static string getConstName(int col) {
		string res;

		res.pb('(');

		while(col-- > 0) {
			res += "L" + OBJECT;
			if(col) {
				res += ' ';
			}
		}
		res.pb(')');

		res += "L" + OBJECT;

		return res;
	}

	void tryNode(Node* node) {
		if(node->label == CLAS) {
			return;
		}

		if(node->label == FUNCT) {
			PHPMethod* mtd = new PHPMethod();

			mtd->body = node->child[1];
			mtd->isStatic = isStat;
			mtd->accessLevel = acc;

			string lab = node->child[0]->child[0]->label;

			curClass->methods[node->child[0]->child[0]->label] = mtd;
			if (!mtd->isStatic) {
				mtd->localVariables["this"] = mp(curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string("this"))), curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string("L" + curClass->name))));
				mtd->sequenceLocalVariables.pb("this");
			}
			
			vector<string> args;
			fillArguments(node->child[0]->child[1], args);

			mtd->methodrefConstantNumber = curClass->putRef(lab, curClass->name, ConstantType::METHOD_REF, getConstName(args.size()));

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

	FinderFunc(Node* node, const string& nameClass) {
		acc = AccessLevel::PUBLIC;
		isStat = false;
		curClass = phpClasses[nameClass];
		tryNode(node);
	}
};

void FillTables(Node* node) {
	for(auto it = node->child.begin(); it != node->child.end(); ++it) {
		Node* curNode = (*it);
		if((*it)->label == CLAS) {
			string name = curNode->child[0]->label;

			PHPClass* curClass = new PHPClass();

			phpClasses[name] = curClass;

			curClass->name = name;
			curClass->classConstantNumber = 2;
			curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string("")));
			curClass->pushConst(new PHPConstant(ConstantType::UTF8, new string(name)));
			curClass->pushConst(new PHPConstant(ConstantType::CLASS, new int(curClass->classConstantNumber - 1)));

			if(curNode->child.size() == 1) {
				return;
			}

			Node* nodeMem = curNode->child[1];
			// Extends
			if(curNode->child[1]->label != "Class Member List") {
				curClass->parent = curNode->child[1]->label;
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

			FinderFunc(nodeMem, name);

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

						prop->fieldrefConstantNumber = curClass->putRef(var->child[0]->label, curClass->name, ConstantType::FIELD_REF);

						curClass->properties[var->child[0]->label] = prop;
					}
				}
			}
		}
	}
}

void prints();

void main() {
	ParseTree tree("finalTree.dot");
	Node* root = tree.parse();

	root = root->child[0];

	string nameClass = "<Base>";
	phpClasses[nameClass] = (new PHPClass());

	auto it = --phpClasses.end();
	it->second->name = nameClass;
	it->second->classConstantNumber = 2;

	it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("")));
	it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string(it->second->name)));
	it->second->pushConst(new PHPConstant(ConstantType::CLASS, new int(it->second->classConstantNumber - 1)));

	// init
	{
		PHPMethod* meth = new PHPMethod();
		string name = "<init>";

		meth->methodrefConstantNumber = it->second->putRef(name, "java/lang/Object", ConstantType::METHOD_REF, "()V");
		meth->accessLevel = AccessLevel::PUBLIC;
		meth->isStatic = false;
		meth->body = root;
		it->second->methods[name] = meth;
		meth->localVariables["this"] = mp(it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("this"))), it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("L" + it->second->name))));
		meth->sequenceLocalVariables.pb("this");
	}

	// main
	{
		PHPMethod* meth = new PHPMethod();
		string name = "<main>";
		meth->methodrefConstantNumber = it->second->putRef(name, it->second->name, ConstantType::METHOD_REF, "([Ljava/lang/String;)V");
		meth->accessLevel = AccessLevel::PUBLIC;
		meth->isStatic = true;
		meth->body = root;
		it->second->methods[name] = meth;
		meth->localVariables["args"] = mp(it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("args"))), it->second->pushConst(new PHPConstant(ConstantType::UTF8, new string("[Ljava/lang/String;"))));
		meth->sequenceLocalVariables.pb("args");
	}

	FinderParam s(root, phpClasses["<Base>"], it->second->methods["<main>"]);
	FinderFunc fs(root, "<Base>");

	FillTables(root);

	prints();
}

string toStr[] = { 
	"UTF8", 
	"CLASS",
	"NAME_AND_TYPE",
	"METHOD_REF",
	"FIELD_REF"
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
			if((*it1)->type == ConstantType::UTF8) {
				cout << *((string*)((*it1)->value)) << endl;
			} else if((*it1)->type == ConstantType::CLASS) {
				cout << *((int*)((*it1)->value)) << endl;
			} else {
				cout << ((int*)((*it1)->value))[0] << ',' << ((int*)((*it1)->value))[1] << endl;
			}
		}
	}

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