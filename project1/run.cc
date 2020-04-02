#include <iostream>
#include <random>

#include "run.h"


bool test_example(std::mt19937 &gen, std::uniform_real_distribution<float> &dis) {
    float A[32][16] = {{0}};
    float golden[32][16] = {{0}};
    float B[32][16] = {{0}};
    float C[32][16] = {{0}};
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            B[i][j] = dis(gen);
            C[i][j] = dis(gen);
        }
    }
    // compute golden
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            golden[i][j] = B[i][j] * C[i][j];
        }
    }
    try {
        kernel_example(B, C, A);
    } catch (...) {
        std::cout << "Failed because of runtime error\n";
        return false;
    }

    // check
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            if (std::abs(golden[i][j] - A[i][j]) >= 1e-5) {
                std::cout << "Wrong answer\n";
                return false;
            }
        }
    }
    // correct
    return true;
}


bool test_case1() {
    float A[32][16] = {{0}};
    float golden[32][16] = {{0}};
    // compute golden
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            golden[i][j] = 2;
        }
    }
    try {
        kernel_case1(A);
    } catch (...) {
        std::cout << "Failed because of runtime error\n";
        return false;
    }

    // check
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            if (std::abs(golden[i][j] - A[i][j]) >= 1e-5) {
                std::cout << "Wrong answer\n";
                return false;
            }
        }
    }
    // correct
    return true;
}


bool test_case2(std::mt19937 &gen, std::uniform_real_distribution<float> &dis) {
    float A[16][8] = {{0}};
    float golden[16][8] = {{0}};
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 8; ++j) {
            A[i][j] = dis(gen);
            golden[i][j] = A[i][j];
        }
    }
    // compute golden
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 8; ++j) {
            golden[i][j] = golden[i][j] + 2;
        }
    }
    try {
        kernel_case2(A);
    } catch (...) {
        std::cout << "Failed because of runtime error\n";
        return false;
    }

    // check
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (std::abs(golden[i][j] - A[i][j]) >= 1e-5) {
                std::cout << "Wrong answer\n";
                return false;
            }
        }
    }
    // correct
    return true;
}


bool test_case3(std::mt19937 &gen, std::uniform_real_distribution<float> &dis) {
    int A[16][32] = {{0}};
    int B[16][32] = {{0}};
    int C[16][32] = {{0}};
    int golden[16][32] = {{0}};
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            B[i][j] = static_cast<int>(dis(gen));
            C[i][j] = static_cast<int>(dis(gen));
        }
    }
    // compute golden
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            golden[i][j] = B[i][j] + C[i][j];
        }
    }
    try {
        kernel_case3(B, C, A);
    } catch (...) {
        std::cout << "Failed because of runtime error\n";
        return false;
    }

    // check
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            if (std::abs(golden[i][j] - A[i][j]) >= 1e-5) {
                std::cout << "Wrong answer\n";
                return false;
            }
        }
    }
    // correct
    return true;
}


bool test_case4(std::mt19937 &gen, std::uniform_real_distribution<float> &dis) {
    float A[16][32] = {{0}};
    float B[16][32] = {{0}};
    float C[32][32] = {{0}};
    float golden[16][8] = {{0}};
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            B[i][j] = dis(gen);
        }
    }
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 32; ++j) {
            C[i][j] = dis(gen);
        }
    }
    // compute golden
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            for (int k = 0; k < 32; ++k) {
                golden[i][j] = golden[i][j] + B[i][k] * C[k][j];
            }
        }
    }
    try {
        kernel_case4(B, C, A);
    } catch (...) {
        std::cout << "Failed because of runtime error\n";
        return false;
    }

    // check
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            if (std::abs(golden[i][j] - A[i][j]) >= 1e-5) {
                std::cout << "Wrong answer\n";
                return false;
            }
        }
    }
    // correct
    return true;
}


bool test_case5(std::mt19937 &gen, std::uniform_real_distribution<float> &dis) {
    float A[16][32] = {{0}};
    float B[16][32] = {{0}};
    float C[32][32] = {{0}};
    float D[16][32] = {{0}};
    float alpha = dis(gen);
    float beta = dis(gen);
    float golden[16][8] = {{0}};
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            B[i][j] = dis(gen);
        }
    }
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 32; ++j) {
            C[i][j] = dis(gen);
        }
    }
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            D[i][j] = dis(gen);
        }
    }
    // compute golden
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            for (int k = 0; k < 32; ++k) {
                golden[i][j] = golden[i][j] + alpha * (B[i][k] * C[k][j]);
            }
            golden[i][j] = golden[i][j] + beta * D[i][j];
        }
    }
    try {
        kernel_case5(B, C, D, alpha, beta, A);
    } catch (...) {
        std::cout << "Failed because of runtime error\n";
        return false;
    }

    // check
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            if (std::abs(golden[i][j] - A[i][j]) >= 1e-5) {
                std::cout << "Wrong answer\n";
                return false;
            }
        }
    }
    // correct
    return true;
}


bool test_case6(std::mt19937 &gen, std::uniform_real_distribution<float> &dis) {
    float A[2][8][5][5] = {{0}};
    float B[2][16][7][7] = {{0}};
    float C[8][16][3][3] = {{0}};
    float golden[2][8][5][5] = {{0}};
    for (int n = 0; n < 2; ++n) {
        for (int c = 0; c < 16; ++c) {
            for (int h = 0; h < 7; ++h) {
                for (int w = 0; w < 7; ++w) {
                    B[n][c][h][w] = dis(gen);
                }
            }
        }
    }
    for (int k = 0; k < 8; ++k) {
        for (int c = 0; c < 16; ++c) {
            for (int r = 0; r < 3; ++r) {
                for (int s = 0; s < 3; ++s) {
                    C[k][c][r][s] = dis(gen);
                }
            }
        }
    }
    // compute golden
    for (int n = 0; n < 2; ++n) {
        for (int k = 0; k < 8; ++k) {
            for (int p = 0; p < 5; ++p) {
                for (int q = 0; q < 5; ++q) {
                    
                }
            }
        }
    }
    try {
        kernel_case5(B, C, D, alpha, beta, A);
    } catch (...) {
        std::cout << "Failed because of runtime error\n";
        return false;
    }

    // check
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 32; ++j) {
            if (std::abs(golden[i][j] - A[i][j]) >= 1e-5) {
                std::cout << "Wrong answer\n";
                return false;
            }
        }
    }
    // correct
    return true;
}


int main() {
    std::random_device rd;  // get random seed
    std::mt19937 gen(rd()); // standard
    std::uniform_real_distribution<float> dis(-10, 10);
    std::cout << "Random distribution ready\n";
    // example
    bool res = test_example(gen, dis);
    if (res) {
        std::cout << "Success!\n";
    }
    return 0;
}