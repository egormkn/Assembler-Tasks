#ifndef ASSEMBLER_TRAMPOLINE_ARGS_H
#define ASSEMBLER_TRAMPOLINE_ARGS_H

template<typename ...Args>
struct trampoline_args;

template<>
struct trampoline_args<> {
    static const int INTEGER = 0;
    static const int SSE = 0;
};

template<typename T, typename ...Args>
struct trampoline_args<T, Args ...> {
    static const int INTEGER = trampoline_args<Args ...>::INTEGER + 1;
    static const int SSE = trampoline_args<Args ...>::SSE;
    static_assert(sizeof(T) <= 8, "Arguments larger than 8 bytes are unsupported.");
};

template<typename ...Args>
struct trampoline_args<float, Args ...> {
    static const int INTEGER = trampoline_args<Args ...>::INTEGER;
    static const int SSE = trampoline_args<Args ...>::SSE + 1;
};

template<typename ... Args>
struct trampoline_args<double, Args...> {
    static const int INTEGER = trampoline_args<Args ...>::INTEGER;
    static const int SSE = trampoline_args<Args ...>::SSE + 1;
};

#endif // ASSEMBLER_TRAMPOLINE_ARGS_H