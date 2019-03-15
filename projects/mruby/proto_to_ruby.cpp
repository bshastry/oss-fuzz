#include "proto_to_ruby.h"
#include "ruby.pb.h"

#include <ostream>
#include <sstream>
#include <fstream>

namespace ruby_fuzzer {

	// Forward decls.
	std::ostream &operator<<(std::ostream &os, const BinaryOp &x);
	std::ostream &operator<<(std::ostream &os, const StatementSeq &x);
	std::ostream &operator<<(std::ostream &os, const ArrType &x);
	std::ostream &operator<<(std::ostream &os, const HashType &x);
	std::ostream &operator<<(std::ostream &os, const Array &x);
	std::ostream &operator<<(std::ostream &os, const Rvalue &x);

	const std::string removeSpecial(const std::string &x) {
		std::string tmp = std::string(x);
		if (!tmp.empty())
			tmp.erase(std::remove_if(tmp.begin(), tmp.end(),
                     [](char c) { return !(std::isalpha(c) || std::isdigit(c)); } ), tmp.end());
		return tmp;
	}

	// Proto to Ruby.
	std::ostream &operator<<(std::ostream &os, const MathConst &x) {
		switch (x.math_const()) {
			case MathConst::PI:
				os << "Math::PI";
				break;
			case MathConst::E:
				os << "Math::E";
				break;
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const MathType &x) {
		switch (x.math_arg_oneof_case()) {
			case MathType::kMathRval:
				os << x.math_rval();
				break;
			case MathType::kMathConst:
				os << x.math_const();
				break;
			case MathType::MATH_ARG_ONEOF_NOT_SET:
				os << "1";
				break;
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const MathOps &x) {
		switch (x.math_op()) {
			case MathOps::CBRT:
				os << "Math.cbrt(" << x.math_arg() << ")";
				break;
			case MathOps::COS:
				os << "Math.cos(" << x.math_arg() << ")";
				break;

			case MathOps::ERF:
				os << "Math.erf(" << x.math_arg() << ")";
				break;

			case MathOps::ERFC:
				os << "Math.erfc(" << x.math_arg() << ")";
				break;

			case MathOps::LOG:
				os << "Math.log(" << x.math_arg() << ")";
				break;

			case MathOps::LOG10:
				os << "Math.log10(" << x.math_arg() << ")";
				break;

			case MathOps::LOG2:
				os << "Math.log2(" << x.math_arg() << ")";
				break;

			case MathOps::SIN:
				os << "Math.sin(" << x.math_arg() << ")";
				break;

			case MathOps::SQRT:
				os << "Math.sqrt(" << x.math_arg() << ")";
				break;

			case MathOps::TAN:
				os << "Math.tan(" << x.math_arg() << ")";
				break;
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const StringExtNoArg &x) {
		os << "\"" << removeSpecial(x.str_arg()) << "\"";
		switch (x.str_op()) {
			case StringExtNoArg::DUMP:
				os << ".dump";
				break;
			case StringExtNoArg::STRIP:
				os << ".strip";
				break;
			case StringExtNoArg::LSTRIP:
				os << ".lstrip";
				break;
			case StringExtNoArg::RSTRIP:
				os << ".rstrip";
				break;
			case StringExtNoArg::STRIPE:
				os << ".strip!";
				break;
			case StringExtNoArg::LSTRIPE:
				os << ".lstrip!";
				break;
			case StringExtNoArg::RSTRIPE:
				os << ".rstrip!";
				break;
			case StringExtNoArg::SWAPCASE:
				os << ".swapcase";
				break;
			case StringExtNoArg::SWAPCASEE:
				os << ".swapcase!";
				break;
			case StringExtNoArg::SQUEEZE:
				os << ".squeeze";
				break;
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const Const &x) {
		switch (x.const_oneof_case()) {
			case Const::kIntLit:
				os << "(" << (x.int_lit() % 13) << ")";
				break;
			case Const::kBoolVal:
				os << "(" << x.bool_val() << ")";
				break;
			case Const::CONST_ONEOF_NOT_SET:
				os << "1";
				break;
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const VarRef &x) {
		return os << "var_" << (static_cast<uint32_t>(x.varnum()) % 10);
	}
	std::ostream &operator<<(std::ostream &os, const Rvalue &x) {

		switch (x.rvalue_oneof_case()) {
			case Rvalue::kVarref:
				os << x.varref();
				break;
			case Rvalue::kCons:
				os << x.cons();
				break;
			case Rvalue::kBinop:
				os << x.binop();
				break;
			case Rvalue::RVALUE_ONEOF_NOT_SET:
				os << "1";
				break;
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const BinaryOp &x) {
		os << "(" << x.left();
		switch (x.op()) {
			case BinaryOp::ADD: os << " + "; break;
			case BinaryOp::SUB: os << " - "; break;
			case BinaryOp::MUL: os << " * "; break;
			case BinaryOp::DIV: os << " / "; break;
			case BinaryOp::MOD: os << " % "; break;
			case BinaryOp::XOR: os << " ^ "; break;
			case BinaryOp::AND: os << " and "; break;
			case BinaryOp::OR: os << " or "; break;
			case BinaryOp::EQ: os << " == "; break;
			case BinaryOp::NE: os << " != "; break;
			case BinaryOp::LE: os << " <= "; break;
			case BinaryOp::GE: os << " >= "; break;
			case BinaryOp::LT: os << " < "; break;
			case BinaryOp::GT: os << " > "; break;
			case BinaryOp::RS: os << " >> "; break;
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
	std::ostream &operator<<(std::ostream &os, const Ternary &x) {
		return os << "(" << x.tern_cond() << " ? "
					<< x.t_branch() << " : " << x.f_branch() << ")\n";
	}
	std::ostream &operator<<(std::ostream &os, const ObjectSpace &x) {
		switch (x.os_func()) {
			case ObjectSpace::COUNT:
				os << "ObjectSpace.count_objects";
				break;
		}
		return os << "(" << x.os_arg() << ")" << "\n";
	}
	std::ostream &operator<<(std::ostream &os, const Time &x) {
		switch (x.t_func()) {
			case Time::AT:
				os << "Time.at";
				break;
			case Time::GM:
				os << "Time.gm";
				break;
		}
		return os << "(" << (x.t_arg()% 13) << ")" << "\n";
	}
	std::ostream &operator<<(std::ostream &os, const ArrType &x) {
		if (x.elements_size() > 0) {
			int i = x.elements_size();
			os << "[";
			for (auto &e : x.elements()) {
				i--;
				if (i == 0) {
					os << e;
				} else {
					os << e << ", ";
				}
			}
			os << "]";
		} else {
			os << "[1]";
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const KVPair &x) {
		os << "\"" << removeSpecial(x.key()) << "\"";
		os << " => ";
		os << "\"" << removeSpecial(x.val()) << "\"";
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const HashType &x) {
		if (x.keyval_size() > 0) {
			int i = x.keyval_size();
			os << "{";
			for (auto &e : x.keyval()) {
				i--;
				if (i == 0) {
					os << e;
				}
				else {
					os << e << ", ";
				}
			}
			os << "}";
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const Array &x) {
		switch (x.arr_func()) {
			case Array::FLATTEN:
				os <<  x.arr_arg() << ".flatten";
				break;
			case Array::COMPACT:
				os <<  x.arr_arg() << ".compact";
				break;
			case Array::FETCH:
				os <<  x.arr_arg() << ".fetch";
				break;
			case Array::FILL:
				os <<  x.arr_arg() << ".fill";
				break;
			case Array::ROTATE:
				os <<  x.arr_arg() << ".rotate";
				break;
			case Array::ROTATE_E:
				os <<  x.arr_arg() << ".rotate!";
				break;
			case Array::DELETEIF:
				os <<  x.arr_arg() << ".delete_if";
				break;
			case Array::INSERT:
				os <<  x.arr_arg() << ".insert";
				break;
			case Array::BSEARCH:
				os <<  x.arr_arg() << ".bsearch";
				break;
			case Array::KEEPIF:
				os <<  x.arr_arg() << ".keep_if";
				break;
			case Array::SELECT:
				os <<  x.arr_arg() << ".select";
				break;
			case Array::VALUES_AT:
				os <<  x.arr_arg() << ".values_at";
				break;
			case Array::BLOCK:
				os <<  x.arr_arg() << ".index";
				break;
			case Array::DIG:
				os <<  x.arr_arg() << ".dig";
				break;
			case Array::SLICE:
				os <<  x.arr_arg() << ".slice";
				break;
			case Array::PERM:
				os <<  x.arr_arg() << ".permutation";
				break;
			case Array::COMB:
				os <<  x.arr_arg() << ".combination";
				break;
			case Array::ASSOC:
				os <<  x.arr_arg() << ".assoc";
				break;
			case Array::RASSOC:
				os <<  x.arr_arg() << ".rassoc";
				break;
		}
		return os << "(" << x.val_arg() << ")";
	}
	std::ostream &operator<<(std::ostream &os, const BuiltinFuncs &x) {
		switch (x.bifunc_oneof_case()) {
			case BuiltinFuncs::kOs:
				os << x.os();
				break;
			case BuiltinFuncs::kTime:
				os << x.time();
				break;
			case BuiltinFuncs::kArr:
				os << x.arr();
				break;
			case BuiltinFuncs::kMops:
				os << x.mops();
				break;
			case BuiltinFuncs::BIFUNC_ONEOF_NOT_SET:
				os << "1";
				break;
		}
		return os << "\n";
	}
	std::ostream &operator<<(std::ostream &os, const Statement &x) {
		switch (x.stmt_oneof_case()) {
			case Statement::kAssignment:
				os << x.assignment();
				break;
			case Statement::kIfelse:
				os << x.ifelse();
				break;
			case Statement::kTernaryStmt:
				os << x.ternary_stmt();
				break;
			case Statement::kBuiltins:
				os << x.builtins();
				break;
			case Statement::kBlockstmt:
				os << x.blockstmt();
				break;
			case Statement::STMT_ONEOF_NOT_SET:
				break;
		}
		return os << "\n";
	}
	std::ostream &operator<<(std::ostream &os, const StatementSeq &x) {
		if (x.statements_size() > 0) {
			os << "@scope ||= begin\n";
			for (auto &st : x.statements())
				os << st;
			os << "end\n";
		}
		return os;
	}
	std::ostream &operator<<(std::ostream &os, const Function &x) {
		return os << "def foo()\n" << x.statements() << "end\n"
					<< "foo\n";
	}

// ---------------------------------

	std::string FunctionToString(const Function &input) {
		std::ostringstream os;
		os << input;
		return os.str();

	}
} // namespace ruby_fuzzer