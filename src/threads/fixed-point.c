// #include <threads/fixed-point.h>
// #include <stdint.h>

// // x and y are fixed-point numbers, n is an integer.
// // fixed-point numbers are in signed p.q format where p + q = 31, and f is 1 << q
// int convert_to_fixed_point(int n);
// int convert_to_int(int f);
// int Add_2_Fixed_Point(int x, int y);
// int Sub_2_Fixed_Point(int x, int y);
// int Add_int_with_FP(int x, int n);
// int Sub_int_from_FP(int x, int n);
// int Multiply_2_FP(int x, int y);
// int Multiply_FP_by_int(int x, int n);
// int Divide_FPx_by_FPy(int x, int y);
// int Divide_x_by_n(int x, int n);
// int makeFP_thenDivide(int x, int y);

// /*int makeFP_thenAdd(int x, int y);
// int makeFirstFP_thenADD(int x, int y);
// int makeFP_thenMultiply(int x, int y);
// int makeFirstFP_thenMultiply(int x, int y);
// int makeFP_thenDivide(int x, int y);
// int makeFirstFP_thenDivide(int x, int y);
// int makeSecondFP_thenDivide(int x, int y);*/

// int convert_to_fixed_point(int n)
// {
//     return n * (1 << 14);
// }
// int convert_to_int(int f)
// {
//     if (f >= 0)
//     {
//         return (f + (1 << 14) / 2) / (1 << 14);
//     }
//     return (f - (1 << 14) / 2) / (1 << 14);
// }
// int Add_2_Fixed_Point(int x, int y)
// {
//     return x + y;
// }
// int Add_int_with_FP(int x, int n)
// {
//     return (x + n * (1 << 14));
// }
// int Sub_int_from_FP(int x, int n)
// {
//     return (x - (n * (1 << 14)));
// }
// int Sub_2_Fixed_Point(int x, int y)
// {
//     return x - y;
// }
// int Multiply_2_FP(int x, int y)
// {
//     (((int64_t)x) * y) / (1 << 14);
// }

// int makeFP_thenMultiply(int x, int y){
//     return (x * y) * (1 << 14);
// }

// int Multiply_FP_by_int(int x, int n)
// {
//     return (x * n);
// }
// int Divide_FPx_by_FPy(int x, int y)
// {
//     return ((int64_t)x) * (1 << 14) / y;
// }
// int Divide_x_by_n(int x, int n)
// {
//     return (x / n);
// }

// int makeFP_thenDivide(int x, int y){//divide 2 integers
//     return (x * (1 << 14) / y) ;
// }

// /*
// int makeFP_thenAdd(int n1, int n2){//Add_2_integers
//     return (x+y)*(1<<14);
// }
// int makeFirstFP_thenADD(int n, int y){//integer + FP
//     return (x*(1<<14)+y);
// }
// int makeFP_thenMultiply(int x, int y){//multiply 2 integers
//     return (x * y) * (1 << 14);
// }
// int makeFirstFP_thenMultiply(int x, int y){//
//     return x * y;
// }
// int makeFP_thenDivide(int x, int y){//divide 2 integers
//     return (x * (1 << 14) / y) ;
// }
// int makeFirstFP_thenDivide(int x, int y){//
//     return x / y;
// }
// int makeSecondFP_thenDivide(int x, int y){//
//     return x / y;
// }
// */


#include <threads/fixed-point.h>
//
// Created by louay on 5/11/22.
//
int makeFP_thenAdd(int x, int y);
int makeFirstFP_thenADD(int x, int y);
int addTwoFP(int x, int y);
int makeFP_thenMultiply(int x, int y);
int makeFirstFP_thenMultiply(int x, int y);
int multiplyTwoFP(int x, int y);
int makeFP_thenDivide(int x, int y);
int makeFirstFP_thenDivide(int x, int y);
int makeSecondFP_thenDivide(int x, int y);
int divideTwoFP(int x, int y);
int convertToFP(int x);
int convertTOInt(int x);


int makeFP_thenAdd(int x, int y){
    return (x + y) * (1 << 14);
}
int makeFirstFP_thenADD(int x, int y){
    return x * (1 << 14) + y;
}
int addTwoFP(int x, int y){
    return x + y;
}
int makeFP_thenMultiply(int x, int y){
    return (x * y) * (1 << 14);
}
int makeFirstFP_thenMultiply(int x, int y){
    return x * y;
}
int multiplyTwoFP(int x, int y){
    return ((int64_t)x) * y/ (1 << 14);
}
int makeFP_thenDivide(int x, int y){
    return (x * (1 << 14) / y) ;
}
int makeFirstFP_thenDivide(int x, int y){
    return x / y;
}
int makeSecondFP_thenDivide(int x, int y){
    return x / y;
}
int divideTwoFP(int x, int y){
    return ((int64_t)x * (1 << 14) / y);
}
int convertToFP(int x){
    return x * (1 << 14);
}
int convertTOInt(int x){
    if(x < 0) return (x - (1<<14) / 2) / (1 << 14);
    return  (x + (1<<14) / 2) / (1 << 14);
}


