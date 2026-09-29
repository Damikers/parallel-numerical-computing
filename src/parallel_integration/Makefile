# === Configuration ===
CXX = clang++
CXXFLAGS = -std=c++20 -O2 -Xpreprocessor -fopenmp
LDFLAGS = -L/opt/homebrew/opt/sfml/lib -L/opt/homebrew/opt/libomp/lib
CPPFLAGS = -I/opt/homebrew/opt/sfml/include -I/opt/homebrew/opt/libomp/include
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lomp

# === Targets ===
integral_app: lab_3.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

clean:
	rm -f lab_3
