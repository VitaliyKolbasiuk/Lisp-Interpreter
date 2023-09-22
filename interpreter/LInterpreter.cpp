#include "LInterpreter.h"
#include <cassert>

LInterpreter* LInterpreter::gLInterpreterInstance = nullptr;

LInterpreter::LInterpreter()
{
    assert( gLInterpreterInstance == nullptr );
    gLInterpreterInstance = this;

    m_nilAtom = new Atom("nil");
    m_globalVariableMap["nil"] = m_nilAtom;

    m_parser.init( m_globalVariableMap, m_builtInFuncMap );

    addPseudoTableFuncs();

    // Add user fuction
	m_builtInFuncMap["defun"] = new BuiltinFunc( "defun", [](List* expr) -> ISExpr*
    {
        // get funcName
        auto* funcName = expr->m_car->toAtom();
        
        // set value
        funcName->setValue( expr->m_cdr );
        
        return funcName->value();
	});
    

    // quote
    m_builtInFuncMap["quote"] = new BuiltinFunc( "quote", [](List* expr) -> ISExpr* {
        return expr->m_car;
    });

    // (print (+ a b) (+ d c) )
    m_builtInFuncMap["print"] = new BuiltinFunc( "print", [](List* expr) -> ISExpr* {
        ISExpr* result = LInterpreter::instance().m_nilAtom;
        //expr->print("\ndbg: ");

        for( auto* it = expr; it != nullptr; it = it->m_cdr ) {
            result = LInterpreter::instance().eval(it->m_car);
            if ( result != nullptr )
            {
                result->print( std::cout );
            }
            else
            {
                std::cout << "NIL";
            }
            if ( it->m_cdr != nullptr )
            {
                std::cout << '_';
            }
		}

        return result;
	});

    m_builtInFuncMap["car"] = new BuiltinFunc( "car", [](List* expr) -> ISExpr* {
        if ( expr == nullptr )
        {
            return LInterpreter::instance().m_nilAtom;
        }
        return expr->m_car;
    });

    m_builtInFuncMap["cdr"] = new BuiltinFunc( "cdr", [](List* expr) -> ISExpr* {
        if ( expr == nullptr )
        {
            return LInterpreter::instance().m_nilAtom;
        }
        return expr->m_cdr;
    });

    m_builtInFuncMap["cons"] = new BuiltinFunc( "cons", [](List* expr) -> ISExpr* {
        if ( expr == nullptr )
        {
            return LInterpreter::instance().m_nilAtom;
        }
        expr->print("dbg:cons:");
        std::cout << std::endl;
        
        auto* firstArg = expr->m_car;
        auto* secondArg = expr->m_cdr->m_car->toList();

        return new List( firstArg, secondArg );
    });

   
    // (set x 1) -> 1   
    // (set x) -> nil
    // (set x 1 2 3) -> ?
    // (set x '(1 2 3)) -> (1 2 3)
    // (set x (+ a b c )) -> atom("abc")
    // (+ a (b c) d) -> atom("ad")

    m_builtInFuncMap["set"] = new BuiltinFunc( "set", [](List* expr) -> ISExpr*
    {
        if ( expr == nullptr )
        {
            return LInterpreter::instance().m_nilAtom;
        }
        auto* var   = expr->m_car->toAtom();
        auto* value = (expr->m_cdr==nullptr) ? LInterpreter::instance().m_nilAtom
                                             : LInterpreter::instance().eval( expr->m_cdr->m_car );
//        var->print0("\nvar: ");
//        if ( var == LInterpreter::instance().getAtom("1playerX") )
//        {
//            value->print0("\nvalue: ");
//        }
        var->setValue( value );
        return value;
    });

    m_builtInFuncMap["-"] = new BuiltinFunc( "-", [](List* expr) -> ISExpr*
    {
        auto* value1 = LInterpreter::instance().eval( expr->m_car );
        auto* value2 = LInterpreter::instance().eval( expr->m_cdr->m_car );
        
        if ( value1->type()==ISExpr::INT_NUMBER && value2->type()==ISExpr::INT_NUMBER )
        {
            return new IntNumber( value1->toIntNumber()->intValue() - value2->toIntNumber()->intValue() );
        }

        return new Double( value1->toNumberBase()->doubleValue() - value2->toNumberBase()->doubleValue() );
    });

    m_builtInFuncMap["/"] = new BuiltinFunc( "/", [](List* expr) -> ISExpr*
    {
        auto* value1 = LInterpreter::instance().eval( expr->m_car );
        auto* value2 = LInterpreter::instance().eval( expr->m_cdr->m_car );

        if ( value2->toNumberBase()->doubleValue() == 0 )
        {
            LOG_ERR( "divide by 0" );
        }
        return new Double( value1->toNumberBase()->doubleValue() / value2->toNumberBase()->doubleValue() );
    });

    m_builtInFuncMap["*"] = new BuiltinFunc( "*", [](List* expr) -> ISExpr*
    {
        auto* value1 = LInterpreter::instance().eval( expr->m_car );
        auto* value2 = LInterpreter::instance().eval( expr->m_cdr->m_car );

        return new Double( value1->toNumberBase()->doubleValue() * value2->toNumberBase()->doubleValue() );
    });

    // OR
    m_builtInFuncMap["OR"] = new BuiltinFunc( "OR", [](List* expr) -> ISExpr*
    {
        auto* value = LInterpreter::instance().eval( expr->m_car );
        auto* secondValue = LInterpreter::instance().eval( expr->m_cdr->m_car );
        bool first = LInterpreter::instance().isNil(value);
        bool second = LInterpreter::instance().isNil(secondValue);
        
        if ( first || second )
        {
            return expr;
        }
        
        return LInterpreter::instance().m_nilAtom;
    });

    // if
    m_builtInFuncMap["if"] = new BuiltinFunc( "if", [](List* expr) -> ISExpr*
    {
        auto* x = LInterpreter::instance().eval(expr->m_car);
        bool istrue = !LInterpreter::instance().isNil(x);
        if (istrue) {
            LInterpreter::instance().eval(expr->m_cdr->m_car);
        }
    });

    m_builtInFuncMap[">"] = new BuiltinFunc( ">", [](List* expr) -> ISExpr*
    {
        auto value1 = LInterpreter::instance().eval( expr->m_car )->toNumberBase()->doubleValue();
        auto value2 = LInterpreter::instance().eval( expr->m_cdr->m_car )->toNumberBase()->doubleValue();
        if ( value1 > value2 )
        {
            return expr;
        }
        return LInterpreter::instance().m_nilAtom;
    });

    m_builtInFuncMap["<"] = new BuiltinFunc( "<", [](List* expr) -> ISExpr*
 {
        auto value1 = LInterpreter::instance().eval( expr->m_car )->toNumberBase()->doubleValue();
        auto value2 = LInterpreter::instance().eval( expr->m_cdr->m_car )->toNumberBase()->doubleValue();
        if ( value1 < value2 )
        {
            return expr;
        }
        return LInterpreter::instance().m_nilAtom;
 });

    m_builtInFuncMap["+"] = new BuiltinFunc( "+", [](List* expr) -> ISExpr*
    {
        ISExpr* result = nullptr;
        ISExpr::Type returnType = ISExpr::INT_NUMBER;
        for (auto* it = expr; it != nullptr; it = it->m_cdr)
        {
            auto* value = LInterpreter::instance().eval( it->m_car );
            if ( value->type() < returnType )
            {
                returnType = value->type();
            }
        }
        
        switch (returnType)
        {
                // (+ "save x: " 10 567) -> "save x: 10567"
            case ISExpr::ATOM:
            {
                std::string returnValue;
                for (auto* it = expr; it != nullptr; it = it->m_cdr)
                {
                    auto* value = LInterpreter::instance().eval( it->m_car );
                    if (value->type() == ISExpr::ATOM) {
                        returnValue += value->toAtom()->name();
                    }
                    else if ( value->type() == ISExpr::DOUBLE ) {
                        std::string str = std::to_string( value->toDouble()->doubleValue() );
                        str.erase ( str.find_last_not_of('0') + 1, std::string::npos );
                        str.erase ( str.find_last_not_of('.') + 1, std::string::npos );
                        returnValue += str;
                    }
                    else if ( value->type() == ISExpr::INT_NUMBER ) {
                        returnValue += std::to_string( value->toIntNumber()->intValue() );
                    }
                }
                return new Atom( returnValue.c_str() );
            }

            case ISExpr::DOUBLE:
            {
                double returnValue = 0;

                //expr->print("+ arguments: ");
                for (auto* it = expr; it != nullptr; it = it->m_cdr)
                {
                    //it->m_car->print("\nit->m_car: ");
                    auto* value = LInterpreter::instance().eval( it->m_car );
                    //value->print("\nvalue: ");
                    assert( value->type() != ISExpr::ATOM );
                    if ( value->type() == ISExpr::DOUBLE ) {
                        returnValue += value->toDouble()->doubleValue();
                    }
                    else if ( value->type() == ISExpr::INT_NUMBER ) {
                        returnValue += value->toIntNumber()->intValue();
                    }
                }
                return new Double( returnValue );
            }

            case ISExpr::INT_NUMBER:
            {
                int64_t returnValue = 0;

                //expr->print("+ arguments: ");
                for (auto* it = expr; it != nullptr; it = it->m_cdr)
                {
                    //it->m_car->print("\nit->m_car: ");
                    auto* value = LInterpreter::instance().eval( it->m_car );
                    //value->print("\nvalue: ");
                    assert( value->type() != ISExpr::ATOM );
                    assert( value->type() != ISExpr::DOUBLE );
                    if ( value->type() == ISExpr::INT_NUMBER ) {
                        returnValue += value->toIntNumber()->intValue();
                    }
                }
                return new IntNumber( returnValue );
            }
            default:
                break;
        }
            return nullptr;
    });

}
