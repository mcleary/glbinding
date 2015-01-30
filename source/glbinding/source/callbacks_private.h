#pragma once

namespace glbinding 
{

class AbstractFunction;
struct FunctionCall;

void unresolved(const AbstractFunction * function);
void before(std::unique_ptr<FunctionCall> const & call);
void after(std::unique_ptr<FunctionCall> const & call);

} // namespace glbinding
