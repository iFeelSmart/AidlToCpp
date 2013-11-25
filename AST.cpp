/* iFeelSmart
 *
 * Copyright © 2012-2013, iFeelSmart.
 *
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation and
 * appearing in the file LICENSE.LGPL included in the packaging of this
 * file. Please review the following information to ensure the GNU Lesser
 * General Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * AST.cpp
 * 
 */
 
#include "AST.h"
#include "Type.h"
#include <algorithm>
#include <assert.h>

void
WriteModifiers(FILE* to, int mod, int mask)
{
    int m = mod & mask;

    if (m & OVERRIDE) {
        fprintf(to, "virtual ");
    }

    if (m & VIRTUAL) {
        fprintf(to, "virtual ");
    }

    if ((m & SCOPE_MASK) == PUBLIC) {
        fprintf(to, "public ");
    }
    else if ((m & SCOPE_MASK) == PRIVATE) {
        fprintf(to, "private ");
    }
    else if ((m & SCOPE_MASK) == PROTECTED) {
        fprintf(to, "protected ");
    }

    if (m & STATIC) {
        fprintf(to, "static ");
    }
    
    if (m & FINAL) {
        fprintf(to, "final ");
    }

    if (m & ABSTRACT) {
        fprintf(to, "abstract ");
    }
}

// +JDN
/*
void
WriteArgumentList(FILE* to, const vector<int>& argumentsTypes, const vector<Expression*>& arguments)
{
    printf( "%d == %d\n", argumentsTypes.size(), arguments.size());
    size_t N = arguments.size();
    assert( argumentsTypes.size() == N );
    for (size_t i=0; i<N; i++)
    {
        Variable* v = dynamic_cast<Variable*>(arguments[i]);
       
        if (v != NULL)
        {
            if( ! argumentsTypes[i] & POINTER && v->modifiers & POINTER )
            {
                fprintf(to, "&");
            }
            v->WriteArgument(to);
        } else {
            arguments[i]->Write(to);
        }
        if (i != N-1) {
            fprintf(to, ", ");
        }
    }
}
*/
// -JDN

void
WriteArgumentList(FILE* to, const vector<Expression*>& arguments)
{
    size_t N = arguments.size();
    for (size_t i=0; i<N; i++) {
        Variable* v = dynamic_cast<Variable*>(arguments[i]);
        if (v != NULL) {
            v->WriteArgument(to);
        } else {
            arguments[i]->Write(to);
        }
        if (i != N-1) {
            fprintf(to, ", ");
        }
    }
}

ClassElement::ClassElement()
{
}

ClassElement::~ClassElement()
{
}

Field::Field()
    :ClassElement(),
     modifiers(0),
     variable(NULL)
{
}

Field::Field(int m, Variable* v)
    :ClassElement(),
     modifiers(m),
     variable(v)
{
}

Field::~Field()
{
}

void
Field::GatherTypes(set<Type*>* types) const
{
    types->insert(this->variable->type);
}

void
Field::Write(FILE* toC, FILE* toH)
{
    if (this->comment.length() != 0) {
        fprintf(toH, "%s\n", this->comment.c_str());
    }
    WriteModifiers(toH, this->modifiers, SCOPE_MASK | STATIC | FINAL | OVERRIDE);
    fprintf(toH, "%s %s", this->variable->type->CPPQualifiedName().c_str(),
            this->variable->name.c_str());
    if (this->value.length() != 0) {
        fprintf(toH, " = %s", this->value.c_str());
    }
    fprintf(toH, ";\n");
}

Expression::~Expression()
{
}

LiteralExpression::LiteralExpression(const string& v)
    :value(v)
{
}

LiteralExpression::~LiteralExpression()
{
}

void
LiteralExpression::Write(FILE* to)
{
    fprintf(to, "%s", this->value.c_str());
}

StringLiteralExpression::StringLiteralExpression(const string& v)
    :value(v)
{
}

StringLiteralExpression::~StringLiteralExpression()
{
}

void
StringLiteralExpression::Write(FILE* to)
{
    fprintf(to, "\"%s\"", this->value.c_str());
}

Variable::Variable()
    :type(NULL),
     name(),
     dimension(0),
     modifiers(0)
{
}

Variable::Variable(Type* t, const string& n)
    :type(t),
     name(n),
     dimension(0),
     modifiers(0)
{
}

Variable::Variable(Type* t, const string& n, int d)
    :type(t),
     name(n),
     dimension(d),
     modifiers(0)
{
}

Variable::~Variable()
{
}

void
Variable::GatherTypes(set<Type*>* types) const
{
    types->insert(this->type);
}

void
Variable::WriteDeclaration(FILE* to)
{
    string dim;
    for (int i=0; i<this->dimension; i++) {
        dim += "[]";
    }
    string prefix;
    if (this->modifiers & CONST) prefix += "const ";
    string suffix;
    if (this->modifiers & POINTER) suffix += "*" ;
    if (this->modifiers & REFERENCE) suffix += "&" ;
    if (this->modifiers & STRONG_POINTER) {
        prefix += "sp<";
        suffix += ">";
    }

    fprintf(to, "%s%s%s%s %s", prefix.c_str(), this->type->CPPQualifiedName().c_str(), suffix.c_str(), dim.c_str(),
            this->name.c_str());
}

void
Variable::Write(FILE* to)
{

    fprintf(to, "%s", name.c_str());
}

void
Variable::WriteArgument(FILE* to)
{
    string prefix;
    if (this->modifiers & CALL_POINTER) prefix += "*" ;
    if (this->modifiers & CALL_REFERENCE) prefix += "&" ;
    fprintf(to, "%s%s", prefix.c_str(), name.c_str());
}

FieldVariable::FieldVariable(Expression* o, const string& n)
    :object(o),
     clazz(NULL),
     name(n)
{
}

FieldVariable::FieldVariable(Type* c, const string& n)
    :object(NULL),
     clazz(c),
     name(n)
{
}

FieldVariable::~FieldVariable()
{
}

void
FieldVariable::Write(FILE* to)
{
    if (this->object != NULL) {
        this->object->Write(to);
    }
    else if (this->clazz != NULL) {
        fprintf(to, "%s", this->clazz->CPPQualifiedName().c_str());
    }
    fprintf(to, ".%s", name.c_str());
}


Statement::~Statement()
{
}

StatementBlock::StatementBlock()
{
}

StatementBlock::~StatementBlock()
{
}

void
StatementBlock::Write(FILE* to)
{
    fprintf(to, "{\n");
    int N = this->statements.size();
    for (int i=0; i<N; i++) {
        this->statements[i]->Write(to);
    }
    fprintf(to, "}\n");
}

void
StatementBlock::Add(Statement* statement)
{
    this->statements.push_back(statement);
}

void
StatementBlock::Add(Expression* expression)
{
    this->statements.push_back(new ExpressionStatement(expression));
}

ExpressionStatement::ExpressionStatement(Expression* e)
    :expression(e)
{
}

ExpressionStatement::~ExpressionStatement()
{
}

void
ExpressionStatement::Write(FILE* to)
{
    this->expression->Write(to);
    fprintf(to, ";\n");
}

Assignment::Assignment(Variable* l, Expression* r)
    :lvalue(l),
     rvalue(r),
     cast(NULL)
{
}

Assignment::Assignment(Variable* l, Expression* r, Type* c)
    :lvalue(l),
     rvalue(r),
     cast(c)
{
}

Assignment::~Assignment()
{
}

void
Assignment::Write(FILE* to)
{
    this->lvalue->Write(to);
    fprintf(to, " = ");
    if (this->cast != NULL) {
        fprintf(to, "(%s)", this->cast->CPPQualifiedName().c_str());
    }
    this->rvalue->Write(to);
}

MethodCall::MethodCall(const string& n)
    :obj(NULL),
     clazz(NULL),
     name(n)
{
}

MethodCall::MethodCall(const string& n, int argc = 0, ...)
    :obj(NULL),
     clazz(NULL),
     name(n)
{
  va_list args;
  va_start(args, argc);
  init(argc, args);
  va_end(args);
}

MethodCall::MethodCall(Expression* o, const string& n)
    :obj(o),
     clazz(NULL),
     name(n)
{
}

MethodCall::MethodCall(Type* t, const string& n)
    :obj(NULL),
     clazz(t),
     name(n)
{
}

MethodCall::MethodCall(Expression* o, const string& n, int argc = 0, ...)
    :obj(o),
     clazz(NULL),
     name(n)
{
  va_list args;
  va_start(args, argc);
  init(argc, args);
  va_end(args);
}

MethodCall::MethodCall(Type* t, const string& n, int argc = 0, ...)
    :obj(NULL),
     clazz(t),
     name(n)
{
  va_list args;
  va_start(args, argc);
  init(argc, args);
  va_end(args);
}

// +JDN
//MethodCall::MethodCall(Expression* o, const string& n, vector<int> argsType, vector<Expression *> args)
//    : obj( o )
//    , clazz( NULL )
//    , name( n )
//{
//    this->argumentsType = argsType;
//    this->arguments = args;
//    
//    printf("constructor %d %d\n", this->argumentsType.size(), this->arguments.size() );
//}
// -JDN

MethodCall::~MethodCall()
{
}

void
MethodCall::init(int n, va_list args)
{
    for (int i=0; i<n; i++) {
        // +JDN
        //this->argumentsType.push_back(0);
        // -JDN
        Expression* expression = (Expression*)va_arg(args, void*);
        this->arguments.push_back(expression);
    }
}

void
MethodCall::Write(FILE* to)
{
    if (this->obj != NULL) {
        this->obj->Write(to);
        Variable* var = dynamic_cast<Variable*>(this->obj);
        if (var != NULL && ((var->modifiers & POINTER) || (var->modifiers & STRONG_POINTER))) fprintf(to, "->");
        else fprintf(to, ".");
    }
    else if (this->clazz != NULL) {
        fprintf(to, "%s::", this->clazz->CPPQualifiedName().c_str());
    }
    fprintf(to, "%s(", this->name.c_str());
    // +JDN
    WriteArgumentList(to, this->arguments);
    //WriteArgumentList(to, this->argumentsType, this->arguments);
    // -JDN
    fprintf(to, ")");
}

Comparison::Comparison(Expression* l, const string& o, Expression* r)
    :lvalue(l),
     op(o),
     rvalue(r)
{
}

Comparison::~Comparison()
{
}

void
Comparison::Write(FILE* to)
{
    fprintf(to, "(");
    this->lvalue->Write(to);
    fprintf(to, "%s", this->op.c_str());
    this->rvalue->Write(to);
    fprintf(to, ")");
}

NewExpression::NewExpression(Type* t)
    :type(t)
{
}

NewExpression::NewExpression(Type* t, int argc = 0, ...)
    :type(t)
{
  va_list args;
  va_start(args, argc);
  init(argc, args);
  va_end(args);
}

NewExpression::~NewExpression()
{
}

void
NewExpression::init(int n, va_list args)
{
    for (int i=0; i<n; i++) {
        Expression* expression = (Expression*)va_arg(args, void*);
        this->arguments.push_back(expression);
    }
}

void
NewExpression::Write(FILE* to)
{
    //fprintf(to, "new %s(", this->type->InstantiableName().c_str());
    fprintf(to, "new %s(", this->type->CPPQualifiedName().c_str());
    WriteArgumentList(to, this->arguments);
    fprintf(to, ")");
}

NewArrayExpression::NewArrayExpression(Type* t, Expression* s)
    :type(t),
     size(s)
{
}

NewArrayExpression::~NewArrayExpression()
{
}

void
NewArrayExpression::Write(FILE* to)
{
    fprintf(to, "new %s[", this->type->CPPQualifiedName().c_str());
    size->Write(to);
    fprintf(to, "]");
}

Ternary::Ternary()
    :condition(NULL),
     ifpart(NULL),
     elsepart(NULL)
{
}

Ternary::Ternary(Expression* a, Expression* b, Expression* c)
    :condition(a),
     ifpart(b),
     elsepart(c)
{
}

Ternary::~Ternary()
{
}

void
Ternary::Write(FILE* to)
{
    fprintf(to, "((");
    this->condition->Write(to);
    fprintf(to, ")?(");
    this->ifpart->Write(to);
    fprintf(to, "):(");
    this->elsepart->Write(to);
    fprintf(to, "))");
}

Cast::Cast()
    :type(NULL),
     expression(NULL)
{
}

Cast::Cast(Type* t, Expression* e)
    :type(t),
     expression(e)
{
}

Cast::~Cast()
{
}

void
Cast::Write(FILE* to)
{
    fprintf(to, "((%s)", this->type->CPPQualifiedName().c_str());
    expression->Write(to);
    fprintf(to, ")");
}

VariableDeclaration::VariableDeclaration(Variable* l, Expression* r, Type* c)
    :lvalue(l),
     cast(c),
     rvalue(r)
{
}

VariableDeclaration::VariableDeclaration(Variable* l)
    :lvalue(l),
     cast(NULL),
     rvalue(NULL)
{
}

VariableDeclaration::~VariableDeclaration()
{
}

void
VariableDeclaration::Write(FILE* to)
{
    this->lvalue->WriteDeclaration(to);
    if (this->rvalue != NULL) {
        fprintf(to, " = ");
        if (this->cast != NULL) {
            fprintf(to, "(%s)", this->cast->CPPQualifiedName().c_str());
        }
        this->rvalue->Write(to);
    }
    fprintf(to, ";\n");
}

IfStatement::IfStatement()
    :expression(NULL),
     statements(new StatementBlock),
     elseif(NULL)
{
}

IfStatement::~IfStatement()
{
}

void
IfStatement::Write(FILE* to)
{
    if (this->expression != NULL) {
        fprintf(to, "if (");
        this->expression->Write(to);
        fprintf(to, ") ");
    }
    this->statements->Write(to);
    if (this->elseif != NULL) {
        // +JDN
        fprintf(to, "else\n");
        // -JDN
        this->elseif->Write(to);
    }
}

ReturnStatement::ReturnStatement(Expression* e)
    :expression(e)
{
}

ReturnStatement::~ReturnStatement()
{
}

void
ReturnStatement::Write(FILE* to)
{
    fprintf(to, "return ");
    this->expression->Write(to);
    fprintf(to, ";\n");
}

TryStatement::TryStatement()
    :statements(new StatementBlock)
{
}

TryStatement::~TryStatement()
{
}

void
TryStatement::Write(FILE* to)
{
    fprintf(to, "try ");
    this->statements->Write(to);
}

CatchStatement::CatchStatement(Variable* e)
    :statements(new StatementBlock),
     exception(e)
{
}

CatchStatement::~CatchStatement()
{
}

void
CatchStatement::Write(FILE* to)
{
    fprintf(to, "catch ");
    if (this->exception != NULL) {
        fprintf(to, "(");
        this->exception->WriteDeclaration(to);
        fprintf(to, ") ");
    }
    this->statements->Write(to);
}

FinallyStatement::FinallyStatement()
    :statements(new StatementBlock)
{
}

FinallyStatement::~FinallyStatement()
{
}

void
FinallyStatement::Write(FILE* to)
{
    fprintf(to, "finally ");
    this->statements->Write(to);
}

Case::Case()
    :statements(new StatementBlock)
{
}

Case::Case(const string& c)
    :statements(new StatementBlock)
{
    cases.push_back(c);
}

Case::~Case()
{
}

void
Case::Write(FILE* to)
{
    int N = this->cases.size();
    if (N > 0) {
        for (int i=0; i<N; i++) {
            string s = this->cases[i];
            if (s.length() != 0) {
                fprintf(to, "case %s:\n", s.c_str());
            } else {
                fprintf(to, "default:\n");
            }
        }
    } else {
        fprintf(to, "default:\n");
    }
    statements->Write(to);
}

SwitchStatement::SwitchStatement(Expression* e)
    :expression(e)
{
}

SwitchStatement::~SwitchStatement()
{
}

void
SwitchStatement::Write(FILE* to)
{
    fprintf(to, "switch (");
    this->expression->Write(to);
    fprintf(to, ")\n{\n");
    int N = this->cases.size();
    for (int i=0; i<N; i++) {
        this->cases[i]->Write(to);
    }
    fprintf(to, "}\n");
}

Break::Break()
{
}

Break::~Break()
{
}

void
Break::Write(FILE* to)
{
    fprintf(to, "break;\n");
}

Method::Method()
    :ClassElement(),
     modifiers(0),
     returnType(NULL), // (NULL means constructor)
     returnTypeDimension(0),
     statements(NULL),
     parentParameters(NULL),
     classType(NULL),
     interfaceType(NULL)
{
}

Method::~Method()
{
}

void
Method::GatherTypes(set<Type*>* types) const
{
    size_t N, i;

    if (this->returnType) {
        types->insert(this->returnType);
    }

    N = this->parameters.size();
    for (i=0; i<N; i++) {
        this->parameters[i]->GatherTypes(types);
    }

    N = this->exceptions.size();
    for (i=0; i<N; i++) {
        types->insert(this->exceptions[i]);
    }
}

void
Method::Write(FILE* toC, FILE* toH)
{
    size_t N, i;
    bool codeInC = this->statements != NULL || this->parentParameters != NULL;

    if (this->comment.length() != 0) {
        fprintf(toH, "%s\n", this->comment.c_str());
    }

    WriteModifiers(toH, this->modifiers, STATIC | ABSTRACT | FINAL | OVERRIDE | VIRTUAL);

    if (this->returnType != NULL) {
        string dim;
        for (i=0; i<this->returnTypeDimension; i++) {
            dim += "[]";
        }
        UserDataType* userDataType = dynamic_cast<UserDataType*>(this->returnType);
        InterfaceType* interfaceType = dynamic_cast<InterfaceType*>(this->returnType);
        if (userDataType != NULL)
            fprintf(toH, "%s*%s ", this->returnType->CPPQualifiedName().c_str(),
                    dim.c_str());
        else if (interfaceType != NULL)
            fprintf(toH, "sp<%s>%s ", this->returnType->CPPQualifiedName().c_str(),
                    dim.c_str());
        else
            fprintf(toH, "%s%s ", this->returnType->CPPQualifiedName().c_str(),
                    dim.c_str());
        if (codeInC) {
            if (userDataType != NULL)
                fprintf(toC, "%s*%s ", this->returnType->CPPQualifiedName().c_str(),
                                                  dim.c_str());
            else if (interfaceType != NULL)
                fprintf(toC, "sp<%s>%s ", this->returnType->CPPQualifiedName().c_str(),
                                                  dim.c_str());
            else
                fprintf(toC, "%s%s ", this->returnType->CPPQualifiedName().c_str(),
                                                  dim.c_str());
        }
    }
   
    fprintf(toH, "%s(", this->name.c_str());
    if (codeInC) {
        if (this->classType != NULL) fprintf(toC, "%s::", this->classType->Name().c_str());
        fprintf(toC, "%s(", this->name.c_str());
    }

    N = this->parameters.size();
    for (i=0; i<N; i++) {
        this->parameters[i]->WriteDeclaration(toH);
        if (codeInC) this->parameters[i]->WriteDeclaration(toC);
        if (i != N-1) {
            fprintf(toH, ", ");
            if (codeInC) fprintf(toC, ", ");
        }
    }

    fprintf(toH, ")");
    if (codeInC) {
        fprintf(toC, ")");
        if (this->parentParameters != NULL && this->classType != NULL) {
            fprintf(toC, ": %s (", this->interfaceType->CPPQualifiedName().c_str());
            N = this->parentParameters->size();
            for (i=0; i<N; i++) {
                fprintf(toC, "%s", (*this->parentParameters)[i]->name.c_str());
                if (i != N-1) {
                    fprintf(toC, ", ");
                }
            }
            fprintf(toC, ")");
        }
    }

    N = this->exceptions.size();
    for (i=0; i<N; i++) {
        if (i == 0) {
            fprintf(toH, " throws ");
        } else {
            fprintf(toH, ", ");
        }
        fprintf(toH, "%s", this->exceptions[i]->CPPQualifiedName().c_str());
    }

    if (this->statements == NULL) {
        fprintf(toH, " = 0;\n");
    } else {
        fprintf(toH, ";\n");

        // To .cpp file we add the definition
        this->statements->Write(toC);
    }
}

Class::Class()
    :modifiers(0),
     what(CLASS),
     type(NULL),
     extends(NULL)
{
}

Class::~Class()
{
}

void
Class::GatherTypes(set<Type*>* types) const
{
    int N, i;

    types->insert(this->type);
    if (this->extends != NULL) {
        types->insert(this->extends);
    }

    N = this->interfaces.size();
    for (i=0; i<N; i++) {
        types->insert(this->interfaces[i]);
    }

    N = this->elements.size();
    for (i=0; i<N; i++) {
        this->elements[i]->GatherTypes(types);
    }
}

void
Class::Write(FILE* toC, FILE* toH)
{
    size_t N, i;

    string name = this->type->Name();
    fprintf(toH, "\n\n// ************************************\n// ********* Class %s *********\n// ************************************\n\n\n", name.c_str());
    fprintf(toC, "\n\n// ************************************\n// ********* Class %s *********\n// ************************************\n\n\n", name.c_str());
    if (this->comment.length() != 0) {
        fprintf(toH, "%s\n", this->comment.c_str());
    }

    //WriteModifiers(toH, this->modifiers, ALL_MODIFIERS);

    // No interface in C++ => always class !
    //if (this->what == Class::CLASS) {
        fprintf(toH, "class ");
    //} else {
    //    fprintf(to, "interface ");
    //}

    size_t pos = name.rfind('.');
    if (pos != string::npos) {
        name = name.c_str() + pos + 1;
    }

    fprintf(toH, "%s", name.c_str());

    N = this->interfaces.size();
    if (this->extends != NULL || N > 0) {
        fprintf(toH, " : ");
    }

    if (this->extends != NULL) {
        fprintf(toH, "public %s", this->extends->CPPQualifiedName().c_str());
    }

    if (N != 0) {
        //if (this->what == Class::CLASS) {
        //    fprintf(to, " implements");
        //} else {
        //    fprintf(to, " extends");
        //}
        for (i=0; i<N; i++) {
            if (i==0) {
                if (this->extends != NULL) fprintf(toH, ", ");
            } else {
                fprintf(toH, ", ");
            }
            fprintf(toH, "public %s", this->interfaces[i]->CPPQualifiedName().c_str());
        }
    }

    fprintf(toH, "\n");
    fprintf(toH, "{\n");
    fprintf(toH, "\npublic:\n");

    N = this->methodEnum != 0 ? this->methodEnum->items.size() : 0;
    if (N > 0)
    {
        fprintf(toH, "enum {\n");
        for (i=0; i<N; i++) {
            fprintf(toH, this->methodEnum->items[i]->name.c_str());
            if (this->methodEnum->items[i]->value.size() > 0) fprintf(toH, " = %s", this->methodEnum->items[i]->value.c_str());
            if (i < N - 1) fprintf(toH, ",");
            fprintf(toH, "\n");
        }
        fprintf(toH, "};\n");
    }

    N = this->elements.size();
    for (i=0; i<N; i++) {
        this->elements[i]->Write(toC, toH);
    }

    if (this->declareMetaInterfaceName.length() > 0)
        fprintf(toH, "\nDECLARE_META_INTERFACE(%s);\n\n", this->declareMetaInterfaceName.c_str());
    if (this->implementMetaInterfaceName.length() > 0 && this->implementMetaInterfaceDesc.length() > 0)
        fprintf(toC, "\nIMPLEMENT_META_INTERFACE(%s, \"%s\");\n\n", this->implementMetaInterfaceName.c_str(), this->implementMetaInterfaceDesc.c_str());

    fprintf(toH, "};\n\n\n");

}

Document::Document()
{
}

Document::~Document()
{
}

static string
escape_backslashes(const string& str)
{
    string result;
    const size_t I=str.length();
    for (size_t i=0; i<I; i++) {
        char c = str[i];
        if (c == '\\') {
            result += "\\\\";
        } else {
            result += c;
        }
    }
    return result;
}

void
Document::Write(FILE* toC, FILE* toH)
{
    size_t N, i;

    string fileN = this->classes[0]->type->Name();
    std::transform(fileN.begin(), fileN.end(),fileN.begin(), ::toupper);
    fprintf(toH, "#ifndef %s\n", fileN.c_str());
    fprintf(toH, "#define %s\n", fileN.c_str());

    Type** allTypes = NAMES.AllTypes();
    while (allTypes[0] != NULL) {
        UserDataType* userDataType = dynamic_cast<UserDataType*>(allTypes[0]);
        if (userDataType != NULL) {
            if (userDataType->Name().compare(string("RpcData")) != 0 && userDataType->Name().compare(string("RpcError")) != 0)
                fprintf(toH, "#include \"%s.h\"\n", userDataType->Name().c_str());
        }
        InterfaceType* interfaceType = dynamic_cast<InterfaceType*>(allTypes[0]);
        if (interfaceType != NULL && interfaceType->Name().compare(this->classes[0]->type->Name()) != 0) {
            fprintf(toH, "#include \"%s.h\"\n", interfaceType->Name().c_str());
        }
        allTypes++;
    }
    fprintf(toH, "\n");
    fprintf(toH, "#include <binder/IInterface.h>\n");
    fprintf(toH, "#include <binder/IBinder.h>\n");
    fprintf(toH, "#include <binder/Parcel.h>\n");
    //fprintf(toH, "#include <binder/ProcessState.h>\n");
    //fprintf(toH, "#include <binder/IServiceManager.h>\n");
    //fprintf(toH, "#include <binder/IPCThreadState.h>\n");
    fprintf(toH, "\n");
    fprintf(toC, "#include \"%s\"\n", (this->classes[0]->type->Name() + string(".h")).c_str());

    if (this->comment.length() != 0) {
        fprintf(toH, "%s\n", this->comment.c_str());
    }

    fprintf(toH, "/*\n"
                " * This file is auto-generated.  DO NOT MODIFY.\n"
                " * Original file: %s\n"
                " */\n", escape_backslashes(this->originalSrc).c_str());

    int nbPackageLevels = 0;
    if (this->package.length() != 0) {
        size_t prev_pos = 0;
        size_t start_pos = 0;
        while((start_pos = this->package.find('.', start_pos)) != std::string::npos) {
            string subP = this->package.substr(prev_pos, start_pos - prev_pos);
            //str.replace(start_pos, from.length(), to);
            //start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
            fprintf(toH, "namespace %s {\n", subP.c_str());
            fprintf(toC, "namespace %s {\n", subP.c_str());
            nbPackageLevels++;
            start_pos++;
            prev_pos = start_pos;
        }
        string lastP = this->package.substr(prev_pos, this->package.length() - prev_pos);
        fprintf(toH, "namespace %s {\n", lastP.c_str());
        fprintf(toC, "namespace %s {\n", lastP.c_str());
        nbPackageLevels++;
    }

    N = this->classes.size();
    for (i=0; i<N; i++) {
        Class* c = this->classes[i];
        c->Write(toC, toH);
    }

    for(int l = 0; l < nbPackageLevels; l++) {
    //if (this->package.length() != 0) {
        fprintf(toH, "}");
        fprintf(toC, "}");
    }

    fprintf(toH, "\n#endif\n");
}

