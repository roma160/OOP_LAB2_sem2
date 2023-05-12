The project configuration was taken from here:
https://github.com/Pesc0/imgui-cmake.git

# The OOP Lab1 Semester2
## Алгоритми роботи з графами
1. Обхід графа - пошук в ширину (10%)
2. Обхід графа - пошук в глибину (10%)
3. [Алгоритм Пріма побудови мінімального кістякового дерева (10%)](https://en.wikipedia.org/wiki/Prim%27s_algorithm)
4. [Алгоритм Дейкстри пошуку найкоротших шляхів (10%)](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm)
5. [Алгоритм A* пошуку найкоротших шляхів (10%)](https://en.wikipedia.org/wiki/A*_search_algorithm)
6. [Алгоритм Форда-Фалкерсона пошуку максимального потоку (10%)](https://en.wikipedia.org/wiki/Ford%E2%80%93Fulkerson_algorithm)
7. Двонаправлений алгоритм Дейкстри для пошуку потенційних друзів в соціальній мережі — тести на запропонованих даних (20%)
8. Двонаправлений алгоритм A* пошуку оптимального шляху на транспортній мережі — тести на запропонованих даних (20%)

# Emscripten build
General guidelines could be found and was stolen from here: 
https://stunlock.gg/posts/emscripten_with_cmake/

The steps for the build are:
1. Install the Emscripten compiler and setup the Env variables (follow the link above for more detailed instructions);
2. Open the terminal in admin mode and go the source dir. Now use following commands:
```console
# Configure the build folder
emcmake cmake -S . -B ./build/emscripten_build

# Build the main target
cmake --build ./build/emscripten --target main
```
3. If there are no errors, this must be the thing. Now you could serve a Python server from a `emscripten.index` folder:
```console
python -m http.server --directory ./emscripten.index 8000
```