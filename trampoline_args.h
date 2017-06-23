#ifndef ASSEMBLER_TRAMPOLINE_ARGS_H
#define ASSEMBLER_TRAMPOLINE_ARGS_H

template<typename T, typename... Other>
struct trampoline_args<std::enable_if<sizeof(T) == 1, T>::type, Other...> {
    const static int num_fractional_args = args_info<Other...>::num_fractional_args;
    const static int num_not_fract_args = args_info<Other...>::num_not_fract_args + 1;
    const static bool is_valid = (sizeof(T) <= 8) & args_info<Other...>::is_valid;
};







template<typename... Args>
struct args_info;

template<>
struct args_info<> {
    const static int num_fractional_args = 0;
    const static int num_not_fract_args = 0;
    const static bool is_valid = 1;
};

template<typename First, typename... Other>
struct args_info<First, Other...> {
    const static int num_fractional_args = args_info<Other...>::num_fractional_args;
    const static int num_not_fract_args = args_info<Other...>::num_not_fract_args + 1;
    const static bool is_valid = (sizeof(First) <= 8) & args_info<Other...>::is_valid;
};

template<typename... Other>
struct args_info<std::is_floating_point<T>::value, Other...> {
    const static int num_fractional_args = args_info<Other...>::num_fractional_args + 1;
    const static int num_not_fract_args = args_info<Other...>::num_not_fract_args;
    const static bool is_valid = args_info<Other...>::is_valid;
};



#endif //ASSEMBLER_TRAMPOLINE_ARGS_H
