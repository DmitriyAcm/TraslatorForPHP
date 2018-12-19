#include <vector>
#include "bytes.h"

vector<char> append(vector<char> left, vector<char> right) {
	vector<char> result(left);
	result.insert(
		result.end(),
		std::make_move_iterator(right.begin()),
		std::make_move_iterator(right.end())
	);
	return result;
}

vector<char> aload(int number) {
	vector<char> result = vector<char>();
	switch (number) {
	case 0:
		result.push_back(0x2A);
		break;
	case 1:
		result.push_back(0x2B);
		break;
	case 2:
		result.push_back(0x2C);
		break;
	case 3:
		result.push_back(0x2D);
		break;
	default:
		result.push_back(0x19);
		result.push_back(number);
		break;
	}
	return result;

}

vector<char> astore(int number) {
	vector<char> result = vector<char>();
	switch (number) {
	case 0:
		result.push_back(0x4B);
		break;
	case 1:
		result.push_back(0x4C);
		break;
	case 2:
		result.push_back(0x4D);
		break;
	case 3:
		result.push_back(0x4E);
		break;
	default:
		result.push_back(0x3A);
		result.push_back(number);
		break;
	}
	return result;
}

vector<char> _new(int classConst) {
	vector<char> result = vector<char>();
	
	result.push_back(0xBB);
	vector<char> cc = get_u2(classConst);
	result = append(result, cc);

	return result;
}

vector<char> invokespecial(int methodRef) {
	vector<char> result = vector<char>();
	
	result.push_back(0xB7);
	vector<char> mr = get_u2(methodRef);
	result = append(result, mr);

	return result;
}

vector<char> invokestatic(int methodRef) {
	vector<char> result = vector<char>();
	
	result.push_back(0xB8);
	vector<char> mr = get_u2(methodRef);
	result = append(result, mr);

	return result;
}

vector<char> invokevirtual(int methodRef) {
	vector<char> result = vector<char>();
	
	result.push_back(0xB6);
	vector<char> mr = get_u2(methodRef);
	result = append(result, mr);

	return result;
}

vector<char> ldc(int constant) {
	vector<char> result = vector<char>();
	result.push_back(0x12);
	result.push_back(constant);
	return result;
}

vector<char> pushIntConstant(int value) {
	vector<char> result = vector<char>();
	switch(value) {
	case -1:
		result.push_back(0x2);
		break;
	case 0:
		result.push_back(0x3);
		break;
	case 1:
		result.push_back(0x4);
		break;
	case 2:
		result.push_back(0x5);
		break;
	case 3:
		result.push_back(0x6);
		break;
	case 4:
		result.push_back(0x7);
		break;
	case 5:
		result.push_back(0x8);
		break;
	default:
		if (value >= -128 && value <= 127) {
			result.push_back(0x10);
			result.push_back(value);
		} else if (value >= -32768 && value <= 32767) {
			result.push_back(0x11);
			result.push_back(value);
		}
		break;
	}
	return result;
}

vector<char> pushDoubleConstant(int doubleConstant) {
	vector<char> result = vector<char>();
	
	result.push_back(0x14);
	vector<char> dc = get_u2(doubleConstant);
	result = append(result, dc);

	return result;
}

vector<char> returnVoid() {
	vector<char> result = vector<char>();
	
	result.push_back(0xB1);
	
	return result;
}

vector<char> returnRef() {
	vector<char> result = vector<char>();
	
	result.push_back(0xB0);
	return result;
}

vector<char> dup() {
	vector<char> result = vector<char>();
	
	result.push_back(0x59);
	return result;
}

