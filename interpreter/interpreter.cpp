#include "Scanner.h"
#include "Parser.h"
#include "LInterpreter.h"

#include <iostream>
#include <string>
#include <vector>

/*
 HomeWork:
 
 size_t width = 120+1;
 size_t height = 60 or 40;
 std::string text( width*height, ' ' );
 for( int i=0; i<height; i++ )
 {
     text[i*width] = '\n';
 }
 //https://symbl.cc/ru/unicode/blocks/box-drawing/
 
 char* cell( int posX, int posY ) { text[posX+posY*width]; };

 text[ posX + posY*width ] = '┌';  <--> cell(posX,posY) = '┌';

 */

struct Date
{
    int mo, da, yr;
public:
    Date(int m, int d, int y)
    {
        mo = m; da = d; yr = y;
    }
};

std::ostream& operator<<(std::ostream& os, const Date& dt)
{
    os << dt.mo << '/' << dt.da << '/' << dt.yr;
    return os;
}



using namespace std;

int main() {
    //string input = "(print (a b c))";
    //string input = "(print (print1 (a b c)) (print2 (a b c)) )";
    //string input = "(print (quote (a b c)) (quote (1 2 3)))";
    //string input = "(set x (quote a)) (print x)";
    //string input = "(set c (quote z)) (defun f(a b) (+ a c b c)) (print (f x y))";
    //string input = "(defun f(a b) (+ a b c) (+ a b c c)) (print (f x y))";
    //string input = "(print (+ (+ a b)(+ c d)))";
    //string input = "(print (+ (+ a b)(+ c d)))";
    //string input = "(print a) (print b) ";
    //string input = "(printRect (25 50))";

    string input = "(set x (quote 11)) (print (+ 25.5 x))";

    LInterpreter& lInterpreter = LInterpreter::getInstance();
    while( lInterpreter.eval(input) != nullptr ) {
    }
    std::cout << "\n\n# LInterpreter ended\n\n";
    
    
    ISExpr* str = new Custom<Date>( new Date(12,31,2023) );
    str->print("\n\nprint: ");

    auto* s = to<Date>( str );
    if ( s != nullptr )
    {
        std::cout << "\n\n#value:  " << *s->value();
        std::cout << "\n\n# ";
    }
    
    s->clear();

    return 0;
}
