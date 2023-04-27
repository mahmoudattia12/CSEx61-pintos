#include <threads/fixed-point.h>
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


