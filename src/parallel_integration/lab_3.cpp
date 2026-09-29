#include <SFML/Graphics.hpp>
#include <cmath>
#include <omp.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Function {
public:
    virtual ~Function() = default;
    virtual double compute(double x) const = 0;
};

class SetFunction : public Function {
public:
    double compute(double x) const override {
        return std::sin(2 * x) * std::cos(3 * x);
    }
};

class Calculator {
public:
    double calculate(double a, double b, int n, int threads, const Function& f, double& timeTaken) {
        double h = (b - a) / n;
        double sum = 0.0;
        omp_set_num_threads(threads);

        double start = omp_get_wtime();
        #pragma omp parallel for reduction(+ : sum)
        for (int i = 0; i < n; ++i) {
            double x = a + i * h + h / 2.0;
            sum += f.compute(x);
        }
        double end = omp_get_wtime();

        timeTaken = end - start;
        return sum * h;
    }
};

bool runTests() {
    std::cout << "--- Running Unit Tests ---\n";
    bool passed = true;

    {
        SetFunction f;
        double x = M_PI / 4.0;
        double expected = std::sin(2 * x) * std::cos(3 * x);
        double actual = f.compute(x);
        if (std::fabs(expected - actual) < 1e-12)
            std::cout << "[PASS] SetFunction\n";
        else {
            std::cout << "[FAIL] SetFunction  Expected: " << expected << "  Got: " << actual << "\n";
            passed = false;
        }
    }

    {
        class SquareFunction : public Function {
        public:
            double compute(double x) const override { return x * x; }
        };

        SquareFunction sq;
        Calculator calc;
        double timeTaken = 0.0;
        double result = calc.calculate(0.0, 1.0, 1'000'000, 4, sq, timeTaken);
        double expected = 1.0 / 3.0;

        if (std::fabs(result - expected) < 1e-6)
            std::cout << "[PASS] Calculator  result=" << result
                      << " time=" << timeTaken << " s\n";
        else {
            std::cout << "[FAIL] Calculator  Expected≈" << expected
                      << " Got=" << result << "\n";
            passed = false;
        }
    }

    std::cout << "--- Tests completed ---\n\n";
    return passed;
}

int main(int argc, char **argv) {
    SetFunction f;
    Calculator calc;
    double result = 0.0, timeTaken = 0.0;
    bool computed = false;

    const bool testsPassed = runTests();
    if (argc > 1 && std::string(argv[1]) == "--test") {
        return testsPassed ? 0 : 1;
    }

    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
        std::cerr << "Could not load font\n";
        return 1;
    }

    sf::RenderWindow window(sf::VideoMode({600u, 450u}), "Integral Calculator");
    window.setFramerateLimit(60);

    sf::Text title(font, "Integral of f(t) = sin(2t) * cos(3t) on [0, pi/2]", 22);
    title.setFillColor(sf::Color::Black);
    title.setPosition({80.f, 30.f});
    
    sf::Text labelN(font, "Intervals:", 18);
    labelN.setFillColor(sf::Color::Black);
    labelN.setPosition({80.f, 120.f});
    sf::Text labelThreads(font, "Threads:", 18);
    labelThreads.setFillColor(sf::Color::Black);
    labelThreads.setPosition({80.f, 200.f});

    sf::RectangleShape inputBoxN({150.f, 40.f});
    inputBoxN.setPosition({250.f, 115.f});
    inputBoxN.setFillColor(sf::Color(200, 200, 200));

    sf::RectangleShape inputBoxThreads({150.f, 40.f});
    inputBoxThreads.setPosition({250.f, 195.f});
    inputBoxThreads.setFillColor(sf::Color(200, 200, 200));

    sf::Text textN(font, "", 18);
    textN.setFillColor(sf::Color::Black);
    textN.setPosition({260.f, 120.f});

    sf::Text textThreads(font, "", 18);
    textThreads.setFillColor(sf::Color::Black);
    textThreads.setPosition({260.f, 200.f});

    sf::RectangleShape computeButton({160.f, 50.f});
    computeButton.setPosition({240.f, 280.f});
    computeButton.setFillColor(sf::Color(100, 180, 100));

    sf::Text computeText(font, "Compute", 18);
    computeText.setFillColor(sf::Color::Black);
    computeText.setPosition({computeButton.getPosition().x + 42.f, computeButton.getPosition().y + 12.f});

    sf::Text resultText(font, "", 18);
    resultText.setFillColor(sf::Color::Black);
    resultText.setPosition({80.f, 360.f});

    std::string inputN, inputThreads;
    bool enteringN = true;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mouse(
                    static_cast<float>(mousePressed->position.x),
                    static_cast<float>(mousePressed->position.y)
                );

                if (inputBoxN.getGlobalBounds().contains(mouse)) {
                    enteringN = true;
                    inputBoxN.setFillColor(sf::Color(220, 220, 220));
                    inputBoxThreads.setFillColor(sf::Color(200, 200, 200));
                } else if (inputBoxThreads.getGlobalBounds().contains(mouse)) {
                    enteringN = false;
                    inputBoxThreads.setFillColor(sf::Color(220, 220, 220));
                    inputBoxN.setFillColor(sf::Color(200, 200, 200));
                }

                else if (computeButton.getGlobalBounds().contains(mouse)) {
                    if (!inputN.empty() && !inputThreads.empty()) {
                        int n = std::stoi(inputN);
                        int threads = std::stoi(inputThreads);
                        double a = 0.0, b = M_PI / 2.0;
                        result = calc.calculate(a, b, n, threads, f, timeTaken);
                        computed = true;

                        std::ostringstream ss;
                        ss << "Result: " << std::setprecision(10) << result
                           << "\nTime: " << std::fixed << std::setprecision(6)
                           << timeTaken << " s";
                        resultText.setString(ss.str());
                    }
                }
            }

            if (auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                uint32_t unicode = textEntered->unicode;
                if (unicode == 8) {
                    if (enteringN && !inputN.empty()) inputN.pop_back();
                    else if (!enteringN && !inputThreads.empty()) inputThreads.pop_back();
                } else if (unicode >= '0' && unicode <= '9') {
                    if (enteringN) inputN += static_cast<char>(unicode);
                    else inputThreads += static_cast<char>(unicode);
                }
            }
        }

        textN.setString(inputN);
        textThreads.setString(inputThreads);

        window.clear(sf::Color(240, 240, 240));
        window.draw(title);
        window.draw(labelN);
        window.draw(labelThreads);
        window.draw(inputBoxN);
        window.draw(inputBoxThreads);
        window.draw(textN);
        window.draw(textThreads);
        window.draw(computeButton);
        window.draw(computeText);
        if (computed) window.draw(resultText);
        window.display();
    }
    return 0;
}


// #include <iostream>
// #include <vector>
// #include <random>
// #include <omp.h>

// using namespace std;

// void generateImage(vector<vector<int>>& img, int H, int W, int COLORS)
// {
//     random_device rd;
//     mt19937 gen(rd());
//     uniform_int_distribution<int> dist(0, COLORS - 1);

//     for (int i = 0; i < H; i++)
//         for (int j = 0; j < W; j++)
//             img[i][j] = dist(gen);
// }

// bool updateLabels(vector<vector<int>>& label, int H, int W, int num_threads)
// {
//     bool changed = false;

//     #pragma omp parallel num_threads(num_threads)
//     {
//         int tid = omp_get_thread_num();
//         int total = omp_get_num_threads();

//         int chunk = H / total;
//         int start = tid * chunk;
//         int end = (tid == total - 1) ? H : start + chunk;

//         bool localChanged = false;

//         for (int i = start; i < end; ++i) {
//             for (int j = 0; j < W; ++j) {

//                 int cur = label[i][j];
//                 int mx = cur;

//                 if (i > 0)     mx = max(mx, label[i-1][j]);
//                 if (i < H - 1) mx = max(mx, label[i+1][j]);
//                 if (j > 0)     mx = max(mx, label[i][j-1]);
//                 if (j < W - 1) mx = max(mx, label[i][j+1]);

//                 if (mx != cur) {
//                     label[i][j] = mx;
//                     localChanged = true;
//                 }
//             }
//         }

//         #pragma omp atomic
//         changed |= localChanged;

//         #pragma omp barrier
//     }

//     return changed;
// }


// double pulsationAlgorithm(int H, int W, int COLORS, int THREADS)
// {
//     vector<vector<int>> image(H, vector<int>(W));
//     generateImage(image, H, W, COLORS);

//     vector<vector<int>> label(H, vector<int>(W));
//     for (int i = 0; i < H; i++)
//         for (int j = 0; j < W; j++)
//             label[i][j] = i * W + j;

//     double t_start = omp_get_wtime();

//     while (true)
//     {
//         bool changed = updateLabels(label, H, W, THREADS);
//         if (!changed) break;
//     }

//     double t_end = omp_get_wtime();
//     return t_end - t_start;
// }


// int main()
// {
//     vector<pair<int,int>> sizes = {
//         {300, 300}, 
//         {600, 600}, 
//         {800, 800}
//     };

//     int COLORS = 10; 

//     for (auto s : sizes) {
//         int H = s.first;
//         int W = s.second;

//         cout << "\nMatrix size " << H << "x" << W << " \n";
//         cout << endl;

//         for (int threads = 1; threads <= 8; threads++) {
//             double time = pulsationAlgorithm(H, W, COLORS, threads);
//             cout << "Threads = " << threads
//                  << " : " << time << " sec\n";
//         }
//     }

//     return 0;
// }
