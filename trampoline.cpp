#include <functional>
#include <iostream>
#include <vector>
#include <cassert>
#include <memory>
#include "trampoline.h"

class debug_function {
public:
    debug_function() {
        printf("Debug function <%p> created\n", this);
    }

    int operator()(int a, int b, int c, int d, int e) {
        printf("Debug function <%p> called: debug_function(%d, %d, %d, %d, %d)\n", this, a, b, c, d, e);
        return 12345;
    }

    ~debug_function() {
        printf("Debug function <%p> deleted\n", this);
    }
};

int sophy();

int main() {
    { // TEST 0
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
        trampoline<int(int)> tr(print);

        trampoline<int(int)>::func_t p = tr.get();
        p(2000000000);
        b = 124;
        p(6);


        int x = 123;
        trampoline<int(int, int, int, int, char)> tr5([&](int a, int b, int c, int d, char e) {
            return printf("%d %d %d %d %c %d\n",
                          a, b, c, d, e, x);
        });
        auto p2 = tr5.get();
        p2(5, 6, 7, 8, 'a');

        x = 124;
        p2(6, 7, 8, 9, 'b');


        trampoline<int(int, int)> tr2([&](int a, int b) { return printf("%d %d %d\n", a, b, x); });
        auto q = tr2.get();
        q(15, 16);


        trampoline<int(int, int, char, int)> tr4([&](int a, int b, char c, int d) {
            return printf("%d %d %c %d %d\n",
                          a, b, c, d, x);
        });
        auto a4 = tr4.get();
        a4(6, 7, 'F', 30);

        for (int i = 0; i < 10000; i++) {
            trampoline<int(int, int, int, float, char, int, \
                    int, int, int, float, char, int)>
                    tr6([&](int a, int b, int c, float d, char e, int f, \
                            int g, int h, int i, float j, char k, int l) {
                return printf("%d %d %d %f %c %d %d %d %d %f %c %d %d\n",
                              a, b, c, d, e, f, g, h, i, j, k, l, x);
            });
            auto qq = tr6.get();
            qq(i, 0, 1, (float) i / 2, static_cast<char>(i % 26 + 'a'), 10, i, 0, 1, (float) 2 / i, static_cast<char>(i % 26 + 'a'), 42);
            if (i % 100 == 0)
                printf("\n%d\n\n", i);
        }
    }

    { // TEST 1
        debug_function tmp;
        trampoline<int(int, int, int, int, int)> t(tmp);
        assert(12345 == t.get()(1, 1, 1, 1, 1));
    }

    { // TEST 2
        std::function<char *(int *)> fun = [](int *) { return std::make_shared<char>('a').get(); };
        trampoline<char *(int *)> t(fun);
        assert ('a' == *t.get()(nullptr));
    }

    { // TEST 3
        trampoline<int(int, int, int, int, int)>
                t([&](int p0, int p1, int p2, int p3, int p4) { return p1 + p2 + p3 + p4 + p0; });
        assert (5 == t.get()(1, 1, 1, 1, 1));
    }

    { // TEST 4
        trampoline<double(double, double, double, double, double)>
                t([&](double p0, double p1, double p2, double p3, double p4) { return p1 + p2 + p3 + p4 + p0; });
        assert (5.2 == t.get()(1.0, 1.2, 1, 1, 1));
    }

    { // TEST 5
        trampoline<float(float, float, float, float, float)>
                t([&](float p0, float p1, float p2, float p3, float p4) { return p1 + p2 + p3 + p4 + p0; });
        float res = 5.2;
        assert (res == t.get()(1.0, 1.2, 1, 1, 1));
    }

    { // TEST 6
        trampoline<float(int, double, int, float, float)>
                t([&](int p0, double p1, int p2, float p3, float p4) { return (p1 + p2 + p3 + p4 + p0); });
        float res = 5.2;
        assert (res == t.get()(1, 1.2, 1, 1.0f, 1.0f));
    }

    { // TEST 7
        trampoline<int(int &)>
                t([&](int p0) { return p0; });
        int a = 1;
        assert (1 == t.get()(a));
    }

    { // TEST 8
        trampoline<long long(int, int, int, int, int, int, int, int)>
                t([&](int p0, int p1, int p2, int p3, int p4, int p5, int p6, int p7) {
            return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;
        });
        assert (8 == t.get()(1, 1, 1, 1, 1, 1, 1, 1));
    }

    { // TEST 9
        trampoline<long long(double, double, double, double, double, double, double, double)>
                t([&](double p0, double p1, double p2, double p3, double p4, double p5, double p6, double p7) {
            return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;
        });
        assert (8 == t.get()(1, 1, 1, 1, 1, 1, 1, 1));
    }

    { // TEST 10
        trampoline<long long(float, float, float, float, float, float, float, float)>
                t([&](float p0, float p1, float p2, float p3, float p4, float p5, float p6, float p7) {
            return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;
        });
        assert (8 == t.get()(1, 1, 1, 1, 1, 1, 1, 1));
    }

    { // TEST 11
        trampoline<long long(double, int, float, int, int, double, double, float)>
                t([&](double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7) {
            return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;
        });
        assert (8 == t.get()(1, 1, 1, 1, 1, 1, 1, 1));
    }

    { // TEST 12
        trampoline<long long(double &, int &, float &, int, int, double, double, float &)>
                t([&](double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7) {
            return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;
        });
        double a = 1;
        int b = 1;
        float c = 1, d = 1;
        assert (8 == t.get()(a, b, c, 1, 1, 1, 1, d));
    }

    { // TEST 13
        trampoline<float(double, int, float, int, int, double, double, float)>
                t0([&](double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7) {
            return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;
        });
        auto p0 = t0.get();

        trampoline<float(double, int, float, int, const int &, double &, double, float &)>
                t1([&](double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7) {
            return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;
        });
        const int a = 1;
        double b = 3.7;
        float c = 4.1;
        auto p1 = t1.get();
        assert ((float) (p0(1, 1, 1, 1, 1, 1, 1, 1) + 103.8) == p1(1, 2, 100, -1, a, b, 1, c));
    }

    { // TEST 14
        trampoline<float(double, int, float, int, int, double, double, float)>
                t([&](double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7) { return p2; });
        auto p = t.get();
        assert(p(1, 2, 3, 4, 5, 6, 7, 8) == 3);
    }

    { // TEST 15
        trampoline<int(double, int, float, int, int, double, double, float)>
                t([&](double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7) { return p7; });
        auto p = t.get();
        p(2, 3, 4, 5, 6, 7, 8, 9);
        assert(p(1, 2, 3, 4, 5, 6, 7, 8.8) == 8);
    }

    { // TEST 16
        typedef std::string st;
        std::function<int(st *c1, st *c2, st *c3, st *c4, st *c5, st *c6, st &c7, st *c8, st &c9)>
                fun = [](st *c1, st *c2, st *c3, st *c4, st *c5, st *c6, st &c7, st *c8, st &c9) {
            return (unsigned long) std::string("")
                    .append(*c1)
                    .append(*c2)
                    .append(*c3)
                    .append(*c4)
                    .append(*c5)
                    .append(*c6)
                    .append(c7).append(*c8).append(c9).size();
        };
        std::string sample = "testfour";
        char const *sp = sample.c_str();
        std::vector<std::string> v;
        for (size_t i = 0; i < 9; i++) {
            v.push_back(std::string(sp + i));
        }
        /*int sz = fun(&v[0], &v[1], &v[2], &v[3], &v[4], &v[5], v[6], &v[7], v[8]);
        int tr_sz = trampoline<int(st *c1, st *c2, st *c3, st *c4, st *c5, st *c6, st &c7, st *c8, st &c9)>(fun)
                .get()(&v[0], &v[1], &v[2], &v[3], &v[4], &v[5], v[6], &v[7], v[8]);
        assert(tr_sz == sz);*/
    }

    { // TEST 17
        trampoline<int(int, int)> t0 = [&](int p0, int p1) { return p0 + p1; };
        assert (2 == t0.get()(1, 1));
        trampoline<int(int, int)> t1(std::move(t0));
        assert (2 == t1.get()(1, 1));
        trampoline<int(int, int)> t2 = [&](int p0, int p1) { return p0 - p1; };
    }
}
