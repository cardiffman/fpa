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

namespace Simple {
struct AddressMode {
	enum Mode { Arg, Label, Super, Num, Marker, List };
	AddressMode(Mode mode, int address) : mode(mode), address(address) {}
	AddressMode(Mode mode, unsigned address) : mode(mode), address(address) {}
	AddressMode(Mode mode, size_t address) : mode(mode), address(address) {}
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
	Mode mode;
	ptrdiff_t address;
	string dest;
};
struct Symbol {
	explicit Symbol(const AddressMode& addressMode, const string& name, int arguments=0)
	: addressMode(addressMode), name(name), arguments(arguments) {}
	Symbol() : addressMode(), arguments(0) {}
	AddressMode addressMode;
	string name;
	int arguments;
};
typedef map<string, Symbol> Environment;
#if 0
enum {
	SELF = -1,
	MARKER = -2
};
struct Instruction {
	enum Ins { Halt, Take, Push, Enter, Op, EnterT };
	enum Operation { Add, Sub, Mul, Div, Lt };
	Ins ins;
	union Params {
		Params() {}
		AddressMode mode;
		unsigned taken;
		unsigned tag;
		Operation op;
		struct {
			unsigned first;
			unsigned second;
		} constructor;
	} params;
	Instruction(Ins ins, const AddressMode& mode) : ins(ins) {
		params.mode = mode;
	}
	Instruction(Ins ins, size_t taken) : ins(ins) {
		params.taken = taken;
	}
	Instruction(Ins ins=Halt) : ins(ins) {}
	Instruction(Operation op) : ins(Op) {
		params.op = op;
	}
	string to_string() const {
		string instr;
		switch (ins) {
		case Take: instr= "Take"; break;
		case Push: instr= "Push"; break;
		case Enter: instr= "Enter"; break;
		case EnterT: instr= "EnterT"; break;
		case Halt: instr= "Halt"; break;
		case Op: instr= "Op"; break;
		default: instr= "isxx"; break;
		}
		switch (ins) {
		case Push:
		case Enter:
		case EnterT:
			instr += " " + params.mode.to_string();
			break;
		case Take:
			instr += " " + std::to_string(params.taken);
			break;
		case Op:
			instr += " ";
			switch (params.op) {
			case Add: instr += "Add"; break;
			case Mul: instr += "Mul"; break;
			case Sub: instr += "Sub"; break;
			case Div: instr += "Div"; break;
			case Lt: instr += "Lt"; break;
			}
			break;
		default:
			break;
		}
		return instr;
	}
};
#endif
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
			mode = AddressMode(AddressMode::Super, 9999);
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
			e.first;
			e.second.addressMode.address;
			e.second.name;
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
			switch (am.mode) {
			case AddressMode::Num:
				code << "    movq $SELF, (%rax)" << endl;
				code << "    add $8,%rax" << endl;
				code << "    movq $" << am.address << ", (%rax)" << endl;
				code << "    add $8,%rax" << endl;
				break;
			case AddressMode::Label:
				code << "    movq $" << addr_to_string(env,am.address) << ", (%rax)" << endl;
				code << "    add $8,%rax" << endl;
				code << "    movq %rbx, (%rax)" << endl;
				code << "    add $8,%rax" << endl;
				break;
			case AddressMode::List:
				code << "    movq $" << addr_to_string(env,am.address) << ", (%rax) #push list" << endl;
				code << "    add $8,%rax" << endl;
				code << "    movq %rbx, (%rax)" << endl;
				code << "    add $8,%rax" << endl;
				//throw to_string(__LINE__)+ " push List" + *(string*)am.address;
				break;
			default:
				throw "push "+am.to_string()+' '+addr_to_string(env,am.address); //code.push_back(Instruction(Instruction::Push, am));
			}
		}
		NLOG("Apply fn term " << expr->terms[0]->to_string());
		compileR(expr->terms[0].get(), code, env);
	}
	void visit(NodeId* expr) {
		auto am = compileA(expr, code, env);
		switch (am.mode) {
		case AddressMode::Super:
			code << "    jmp " << am.dest << endl;
			break;
		default:
			if (am.mode == AddressMode::List) {
				throw "enter " + am.to_string();//append(code, *(list<Instruction>*)am.address);
			} else {
				throw "enter " + am.to_string();//code.push_back(Instruction(Instruction::Enter, am));
			}
		}
	}
	void visit(NodeNum* expr) {
		// what pushing a number looks like
		//code << "    movq $SELF, [$rax]" << endl;
		//code << "    add $8,%rax" << endl;
		//code << "    movq $" << expr->value << ", [$rax]" << endl;
		//code << "    add $8,%rax" << endl;
		code << "   movq $"<< expr->value << ", %rbx ;; mov value to frame register " << endl;
		code << "   jmp SELF" << endl;
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
void compileSC(const CoreScDefn& def, ostream& code, Environment& env) {
	NLOG("Generating take instr with " << def.args.size() << " slots ");
	code << "    mov $" << def.args.size() << ",%rcx" << endl;
	code << "    call take" << endl;
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
}
#if 0
void State::step() {
	if (stopped())
		return;
	const Instruction& ins = code[pc++];
	switch (ins.ins) {
	case Instruction::Take:
		if (ins.params.taken != 0) {
			fptr.frame = new Frame();
			fptr.frame->elements.resize(ins.params.taken);
			for (unsigned i=0; i<ins.params.taken; ++i) {
				fptr.frame->elements[i] = closureStack.back();
				closureStack.pop_back();
			}
		}
		break;
	case Instruction::Push:
		switch (ins.params.mode.mode) {
		case AddressMode::Label:
			closureStack.push_back(new Closure());
			closureStack.back()->address = ins.params.mode.address;
			closureStack.back()->fptr = fptr;
			break;
		case AddressMode::Super:
			closureStack.push_back(new Closure());
			closureStack.back()->address = ins.params.mode.address;
			closureStack.back()->fptr.frame = 0;
			break;
		case AddressMode::Num:
			closureStack.push_back(new Closure());
			closureStack.back()->address = -1;
			closureStack.back()->fptr.value = ins.params.mode.address;
			break;
		case AddressMode::Arg:
			closureStack.push_back(fptr.frame->elements[ins.params.mode.address]);
			break;
		case AddressMode::List:
			throw "List address mode shouldn't be executed";
		case AddressMode::Marker:
			//LOG("Not handling marker yet");
			closureStack.push_back(new Closure());
			closureStack.back()->address = MARKER - ins.params.mode.address;
			closureStack.back()->fptr = fptr;//.value = ins.params.mode.address;
			break;
		}
		break;
	case Instruction::Enter:
		switch (ins.params.mode.mode) {
		case AddressMode::Label:
			pc = ins.params.mode.address;
			break;
		case AddressMode::Super:
			pc = ins.params.mode.address;
			break;
		case AddressMode::Arg:
			pc = fptr.frame->elements[ins.params.mode.address]->address;
			fptr = fptr.frame->elements[ins.params.mode.address]->fptr;
			//if (pc < 0) {
			//	LOG("PC is negative: " << pc << ',' << fptr.value);
			//}
			break;
		case AddressMode::Num:
			pc = -1;
			fptr.value = ins.params.mode.address;
			break;
		case AddressMode::List:
			throw "List address mode shouldn't be executed";
		}
		break;
	case Instruction::EnterT: {
		auto tos = closureStack.back(); closureStack.pop_back();
		if (tos->address == SELF && tos->fptr.value != 0) {
			switch (ins.params.mode.mode) {
			case AddressMode::Label:
				pc = ins.params.mode.address;
				break;
			case AddressMode::Super:
				pc = ins.params.mode.address;
				break;
			case AddressMode::Arg:
				pc = fptr.frame->elements[ins.params.mode.address]->address;
				fptr = fptr.frame->elements[ins.params.mode.address]->fptr;
				//if (pc < 0) {
				//	LOG("PC is negative: " << pc << ',' << fptr.value);
				//}
				break;
			case AddressMode::Num:
				pc = -1;
				fptr.value = ins.params.mode.address;
				break;
			case AddressMode::List:
				throw "List address mode shouldn't be executed";
			}
			break;
		}
		}
		break;
	case Instruction::Op: {
		auto left = closureStack.back(); closureStack.pop_back();
		auto right = closureStack.back(); closureStack.pop_back();
		pc = -1;
		switch (ins.params.op) {
		case Instruction::Add:
		fptr.value = left->fptr.value + right->fptr.value;
		break;
		case Instruction::Mul:
		fptr.value = left->fptr.value * right->fptr.value;
		break;
		case Instruction::Sub:
		fptr.value = left->fptr.value - right->fptr.value;
		break;
		case Instruction::Div:
		fptr.value = left->fptr.value / right->fptr.value;
		break;
		case Instruction::Lt:
		fptr.value = left->fptr.value < right->fptr.value;
		break;
		}
		break;
	}
	}
	while (pc < 0) {
		//LOG("PC is negative: " << pc << ',' << fptr.value);
		//LOG("TOS " << closureStack.back()->address << "|" << closureStack.back()->fptr.value);
		while (closureStack.back()->address <= MARKER) {
			auto oldClosure = closureStack.back();
			closureStack.pop_back();
			int i = MARKER - oldClosure->address;
			//LOG("Updating marked frame element " << i << " based on address " << oldClosure->address);
			//LOG("The frame element will become " << pc << "," << fptr.frame);
			oldClosure->fptr.frame->elements[i]->address = pc;
			oldClosure->fptr.frame->elements[i]->fptr = fptr;
			//throw "About to enter SELF with marker on stack";
			//LOG(" pc is " << pc);
		}
		//LOG("PC is negative: " << pc << ',' << fptr.value);
		if (pc == -1){//else {
			//LOG("Traditional handling of SELF: " << pc << ',' << fptr.value);
			auto oldClosure = closureStack.back();
			closureStack.pop_back();
			auto newClosure = new Closure();
			newClosure->address = pc;
			newClosure->fptr = fptr;
			closureStack.push_back(newClosure);
			pc = oldClosure->address;
			fptr = oldClosure->fptr;
		}
	}
}
#endif
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
	output << "# Assembly code implemnting TIM take/push/enter" << endl;
	output << "# rax is the stack pointer, pushing to stack increases rax" << endl;
	output << "# rbx is the frame pointer, Arg 0 will be at rbx, Arg 1 at 8(rbx)" << endl;
	output << "# Any 'non-linear' flow is done explicitly with enter, translated to some jmp" << endl;
	output << "# Thus any need for control to 'return' needs to be done explicitly by pushing" << endl;
	output << "# the desired destination using equivalent of push label, and best push closure" << endl;
	output << "take:" << endl;
	output << "	mov aptr(%rip), %rbx" << endl;
	output << "	mov %rbx,cframe(%rip)" << endl;
	output << "	lea (%rbx,%rcx,8), %rbx" << endl;
	output << "	mov %rbx,aptr(%rip)" << endl;

	output << "	mov tos(%rip), %rbx" << endl;
	output << "	rep movsd" << endl;
	output << "	ret" << endl;
	output << "	" << endl;
	output << "SELF:" << endl;
	output << "	# We come here upon entering certain closures." << endl;
	output << "	# The intention is to end up with the entered closure transfered to the stack" << endl;
	output << "	# and the closure that was on top gets popped and enterd." << endl;
	output << "	# The intention is to enter the closure on top of the stack, but replace it" << endl;
	output << "	# with the SELF PC and value." << endl;
	output << "	# One implementation had this code:" << endl;
		/*
			struct Closure* top = head(stack);
			stack = tail(stack);
			struct Closure* n = NEW(struct Closure);
			n->pc = state.pc;
			n->value = state.value;
			stack = cons(n, stack);
			state = *top;
		*/
	output << "	ret" << endl;
}
void addoper(ostream& output, const string& name) {
	output << labelize(name)<<": #" << name << endl;
	output << "    mov $2,%rcx" << endl;
	output << "    call take" << endl;
	output << "    movq $1f, (%rax) #; push pc" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    mov %rbx, (%rax) #; push frame" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    jmp SI1" << endl;
	output << "1:  movq $2f, (%rax) #; push pc" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    mov %rbx, (%rax) #; push frame" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    jmp SI0" << endl;
	output << "2: #;; [rax+8] is left [rax+24] is right" << endl;
	if (name == "+") {
		output << "    mov 8(%rax),%rcx" << endl;
		output << "    add 24(%rax),%rcx" << endl;
	} else {
		output << "###;;; code needed for operator " << name << endl;
	}
	output << "    mov %rcx, %rbx;  #;; put value in frame pointer" << endl;
	output << "    jmp SELF        #;; and do the SELF thing" << endl;
}

void addcond(ostream& output) {
#if 0
	auto rel = state.code.size();
	append(state.code, vector<Instruction>{
		Instruction(Instruction::Take,3),
		Instruction(Instruction::Push,AddressMode(AddressMode::Label,rel+3)),
		Instruction(Instruction::Enter,AddressMode(AddressMode::Label, 0)),
		Instruction(Instruction::EnterT,AddressMode(AddressMode::Label, 2)),
		Instruction(Instruction::Enter,AddressMode(AddressMode::Label, 4)),
	});
	env["cond"] = Symbol(AddressMode(AddressMode::Super,rel),3);
#endif
	output << labelize("if")<<": #" << "if" << endl;
	output << "    mov $2,%rcx" << endl;
	output << "    call take" << endl;
	output << "    movq $1f, (%rax) #; push pc" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    mov %rbx, (%rax) #; push frame" << endl;
	output << "    add $8, %rax;" << endl;
	output << "    jmp SI0" << endl;
	output << "    test 8(%rax) #get truth" << endl;
	output << "    jmpz $1f #go do false action" << endl;
	output << " # enter 2nd arg if true" << endl;
	output << " # thus mov 8(rbx) to "
	output << "    mov "
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
	using namespace Simple;
	//State state;
	try {
		auto defcode = Definitions(read);
		Environment env;
		addRtl(output);
		for (int i=0; i<3; ++i) {
			output << "marker" << i<<":" << endl;
		}
		for (int i=0; i<3; ++i) {
			output << "SI"<<i<<": movq $marker"<<i<<",(%rax) #;; rax==sp, push marker0" << endl;
			output << "     add $8,%rax         #;; sp++"<<endl;
			output << "     mov %rbx,(%rax)    #;; push frame" << endl;
			output << "     add $8,%rax         #;; sp++" << endl;
			output << "     mov %rbx,%rcx        #;; old frame ptr to rcx" << endl;
			output << "     mov "<<i*16+8<<"(%rcx),%rbx   #;; new frame pointer to frame ptr" << endl;
			output << "     jmp *"<<i*16<<"(%rcx) #;; enter arg"<<i<<"" << endl;
		}
		addoper(output, "+"); env["+"] = Symbol(AddressMode(AddressMode::Super, labelize("+")),labelize("+"));
		addoper(output, "-"); env["-"] = Symbol(AddressMode(AddressMode::Super, labelize("-")),labelize("-"));
		addoper(output, "*"); env["*"] = Symbol(AddressMode(AddressMode::Super, labelize("*")),labelize("*"));
		addoper(output, "/"); env["/"] = Symbol(AddressMode(AddressMode::Super, labelize("/")),labelize("/"));
		addoper(output, "<"); env["<"] = Symbol(AddressMode(AddressMode::Super, labelize("<")),labelize("<"));
		addcond(output);      env["if"]= Symbol(AddressMode(AddressMode::Super, labelize("if")),labelize("if"));

		for (auto def : defcode) {
#if 1
			NLOG("def->to_string: " << def.to_string());
			env[def.name] = Symbol(AddressMode(AddressMode::Super, def.name), def.name, def.args.size());
			output << def.name<<":" << endl;
			compileSC(def,output, env);
#if 0
			auto start = state.code.size();
			env[def.name] = Symbol(AddressMode(AddressMode::Super, start), def.args.size());
			auto instrs = compileSC(def, env);
			NLOG(join(mapf(instrs.begin(), instrs.end(), [](auto const& i){return i.to_string();}),'\n'));
			start = state.code.size();
			env[def.name] = Symbol(AddressMode(AddressMode::Super, start), def.args.size());
			state.code.insert(state.code.end(), instrs.begin(), instrs.end());
			for (unsigned i = 0; i<state.code.size(); ++i) {
				switch (state.code[i].ins) {
				case Instruction::Push:
				case Instruction::Enter:
					if (state.code[i].params.mode.mode == AddressMode::List) {
						unsigned target = state.code.size();
						const list<Instruction>& to_add = *(list<Instruction>*)(state.code[i].params.mode.address);
						state.code.insert(state.code.end(), to_add.begin(), to_add.end());
						state.code[i].params.mode = AddressMode(AddressMode::Label, target);
					}
					break;
				default:
					break;
				}
			}
#endif
		}
#if 0
		auto start = state.code.size();
		auto theMain = env["main"].addressMode;
		state.code.push_back(Instruction(Instruction::Push,AddressMode(AddressMode::Label, start+2)));
		state.code.push_back(Instruction(Instruction::Enter,theMain));
		state.code.push_back(Instruction(Instruction::Halt));
#endif
		output << "start:" << endl;
		output << "     mov $frame, %rbx;" << endl;
		output << "     mov $stack, %rax;" << endl;
		output << "     jmp main" << endl;

	} catch (const char* ex) {
		cout << "Exception " << ex << endl;
	} catch (const string& ex) {
		cout << "Exception " << ex << endl;
	}
	return 0;
}
#endif


