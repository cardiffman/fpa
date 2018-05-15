/*
 * fpa.cpp
 *
 *  Created on: May 10, 2018
 *      Author: menright
 */

/*
 * fps.cpp
 *
 *  Created on: Mar 23, 2018
 *      Author: menright
 */
#include "fptoken.h"
#include "fpnode.h"
#include "fputil.h"
#include "fpsuper.h"

#include <iostream>
#include <fstream>
#include <map>
#include <cstring>
#include <sstream>

using namespace std;

#if 0
#define LOG(x) \
	cout << __FUNCTION__ << ": " << x << ' ' << __FILE__ << ':' << __LINE__ << endl

#define LOGB(x) \
	cout << __PRETTY_FUNCTION__ << ": " << x
#define LOGM(x) \
	cout << x
#define LOGE(x) \
	cout << x << ' ' << __FILE__ << ':' << __LINE__ << endl
#elif 0
#define LOG(x) \
	cout << x << ' ' << __FILE__ << ':' << __LINE__ << endl

#define LOGB(x) \
	cout << x
#define LOGM(x) \
	cout << x
#define LOGE(x) \
	cout << x << ' ' << __FILE__ << ':' << __LINE__ << endl
#else

#define LOGB(x) \
	cout << x
#define LOGM(x) \
	cout << x
#define LOGE(x) \
	cout << x << " :" << __LINE__ << endl

#define LOG(x) \
	do { LOGB(""); LOGM(""); LOGE(x); } while (false)

#define NLOGB(x)
#define NLOGM(x)
#define NLOGE(x)
#define NLOG(x) \
		do { NLOGB(""); NLOGM(""); NLOGE(x); } while (false)
#endif

template <typename E1, typename UnaryOp, typename C2=std::list<std::string>>
C2 mapfl(const E1& b, const E1& e, UnaryOp f) {
	C2 res;
	unsigned index =0;
	//std::transform(b, e, std::back_inserter(res), f);
	for (auto p = b; p != e; ++p) {
		res.push_back(f(*p, index));
	}
	return res;
}

namespace assembly {
struct AddressMode {
	enum Mode { Arg, Label, Super, Num, Marker, List };
#if 0
	AddressMode(Mode mode, int address) : mode(mode), address(address) {}
	AddressMode(Mode mode, unsigned address) : mode(mode), address(address) {}
	AddressMode(Mode mode, size_t address) : mode(mode), address(address) {}
#endif
	AddressMode(Mode mode, ptrdiff_t address) : mode(mode), address(address) {}
	AddressMode(Mode mode, void* address) : mode(mode), address((ptrdiff_t)address) {}
	AddressMode(Mode mode, const string& dest) : mode(mode), address(0), dest(dest) {}
	AddressMode() : mode(List), address(0) {}
	string to_string() const {
		string as;
		switch (mode) {
		case Arg: as = "Arg"; break;
		case Label: as = "Label"; break;
		case Super: as = "Super"; break;
		case Num: as = "Num"; break;
		case Marker: as = "Marker"; break;
		case List: as = "List"; break;
		}
		switch (mode) {
		case Arg:
		case Label:
		case Super:
		case Marker:
			if (dest.size())
				as += " " + dest;
			else
				as += " "+std::to_string(address);
			break;
		case Num:
			as += " "+std::to_string(address);
			break;
		case List:
			as += " ...";
		}
		return as;
	}
	string push() {
		ostringstream output;
		output << "# Push " << to_string() << endl;
		switch (mode) {
		case Arg:
		default: throw std::to_string(__LINE__)+" push " + to_string();
		case Label:
#if 1
			output << " movabsq $"<<dest<<",%rsi" << endl;
			output << " movq %rsi, (%rax) #; push pc" << endl;
#else
#if 1
			if (isdigit(dest[0]))
				//output << " movabsq "<<dest<<",%rcx" << endl;
				output << " movq "<<dest<<"(%rip),%rcx" << endl;
			else
				//output << " movabsq $"<<dest<<",%rcx" << endl;
				output << " movq "<<dest<<"(%rip),%rcx" << endl;
#else
			if (isdigit(dest[0]))
				output << " mov "<<dest<<",%rcx" << endl;
			else
				output << " mov $"<<dest<<",%rcx" << endl;
#endif
			output << " movq %rcx, (%rax) #; push pc" << endl;
#endif
			output << " addq $8, %rax;" << endl;
			output << " mov %rbx, (%rax) #; push frame" << endl;
			output << " add $8, %rax;";
			return output.str();
		case Marker:
			output << " movq marker"<<address<<"(%rip),%rcx" << endl;
			output << " movq %rcx,(%rax) #;; rax==sp, push marker0" << endl;
			output << " addq $8,%rax         #;; sp++"<<endl;
			output << " mov %rbx,(%rax)    #;; push frame" << endl;
			output << " add $8,%rax         #;; sp++";// << endl;
			return output.str();
		case Num:
			output << "    movabsq $SELF, %rcx"<<endl;
			output << "    movq %rcx,(%rax)" << endl;
			output << "    add $8,%rax" << endl;
			output << "    movq $" << address << ", (%rax)" << endl;
			output << "    add $8,%rax";
			return output.str();
		case List:
			{	auto dest = save_pending(*this);
				return dest.push();
			}
			break;
		}
		return output.str();
	}
	ostream& push(ostream& output) {
		return output << push();
	}
	string enter() {
		ostringstream output;
		output << "# Enter " << to_string() << endl;
		switch (mode) {
		case Label:
			output << " jmp " << dest;
			break;
		case Super:
			output << "    jmp " << dest;
			break;
		case Arg:
			// copy frame ptr to rcx
			// load frame ptr rbx from rbx+8+arg*16
			// jmp (*rcx)
			output << "     mov %rbx,%rcx        #;; old frame ptr to rcx" << endl;
			output << "     mov "<<address*16+8<<"(%rcx),%rbx   #;; new frame pointer to frame ptr" << endl;
			output << "     jmp *"<<address*16<<"(%rcx) #;; enter arg"<<address;//<<"" << endl;
			break;
		case Num:
			output << "   mov $"<< address << ", %rbx ;; mov value to frame register " << endl;
			output << "   jmp SELF";
			break;
		default: throw std::to_string(__LINE__)+" enter " + to_string();
		}
		return output.str();
	}
	ostream& enter(ostream& output) {
		return output << enter();
    }
	static void code_pending(ostream& output) {
		while (pending.size()) {
			auto next = pending.front(); pending.pop_front();
			output << next.second << ":" << endl;
			output << *(string*)next.first.address;
		}
	}
	static AddressMode save_pending(const AddressMode& am) {
		auto dest = seq_label();
		AddressMode label(Label, dest);
		pending.push_back(make_pair(am,dest));
		return label;
	}
	static string seq_label() {
		return "LCL_"+std::to_string(next_label++);
	}
	static int next_label;
	static list<pair<AddressMode,string>> pending;
	Mode mode;
	ptrdiff_t address;
	string dest;
};
int AddressMode::next_label = 0;
list<pair<AddressMode,string>> AddressMode::pending;
struct Symbol {
	explicit Symbol(const AddressMode& addressMode, const string& name, int arguments=0)
	: addressMode(addressMode), name(name), arguments(arguments) {}
	Symbol() : addressMode(), arguments(0) {}
	AddressMode addressMode;
	string name;
	int arguments;
};
typedef map<string, Symbol> Environment;
void compileR(Node* expr, ostream& code, Environment& env);
struct CompileAVisitor : NodeVisitor {
	void visit(NodeDefine*) {}
	void visit(NodeAbstract*) {}
	void visit(NodeLet*){}
	void visit(NodeConstructor*){}
	void visit(NodeCase*){}
	void visit(NodeApply* expr) {
		ostringstream inner;
		compileR(expr, inner, env);
		mode = AddressMode(AddressMode::List, new string(inner.str()));
		NLOG("Address mode for Apply " << expr->to_string() << " is " << mode.to_string());
	}
	void visit(NodeId* expr) {
		auto p = env.find(expr->id);
		if (p != env.end()) {
			mode = p->second.addressMode;
		} else {
			throw "Can't find " + expr->id + " in compileA";
			mode = AddressMode(AddressMode::Super, expr->id);
		}
	}
	void visit(NodeNum* expr) {
		mode = AddressMode(AddressMode::Num, expr->value);
	}
	CompileAVisitor(ostream& code, Environment& env) : code(code), env(env), mode() {}
	ostream& code;
	Environment& env;
	AddressMode mode;
};
AddressMode compileA(Node* expr, ostream& code, Environment& env) {
	CompileAVisitor visitor(code, env);
	expr->acceptVisit(&visitor);
	return visitor.mode;
}

struct CompileRVisitor : NodeVisitor {
	void visit(NodeDefine*) {}
	void visit(NodeAbstract*) {}
	void visit(NodeLet*){}
	void visit(NodeConstructor*){}
	void visit(NodeCase*){}
	string addr_to_string(Environment& env, ptrdiff_t addr) {
		for (auto e : env) {
			//e.first;
			//e.second.addressMode.address;
			//e.second.name;
			if (e.second.addressMode.mode == AddressMode::Label ||
					e.second.addressMode.mode == AddressMode::Super) {
				return e.second.name;
			}
		}
		return "";
	}
	void visit(NodeApply* expr) {
		NLOG("Evaluation of Apply " << expr->to_string());
		for (unsigned n=expr->terms.size()-1; n>0; --n) {
			auto am = compileA(expr->terms[n].get(), code, env);
			NLOG("Apply argument term " << expr->terms[n]->to_string() << " using " << am.to_string());
			code << am.push() << endl;
		}
		NLOG("Apply fn term " << expr->terms[0]->to_string());
		compileR(expr->terms[0].get(), code, env);
	}
	void visit(NodeId* expr) {
		auto am = compileA(expr, code, env);
		code << am.enter() << endl;
	}
	void visit(NodeNum* expr) {
		code << AddressMode(AddressMode::Num, expr->value).enter() << endl;
	}
	CompileRVisitor(ostream& code, Environment& env) : code(code), env(env) {}
	ostream& code;
	Environment& env;
};
void compileR(Node* expr, ostream& code, Environment& env) {
	CompileRVisitor visitor(code, env);
	expr->acceptVisit(&visitor);
}
void compileR(const shared_ptr<Node>& expr, ostream& code, Environment& env) {
	compileR(expr.get(), code, env);
}
void compileSC(const CoreScDefn& def, ostream& code, const Environment& env) {
	NLOG("Generating take instr with " << def.args.size() << " slots ");
	if (def.args.size()) {
		code << "    mov $" << def.args.size() << ",%rcx" << endl;
		code << "    call take" << endl;
	}
	Environment new_env = env;
	int kArg = 0;
	for (auto arg: def.args) {
#if 1
		auto dest = "SI" + to_string(kArg);
		new_env[arg] = Symbol(AddressMode(AddressMode::Label, dest), dest,0);
#else
		new_env[arg] = Symbol(AddressMode(AddressMode::Arg, kArg),0);
#endif
		++kArg;
	}
	compileR(def.expr, code, new_env);
	AddressMode::code_pending(code);
}
string labelize(const string& name) {
	string out="L_";
	for (auto c : name) {
		if (isalnum(c))
		out.push_back(c);
		else {
			auto piece = to_string((int)c);
			out.append(piece);
		}
	}
	return out;
}
void addRtl(ostream& output) {
	ifstream rtl;
	rtl.open("rtl.s", ios::in);
	while (rtl.good()) {
		string ln;
		getline(rtl, ln);
		output << ln << endl;
	}
}
void addoper(ostream& output, const string& name) {
	output << labelize(name)<<": #" << name << endl;
	output << "    mov $2,%rcx" << endl;
	output << "    call take" << endl;
#if 1
	output << AddressMode(AddressMode::Label,"1f").push() << endl;
	output << AddressMode(AddressMode::Label,"SI1").enter() << endl;
	output << "1: "<<AddressMode(AddressMode::Label,"2f").push() << endl;
	output << AddressMode(AddressMode::Label,"SI0").enter() << endl;
#else
	output << "    movq 1f(%rip),%rcx" << endl;
	output << "    movq %rcx, (%rax) #; push pc" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    mov %rbx, (%rax) #; push frame" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    jmp SI1" << endl;
	output << "1:  movq 2f(%rip), %rcx" << endl;
	output << "    movq %rcx, (%rax) #; push pc" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    mov %rbx, (%rax) #; push frame" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    jmp SI0" << endl;
#endif
	output << "2: #;; [rax+8] is left [rax+24] is right" << endl;
	if (name == "+") {
		output << "    mov 8(%rax),%rcx" << endl;
		output << "    add 24(%rax),%rcx" << endl;
	} else if (name == "-") {
		output << "    mov 8(%rax),%rcx" << endl;
		output << "    sub 24(%rax),%rcx" << endl;
	} else if (name == "<") {
		output << "    mov 8(%rax),%rcx" << endl;
		output << "    cmp 24(%rax),%rcx" << endl;
		output << "    mov $0,%rcx" << endl;
		output << "    jnz 1f" << endl;
		output << "    add $1,%rcx" << endl;
		output << "1:" << endl;
	} else {
		output << "###;;; code needed for operator " << name << endl;
	}
	output << "    mov %rcx, %rbx;  #;; put value in frame pointer" << endl;
	output << "    jmp SELF        #;; and do the SELF thing" << endl;
}

void addcond(ostream& output) {
	output << "cond: ## TK" << endl;
	output << "if: ## TK" << endl;
	output << "    mov $3,%rcx" << endl;
	output << "    call take" << endl;
	output << AddressMode(AddressMode::Label,"1f").push() << endl;
	output << AddressMode(AddressMode::Label, "SI0").enter() << endl;
	output << "1:" << endl;
	output << " ## This needs to pop the result!" << endl;
	output << "	xor %rcx,%rcx" << endl;
	output << "   test %rcx,8(%rax) #get truth" << endl;
	output << "   jz 2f #go do false action" << endl;
	output << " # enter 2nd arg if true" << endl;
	output << " # thus mov 8(rbx) to rbx jmp 0(rbx)" << endl;
	output << AddressMode(AddressMode::Arg, 1).enter() << endl;
	output << "2:" << endl;
	output << AddressMode(AddressMode::Arg, 2).enter() << endl;
}
template <class T>
unsigned maxArgs(const T& defs) {
	unsigned m = 3;
	for (auto def: defs) {
		if (def.args.size() > m) {
			m = def.args.size();
		}
	}
	return m;
}
}
int main(int argc, char** argv) {
	if (argc != 3)
	{
		cout << "requires an input file and output file" <<endl;
		return 1;
	}
	ifstream input;
	input.open(argv[1], ios::in);
	if (!input.good())
	{
		cout << "Failed to open " << argv[1] << endl;
		return 1;
	}
	ofstream output_file;
	auto which_output = [&]()->ostream& {
		if (strcmp(argv[2],"--")!=0) {
			output_file.open(argv[2], ios::out);
			if (!output_file.good())
			{
				cout << "Failed to open " << argv[2] << endl;
			}
			return output_file;
		}
		return std::cout;
	};
	ostream& output = which_output();
	if (!output.good()) {
		return 1;
	}
	Token read(input);
	read.next();

	//vector<Instruction> code;
	using namespace assembly;
	//State state;
	try {
		auto defcode = Definitions(read);
		auto max_args = maxArgs(defcode);
		Environment env;
		addRtl(output);
		for (unsigned i=0; i<max_args; ++i) {
			output << "marker" << i<<": ret" << endl;
		}
		for (unsigned i=0; i<max_args; ++i) {
#if 1
			output << "SI"<<i<<": ";
#if 0
			output << AddressMode(AddressMode::Marker,i).push();
#endif
			output << endl;
			output << " " << AddressMode(AddressMode::Arg, i).enter() << endl;
#else
			output << "SI"<<i<<": " << AddressMode(AddressMode::Marker,i).push() << endl;
			output << "SI"<<i<<": movq marker"<<i<<"(%rip),%rcx" << endl;
			output << "     movq %rcx,(%rax) #;; rax==sp, push marker0" << endl;
			output << "     addq $8,%rax         #;; sp++"<<endl;
			output << "     mov %rbx,(%rax)    #;; push frame" << endl;
			output << "     add $8,%rax         #;; sp++" << endl;
			output << "     mov %rbx,%rcx        #;; old frame ptr to rcx" << endl;
			output << "     mov "<<i*16+8<<"(%rcx),%rbx   #;; new frame pointer to frame ptr" << endl;
			output << "     jmp *"<<i*16<<"(%rcx) #;; enter arg"<<i<<"" << endl;
#endif
		}
		addoper(output, "+"); env["+"] = Symbol(AddressMode(AddressMode::Super, labelize("+")), "+", 2);
		addoper(output, "-"); env["-"] = Symbol(AddressMode(AddressMode::Super, labelize("-")), labelize("-"), 2);
		addoper(output, "*"); env["*"] = Symbol(AddressMode(AddressMode::Super, labelize("*")), "*", 2);
		addoper(output, "/"); env["/"] = Symbol(AddressMode(AddressMode::Super, labelize("/")), "/", 2);
		addoper(output, "<"); env["<"] = Symbol(AddressMode(AddressMode::Super, labelize("<")), "<", 2);
		addcond(output); env["if"] = Symbol(AddressMode(AddressMode::Super, "if"),"if", 2);

		for (auto def : defcode) {
			NLOG("def->to_string: " << def.to_string());
			env[def.name] = Symbol(AddressMode(AddressMode::Super, def.name), def.name, def.args.size());
			output << def.name<<":" << endl;
			compileSC(def,output, env);
		}
		output << ".global start" << endl;
		output << "start:" << endl;
		output << "     lea frame(%rip), %rbx;" << endl;
		output << "     mov %rbx, fptr(%rip)" << endl;
		output << "     lea stack(%rip), %rax;" << endl;
		output << "     jmp main" << endl;

	} catch (const char* ex) {
		cout << "Exception " << ex << endl;
	} catch (const string& ex) {
		cout << "Exception " << ex << endl;
	}
	return 0;
}
