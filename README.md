The project configuration was taken from here:
https://github.com/Pesc0/imgui-cmake.git

# The OOP Lab1 Semester2
ImGui realtime graph visualisation tool.

https://github.com/roma160/OOP_LAB2_sem2/assets/17747914/2ba05d48-cb5d-4dc8-ae1c-5eb60ba1bebb

## Building process
As this project uses several external libs ([ImGui](https://github.com/ocornut/imgui), [SDL](https://github.com/libsdl-org/SDL.git)), to build the project you will first need to init the git submodules:
```console
git submodule update --init --recursive
```

CMake is used for the project building.

Then building for PC would be done as usual:
```console
# Generate the CMake cache
cmake -S "." -B "./build"

# Build the sources
cmake --build "./build" --target main
```

### Emscripten build
https://github.com/roma160/OOP_LAB2_sem2/assets/17747914/bcdc84dc-9ddf-4d09-9cdc-7c178c31bd97

General guidelines could be found and was stolen from here: 
https://stunlock.gg/posts/emscripten_with_cmake/

The steps for the build are:
1. Install the Emscripten compiler and setup the Env variables (follow the link above for more detailed instructions);
2. Open the terminal in admin mode and go the source dir. Now use following commands:
```console
# Configure the build folder
emcmake cmake -S . -B ./build/emscripten_build

# Build the main target
cmake --build ./build/emscripten_build --target main
```
3. If there are no errors, this must be the thing. Now you could serve a Python server from a `docs` folder:
```console
python -m http.server --directory ./docs 8000
```

## Console arguments

* `--wheel` - by default mouse wheel event is disable, but you can use this flag to enable it back
* `--light` - flag to force the *light* theme
* `--dark` - flag to force the *dark* theme


## Алгоритми роботи з графами
1. Обхід графа - пошук в ширину (10%) - [code](./include/algorithms.tpp#L15)
2. Обхід графа - пошук в глибину (10%) - [code](./include/algorithms.tpp#L47)
3. [Алгоритм Пріма побудови мінімального кістякового дерева (10%)](https://en.wikipedia.org/wiki/Prim%27s_algorithm) - [code](./include/algorithms.tpp#L75)
4. [Алгоритм Дейкстри пошуку найкоротших шляхів (10%)](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm) - [code](./include/algorithms.tpp#L108)
5. [Алгоритм A* пошуку найкоротших шляхів (10%)](https://en.wikipedia.org/wiki/A*_search_algorithm) - [code](./include/algorithms.tpp#L145)
6. [Алгоритм Форда-Фалкерсона пошуку максимального потоку (10%)](https://en.wikipedia.org/wiki/Ford%E2%80%93Fulkerson_algorithm) - [code](./include/algorithms.tpp#L191)
7. Двонапрямлений алгоритм Дейкстри для пошуку потенційних друзів в соціальній мережі — тести на запропонованих даних (20%) - [code](./include/algorithms.tpp#L254)
8. Двонапрямлений алгоритм A* пошуку оптимального шляху на транспортній мережі — тести на запропонованих даних (20%) - [code](./include/algorithms.tpp#L393)
