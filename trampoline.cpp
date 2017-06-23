#include <functional>
#include <iostream>
#include "trampoline.h"

class debug_function {
public:
    debug_function() {
        printf("Debug function <%p> created\n", this);
    }

    int operator()(int n) {
        printf("Debug function <%p> called: debug_function(%d)\n", this, n);
        return 12345;
    }

    ~debug_function() {
        printf("Debug function <%p> deleted\n", this);
    }
};

int main() {
    int b = 123;

    std::function<int(int)> factorial = [&](int i) {
        return (i == 1) ? 1 : i * factorial(i - 1);
    };

    std::function<int(int)> sum = [&](int i) {
        return (i == 1) ? 1 : i + sum(i - 1);
    };

    std::function<int(int)> loopy = [&](int i) {
        printf("> %d\n", i);
        return (i == 0) ? 0 : loopy(i - 1);
    };

    std::function<int(int)> print = [&](int a) {
        return printf("%d %d\n", a, b);
    };

    debug_function fo;
    fo(12);
    trampoline<int(int)> tr(fo);

    trampoline<int(int)>::func_ptr_t p = tr.get();
    printf("TEST\n");
    p(2000000000);
    printf("TEST\n");
    /*b = 124;
    p(6);*/
}