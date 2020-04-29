/*
 * MIT License
 * 
 * Copyright (c) 2020 Size Zheng

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef BOOST_DEBUG_H
#define BOOST_DEBUG_H

#include <iostream>
#include <sstream>
#include <chrono>


#define CHECK(cond, ...)                                \
    {                                                   \
        if (!(cond)) {                                  \
            std::cerr << "Check failed: "               \
                      << #cond << "\n";                 \
            std::cerr << "file: "                       \
                      << __FILE__ << "\n";              \
            std::cerr << "line:"                        \
                      << __LINE__ << "\n";              \
            std::cerr << "Hint: ";                      \
            fprintf(stderr, __VA_ARGS__);               \
            abort();                                    \
        }                                               \
    }


#define ABORT(msg) {fprintf(stderr, msg); abort(); }


enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class LazyLogging {
 private:
    LogLevel log_level;
    bool do_print;
    std::string file_;
    int lineno_;
    std::ostringstream oss;
 public:
    LazyLogging() = default;
    LazyLogging(const LazyLogging &&other) : log_level(other.log_level), do_print(other.do_print) {}
    LazyLogging(LogLevel level, bool do_print=true, std::string file=__FILE__, int lineno=__LINE__) :
        log_level(level), do_print(do_print), file_(file), lineno_(lineno) {}
    ~LazyLogging() {
        std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::system_clock::now().time_since_epoch()
        );
        if (do_print) {
            switch (log_level)
            {
            case LogLevel::INFO:
                std::cerr << "[Info] " << "[time=" << ms.count() << "] ";
                break;
            case LogLevel::WARNING:
                std::cerr << "[Warning] " << "[time=" << ms.count() << "] file:"
                          << file_ << " line:" << lineno_ << " ";
                break;
            case LogLevel::ERROR:
                std::cerr << "[Error] " << "[time=" << ms.count() << "] "
                          << file_ << " line:" << lineno_ << " ";
                break;
            default:
                break;
            }
            if (oss.str().size() != 0)
                std::cerr << oss.str() << "\n";
        }
    }

    template<typename T>
    LazyLogging &operator<<(T &other) {
        oss << other;
        return *this;
    }

    template<typename T>
    LazyLogging &operator<<(T &&other) {
        oss << other;
        return *this;
    }
};


#define LOG(T) LazyLogging(LogLevel::T, true, __FILE__, __LINE__)


#define ASSERT(cond)                                                                  \
    (                                                                                 \
        [&]()-> LazyLogging {                                                         \
            if (!(cond)) {                                                            \
                return LazyLogging(LogLevel::ERROR, true, __FILE__, __LINE__);        \
            } else {                                                                  \
                return LazyLogging(LogLevel::INFO, false, __FILE__, __LINE__);        \
            }                                                                         \
        }()                                                                           \
    )                                                                                 \

/*
The following code is copied from Halide
We may change it later
*/
class debug {
 private:
    bool do_print = false;
 public:
    debug(int level) : do_print(level <= debug_level()) {}

    template<typename T>
    debug &operator<<(T &other) {
        if (do_print) {
            std::cerr << other;
        }
        return *this;
    }

    static int debug_level() {
        static int cached_debug_level = ([]() -> int {
            std::string lvl = getenv("DB_DEBUG_CODEGEN");
            return !lvl.empty() ? atoi(lvl.c_str()) : 0;
        })();
        return cached_debug_level;
    }
};


#endif  // BOOST_DEBUG_H