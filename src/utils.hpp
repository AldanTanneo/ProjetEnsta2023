#ifndef _UTILS_HPP_
#define _UTILS_HPP_

namespace term_colours {
    constexpr const char * RESET = "\x1B[0m";

    // Font styles
    constexpr const char * BOLD = "\x1B[1m";
    constexpr const char * DIM = "\x1B[2m";
    constexpr const char * ITALIC = "\x1B[3m";
    constexpr const char * UNDERLINE = "\x1B[4m";
    constexpr const char * BLINKING = "\x1B[5m";
    constexpr const char * INVERSE = "\x1B[7m";
    constexpr const char * HIDDEN = "\x1B[8m";
    constexpr const char * STRIKETHROUGH = "\x1B[9m";
    constexpr const char * DOUBLE_UNDERLINE = "\x1B[21m";

    // Font styles reset
    constexpr const char * NO_BOLD = "\x1B[22m";
    constexpr const char * NO_DIM = "\x1B[22m";
    constexpr const char * NO_ITALIC = "\x1B[23m";
    constexpr const char * NO_UNDERLINE = "\x1B[24m";
    constexpr const char * NO_BLINKING = "\x1B[25m";
    constexpr const char * NO_INVERSE = "\x1B[27m";
    constexpr const char * NO_HIDDEN = "\x1B[28m";
    constexpr const char * NO_STRIKETHROUGH = "\x1B[29m";

    // Foreground colours
    constexpr const char * BLACK = "\x1B[30m";
    constexpr const char * RED = "\x1B[31m";
    constexpr const char * GREEN = "\x1B[32m";
    constexpr const char * YELLOW = "\x1B[33m";
    constexpr const char * BLUE = "\x1B[34m";
    constexpr const char * MAGENTA = "\x1B[35m";
    constexpr const char * CYAN = "\x1B[36m";
    constexpr const char * WHITE = "\x1B[37";
    constexpr const char * DEFAULT_FOREGROUND = "\x1B[39m";

    // Background colours
    constexpr const char * BLACK_BACKGROUND = "\x1B[40m";
    constexpr const char * RED_BACKGROUND = "\x1B[41m";
    constexpr const char * GREEN_BACKGROUND = "\x1B[42m";
    constexpr const char * YELLOW_BACKGROUND = "\x1B[43m";
    constexpr const char * BLUE_BACKGROUND = "\x1B[44m";
    constexpr const char * MAGENTA_BACKGROUND = "\x1B[45m";
    constexpr const char * CYAN_BACKGROUND = "\x1B[46m";
    constexpr const char * WHITE_BACKGROUND = "\x1B[47";
    constexpr const char * DEFAULT_BACKGROUND = "\x1B[49m";
} // namespace term_colours

#define STRINGIFY(args) #args

#ifdef DISABLE_DEBUGGING
    #define DEBUG(...) ;
#else
  // Debugging macro
    #define DEBUG(args, ...)                                                                       \
        __VA_OPT__(std::cout << term_colours::CYAN << term_colours::BOLD << "[" __FILE__ ":"       \
                             << __FUNCTION__ << ":" << __LINE__ << "] "                            \
                             << term_colours::DEFAULT_FOREGROUND << __VA_ARGS__                    \
                             << term_colours::NO_BOLD << std::endl;)                               \
        std::cout << term_colours::CYAN << term_colours::BOLD << "[" __FILE__ ":" << __FUNCTION__  \
                  << ":" << __LINE__ << "] " << term_colours::DEFAULT_FOREGROUND                   \
                  << term_colours::NO_BOLD << (#args) << " = " << (args) << std::endl;
#endif

#endif
