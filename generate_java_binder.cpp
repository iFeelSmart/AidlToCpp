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
 * File: generate_java_binder.cpp
 *
 */
 
#include "generate_java.h"
#include "Type.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =================================================
class StubClass : public Class
{
public:
    StubClass(Type* type, Type* interfaceType);
    virtual ~StubClass();

    Variable* transact_code;
    Variable* transact_data;
    Variable* transact_reply;
    Variable* transact_flags;
    SwitchStatement* transact_switch;
private:
    void make_as_interface(Type* interfaceType);
};

StubClass::StubClass(Type* type, Type* interfaceType)
    :Class()
{
    this->comment = "\n/** Local-side IPC implementation stub class. */";
    this->modifiers = PUBLIC;
    this->what = Class::CLASS;
    this->type = type;
    //this->extends = BINDER_NATIVE_TYPE;
    vector<Type *> genericArgs;
    genericArgs.push_back(interfaceType);
    Type* genericInterfaceType = new GenericType("android", "BnInterface", genericArgs);
    this->interfaces.push_back(genericInterfaceType);
    //this->interfaces.push_back(interfaceType);

    // descriptor
    //Field* descriptor = new Field(STATIC | FINAL | PRIVATE,
    //                        new Variable(STRING_TYPE, "DESCRIPTOR"));
    //descriptor->value = "\"" + interfaceType->QualifiedName() + "\"";
    //this->elements.push_back(descriptor);

    // ctor
    //Method* ctor = new Method;
    //    ctor->modifiers = PUBLIC;
    //    ctor->comment = "/** Construct the stub at attach it to the "
    //                    "interface. */";
    //    ctor->name = "Stub";
    //    ctor->statements = new StatementBlock;
    //MethodCall* attach = new MethodCall(THIS_VALUE, "attachInterface",
    //                        2, THIS_VALUE, new LiteralExpression("DESCRIPTOR"));
    //ctor->statements->Add(attach);
    //this->elements.push_back(ctor);

    // asInterface
    // make_as_interface(interfaceType);

    // asBinder
    //Method* asBinder = new Method;
    //    asBinder->modifiers = PUBLIC | OVERRIDE;
    //    asBinder->returnType = IBINDER_TYPE;
    //    asBinder->name = "asBinder";
    //    asBinder->statements = new StatementBlock;
    //asBinder->statements->Add(new ReturnStatement(THIS_VALUE));
    //this->elements.push_back(asBinder);

    // onTransact
    this->transact_code = new Variable(UINT_TYPE, "code");
    this->transact_data = new Variable(PARCEL_TYPE, "data");
    this->transact_data->modifiers = CONST | REFERENCE;
    this->transact_reply = new Variable(PARCEL_TYPE, "reply");
    this->transact_reply->modifiers = POINTER;
    this->transact_flags = new Variable(UINT_TYPE, "flags");
    Method* onTransact = new Method;
        onTransact->modifiers = PUBLIC | OVERRIDE;
        onTransact->returnType = STATUS_TYPE;
        onTransact->name = "onTransact";
        onTransact->parameters.push_back(this->transact_code);
        onTransact->parameters.push_back(this->transact_data);
        onTransact->parameters.push_back(this->transact_reply);
        onTransact->parameters.push_back(this->transact_flags);
        onTransact->statements = new StatementBlock;
        onTransact->classType = type;
        //onTransact->exceptions.push_back(REMOTE_EXCEPTION_TYPE);
    this->elements.push_back(onTransact);
        onTransact->statements->Add(new LiteralExpression("if (!data.checkInterface(this)) { return PERMISSION_DENIED; }"));
    this->transact_switch = new SwitchStatement(this->transact_code);

    onTransact->statements->Add(this->transact_switch);
    MethodCall* superCall = new MethodCall( new Type("android", "BBinder", Type::BUILT_IN, false, false, false)/*SUPER_VALUE*/, "onTransact", 4,
                                    this->transact_code, this->transact_data,
                                    this->transact_reply, this->transact_flags);
    onTransact->statements->Add(new ReturnStatement(superCall));
}

StubClass::~StubClass()
{
}

void
StubClass::make_as_interface(Type *interfaceType)
{
    Variable* obj = new Variable(IBINDER_TYPE, "obj");

    Method* m = new Method;
        m->comment = "/**\n * Cast an IBinder object into an ";
        m->comment += interfaceType->CPPQualifiedName();
        m->comment += " interface,\n";
        m->comment += " * generating a proxy if needed.\n */";
        m->modifiers = PUBLIC | STATIC;
        m->returnType = interfaceType;
        m->name = "asInterface";
        m->parameters.push_back(obj);
        m->statements = new StatementBlock;

    IfStatement* ifstatement = new IfStatement();
        ifstatement->expression = new Comparison(obj, "==", NULL_VALUE);
        ifstatement->statements = new StatementBlock;
        ifstatement->statements->Add(new ReturnStatement(NULL_VALUE));
    m->statements->Add(ifstatement);

    // IInterface iin = obj.queryLocalInterface(DESCRIPTOR)
    MethodCall* queryLocalInterface = new MethodCall(obj, "queryLocalInterface");
    queryLocalInterface->arguments.push_back(new LiteralExpression("DESCRIPTOR"));
    IInterfaceType* iinType = new IInterfaceType();
    Variable *iin = new Variable(iinType, "iin");
    VariableDeclaration* iinVd = new VariableDeclaration(iin, queryLocalInterface, NULL);
    m->statements->Add(iinVd);

    // Ensure the instance type of the local object is as expected.
    // One scenario where this is needed is if another package (with a
    // different class loader) runs in the same process as the service.

    // if (iin != null && iin instanceof <interfaceType>) return (<interfaceType>) iin;
    Comparison* iinNotNull = new Comparison(iin, "!=", NULL_VALUE);
    Comparison* instOfCheck = new Comparison(iin, " instanceof ",
            new LiteralExpression(interfaceType->CPPQualifiedName()));
    IfStatement* instOfStatement = new IfStatement();
        instOfStatement->expression = new Comparison(iinNotNull, "&&", instOfCheck);
        instOfStatement->statements = new StatementBlock;
        instOfStatement->statements->Add(new ReturnStatement(new Cast(interfaceType, iin)));
    m->statements->Add(instOfStatement);

    string proxyType = interfaceType->CPPQualifiedName();
    proxyType += ".Stub.Proxy";
    NewExpression* ne = new NewExpression(NAMES.Find(proxyType));
    ne->arguments.push_back(obj);
    m->statements->Add(new ReturnStatement(ne));

    this->elements.push_back(m);
}



// =================================================
class ProxyClass : public Class
{
public:
    ProxyClass(Type* type, InterfaceType* interfaceType);
    virtual ~ProxyClass();

    //Variable* mRemote;
    bool mOneWay;
};

ProxyClass::ProxyClass(Type* type, InterfaceType* interfaceType)
    :Class()
{
    this->modifiers = PRIVATE | STATIC;
    this->what = Class::CLASS;
    this->type = type;
    //this->interfaces.push_back(interfaceType);

    vector<Type *> genericArgs;
    genericArgs.push_back(interfaceType);
    Type* genericInterfaceType = new GenericType("android", "BpInterface", genericArgs);
    this->interfaces.push_back(genericInterfaceType);

    mOneWay = interfaceType->OneWay();

    // IBinder mRemote
    //mRemote = new Variable(IBINDER_TYPE, "mRemote");
    //this->elements.push_back(new Field(PRIVATE, mRemote));

    // Proxy()
    //Variable* remote = new Variable(IBINDER_TYPE, "remote");
    string nullPackage = "";
    string sp = "sp";
    vector<Type *> genericTypes;
    genericTypes.push_back(IBINDER_TYPE);
    GenericType* implType = new GenericType("android", sp, genericTypes);
    Variable* impl = new Variable( implType/*IBINDER_TYPE*/, "impl");
    impl->modifiers = CONST | REFERENCE;

    Method* ctor = new Method;
        ctor->modifiers = PUBLIC;
        ctor->name = this->type->Name().c_str(); //"Proxy";
        ctor->statements = new StatementBlock;
        ctor->parameters.push_back(impl);
    //ctor->statements->Add(new Assignment(mRemote, remote));
        ctor->parentParameters = new vector<Variable*>();
        ctor->parentParameters->push_back(impl);
        ctor->classType = type;
        ctor->interfaceType = genericInterfaceType;
    this->elements.push_back(ctor);

    // IBinder asBinder()
    //Method* asBinder = new Method;
    //    asBinder->modifiers = PUBLIC | OVERRIDE;
    //    asBinder->returnType = IBINDER_TYPE;
    //    asBinder->name = "asBinder";
    //    asBinder->statements = new StatementBlock;
    //asBinder->statements->Add(new ReturnStatement(mRemote));
    //this->elements.push_back(asBinder);
}

ProxyClass::~ProxyClass()
{
}

// =================================================
static void
generate_new_array(Type* t, StatementBlock* addTo, Variable* v,
                            Variable* parcel)
{
    Variable* len = new Variable(INT_TYPE, v->name + "_length");
    addTo->Add(new VariableDeclaration(len, new MethodCall(parcel, "readInt")));
    IfStatement* lencheck = new IfStatement();
    lencheck->expression = new Comparison(len, "<", new LiteralExpression("0"));
    lencheck->statements->Add(new Assignment(v, NULL_VALUE));
    lencheck->elseif = new IfStatement();
    lencheck->elseif->statements->Add(new Assignment(v,
                new NewArrayExpression(t, len)));
    addTo->Add(lencheck);
}

static void
generate_write_to_parcel(Type* t
        , StatementBlock* addTo
        , Variable* v
        , Variable* parcel
        , int flags)
{
    if (v->dimension == 0) {
        t->WriteToParcel(addTo, v, parcel, flags);
    }
    if (v->dimension == 1) {
        t->WriteArrayToParcel(addTo, v, parcel, flags);
    }
}

static void
generate_create_from_parcel(Type* t, StatementBlock* addTo, Variable* v,
                            Variable* parcel, Variable** cl)
{
    if (v->dimension == 0) {
        t->CreateFromParcel(addTo, v, parcel, cl);
    }
    if (v->dimension == 1) {
        t->CreateArrayFromParcel(addTo, v, parcel, cl);
    }
}

static void
generate_read_from_parcel(Type* t, StatementBlock* addTo, Variable* v,
                            Variable* parcel, Variable** cl)
{
    if (v->dimension == 0) {
        t->ReadFromParcel(addTo, v, parcel, cl);
    }
    if (v->dimension == 1) {
        t->ReadArrayFromParcel(addTo, v, parcel, cl);
    }
}


static void
generate_method(const method_type* method, Class* interface,
                    StubClass* stubClass, ProxyClass* proxyClass, int index)
{
    arg_type* arg;
    int i;
    bool hasOutParams = false;

    const bool oneway = proxyClass->mOneWay || method->oneway;

    // == the TRANSACT_ constant =============================================
    string transactCodeName = "TRANSACTION_";
    transactCodeName += method->name.data;

    //char transactCodeValue[60];
    //sprintf(transactCodeValue, "(android.os.IBinder.FIRST_CALL_TRANSACTION + %d)", index);

    //Field* transactCode = new Field(STATIC | FINAL,
    //                        new Variable(INT_TYPE, transactCodeName));
    //transactCode->value = transactCodeValue;
    //interface->elements.push_back(transactCode);

    if (interface->methodEnum == 0) interface->methodEnum = new Enum();
    EnumItem* item = new EnumItem();
    item->name = transactCodeName;
    if (interface->methodEnum->items.size() == 0) item->value = "IBinder::FIRST_CALL_TRANSACTION";
    else item->value = "";
    interface->methodEnum->items.push_back(item);
    interface->declareMetaInterfaceName = string(interface->type->Name().c_str() + 1);
    interface->implementMetaInterfaceName = string(interface->type->Name().c_str() + 1);

    // +JDN
    //interface->implementMetaInterfaceDesc = interface->type->CPPQualifiedName();
    interface->implementMetaInterfaceDesc = interface->type->QualifiedName();
    // -JDN

    // == the declaration in the interface ===================================
    Method* decl = new Method;
        decl->comment = gather_comments(method->comments_token->extra);
        decl->modifiers = VIRTUAL;
        decl->returnType = NAMES.Search(method->type.type.data);
        decl->returnTypeDimension = method->type.dimension;
        decl->name = method->name.data;

    arg = method->args;
    while (arg != NULL) {
        Type* vType = NAMES.Search(arg->type.type.data);
        Variable* v = new Variable(
                    vType, arg->name.data,
                    arg->type.dimension);
        UserDataType* userDataType = dynamic_cast<UserDataType*>(vType);
        if (userDataType != NULL) {
            v->modifiers = POINTER;
        }
        decl->parameters.push_back(v);
        arg = arg->next;
    }

    // No exception in C++
    //decl->exceptions.push_back(REMOTE_EXCEPTION_TYPE);

    interface->elements.push_back(decl);

    // == the stub method ====================================================

    Case* c = new Case(transactCodeName);

    Variable* thisVar = new Variable(interface->type, "this");
    thisVar->modifiers = POINTER;
    MethodCall* realCall = new MethodCall(thisVar/*THIS_VALUE*/, method->name.data);

    // interface token validation is the very first thing we do

    string getDescriptor = interface->type->Name() + string("::getInterfaceDescriptor()");

    // +JDN Don t do checkInterface + enforceInterface it broke serialization
    // c->statements->Add(new MethodCall(stubClass->transact_data,
    //        "enforceInterface", 1, new LiteralExpression(getDescriptor/*"DESCRIPTOR"*/)));
    // -JDN

    // args
    Variable* cl = NULL;
    VariableFactory stubArgs("_arg");
    arg = method->args;
    while (arg != NULL) {
        Type* t = NAMES.Search(arg->type.type.data);
        Variable* v = stubArgs.Get(t);
        UserDataType* userDataType = dynamic_cast<UserDataType*>(t);
        InterfaceType* interfaceType = dynamic_cast<InterfaceType*>(t);
        if (userDataType != NULL) v->modifiers |= POINTER;
        else if (interfaceType != NULL) v->modifiers |= STRONG_POINTER;
        //if (userDataType != NULL) v->modifiers |= CALL_POINTER;
        v->dimension = arg->type.dimension;

        c->statements->Add(new VariableDeclaration(v));

        if (convert_direction(arg->direction.data) & IN_PARAMETER) {
            generate_create_from_parcel(t, c->statements, v,
                    stubClass->transact_data, &cl);
        } else {
            if (arg->type.dimension == 0) {
                c->statements->Add(new Assignment(v, new NewExpression(v->type)));
            }
            else if (arg->type.dimension == 1) {
                generate_new_array(v->type, c->statements, v,
                        stubClass->transact_data);
            }
            else {
                fprintf(stderr, "aidl:internal error %s:%d\n", __FILE__,
                        __LINE__);
            }
        }

        realCall->arguments.push_back(v);

        arg = arg->next;
    }

    // the real call
    Variable* _result = NULL;
    if (0 == strcmp(method->type.type.data, "void")) {
        c->statements->Add(realCall);

        if (!oneway) {
            // report that there were no exceptions
            MethodCall* ex = new MethodCall(stubClass->transact_reply,
                    "writeNoException", 0);
            c->statements->Add(ex);
        }
    } else {
        _result = new Variable(decl->returnType, "_result",
                                decl->returnTypeDimension);
        UserDataType* userDataType = dynamic_cast<UserDataType*>(decl->returnType);
        InterfaceType* interfaceType = dynamic_cast<InterfaceType*>(decl->returnType);
        if (userDataType != NULL) _result->modifiers = POINTER;
        else if (interfaceType != NULL) _result->modifiers = STRONG_POINTER;
        c->statements->Add(new VariableDeclaration(_result, realCall));

        if (!oneway) {
            // report that there were no exceptions
            MethodCall* ex = new MethodCall(stubClass->transact_reply,
                    "writeNoException", 0);
            c->statements->Add(ex);
        }

        // marshall the return value
        generate_write_to_parcel(decl->returnType, c->statements, _result,
                                    stubClass->transact_reply,
                                    Type::PARCELABLE_WRITE_RETURN_VALUE);
    }

    // out parameters
    i = 0;
    arg = method->args;
    while (arg != NULL) {
        Type* t = NAMES.Search(arg->type.type.data);
        Variable* v = stubArgs.Get(i++);

        if (convert_direction(arg->direction.data) & OUT_PARAMETER) {
            generate_write_to_parcel(t, c->statements, v,
                                stubClass->transact_reply,
                                Type::PARCELABLE_WRITE_RETURN_VALUE);
            hasOutParams = true;
        }

        arg = arg->next;
    }

    // return true
    c->statements->Add(new ReturnStatement(new LiteralExpression("NO_ERROR")));
    stubClass->transact_switch->cases.push_back(c);

    // == the proxy method ===================================================
    Method* proxy = new Method;
        proxy->comment = gather_comments(method->comments_token->extra);
        proxy->modifiers = PUBLIC | OVERRIDE;
        proxy->returnType = NAMES.Search(method->type.type.data);
        proxy->returnTypeDimension = method->type.dimension;
        proxy->name = method->name.data;
        proxy->statements = new StatementBlock;
        proxy->classType = proxyClass->type;
        arg = method->args;
        while (arg != NULL) {
            Type* vType = NAMES.Search(arg->type.type.data);
            Variable* v = new Variable(
                        vType, arg->name.data,
                        arg->type.dimension);
            UserDataType* userDataType = dynamic_cast<UserDataType*>(vType);
            InterfaceType* interfaceType = dynamic_cast<InterfaceType*>(vType);
            if (userDataType != NULL) {
                v->modifiers |= POINTER;
            } else if (interfaceType != NULL) {
                v->modifiers |= STRONG_POINTER;
            }
            proxy->parameters.push_back(v/*new Variable(
                            NAMES.Search(arg->type.type.data), arg->name.data,
                            arg->type.dimension)*/);
            arg = arg->next;
        }
        //proxy->exceptions.push_back(REMOTE_EXCEPTION_TYPE);
    proxyClass->elements.push_back(proxy);

    // the parcels
    Variable* _data = new Variable(PARCEL_TYPE, "_data");
    //proxy->statements->Add(new VariableDeclaration(_data,
    //                            new MethodCall(PARCEL_TYPE, "obtain")));
    proxy->statements->Add(new VariableDeclaration(_data));

    Variable* _reply = NULL;
    if (!oneway) {
        _reply = new Variable(PARCEL_TYPE, "_reply");
        //proxy->statements->Add(new VariableDeclaration(_reply,
        //                            new MethodCall(PARCEL_TYPE, "obtain")));
        proxy->statements->Add(new VariableDeclaration(_reply));
    }

    // the return value
    _result = NULL;
    if (0 != strcmp(method->type.type.data, "void")) {
        _result = new Variable(proxy->returnType, "_result",
                method->type.dimension);
        UserDataType* userDataType = dynamic_cast<UserDataType*>(proxy->returnType);
        InterfaceType* interfaceType = dynamic_cast<InterfaceType*>(proxy->returnType);
        if (userDataType != NULL) {
            _result->modifiers = POINTER;
        } else if (interfaceType != NULL) {
            _result->modifiers = STRONG_POINTER;
        }
        proxy->statements->Add(new VariableDeclaration(_result));
    }

    // try and finally
    //TryStatement* tryStatement = new TryStatement();
    //proxy->statements->Add(tryStatement);
    //FinallyStatement* finallyStatement = new FinallyStatement();
    //proxy->statements->Add(finallyStatement);

    // the interface identifier token: the DESCRIPTOR constant, marshalled as a string

    /*tryStatement*/proxy->statements->Add(new MethodCall(_data, "writeInterfaceToken",
            1, new LiteralExpression(getDescriptor/*"DESCRIPTOR"*/)));

    // the parameters
    arg = method->args;
    while (arg != NULL) {
        Type* t = NAMES.Search(arg->type.type.data);
        Variable* v = new Variable(t, arg->name.data, arg->type.dimension);
        int dir = convert_direction(arg->direction.data);
        v->modifiers |= POINTER;
        if (dir == OUT_PARAMETER && arg->type.dimension != 0)
		{
            IfStatement* checklen = new IfStatement();
            checklen->expression = new Comparison(v, "==", NULL_VALUE);
            checklen->statements->Add(new MethodCall(_data, "writeInt", 1,
                        new LiteralExpression("-1")));
            checklen->elseif = new IfStatement();
            checklen->elseif->statements->Add(new MethodCall(_data, "writeInt",
                        1, new FieldVariable(v, "length")));
            /*tryStatement*/proxy->statements->Add(checklen);
        }
        else if (dir & IN_PARAMETER)
        {
            // +JDN
            // ALERT LEAK !!!!
            Variable* localParcel = new Variable();
            localParcel->type = _data->type;
            localParcel->name = _data->name;
            localParcel->dimension = _data->dimension;
            localParcel->modifiers = _data->modifiers;
            localParcel->modifiers |= CALL_REFERENCE;
            // printf("generating parcel for %s (%s)\n", v->name.c_str(), v->type->Name().c_str());
            //generate_write_to_parcel(t, /*tryStatement*/proxy->statements, v, _data, 0);
            generate_write_to_parcel(t, /*tryStatement*/proxy->statements, v, localParcel, 0);
            // -JDN
        }
        else
        {
            // printf("else\n", "");
            // Do nothing
        }
        arg = arg->next;
    }

    // the transact call
    Variable* callReply = NULL;
    if (_reply != NULL) {
        callReply = new Variable(_reply->type, _reply->name);
        callReply->modifiers = CALL_REFERENCE;
    }
    MethodCall* call = new MethodCall(/*proxyClass->mRemote*/ new LiteralExpression("(*(remote()))"), "transact", 4,
                            new LiteralExpression(/*"Stub." +*/ transactCodeName),
                            _data, callReply ? callReply : NULL_VALUE,
                            new LiteralExpression(
                                          oneway ? /*"android.os.IBinder.FLAG_ONEWAY"*/ "IBinder::FLAG_ONEWAY" : "0"));
    /*tryStatement*/proxy->statements->Add(call);

    // throw back exceptions.
    if (_reply) {
        MethodCall* ex = new MethodCall(_reply, "readExceptionCode", 0);
        /*tryStatement*/proxy->statements->Add(ex);
    }

    // returning and cleanup
    if (_reply != NULL) {
        if (_result != NULL) {
            generate_create_from_parcel(proxy->returnType,
                    /*tryStatement*/proxy->statements, _result, _reply, &cl);
        }

        // the out/inout parameters
        arg = method->args;
        while (arg != NULL) {
            Type* t = NAMES.Search(arg->type.type.data);
            Variable* v = new Variable(t, arg->name.data, arg->type.dimension);
            UserDataType* userDataType = dynamic_cast<UserDataType*>(t);
            if (userDataType != NULL) v->modifiers = POINTER;
            if (convert_direction(arg->direction.data) & OUT_PARAMETER) {
                generate_read_from_parcel(t, /*tryStatement*/proxy->statements,
                                            v, _reply, &cl);
            }
            arg = arg->next;
        }

        //finallyStatement->statements->Add(new MethodCall(_reply, "recycle"));
    }
    //finallyStatement->statements->Add(new MethodCall(_data, "recycle"));

    if (_result != NULL) {
        proxy->statements->Add(new ReturnStatement(_result));
    }
}

static void
generate_interface_descriptors(StubClass* stub, ProxyClass* proxy)
{
    // the interface descriptor transaction handler
    //Case* c = new Case("INTERFACE_TRANSACTION");
    //c->statements->Add(new MethodCall(stub->transact_reply, "writeString",
    //        1, new LiteralExpression("DESCRIPTOR")));
    //c->statements->Add(new ReturnStatement(TRUE_VALUE));
    //stub->transact_switch->cases.push_back(c);

    // and the proxy-side method returning the descriptor directly

    /*
    Method* getDesc = new Method;
    getDesc->modifiers = PUBLIC;
    getDesc->returnType = STRING_TYPE;
    getDesc->returnTypeDimension = 0;
    getDesc->name = "getInterfaceDescriptor";
    getDesc->statements = new StatementBlock;
    getDesc->statements->Add(new ReturnStatement(new LiteralExpression("DESCRIPTOR")));
    proxy->elements.push_back(getDesc);
    */
}

Class**
generate_binder_interface_class(const interface_type* iface)
{
    Class** ret = new Class*[4];
    ret[3] = 0;
    InterfaceType* interfaceType = static_cast<InterfaceType*>(
        NAMES.Find(iface->package, iface->name.data));

    // the interface class
    Class* interface = new Class;
        interface->comment = gather_comments(iface->comments_token->extra);
        interface->modifiers = PUBLIC;
        interface->what = Class::INTERFACE;
        interface->type = interfaceType;
        interface->interfaces.push_back(IINTERFACE_TYPE);
    ret[0] = interface;

    // the stub inner class
    // JDN printf("%s %s\n", iface->package, append("Bn", iface->name.data + 1).c_str());
    Type* type = NAMES.Find(iface->package,
                            //append(iface->name.data, ".Stub").c_str()),
                            append("Bn", iface->name.data + 1).c_str());
    if (type == 0) printf("Cannot find the type \n");
    StubClass* stub = new StubClass(
        NAMES.Find(iface->package,
                   //append(iface->name.data, ".Stub").c_str()),
                   append("Bn", iface->name.data + 1).c_str()),
        interfaceType);
    ret[1] = stub;
    //interface->elements.push_back(stub);


    // the proxy inner class
    // JDN printf("%s %s\n", iface->package, append("Bp", iface->name.data + 1).c_str());
    type = NAMES.Find(iface->package,
                      //append(iface->name.data, ".Stub.Proxy").c_str()),
                      append("Bp", iface->name.data + 1).c_str());
    if (type == 0) printf("Cannot find the type \n");
    ProxyClass* proxy = new ProxyClass(
        NAMES.Find(iface->package,
                   //append(iface->name.data, ".Stub.Proxy").c_str()),
                   append("Bp", iface->name.data + 1).c_str()),
        interfaceType);
    ret[2] = proxy;
    //stub->elements.push_back(proxy);

    // stub and proxy support for getInterfaceDescriptor()
    generate_interface_descriptors(stub, proxy);

    // all the declared methods of the interface
    int index = 0;
    interface_item_type* item = iface->interface_items;
    while (item != NULL) {
        if (item->item_type == METHOD_TYPE) {
            method_type * method_item = (method_type*) item;
            generate_method(method_item, interface, stub, proxy, method_item->assigned_id);
        }
        item = item->next;
        index++;
    }

    return ret;
}

