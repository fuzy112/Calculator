#include "interpreter.hpp"
#include "ast.hpp"
#include "parser.hpp"

namespace lince {

void Interpreter::eval(AST *MyAST, Value &Result)
{
    Result = MyAST->eval(this);
}

std::unique_ptr<AST> Interpreter::parse(const std::string &Expr) const
{
    Parser P{ std::istringstream{ Expr } };
    return P();
}

Value Interpreter::callFunction(const std::string &Name, std::vector<Value> Args)
{
    const auto Functions = findFunctions(Name);
    std::vector<std::type_index> ArgTypes;
    std::transform(Args.cbegin(), Args.cend(), std::back_inserter(ArgTypes), [](const Value &V) {
        return std::type_index(V.Data.type());
    });

    auto F = std::find_if(Functions.cbegin(), Functions.cend(), [&](const Function &Func) {
        return Func.matchType(ArgTypes);
    });

    if (F == Functions.cend())
        throw EvalError("No such function");

    return std::invoke(*F, this, std::move(Args));
}

std::set<std::string>
Interpreter::getCompletionList(const std::string &Text) const
{
    std::set<std::string> Ret;

    for (const auto &Scope : VariableScopes) {
        for (const auto &Pair : Scope) {
            if (Pair.first.find(Text) == 0 && Pair.first.length() != Text.length())
                Ret.insert(Pair.first);
        }
    }
    for (const auto &Scope : FunctionScopes) {
        for (const auto &Pair : Scope) {
            if (Pair.first.find(Text) == 0 && Pair.first.length() != Text.length())
                Ret.insert(Pair.first);
        }
    }
    return Ret;
}

} // namespace lince