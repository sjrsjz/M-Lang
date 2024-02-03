#define INCPP
#include "../header/Common.h"
#include "../header/x86.h"
#include "../header/base64.h"

using namespace MLang;
namespace MLang {
	type analyzeArg(lstring tk) {
		lstring left = tk.substr(0, 1);
		lstring right = tk.size()>=2?tk.substr(1, max_(tk.size() - 1, 0)):R("");
		type ret{};
		if (tk.substr(0, 2) == R("&%")) {
			ret.typeName = R("&%");
			ret.name = tk.substr(2, tk.size() - 2);
		}
		else if (left == R("#")) {
			ret.typeName = R("label");
			ret.name = right;
		}
		else if (left == R("&"))
		{
			ret.typeName = R("local");
			ret.name = right;
		}
		else if (left == R("$")) {
			ret.typeName = R("global");
			ret.name = right;
		}
		else if (left == R("%")) {
			ret.typeName = R("tmp");
			ret.name = right;
		}
		else if (left == R("@")) {
			ret.typeName = R("const");
			ret.name = right;
		}
		else if (left == R("!")) {
			ret.typeName = R("arg");
			ret.name = right;
		}
		else if (left == R("?")) {
			left = tk.substr(0, 3);
			right = tk.substr(3, tk.size() - 3);
			if (left == R("?uI")) {
				ret.typeName = R("uI");
				ret.name = right;
			}
			else {
				left = tk.substr(0, 2);
				right = tk.substr(2, tk.size() - 2);
				if (left == R("?I")) {
					ret.typeName = R("I");
					ret.name = right;
				}
				else if (left == R("?D")) {
					ret.typeName = R("D");
					ret.name = right;
				}
				else if (left == R("?T")) {
					ret.typeName = R("T");
					ret.name = right;
				}
				else {
					
				}

			}
		}
		else {
			ret.typeName = R("I");
			ret.name = tk;
		}
		return ret;
	}
}
void x86Generator::ins(ByteArray<unsigned char> bytes) {
	codes = codes.Attach(bytes);
}
void x86Generator::error(lstring err) {
	Error = true;
	std_lcout << RED << R("[����]") << CYAN << R("[") << ErrorType << R("]")  << (ErrorLine != -1 ? (R("[��:") + to_lstring(ErrorLine) + R("]")) : R("")) << RESET << err << std::endl;

}
void x86Generator::warning(lstring warn) {
	if (!enableWarings) return;
	std_lcout << YELLOW << R("[����]") << CYAN << R("[") << ErrorType << R("]")  << (ErrorLine != -1 ? (R("[��:") + to_lstring(ErrorLine) + R("]")) : R("")) << RESET << warn << std::endl;
}
size_t x86Generator::tmpSize(size_t id, const std::vector<size_t>& stack) {
	if (!id || id >= stack.size()) {
		error(R("��ʱ����������"));
		return 0;
	}
	else {
		return stack[id - 1];
	}
}
size_t x86Generator::tmpOffset(size_t id,const std::vector<size_t>& stack) {
	if (stack.size() <= id) {
		error(R("��ʱ����������"));
		return 0;
	}
	size_t ret{};
	for (size_t i = 0; i < id; i++) {
		ret += stack[i];
	}
	return ret;
}
void x86Generator::generate(lstring IR) {

	constStr.clear();
	constStr2.clear();
	linkTable.clear();
	apiTable.clear();
	builtInFunction.clear();
	ErrorType = R("x86����");
	std::vector<lstring> lines = split(IR, R("\n"));
	Error = false;
	codes.~ByteArray();

	std::vector<size_t> tmp_stack{};
	std::vector<size_t> tmp_stack2{};
	
	std::vector<redirection> redirections{};
	std::vector<redirection> redirections_label{};

	bool validCode{};
	bool haveValidCode{};
	size_t clear_ip{};
	size_t localSize{};
	for (size_t i = 0; i < lines.size(); i++) {
		ErrorLine = i;
		lstring& x = lines[i];
		if (x == R("") || x.substr(0, 1) == R(";")) {
			continue;
		}
		std::vector<lstring> tk = split(lines[i], R(" "));
		if (!tk.size()) continue;
		if (tk[0].substr(0, 1) == R("#")) {
			redirection tmp{};
			tmp.ip = codes.size;
			tmp.name = R("[Label]") + tk[0].substr(1, tk[0].size() - 1);
			redirections_label.push_back(tmp);
			continue;
		}
		lstring op = tk[0];
		size_t arg_size = tk.size() - 1;
		validCode = true;
		std::vector<type> args{};
		for (size_t i = 0; i < arg_size; i++) {
			args.push_back(analyzeArg(tk[i + 1]));
		}
		if (op == R("tmp")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			tmp_stack2.resize(max_(st_size_t(args[0].name) + 1, tmp_stack.size()));
			for (size_t i = 0; i < tmp_stack.size(); i++) {
				tmp_stack2[i] = tmp_stack[i];
			}
			validCode = false;
			std::swap(tmp_stack2, tmp_stack);
			tmp_stack[st_size_t(args[0].name) - 1] = st_size_t(args[1].name);
		}
		else if (op == R("tmpBegin")) {
			tmp_stack.clear();
			clear_ip = codes.size + 1;
			validCode = false;
			haveValidCode = false;
		}
		else if (op == R("tmpEnd")) {
			size_t size{};
			if (clear_ip && haveValidCode) {
				for (auto& x : tmp_stack) size += x;
				ByteArray<unsigned char> tmp_codes{};
				tmp_codes << 137 << 236 << 129 << 236;
				tmp_codes += (int)(localSize + size);
				size = tmp_codes.size;
				for (auto& x : redirections) {
					if (x.ip >= clear_ip) x.ip += size;
				}
				for (auto& x : redirections_label) {
					if (x.ip >= clear_ip) x.ip += size;
				}
				codes.selfInsert(clear_ip - 1, tmp_codes);
			}
		}
		else if (op == R("local")) {
			if (arg_size != 1) {
				error(R("������������"));
				continue;
			}
			localSize = st_size_t(args[0].name);
			codes << 137 << 236 << 129 << 236;
			codes += (int)(localSize);
		}
		else if (op == R("num")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			if (args[1].typeName == R("I")) {
				codes << 199 << 133;
				codes += (int)offset;
				codes += (int)st_intptr_t(args[1].name);
			}
			else if (args[1].typeName == R("D")) {
				
				double D = std::stod(args[1].name);
				codes << 199 << 133;
				codes += (int)offset;
				codes += *(int*)&D;
				codes << 199 << 133;
				codes += (int)offset + 4;
				codes += *((int*)&D + 1);

			}
			else if (args[1].typeName == R("uI")) {
				codes << 199 << 133;
				codes += (int)offset;
				codes += (unsigned int)st_size_t(args[1].name);
			}
			else {
				error(R("�ڶ�����������Ϊ����"));
				continue;
			}
		}
		else if (op == R("address")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			if (args[1].typeName == R("local")) {
				codes << 141 << 133;
				codes += -(int)st_intptr_t(args[1].name);
				codes << 137 << 133;
				codes += (int)offset;
			}
			else if (args[1].typeName == R("global")) {
				codes << 104;
				codes += (int)st_intptr_t(args[1].name);
				codes << 232 << 0 << 0 << 0 << 0;
				redirection tmp{};
				tmp.ip = codes.size;
				tmp.name = R("[System]global");
				tmp.line = i;
				redirections.push_back(tmp);
				codes << 137 << 133;
				codes += (int)offset;
			}
			else if (args[1].typeName == R("arg")) {
				codes << 141 << 133;
				codes += 2 * sizeof(int) + (int)st_intptr_t(args[1].name);
				codes << 137 << 133;
				codes += (int)offset;
			}
			else if (args[1].typeName == R("T")) {
				codes << 104;
				codes += (int)st_intptr_t(args[1].name);
				codes << 232 << 0 << 0 << 0 << 0;
				redirection tmp{};
				tmp.ip = codes.size;
				tmp.name = R("[System]string");
				tmp.line = i;
				redirections.push_back(tmp);
				codes << 137 << 133;
				codes += (int)offset;
			}
			else if (args[1].typeName == R("&tmp")) {
				intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
				codes << 141 << 133;
				codes += (int)offset2;
				codes << 137 << 133;
				codes += (int)offset;
			}
			else {
				error(R("�ڶ�����������Ϊ��ַ"));
			}
		}
		else if (op == R("jmp")) {
			if (arg_size != 1) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("label")) {
				error(R("���������ṩ��ǩ"));
				continue;
			}
			codes << 233 << 0 << 0 << 0 << 0;
			redirection tmp{};
			tmp.ip = codes.size;
			tmp.name = R("[Label]") + args[0].name;
			tmp.line = i;
			redirections.push_back(tmp);
		}
		else if (op == R("jz")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			if (args[1].typeName != R("label")) {
				error(R("�ڶ������������ṩ��ǩ"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			codes << 155 << 219 << 227 << 131 << 189;
			codes += (int)offset;
			codes << 0 << 15 << 132 << 0 << 0 << 0 << 0;
			redirection tmp{};
			tmp.ip = codes.size;
			tmp.name = R("[Label]") + args[1].name;
			tmp.line = i;
			redirections.push_back(tmp);

			codes << 129 << 189;
			codes += (int)offset;
			codes << 0 << 0 << 128 << 63 << 116 << 26 << 232 << 0 << 0 << 0 << 0;
			tmp.ip = codes.size;
			tmp.name = R("[System]random");
			tmp.line = i;
			redirections.push_back(tmp);

			codes << 80 << 217 << 133;
			codes += (int)offset;
			codes << 216 << 28 << 36 << 155 << 223 << 224 << 158 << 88 << 15 << 134 << 0 << 0 << 0 << 0;
			tmp.ip = codes.size;
			tmp.name = R("[Label]") + args[1].name;
			tmp.line = i;
			redirections.push_back(tmp);

		}
		else if (op == R("load")) {
			if (arg_size != 3) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			if (args[1].typeName != R("tmp")) {
				error(R("�ڶ������������ṩ��ʱ����"));
				continue;
			}
			if (args[2].typeName != R("I")) {
				error(R("���������������ṩ����"));
				continue;
			}
			size_t size = st_size_t(args[2].name);
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			switch (size)
			{
			case 1:
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 139 << 0;
				codes << 136 << 133;
				codes += (int)offset;
				break;
			case 2:
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 139 << 0;
				codes << 102 << 137 << 133;
				codes += (int)offset;
				break;
			case 4:
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 139 << 0;
				codes << 137 << 133;
				codes += (int)offset;
				break;
			case 8:
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 139 << 80 << 4 << 139 << 0;
				codes << 137 << 133;
				codes += (int)offset;
				codes << 137 << 149;
				codes += (int)offset2 + 4;
				break;
			default:
				error(R("��֧�ֵ����ݴ�С"));
				break;
			}
		}
		else if (op == R("ret")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			if (args[1].typeName != R("I")) {
				error(R("�ڶ������������ṩ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			size_t size = st_size_t(args[1].name);
			switch (size)
			{
			case 1:
				codes << 138 << 133;
				codes += (int)offset;
				break;
			case 2:
				codes << 102 << 139 << 133;
				codes += (int)offset;
				break;
			case 4:
				codes << 139 << 133;
				codes += (int)offset;
				break;
			case 8:
				codes << 139 << 133;
				codes += (int)offset;
				codes << 139 << 149;
				codes += (int)offset + 4;
				break;
			default:
				error(R("��֧�ֵ����ݴ�С"));
				break;
			}
		}
		else if (op == R("store")) {
			if (arg_size != 3) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			if (args[1].typeName != R("tmp")) {
				error(R("�ڶ������������ṩ��ʱ����"));
				continue;
			}
			if (args[2].typeName != R("I")) {
				error(R("���������������ṩ����"));
				continue;
			}
			size_t size = st_size_t(args[2].name);
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			switch (size)
			{
			case 1:
				codes << 139 << 157;
				codes += (int)offset;
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 136 << 3;
				break;
			case 2:
				codes << 139 << 157;
				codes += (int)offset;
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 102 << 137 << 3;
				break;
			case 4:
				codes << 139 << 157;
				codes += (int)offset;
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 137 << 3;
				break;
			case 8:
				codes << 139 << 157;
				codes += (int)offset;
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 139 << 149;
				codes += (int)offset2 + 4;
				codes << 137 << 3 << 137 << 83 << 4;
				break;
			default:
				error(R("��֧�ֵ����ݴ�С"));
				break;
			}
		}
		else if (op == R("mov")) {
			if (arg_size != 3) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			if (args[1].typeName != R("tmp")) {
				error(R("�ڶ������������ṩ��ʱ����"));
				continue;
			}
			if (args[2].typeName != R("I")) {
				error(R("���������������ṩ����"));
				continue;
			}
			size_t size = st_size_t(args[2].name);
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			codes << 139 << 189;
			codes += (int)offset;
			codes << 139 << 181;
			codes += (int)offset2;
			codes << 185;
			codes += (int)size;
			codes << 252 << 243 << 164;
		}
		else if (op == R("return")) {
			if (arg_size != 1)
			{
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("I")) {
				error(R("��һ�����������ṩ����"));
				continue;
			}
			size_t size = st_size_t(args[0].name);
			codes << 201 << 194;
			codes += (short)size;
		}
		else if (op == R("enter")) {
			codes << 85 << 137 << 229;
		}
		else if (op == R("opI")) {
			if (arg_size != 4) {
				error(R("������������"));
				continue;
			}
			if (args[1].typeName != R("tmp")) {
				error(R("�ڶ������������ṩ��ʱ����"));
				continue;
			}
			if (args[2].typeName != R("tmp")) {
				error(R("���������������ṩ��ʱ����"));
				continue;
			}
			if (args[3].typeName != R("tmp")) {
				error(R("���ĸ����������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[2].name), tmp_stack));
			intptr_t offset3 = -intptr_t(localSize + tmpOffset(st_size_t(args[3].name), tmp_stack));
			codes << 139 << 133;
			codes += (int)offset2;
			codes << 139 << 157;
			codes += (int)offset3;
			if (tk[1] == R("+")) {
				codes << 3 << 133;
				codes += (int)offset3;
			}
			else if (tk[1] == R("-")) {
				codes << 43 << 133;
				codes += (int)offset3;
			}
			else if (tk[1] == R("*")) {
				codes << 247 << 173;
				codes += (int)offset3;
			}
			else if (tk[1] == R("/") || tk[1] == R("\\")) {
				codes << 153 << 247 << 189;
				codes += (int)offset3;
			}
			else if (tk[1] == R(">")) {
				codes << 57 << 216 << 184 << 0 << 0 << 128 << 63 << 119 << 2 << 49 << 192;
			}
			else if (tk[1] == R("<")) {
				codes << 57 << 216 << 184 << 0 << 0 << 128 << 63 << 114 << 2 << 49 << 192;
			}
			else if (tk[1] == R(">=")) {
				codes << 57 << 216 << 184 << 0 << 0 << 128 << 63 << 115 << 2 << 49 << 192;
			}
			else if (tk[1] == R("<=")) {
				codes << 57 << 216 << 184 << 0 << 0 << 128 << 63 << 118 << 2 << 49 << 192;
			}
			else if (tk[1] == R("!=")) {
				codes << 57 << 216 << 184 << 0 << 0 << 128 << 63 << 117 << 2 << 49 << 192;
			}
			else if (tk[1] == R("==")) {
				codes << 57 << 216 << 184 << 0 << 0 << 128 << 63 << 116 << 2 << 49 << 192;
			}
			else if (tk[1] == R("%")) {
				codes << 153 << 247 << 243 << 137 << 216;
			}
			else {
				error(R("��֧�ֵ������"));
				continue;
			}
			codes << 137 << 133;
			codes += (int)offset;
		}
		else if (op == R("opR")) {
			if (arg_size != 4) {
				error(R("������������"));
				continue;
			}
			if (args[1].typeName != R("tmp") || args[2].typeName != R("tmp") || args[3].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[2].name), tmp_stack));
			intptr_t offset3 = -intptr_t(localSize + tmpOffset(st_size_t(args[3].name), tmp_stack));
			if (tk[1] == R("+")) {
				codes << 221 << 133;
				codes += (int)offset2;
				codes << 220 << 133;
				codes += (int)offset3;
				codes << 221 << 157;
				codes += (int)offset;
			}
			else if (tk[1] == R("-")) {
				codes << 221 << 133;
				codes += (int)offset2;
				codes << 220 << 165;
				codes += (int)offset3;
				codes << 221 << 157;
				codes += (int)offset;
			}
			else if (tk[1] == R("*")) {
				codes << 221 << 133;
				codes += (int)offset2;
				codes << 220 << 141;
				codes += (int)offset3;
				codes << 221 << 157;
				codes += (int)offset;
			}
			else if (tk[1] == R("/")) {
				codes << 221 << 133;
				codes += (int)offset2;
				codes << 220 << 181;
				codes += (int)offset3;
				codes << 221 << 157;
				codes += (int)offset;
			}
			else if (tk[1] == R(">")) {
				codes << 141 << 133;
				codes += (int)offset;
				codes << 242 << 15 << 16 << 133;
				codes += (int)offset2;
				codes << 102 << 15 << 47 << 133;
				codes += (int)offset3;
				codes << 199 << 0 << 0 << 0 << 0 << 0 << 118 << 6 << 199 << 0 << 0 << 0 << 128 << 63;
			}
			else if (tk[1] == R("<")) {
				codes << 141 << 133;
				codes += (int)offset;
				codes << 242 << 15 << 16 << 133;
				codes += (int)offset3;
				codes << 102 << 15 << 47 << 133;
				codes += (int)offset2;
				codes << 199 << 0 << 0 << 0 << 0 << 0 << 115 << 6 << 199 << 0 << 0 << 0 << 128 << 63;
			}
			else if (tk[1] == R(">=")) {
				codes << 141 << 133;
				codes += (int)offset;
				codes << 242 << 15 << 16 << 133;
				codes += (int)offset2;
				codes << 102 << 15 << 47 << 133;
				codes += (int)offset3;
				codes << 199 << 0 << 0 << 0 << 0 << 0 << 114 << 6 << 199 << 0 << 0 << 0 << 128 << 63;
			}
			else if (tk[1] == R("<=")) {
				codes << 141 << 133;
				codes += (int)offset;
				codes << 242 << 15 << 16 << 133;
				codes += (int)offset3;
				codes << 102 << 15 << 47 << 133;
				codes += (int)offset2;
				codes << 199 << 0 << 0 << 0 << 0 << 0 << 119 << 6 << 199 << 0 << 0 << 0 << 128 << 63;
			}
			else if (tk[1] == R("==")) {
				codes << 141 << 133;
				codes += (int)offset;
				codes << 242 << 15 << 16 << 133;
				codes += (int)offset2;
				codes << 102 << 15 << 47 << 133;
				codes += (int)offset3;
				codes << 199 << 0 << 0 << 0 << 0 << 0 << 117 << 6 << 199 << 0 << 0 << 0 << 128 << 63;
			}
			else if (tk[1] == R("!=")) {
				codes << 141 << 133;
				codes += (int)offset;
				codes << 242 << 15 << 16 << 133;
				codes += (int)offset2;
				codes << 102 << 15 << 47 << 133;
				codes += (int)offset3;
				codes << 199 << 0 << 0 << 0 << 0 << 0 << 116 << 6 << 199 << 0 << 0 << 0 << 128 << 63;
			}
			else if (tk[1] == R("Minus")) {
				codes << 221 << 133;
				codes += (int)offset2;
				codes << 217 << 224;
				codes << 221 << 157;
				codes += (int)offset;
			}
			else if (tk[1] == R("Abs")) {
				codes << 221 << 133;
				codes += (int)offset2;
				codes << 217 << 225;
				codes << 221 << 157;
				codes += (int)offset;
			}
			else {
				error(R("��֧�ֵ������"));
				continue;
			}
		}
		else if (op == R("opB")) {
			if (arg_size != 4) {
				error(R("������������"));
				continue;
			}
			if (args[1].typeName != R("tmp") || args[2].typeName != R("tmp") || args[3].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[2].name), tmp_stack));
			intptr_t offset3 = -intptr_t(localSize + tmpOffset(st_size_t(args[3].name), tmp_stack));
			codes << 139 << 133;
			codes += (int)offset2;
			codes << 139 << 157;
			codes += (int)offset3;
			bool cmp = true;
			if (tk[1] == R("+")) {
				codes << 0 << 216;
				cmp = false;
			}
			else if (tk[1] == R("-")) {
				codes << 40 << 216;
				cmp = false;
			}
			else if (tk[1] == R("*")) {
				codes << 246 << 227;
				cmp = false;
			}
			else if (tk[1] == R("/") || tk[1] == R("\\")) {
				codes << 102 << 152 << 246 << 243;
				cmp = false;
			}
			else if (tk[1] == R(">")) {
				codes << 56 << 216 << 184 << 0 << 0 << 128 << 63 << 119 << 2 << 49 << 192;
			}
			else if (tk[1] == R("<")) {
				codes << 56 << 216 << 184 << 0 << 0 << 128 << 63 << 114 << 2 << 49 << 192;
			}
			else if (tk[1] == R(">=")) {
				codes << 56 << 216 << 184 << 0 << 0 << 128 << 63 << 115 << 2 << 49 << 192;
			}
			else if (tk[1] == R("<=")) {
				codes << 56 << 216 << 184 << 0 << 0 << 128 << 63 << 118 << 2 << 49 << 192;
			}
			else if (tk[1] == R("!=")) {
				codes << 56 << 216 << 184 << 0 << 0 << 128 << 63 << 117 << 2 << 49 << 192;
			}
			else if (tk[1] == R("==")) {
				codes << 56 << 216 << 184 << 0 << 0 << 128 << 63 << 116 << 2 << 49 << 192;
			}
			else if (tk[1] == R("%")) {
				codes << 102 << 152 << 246 << 243 << 136 << 216;
				cmp = false;
			}
			else if (tk[1] == R("Minus")) {
				codes << 246 << 216;
				cmp = false;
			}
			else {
				error(R("��֧�ֵ������"));
				continue;
			}
			if (cmp)
				codes << 137 << 133;
			else
				codes << 136 << 133;
			codes += (int)offset;

		}
		else if (op == R("opBoolen")) {
			if (arg_size != 4) {
				error(R("������������"));
				continue;
			}
			if (args[1].typeName != R("tmp") || args[2].typeName != R("tmp") || args[3].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[2].name), tmp_stack));
			intptr_t offset3 = -intptr_t(localSize + tmpOffset(st_size_t(args[3].name), tmp_stack));
			if (tk[1] == R("!=")) {
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 59 << 133;
				codes += (int)offset3;
				codes << 15 << 149 << 192 << 37 << 255 << 0 << 0 << 0;
				codes << 137 << 133;
				codes += (int)offset;
			}
			else if (tk[1] == R("==")) {
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 59 << 133;
				codes += (int)offset3;
				codes << 15 << 148 << 192 << 37 << 255 << 0 << 0 << 0;
				codes << 137 << 133;
				codes += (int)offset;
			}
			else if (tk[1] == R("and")) {
				codes << 243 << 15 << 16 << 133;
				codes += (int)offset2;
				codes << 243 << 15 << 89 << 133;
				codes += (int)offset3;
				codes << 243 << 15 << 17 << 133;
				codes += (int)offset;
			}
			else if (tk[1] == R("xor")) {
				codes << 104;
				codes += (float)1;
				codes << 141 << 133;
				codes += (int)offset2;
				codes << 141 << 157;
				codes += (int)offset3;
				codes << 243 << 15 << 16 << 0 << 243 << 15 << 89 << 3 << 243 << 15 << 16 << 12 << 36 << 243 << 15
					<< 92 << 200 << 243 << 15 << 16 << 0 << 243 << 15 << 88 << 3 << 243 << 15 << 16 << 16 << 243
					<< 15 << 89 << 19 << 243 << 15 << 92 << 194 << 243 << 15 << 89 << 200 << 243 << 15 << 17
					<< 140 << 36;
			}
			else if (tk[1] == R("or")) {
				codes << 243 << 15 << 16 << 133;
				codes += (int)offset2;
				codes << 243 << 15 << 88 << 133;
				codes += (int)offset3;
				codes << 243 << 15 << 16 << 141;
				codes += (int)offset2;
				codes << 243 << 15 << 89 << 141;
				codes += (int)offset3;
				codes << 243 << 15 << 92 << 193 << 243 << 15 << 17 << 133;
				codes += (int)offset3;
			}
			else if (tk[1] == R("not")) {
				codes << 104;
				codes += (float)1;
				codes << 243 << 15 << 16 << 4 << 36 << 243 << 15 << 92 << 133;
				codes += (int)offset2;
				codes << 243 << 15 << 17 << 133;
				codes += (int)offset;
				codes << 88;
			}
			else {
				error(R("��֧�ֵ������"));
				continue;
			}
		}
		else if (op == R("Z2R")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp") || args[1].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			codes << 219 << 133;
			codes += (int)offset2;
			codes << 221 << 157;
			codes += (int)offset;
		}
		else if (op == R("R2Z")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp") || args[1].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			codes << 221 << 133;
			codes += (int)offset2;
			codes << 219 << 157;
			codes += (int)offset;
		}
		else if (op == R("Z2B")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp") || args[1].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			codes << 139 << 133;
			codes += (int)offset2;
			codes << 136 << 133;
			codes += (int)offset;
		}
		else if (op == R("B2Z")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp") || args[1].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			codes << 138 << 133;
			codes += (int)offset2;
			codes << 37 << 255 << 0 << 0 << 0;
			codes << 137 << 133;
			codes += (int)offset;
		}
		else if (op == R("N2Z") || op == R("Z2N")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp") || args[1].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			if (offset2 != offset) {
				codes << 139 << 133;
				codes += (int)offset2;
				codes << 137 << 133;
				codes += (int)offset;
			}
		}
		else if (op == R("Call") || op == R("Call_cdecl")) {
			size_t size{},size2{};
			if (arg_size < 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("label")) {
				error(R("��һ�����������ṩ��ǩ"));
				continue;
			}
			for (size_t j = arg_size - 1; j >= 3; j--) {
				size_t id = st_size_t(args[j].name);
				if (args[j].typeName == R("&tmp")) {
					size = tmpSize(id, tmp_stack);
					codes << 141 << 133;
					codes += (int)( -intptr_t(localSize + tmpOffset(id, tmp_stack)));
					codes << 80;
					size2 += 4;
				}
				else if (args[j].typeName == R("tmp")) {
					size = tmpSize(id, tmp_stack);
				    intptr_t offset2 = -intptr_t(localSize + tmpOffset(id, tmp_stack));
					switch (size)
					{
					case 1:
						codes << 139 << 133;
						codes += (int)offset2;
						codes << 102 << 37 << 255 << 0;
						codes << 102 << 80;
						size2 += 2;
						break;
					case 2:
						codes << 139 << 133;
						codes += (int)offset2;
						codes << 102 << 80;
						size2 += 2;
						break;
					case 4:
						codes << 255 << 181;
						codes += (int)offset2;
						size2 += 4;
						break;
					case 8:
						codes << 139 << 133;
						codes += (int)offset2;
						codes << 139 << 149;
						codes += (int)offset2 + 4;
						codes << 82 << 80;
						size2 += 8;
						break;
					default:
						error(R("��֧�ֵ����ݴ�С"));
						break;
					}
				}
			}
			codes << 232 << 0 << 0 << 0 << 0;
			redirection tmp{};
			tmp.ip = codes.size;
			tmp.name = R("[Label]") + args[0].name;
			tmp.line = i;
			redirections.push_back(tmp);
			if (op == R("Call_cdecl")) {
				codes << 129 << 196;
				codes += (int)size2;
			}
			else if (args[1].name != R("null")) {
				size_t id = st_size_t(args[1].name);
				size = tmpSize(id, tmp_stack);
				intptr_t offset2 = -intptr_t(localSize + tmpOffset(id, tmp_stack));
				switch (size)
				{
				case 1:
					codes << 136 << 133;
					codes += (int)offset2;
					break;
				case 2:
					codes << 102 << 137 << 133;
					codes += (int)offset2;
					break;
				case 4:
					codes << 137 << 133;
					codes += (int)offset2;
					break;
				case 8:
					codes << 137 << 133;
					codes += (int)offset2;
					codes << 137 << 149;
					codes += (int)offset2 + 4;
					break;
				default:
					error(R("��֧�ֵ����ݴ�С"));
					break;
				}
			}
		}
		else if (op == R("CallA") || op == R("CallA_cdecl")) {
			size_t size{}, size2{};
			if (arg_size < 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("label")) {
				error(R("��һ�����������ṩ��ǩ"));
				continue;
			}
			for (size_t j = arg_size - 1; j >= 3; j--) {
				size_t id = st_size_t(args[j].name);
				if (args[j].typeName == R("&tmp")) {
					size = tmpSize(id, tmp_stack);
					codes << 141 << 133;
					codes += (int)(-intptr_t(localSize + tmpOffset(id, tmp_stack)));
					codes << 80;
					size2 += 4;
				}
				else if (args[j].typeName == R("tmp")) {
					size = tmpSize(id, tmp_stack);
					intptr_t offset2 = -intptr_t(localSize + tmpOffset(id, tmp_stack));
					switch (size)
					{
					case 1:
						codes << 139 << 133;
						codes += (int)offset2;
						codes << 102 << 37 << 255 << 0;
						codes << 102 << 80;
						size2 += 2;
						break;
					case 2:
						codes << 139 << 133;
						codes += (int)offset2;
						codes << 102 << 80;
						size2 += 2;
						break;
					case 4:
						codes << 255 << 181;
						codes += (int)offset2;
						size2 += 4;
						break;
					case 8:
						codes << 139 << 133;
						codes += (int)offset2;
						codes << 139 << 149;
						codes += (int)offset2 + 4;
						codes << 82 << 80;
						size2 += 8;
						break;
					default:
						error(R("��֧�ֵ����ݴ�С"));
						break;
					}
				}
			}
			codes << 232 << 0 << 0 << 0 << 0;
			redirection tmp{};
			tmp.ip = codes.size;
			tmp.name = R("[Label]") + args[0].name;
			tmp.line = i;
			redirections.push_back(tmp);
			if (op == R("CallA_cdecl")) {
				codes<< 129 << 196;
				codes += (int)size2;
			}
		}
		else if (op == R("[string]")) {
			if (arg_size < 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("I")) {
				error(R("��һ�����������ṩ����"));
				continue;
			}
			if (args[1].typeName != R("T")) {
				error(R("�ڶ������������ṩBase64������ı�"));
				continue;
			}
			size_t size = st_size_t(args[0].name);
			std::vector<lstring> tmp;
			tmp.resize(max_(size, constStr.size()));
			for (size_t i = 0; i < constStr.size(); i++) {
				tmp[i] = constStr[i];
			}
			std::swap(tmp, constStr);
			*(constStr.end()-1) = base64_decode(args[1].name);
		}
		else if (op == R("[GlobalSize]")) {
			if (!arg_size) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("I")) {
				error(R("��һ�����������ṩ����"));
				continue;
			}
			globalSize = st_size_t(args[0].name);
		}
		else if (op == R("offset")) {
			if (arg_size < 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			if (args[1].typeName != R("I")) {
				error(R("�ڶ������������ṩ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			size_t size = st_size_t(args[1].name);
			if (size) {
				codes << 129 << 133;
				codes += (int)offset;
				codes += (int)size;
			}
		}
		else if (op==R("[API]")) {
			if (arg_size < 2) {
				error(R("������������"));
				continue;
			}
			apiTable.push_back(args[0].name + R(" ") + args[1].name);
			addBuiltInFunction(R("[API]") + args[0].name + R(" ") + args[1].name, redirections_label);
		}
		else if (op == R("[System]")) {
			if (!arg_size) {
				error(R("������������"));
				continue;
			}
			builtInFunction.push_back(args[0].name);
		}
		else if (op == R("storeQ")) {
			if (arg_size != 1) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			codes << 137 << 149;
			codes += (int)offset + 4;
			codes << 137 << 133;
			codes += (int)offset;

		}
		else if (op == R("loadQ")) {
			if (arg_size != 1) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			codes << 139 << 133;
			codes += (int)offset;
			codes << 139 << 149;
			codes += (int)offset + 4;
		}
		else if (op == R("ExactlyAddress")) {
			if (arg_size > 2 || arg_size < 1) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("��һ�����������ṩ��ʱ����"));
				continue;
			}
			if (arg_size == 2 && args[1].typeName != R("label")) {
				error(R("�ڶ������������ṩ��ǩ"));
				continue;
			}
			codes << 232 << 0 << 0 << 0 << 0 << 88 << 131 << 192 << 9 << 5 << 0 << 0 << 0 << 0;
			if (arg_size == 2) {
				redirection tmp{};
				tmp.ip = codes.size;
				tmp.name = R("[Label]") + args[1].name;
				tmp.line = i;
				redirections.push_back(tmp);
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			codes << 137 << 133;
			codes += (int)offset;
		}
		else if (op == R("pause")) {
			codes << 204;
		}
		else if (op == R("Boolen2Z")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp") || args[1].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			codes << 104;
			codes += (float)0.5;
			codes << 243 << 15 << 16 << 133;
			codes += (int)offset2;
			codes << 15 << 47 << 4 << 36 << 118 << 12 << 199 << 133;
			codes += (int)offset;
			codes << 1 << 0 << 0 << 0 << 235 << 10 << 199 << 133;
			codes += (int)offset;
			codes << 0 << 0 << 0 << 0;
		}
		else if (op == R("Z2Boolen")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp") || args[1].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}

			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			codes << 131 << 189;
			codes += (int)offset2;
			codes << 0 << 199 << 133;
			codes += (int)offset;
			codes += float(1);
			codes << 117 << 10 << 199 << 133;
			codes += (int)offset;
			codes << 0 << 0 << 0 << 0;

		}
		else if (op == R("R2Boolen")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp") || args[1].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			codes << 221 << 133;
			codes += (int)offset2;
			codes << 217 << 157;
			codes += (int)offset;
		}
		else if (op == R("Boolen2R")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("tmp") || args[1].typeName != R("tmp")) {
				error(R("���������ṩ��ʱ����"));
				continue;
			}
			intptr_t offset = -intptr_t(localSize + tmpOffset(st_size_t(args[0].name), tmp_stack));
			intptr_t offset2 = -intptr_t(localSize + tmpOffset(st_size_t(args[1].name), tmp_stack));
			codes << 217 << 133;
			codes += (int)offset2;
			codes << 221 << 157;
			codes += (int)offset;
		}
		else if (op == R("jmp_address")) {
			if (arg_size != 0) {
				error(R("������������"));
				continue;
			}
			codes << 201 << 255 << 224;
		}
		else if (op == R("storeThis")) {
			if (arg_size != 1) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("local")) {
				error(R("��һ�����������ṩ�ֲ�����"));
				continue;
			}
			intptr_t offset = -st_intptr_t(args[0].name);
			codes << 137 << 141;
			codes += (int)offset;
		}
		else if(op == R("storeThisArg")) {
			if (arg_size != 2) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("local")) {
				error(R("��һ�����������ṩ�ֲ�����"));
				continue;
			}
			if (args[1].typeName != R("arg")) {
				error(R("�ڶ������������ṩ����"));
				continue;
			}
			intptr_t offset = -st_intptr_t(args[0].name);
			intptr_t offset2= st_intptr_t(args[1].name);
			codes << 139 << 133;
			codes += (int)offset;
			codes << 137 << 133;
			codes += (int)offset2;
		}
		else if (op == R("loadThisArg")) {
			if (arg_size != 1) {
				error(R("������������"));
				continue;
			}
			if (args[0].typeName != R("local")) {
				error(R("��һ�����������ṩ�ֲ�����"));
				continue;
			}
			intptr_t offset = -st_intptr_t(args[0].name);
			codes << 139 << 141;
			codes += (int)offset;
		}
		else {
			error(R("δָ֪��:") + lines[i]);
			continue;
		}
		haveValidCode = validCode;
	}
	ErrorType = R("�ض�λ");
	addBuiltInFunction(R("[System]string"), redirections_label);
	addBuiltInFunction(R("[System]global"), redirections_label);
	addBuiltInFunction(R("[System]random"), redirections_label);
	for (auto& x : builtInFunction) {
		addBuiltInFunction(R("[Label]") + x, redirections_label);
	}
	size_t size = redirections.size();
	std::vector<bool> used; used.resize(size, false);
	std::vector<bool> used2; used2.resize(redirections_label.size(), true);
	for (size_t i = 0; i < redirections_label.size(); i++) {
		for (size_t j = 0; j < size; j++) {
			if (used[j]) continue;
			if (redirections_label[i].name == redirections[j].name) {
				used[j] = true;
				used2[i] = false;
				ByteArray<unsigned char> tmp{};
				tmp += int(redirections_label[i].ip - redirections[j].ip);
				codes = codes.Replace(redirections[j].ip - 4, tmp, sizeof(int));
			}
		}
	}
	for (size_t i = 0; i < size; i++) {
		if (!used[i]) {
			ErrorLine = redirections[i].line;
			error(R("δ֪��ǩ:") + redirections[i].name + R(" ip:") + to_lstring(redirections[i].ip));
		}
	}
	for (size_t i = 0; i < redirections_label.size(); i++) {
		if (used2[i]) {
			ErrorLine = redirections_label[i].line;
			warning(R("δ�����ӵı�ǩ:") + redirections_label[i].name + R(" ip:") + to_lstring(redirections_label[i].ip));
		}
	}
	DebugOutput(codes);
}
void x86Generator::addBuiltInFunction(const lstring& name, std::vector<redirection>& redirections) {
	redirection tmp{};
	tmp.ip = codes.size;
	tmp.name = name;
	tmp.line = -1;
	redirections.push_back(tmp);
	codes << 184;
	tmp.ip = (int)codes.size;
	tmp.name = name;
	linkTable.push_back(tmp);
	codes << 0 << 0 << 0 << 0 << 255 << 224;
}
void error(lstring err) {
	std_lcout << err << std::endl;
}
namespace MLang::x86Runner {
	void NewSysFunction() {
		sys_redirectTable.clear();
		sys_redirect.clear();
	}
	void NewSysFunction(const lstring& name, void* func) {
		sys_redirectTable.push_back(name);
		sys_redirect.push_back(func);
	}

	void* LoadLibrary_(lstring lib) {
		return (void*)LoadLibrary(lib.c_str());
	}
	void* InitApi(lstring lib, lstring api) {
#ifdef G_UNICODE_
		return (void*)GetProcAddress((HMODULE)GetModuleHandle(lib.c_str()), to_byte_string(api).c_str());
#else
		return (void*)GetProcAddress((HMODULE)GetModuleHandle(lib.c_str()), api.c_str());
#endif // G_UNICODE_

	}
	void FreeLibrary_(void* lib) {
		FreeLibrary((HMODULE)lib);
	}

	void release() {
		delete[] GlobalAddress;
		GlobalAddress = nullptr;
		GlobalSize = 0;
		VirtualFree(ProgramAddress,0, MEM_RELEASE);
		ProgramAddress = nullptr;

		strings.clear();
	}
	void LoadMEXE(const ByteArray<unsigned char>& mexe) {
		SectionManager sm{}, data{};
		sm.translate(mexe);
		data.translate(sm.Get(R("API table")));
		std::vector<lstring> apiTable = data.names;
		data.translate(sm.Get(R("Strings")));
		std::vector<lstring> constStr = data.names;
		data.translate(sm.Get(R("Redirect table")));
		std::vector<redirection> redirections{};
		redirections.resize(data.names.size());
		for (size_t i = 0; i < data.names.size(); i++)
		{
			redirections[i].name = data.names[i];
			redirections[i].ip = data.bins[i].Get<int>(0);
		}
		Load(sm.Get(R("Code")), constStr, redirections, sm.Get(R("Global size")).Get<int>(0), apiTable);
	}
	void LinkFunction(ByteArray<unsigned char>& code, lstring name, void* address,const std::vector<redirection>& redirections) {
		for (auto& x : redirections) {
			if (x.name == name) {
				ByteArray<unsigned char> tmp{};
				tmp = (int)address;
				code = code.Replace(x.ip, tmp, 4);
			}
		}
	}
	void LinkSysFunction(ByteArray<unsigned char>& code,const std::vector<redirection>& redirections) {
		for (size_t i = 0; i < sys_redirectTable.size(); i++) {
			LinkFunction(code, sys_redirectTable[i], sys_redirect[i], redirections);
		}
	}


	unsigned int __stdcall string(unsigned int id) {
		if (!id || id > (int)strings.size()) return 0;
		return (unsigned int)strings[id - 1].c_str();
	}
	unsigned int __stdcall global(int offset) {
		return (unsigned int)GlobalAddress + offset;
	}
	unsigned int __stdcall input(unsigned int str, unsigned int size) {
		scanf_s("%s", (char*)str, size);
		return str;
	}
	float __stdcall CmpStr(unsigned int A, unsigned int B) {
		return (float)strcmp((char*)A, (char*)B);
	}
	float __stdcall CmpMem(unsigned int A, unsigned int B, unsigned int size) {
		for (unsigned int i = 0; i < size; i++) {
			if ((unsigned char*)(A + i) != (unsigned char*)(B + i)) return 0.0;
		}
		return 1.0;
	}
	unsigned int __stdcall memcopy(unsigned int from, unsigned int to, unsigned int size) {
		memcpy((void*)from, (void*)to, size);
		return from;
	}
	void __stdcall DebugOutput_(unsigned int str) {
		DebugOutput((char*)str);
	}
	unsigned int __stdcall R2T(double data, unsigned int str) {
		sprintf_s((char*)str, 256, "%f", data);
		return str;
	}
	double __stdcall T2R(unsigned int str) {
		return std::stod((char*)str);
	}
	unsigned int __stdcall new_(unsigned int size) {
		return (unsigned int)malloc(size);
	}
	void __stdcall free_(unsigned int address) {
		free((void*)address);
	}
	void __stdcall print(unsigned int str) {
		 std_lcout << (lstring)(lchar*)str;
	}
	void __stdcall printR(double data) {
		printf("%f", data);
	}
	void __stdcall printZ(int data) {
		printf("%d", data);
	}
	void __stdcall printN(unsigned int data) {
		printf("%u", data);
	}
	void __stdcall printB(unsigned int data) {
		printf("%d",(unsigned int)data);
	}
	void __stdcall printBoolen(float data) {
		if (data == 1.0) {
			printf("true"); return; 
		}
		if (data == 0.0) {
			printf("false"); return;
		}
		else printf("unclear");
	}
	float __stdcall random() {
		return (float)rand() / RAND_MAX;
	}
	void Load(const ByteArray<unsigned char>& code, const std::vector<lstring>& constStr, const std::vector<redirection>& redirections, size_t GlobalSize_, const std::vector<lstring> apiTable) {
		release();
		GlobalSize = GlobalSize_;
		GlobalAddress = new unsigned char[GlobalSize];
		strings = constStr;
		auto tcode = code;
		DebugOutput(tcode);
		LinkFunction(tcode, R("[System]string"), string, redirections);
		LinkFunction(tcode, R("[System]global"), global, redirections);
		LinkFunction(tcode, R("[System]random"), random, redirections);
		
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$print"), print, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$printN"), printN, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$printZ"), printZ, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$printR"), printR, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$printB"), printB, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$printBoolen"), printBoolen, redirections);

		LinkFunction(tcode, R("[Label]label_function_Local$[System]$memcopy"), memcopy, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$CmpStr"), CmpStr, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$CmpMem"), CmpMem, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$input"), input, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$free"), free_, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$new"), new_, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$T2R"), T2R, redirections);
		LinkFunction(tcode, R("[Label]label_function_Local$[System]$R2T"), R2T, redirections);

		LinkSysFunction(tcode, redirections);

		for (auto& x : apiTable) {
			std::vector<lstring> tk = split(x, R(" "));
			if (tk.size() != 2) {
				error(R("API�����"));
				continue;
			}
			lstring lib = base64_decode(tk[0]);
			lstring api = base64_decode(tk[1]);
			void* addr = InitApi(lib, api);
			if (!addr) {
				void* handle = LoadLibrary_(lib);
				if (!handle) {
					error(R("�޷����ؿ�:") + lib);
					continue;
				}
				lib_h.push_back(handle);
				addr = InitApi(lib, api);
			}
			if (!addr) {
				error(R("�޷�����API:") + api);
				continue;
			}
			LinkFunction(tcode, R("[API]") + x, addr, redirections);
		}
		ProgramAddress = VirtualAlloc(NULL, tcode.size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memcpy(ProgramAddress, tcode.ptr, tcode.size);
		DebugOutput(tcode, ProgramAddress);
	}
	void run() {
		try {
			void (__stdcall *func)() = (void(__stdcall *)())ProgramAddress;
			func();
		}
		catch (std::exception e) {
#ifdef G_UNICODE_
			error(to_wide_string((std::string)e.what()));
#else
			error((std::string)e.what());
#endif // G_UNICODE_
		}
	}
}