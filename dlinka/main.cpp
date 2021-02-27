#include <iostream>
#include <cmath>
#include "dlinka.h"

int main ()
{
    bignum A(sqrt (-1));
    bignum B(9.99999);
    bignum C(0.00001);
    bignum D = B;
    bignum E = B + C;
    std::cout << A << B << C << D << E;
    return 0;
}