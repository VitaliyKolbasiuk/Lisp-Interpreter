#pragma once

#include <iostream>
#include <csignal>
#include <cstring>
#include <functional>

//---------------------------------------------------------------
//
// SExpr - symbolic expression
//
//---------------------------------------------------------------
//
//  Example of how implemented s-expression: ( a b )
//
//  !-------------!        !-------------!
//  ! SExpr: LIST !   +--->! SExpr: LIST !
//  !.............!   |    !.............!
//  ! CAR  ! CDR  !---+    ! CAR  ! CDR  !-->(nullptr)
//  !------!------!        !------!------!
//     |                      |
//     V                      V
//  !-----------------!    !-----------------!
//  ! SExpr: ATOM     !    ! SExpr: ATOM     !
//  !.................!    !.................!
//  ! value ! name: a !    ! value ! name: b !
//  !-------!---------!    !-------!---------!
//
//---------------------------------------------------------------
//

class List;
class Atom;
class BuiltinFunc;
class NumberBase;
class IntNumber;
class Double;


//----------
// ISExpr
//----------
class ISExpr
{
public:
    enum Type {
        LIST=0,
        BUILT_IN_FUNC,
        ATOM,
        DOUBLE,
        NUMBER_BASE,
        INT_NUMBER,
        STRING,
        WSTRING,
        ARRAY,
        CUSTOM
    };

protected:
    ISExpr() = default;
    virtual ~ISExpr() = default;

public:
    virtual Type type() const = 0;
    virtual ISExpr* print( std::ostream& stream ) const = 0;
    virtual ISExpr* eval() = 0;

    virtual ISExpr* print0( const char* prefix ) const { std::cout << prefix; print(std::cout); std::cout << "\n"; return nullptr;};

    ISExpr* toExpr() { return this; }
    
    List*   toList() {
        if ( type() != LIST )
        {
            std::raise(SIGINT);
            return nullptr;
        }
        return (List*) this;
    }
    
    Atom*   toAtom() {
        if ( type() != ATOM )
        {
            std::raise(SIGINT);
            return nullptr;
        }
        return (Atom*) this;
    }
    
    BuiltinFunc*  toBuiltinFunc() {
        if ( type() != BUILT_IN_FUNC )
        {
            return nullptr;
        }
        return (BuiltinFunc*) this;
    }
    
    NumberBase*  toNumberBase() {
        if ( type() != INT_NUMBER && type() != DOUBLE )
        {
            std::raise(SIGINT);
            return nullptr;
        }
        return (NumberBase*) this;
    }
    
    IntNumber*  toIntNumber() {
        if ( type() != INT_NUMBER )
        {
            std::raise(SIGINT);
            return nullptr;
        }
        return (IntNumber*) this;
    }
    
    Double*  toDouble() {
        if ( type() != DOUBLE )
        {
            std::raise(SIGINT);
            return nullptr;
        }
        return (Double*) this;
    }
    
protected:
    const char* copyString( const char* name )
    {
        auto len = std::strlen(name)+1;
        char* string = new char[len];
        std::memcpy( string, name, len );
        return string;
    }
    

};

//------------------------
// List
//------------------------
class List : public ISExpr
{
public:
    ISExpr* m_car = nullptr;
    List*   m_cdr = nullptr;

public:
    List() : m_car(nullptr), m_cdr(nullptr) {};
    List( ISExpr* car ) : m_car(car), m_cdr(nullptr) {};
    List( ISExpr* car, List* cdr ) : m_car(car), m_cdr(cdr) {};
    virtual ~List() {}

    Type type() const override { return LIST; }
    
    virtual ISExpr* eval() override { return this; } //TODO

    bool isEmptyList() { return m_car == nullptr && m_cdr == nullptr;}

    ISExpr* print(  const char* prefix ) const { std::cout << prefix; print(std::cout); return nullptr;};

    ISExpr* print( std::ostream& stream = std::cout ) const override
    {
        stream << "( ";
        if ( m_car == nullptr && m_cdr == nullptr )
        {
            stream << ")";
            return;
        }
        
        if ( m_car == nullptr )
        {
            stream << "nil(nullptr)";
        }
        else
        {
            m_car->print(stream);
        }
        
        for( List* it = m_cdr; it != nullptr; it=it->m_cdr )
        {
            stream << " ";
            it->m_car->print(stream);
        }

        stream << " )";
        return nullptr;
    }
};

//------------------------
// Atom
//------------------------
class Atom : public ISExpr
{
    const char* m_name;
    ISExpr*     m_value = this;

public:
    Atom( const char* name ) : m_name( copyString(name) ), m_value(this) {
        if ( strcmp(m_name,"nil") == 0 )
        {
            LOG("nil");
        }
    };
    Atom( const char* name, ISExpr* value ) : m_name( copyString(name) ), m_value(value) {};
    virtual ~Atom() { delete [] m_name; }

    Type type() const override { return ATOM; }

    virtual ISExpr* eval() override { return m_value; }

    ISExpr* print( std::ostream& stream = std::cout ) const override
    {
        stream << m_name;
        return nullptr;
    }
    const char* name() const { return m_name; }
    ISExpr*     value() const { return m_value; }
    void        setValue( ISExpr* newValue ) { m_value = newValue; }
};

//------------------------
// BuiltinFunc
//------------------------
using BuiltInLambda = std::function< ISExpr* (List*) >;

class BuiltinFunc : public ISExpr
{
    const char* m_name;
    BuiltInLambda m_lambdaFunc;
    
public:
    BuiltinFunc( const char* name, BuiltInLambda lambdaFunc ) : m_name( copyString(name) ), m_lambdaFunc(lambdaFunc) {};
    virtual ~BuiltinFunc() { delete [] m_name; }

    Type type() const override { return BUILT_IN_FUNC; }

    virtual ISExpr* eval() override { return this; }

    ISExpr* print( std::ostream& stream = std::cout ) const override
    {
        stream << m_name;
        return nullptr;
    }

    const char* name() const { return m_name; }
    BuiltInLambda func() const { return m_lambdaFunc; }
};


//------------------------
// NumberBase
//------------------------

class NumberBase: public ISExpr
{
protected:
    union
    {
        int64_t m_intValue;
        
        // !!! reserved DoubleNumber !!!
        double  m_doubleValue;
    };

public:
    
    //Type type() const override { return NUMBER_BASE; }

    virtual ISExpr* eval() override { return this; }

    ISExpr* print( std::ostream& stream = std::cout ) const override
    {
        stream << "NUMBER_BASE";
        return nullptr;
    }

    virtual int64_t intValue() = 0;
    virtual double doubleValue() = 0;

    virtual void setIntValue( int64_t ) = 0;
    virtual void setDoubleValue( double ) = 0;
};


//------------------------
// IntNumber
//------------------------

class IntNumber: public NumberBase
{
protected:
public:
    IntNumber( int64_t value ) { m_intValue = value; }
    
    Type type() const override { return INT_NUMBER; }

    virtual ISExpr* eval() override { return this; }

    ISExpr* print( std::ostream& stream = std::cout ) const override
    {
        stream << m_intValue;
        return nullptr;
    }

    virtual int64_t intValue() override { return m_intValue; }
    virtual double doubleValue() override { return m_intValue; }
    
    virtual void setIntValue( int64_t value ) { m_intValue = value; }
    virtual void setDoubleValue( double value );
};

//------------------------
// DoubleNumber
//------------------------

class Double: public NumberBase
{
public:
    Double( double value ) { m_doubleValue = value; }
    
    Type type() const override { return DOUBLE; }

    virtual ISExpr* eval() override { return this; }

    ISExpr* print( std::ostream& stream = std::cout ) const override
    {
        std::string str = std::to_string(m_doubleValue);
        str.erase ( str.find_last_not_of('0') + 1, std::string::npos );
        str.erase ( str.find_last_not_of('.') + 1, std::string::npos );
        stream << str;
        return nullptr;
    }

    virtual int64_t intValue() override { return m_doubleValue; }
    virtual double doubleValue() override { return m_doubleValue; }
    
    virtual void setIntValue( int64_t value ) { new(this) IntNumber(value); }
    virtual void setDoubleValue( double value ) { m_doubleValue = value; }
};

inline void IntNumber::setDoubleValue( double value ) { new(this) Double(value); }


//------------------------
// Custom
//------------------------

template<class T>
class Custom: public ISExpr
{
    T* m_value = nullptr;

public:
    Custom( T* value = nullptr ) : m_value(value) {}
    
    Type type() const override { return CUSTOM; }

    virtual ISExpr* eval() override { return this; }

    ISExpr* print( std::ostream& stream = std::cout ) const override
    {
        stream << "Custom";
        return nullptr;
    }

    T* value() { return m_value; }
    
    void clear() { delete m_value; m_value = nullptr; }
};

template<class T>
inline Custom<T>* to( ISExpr* expr ) { return dynamic_cast<Custom<T>*>(expr); }
