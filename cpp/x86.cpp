#include "../header/x86.h"
using namespace MLang;
namespace MLang {
	type analyzeArg(lstring tk) {
		lstring left = tk.substr(0, 1);
		lstring right = tk.substr(1, tk.size() - 1);
		type ret{};
		if (tk.substr(0, 2) == R("&%")) {
			ret.typeName = R("&%");
			ret.name = tk.substr(2, tk.size() - 2);
		}
		else if(left==R("#")) {
			ret.typeName = R("label");
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
	std_lcout << ErrorType << R(" Error at line ") << ErrorLine << R(": ") << err << std::endl;

}
size_t x86Generator::tmpOffset(size_t id,const std::vector<size_t>& stack) {
	if (stack.size() <= id) {
		error(R("临时变量不存在"));
		return 0;
	}
	size_t ret{};
	for (size_t i = 0; i < id; i++) {
		ret += stack[i];
	}
	return ret;
}
SectionManager x86Generator::generate(lstring IR) {
	SectionManager tmp{};

	constStr.clear();
	constStr2.clear();
	linkTable.clear();
	apiTable.clear();
	builtInFunction.clear();
	ErrorType = R("Translate_x86");
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
				error(R("参数个数不符"));
				continue;
			}
			tmp_stack2.resize(max_(st_size_t(args[0].name) + 1, tmp_stack.size()));
			for (size_t i = 0; i < tmp_stack.size(); i++) {
				tmp_stack2[i] = tmp_stack[i];
			}
			validCode = false;
			std::swap(tmp_stack2, tmp_stack);
			tmp_stack[st_size_t(args[0].name)] = st_size_t(args[1].name);
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
				codes.selfInsert(clear_ip, tmp_codes);
			}
		}
		else if (op == R("Local")) {
			if (arg_size != 1) {
				error(R("参数个数不符"));
				continue;
			}
			localSize = st_size_t(args[0].name);
			codes << 137 << 236 << 129 << 236;
			codes += (int)(localSize);
		}
		else if (op == R("num")) {
			if (arg_size != 2) {
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("第一个参数必须提供临时变量"));
				continue;
			}
			intptr_t offset = -localSize - tmpOffset(st_size_t(args[0].name), tmp_stack);
			if (args[1].typeName == R("I")) {
				codes << 199 << 133;
				codes += (int)offset;
				codes += (int)st_intptr_t(args[1].name);
			}
			else if (args[1].typeName == R("D")) {
				union D2W {
					double D;
					int W1;
					int W2;
				}tmp;
				tmp.D = std::stod(args[1].name);
				codes << 199 << 133;
				codes += (int)offset;
				codes += (int)tmp.W1;
				codes << 199 << 133;
				codes += (int)offset + 4;
				codes += (int)tmp.W2;

			}
			else if (args[1].typeName == R("uI")) {
				codes << 199 << 133;
				codes += (int)offset;
				codes += (unsigned int)st_size_t(args[1].name);
			}
			else {
				error(R("第二个参数必须为数字"));
				continue;
			}
		}
		else if (op == R("address")) {
			if (arg_size != 2) {
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("第一个参数必须提供临时变量"));
				continue;
			}
			intptr_t offset = -localSize - tmpOffset(st_size_t(args[1].name), tmp_stack);
			if (args[1].typeName == R("local")) {
				codes << 141 << 133;
				codes += -(int)st_intptr_t(args[1].name);
				codes << 137 << 133;
				codes += (int)offset;
			}
			else if (args[1].typeName == R("global")) {
				codes << 104;
				codes += (int)st_intptr_t(args[2].name);
				codes << 232 << 0 << 0 << 0 << 0;
				redirection tmp{};
				tmp.ip = codes.size;
				tmp.name = R("[System]global");
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
				codes += (int)st_intptr_t(args[2].name);
				codes << 232 << 0 << 0 << 0 << 0;
				redirection tmp{};
				tmp.ip = codes.size;
				tmp.name = R("[System]string");
				redirections.push_back(tmp);
				codes << 137 << 133;
				codes += (int)offset;
			}
			else if (args[1].typeName == R("&tmp")) {
				intptr_t offset2 = -localSize - tmpOffset(st_size_t(args[2].name), tmp_stack);
				codes << 141 << 133;
				codes += (int)offset2;
				codes << 137 << 133;
				codes += (int)offset;
			}
			else {
				error(R("第二个参数必须为地址"));
			}
		}
		else if (op == R("jmp")) {
			if (arg_size != 1) {
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("label")) {
				error(R("参数必须提供标签"));
				continue;
			}
			codes << 233 << 0 << 0 << 0 << 0;
			redirection tmp{};
			tmp.ip = codes.size;
			tmp.name = args[0].name;
			redirections.push_back(tmp);
		}
		else if (op == R("jz")) {
			if (arg_size != 2) {
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("第一个参数必须提供临时变量"));
				continue;
			}
			if (args[1].typeName != R("label")) {
				error(R("第二个参数必须提供标签"));
				continue;
			}
			intptr_t offset = -localSize - tmpOffset(st_size_t(args[0].name), tmp_stack);
			codes << 155 << 219 << 227 << 131 << 189;
			codes += (int)offset;
			codes << 0 << 15 << 132 << 0 << 0 << 0 << 0;
			redirection tmp{};
			tmp.ip = codes.size;
			tmp.name = R("[Label]") + args[1].name;
			redirections.push_back(tmp);

			codes << 129 << 189;
			codes += (int)offset;
			codes << 0 << 0 << 128 << 63 << 116 << 26 << 232 << 0 << 0 << 0 << 0;
			tmp.ip = codes.size;
			tmp.name = R("[System]random");
			redirections.push_back(tmp);

			codes << 80 << 217 << 133;
			codes += (int)offset;
			codes << 216 << 28 << 36 << 155 << 223 << 224 << 158 << 88 << 15 << 134 << 0 << 0 << 0 << 0;
			tmp.ip = codes.size;
			tmp.name = R("[Label]") + args[1].name;
			redirections.push_back(tmp);

		}
		else if (op == R("load")) {
			if (arg_size != 3) {
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("第一个参数必须提供临时变量"));
				continue;
			}
			if (args[1].typeName != R("tmp")) {
				error(R("第二个参数必须提供临时变量"));
				continue;
			}
			if (args[2].typeName != R("I")) {
				error(R("第三个参数必须提供整数"));
				continue;
			}
			size_t size = st_size_t(args[2].name);
			intptr_t offset = -localSize - tmpOffset(st_size_t(args[0].name), tmp_stack);
			intptr_t offset2 = -localSize - tmpOffset(st_size_t(args[1].name), tmp_stack);
			switch (size)
			{
			case 1:
				codes << 141 << 133;
				codes += (int)offset2;
				codes << 139 << 0;
				codes << 136 << 133;
				codes += (int)offset;
				break;
			case 2:
				codes << 141 << 133;
				codes += (int)offset2;
				codes << 139 << 0;
				codes << 102 << 137 << 133;
				codes += (int)offset;
				break;
			case 4:
				codes << 141 << 133;
				codes += (int)offset2;
				codes << 139 << 0;
				codes << 137 << 133;
				codes += (int)offset;
				break;
			case 8:
				codes << 141 << 133;
				codes += (int)offset2;
				codes << 139 << 0;
				codes << 137 << 133;
				codes += (int)offset;
				codes << 137 << 149;
				codes += (int)offset2 + 4;
				break;
			default:
				error(R("不支持的数据大小"));
				break;
			}
		}
		else if (op == R("ret")) {
			if (arg_size != 2) {
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("第一个参数必须提供临时变量"));
				continue;
			}
			if (args[1].typeName != R("I")) {
				error(R("第二个参数必须提供整数"));
				continue;
			}
			intptr_t offset = -localSize - tmpOffset(st_size_t(args[0].name), tmp_stack);
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
				error(R("不支持的数据大小"));
				break;
			}
		}
		else if (op == R("store")) {
			if (arg_size != 3) {
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("第一个参数必须提供临时变量"));
				continue;
			}
			if (args[1].typeName != R("tmp")) {
				error(R("第二个参数必须提供临时变量"));
				continue;
			}
			if (args[2].typeName != R("I")) {
				error(R("第三个参数必须提供整数"));
				continue;
			}
			size_t size = st_size_t(args[2].name);
			intptr_t offset = -localSize - tmpOffset(st_size_t(args[0].name), tmp_stack);
			intptr_t offset2 = -localSize - tmpOffset(st_size_t(args[1].name), tmp_stack);
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
				error(R("不支持的数据大小"));
				break;
			}
		}
		else if (op == R("mov")) {
			if (arg_size != 3) {
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("第一个参数必须提供临时变量"));
				continue;
			}
			if (args[1].typeName != R("tmp")) {
				error(R("第二个参数必须提供临时变量"));
				continue;
			}
			if (args[2].typeName != R("I")) {
				error(R("第三个参数必须提供整数"));
				continue;
			}
			size_t size = st_size_t(args[2].name);
			intptr_t offset = -localSize - tmpOffset(st_size_t(args[0].name), tmp_stack);
			intptr_t offset2 = -localSize - tmpOffset(st_size_t(args[1].name), tmp_stack);
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
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("I")) {
				error(R("第一个参数必须提供整数"));
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
				error(R("参数个数不符"));
				continue;
			}
			if (args[0].typeName != R("tmp")) {
				error(R("第一个参数必须提供临时变量"));
				continue;
			}
			if (args[1].typeName != R("tmp")) {
				error(R("第二个参数必须提供临时变量"));
				continue;
			}
			if (args[2].typeName != R("tmp")) {
				error(R("第三个参数必须提供临时变量"));
				continue;
			}
			if (args[3].typeName != R("tmp")) {
				error(R("第四个参数必须提供临时变量"));
				continue;
			}
			intptr_t offset = -localSize - tmpOffset(st_size_t(args[1].name), tmp_stack);
			intptr_t offset2 = -localSize - tmpOffset(st_size_t(args[2].name), tmp_stack);
			intptr_t offset3 = -localSize - tmpOffset(st_size_t(args[3].name), tmp_stack);
			codes << 139 << 133;
			codes += (int)offset2;
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
		}
	}

	return tmp;
}