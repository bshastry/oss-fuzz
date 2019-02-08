#include "proto_to_ruby.h"
#include "ruby.pb.h"

#include <ostream>
#include <sstream>
#include <fstream>

namespace ruby_fuzzer {

	// Forward decls.
	std::ostream &operator<<(std::ostream &os, const BinaryOp &x);
	std::ostream &operator<<(std::ostream &os, const StatementSeq &x);

	// Proto to Ruby.
	std::ostream &operator<<(std::ostream &os, const Const &x) {
		if (x.has_int_lit()) return os << "(" << x.int_lit() << ")";
		if (x.has_str_lit()) return os << "(\"" << x.str_lit() << "\")";
		if (x.has_data_struct()) {
			switch (x.data_struct()) {
				case Const::ARRAY:
					os << "[1, 2, 3, 4, 5]";
					break;
				case Const::HASH:
					os << "{\"name\" => \"Leandro\",  \"nickname\" => \"Tk\","
		                << "\"nationality\" => \"Brazilian\", \"age\" => 24}";
			}
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const VarRef &x) {
		return os << "var_" << (static_cast<uint32_t>(x.varnum()) % 10);
	}
	std::ostream &operator<<(std::ostream &os, const Lvalue &x) {
		return os << x.varref();
	}
	std::ostream &operator<<(std::ostream &os, const Rvalue &x) {
		if (x.has_varref()) return os << x.varref();
		if (x.has_cons())   return os << x.cons();
		if (x.has_binop())  return os << x.binop();
		return os << "1";
	}
	std::ostream &operator<<(std::ostream &os, const BinaryOp &x) {
		os << "(" << x.left();
		switch (x.op()) {
			case BinaryOp::ADD: os << "+"; break;
			case BinaryOp::SUB: os << "-"; break;
			case BinaryOp::MUL: os << "*"; break;
			case BinaryOp::DIV: os << "/"; break;
			case BinaryOp::MOD: os << "%"; break;
			case BinaryOp::XOR: os << "^"; break;
			case BinaryOp::AND: os << "and"; break;
			case BinaryOp::OR: os << "or"; break;
			case BinaryOp::EQ: os << "=="; break;
			case BinaryOp::NE: os << "!="; break;
			case BinaryOp::LE: os << "<="; break;
			case BinaryOp::GE: os << ">="; break;
			case BinaryOp::LT: os << "<"; break;
			case BinaryOp::GT: os << ">"; break;
			case BinaryOp::LS: os << "<<"; break;
			case BinaryOp::RS: os << ">>"; break;
		}
		return os << x.right() << ")";
	}
	std::ostream &operator<<(std::ostream &os, const AssignmentStatement &x) {
		return os << x.lvalue() << " = " << x.rvalue() << "\n";
	}
	std::ostream &operator<<(std::ostream &os, const IfElse &x) {
		return os << "if " << x.cond() << "\n"
		          << x.if_body() << "\nelse\n"
		          << x.else_body() << "\nend\n";
	}
	std::ostream &operator<<(std::ostream &os, const While &x) {
		return os << "while " << x.cond() << "\n" << x.body() << "\nend\n";
	}
	std::ostream &operator<<(std::ostream &os, const Ternary &x) {
		return os << "(" << x.tern_cond() << " ? "
					<< x.t_branch() << " : " << x.f_branch() << ")\n";
	}
	std::ostream &operator<<(std::ostream &os, const BuiltinFuncs &x) {
		switch (x.bifunc()) {
			case BuiltinFuncs::PRINT:
				os << "print " << x.arg() << "\n";
				break;
			case BuiltinFuncs::PUTS:
				os << "puts " << x.arg() << "\n";
				break;
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const Statement &x) {
		if (x.has_assignment()) return os << x.assignment();
		if (x.has_ifelse())     return os << x.ifelse();
		if (x.has_while_loop()) return os << x.while_loop();
		if (x.has_ternary_stmt()) return os << x.ternary_stmt();
		if (x.has_builtins()) return os << x.builtins();
		return os << "\n";
	}
	std::ostream &operator<<(std::ostream &os, const StatementSeq &x) {
		for (auto &st : x.statements()) os << st;
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const Function &x) {
		return os << "def foo()\n" << x.statements() << "end\n";
	}

// ---------------------------------

	std::string FunctionToString(const Function &input) {
		std::ostringstream os;
		os << input;
		return os.str();

	}
//	std::string ProtoToRb(const uint8_t *data, size_t size) {
//		Function message;
//		if (!message.ParsePartialFromArray(data, size))
//			return "#error invalid proto\n";
//		std::string outrb = FunctionToString(message);
//
//		return outrb;
//	}

} // namespace ruby_fuzzer