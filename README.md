# CppVue Framework

CppVue est un framework moderne pour créer des applications web en utilisant C++ et WebAssembly. Il combine la puissance et les performances du C++ avec une syntaxe inspirée de Vue.js.

## Table des matières

- [Installation](#installation)
- [Configuration](#configuration)
- [Utilisation](#utilisation)
- [Structure d'un projet](#structure-dun-projet)
- [Composants](#composants)
- [Store](#store)
- [Extension VS Code](#extension-vs-code)
- [Exemples](#exemples)

## Installation

### Prérequis

- CMake (version 3.15 ou supérieure)
- Emscripten (pour la compilation WebAssembly)
- Un compilateur C++ moderne supportant C++20

### Installation d'Emscripten

```bash
# Cloner le SDK Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Installer et activer la dernière version
./emsdk install latest
./emsdk activate latest

# Configurer l'environnement
source ./emsdk_env.sh
```

### Installation du framework

```bash
git clone https://github.com/d-yatss/CppVue.git
cd cppvue
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
make
```

## Configuration

### CMakeLists.txt

Configuration typique pour un projet CppVue :

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_app)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration Emscripten
set(CMAKE_TOOLCHAIN_FILE "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
set(CMAKE_EXECUTABLE_SUFFIX ".js")

# Options WebAssembly
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS=['_main']")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
```

## Utilisation

### Structure d'un composant (.cvue)

Les composants CppVue utilisent une syntaxe similaire à Vue.js :

```vue
<template>
  <div class="my-component">
    <h1>{{ title }}</h1>
    <button @click="increment">Count: {{ count }}</button>
  </div>
</template>

<script>
#include <cppvue/app.hpp>

class MyComponent : public cppvue::Component {
public:
    void setup() {
        title = ref("Hello CppVue!");
        count = ref(0);
    }
    
    void increment() {
        count = count.get() + 1;
    }
    
private:
    Ref<std::string> title;
    Ref<int> count;
};
</script>

<style>
.my-component {
    padding: 20px;
}
</style>
```

### Store

Gestion de l'état avec le store :

```cpp
// store.hpp
struct AppState {
    std::vector<Todo> todos;
    std::string filter;
};

class AppStore : public cppvue::Store<AppState> {
public:
    void addTodo(const std::string& text) {
        state.todos.push_back({nextId++, text, false});
        save();
    }
    
    void toggleTodo(int id) {
        auto it = findTodo(id);
        if (it != state.todos.end()) {
            it->completed = !it->completed;
            save();
        }
    }
    
private:
    int nextId = 1;
    
    void save() {
        localStorage.setItem("todos", json::stringify(state));
    }
};
```

## Extension VS Code

### Installation de l'extension CppVue

1. Copiez le dossier d'extension dans VS Code :
```bash
cp -r vscode-cppvue ~/.vscode/extensions/vscode-cppvue-0.0.1
```

2. Redémarrez VS Code

3. Les fichiers .cvue auront maintenant la coloration syntaxique pour :
   - HTML dans les sections `<template>`
   - C++ dans les sections `<script>`
   - CSS dans les sections `<style>`

## Exemples

### Application Todo List

Un exemple complet est disponible dans `examples/todo`. Pour l'exécuter :

```bash
cd examples/todo
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
make
python3 -m http.server 8080
```

Visitez http://localhost:8080 dans votre navigateur.

## Fonctionnalités

### Composants
- Templates réactifs
- Cycle de vie des composants
- Props et événements
- Slots et scoped slots
- Directives intégrées (v-if, v-for, v-model)

### Store
- État centralisé
- Actions et mutations
- Getters calculés
- Persistance automatique

### Performance
- DOM virtuel
- Compilation WebAssembly
- Lazy loading des composants
- Mise en cache des rendus

### Développement
- Hot Module Replacement (HMR)
- Outils de débogage
- Extension VS Code
- Compilation incrémentale

## Contribution

Les contributions sont les bienvenues ! Pour contribuer :

1. Fork le projet
2. Créez une branche (`git checkout -b feature/AmazingFeature`)
3. Committez vos changements (`git commit -m 'Add AmazingFeature'`)
4. Push vers la branche (`git push origin feature/AmazingFeature`)
5. Ouvrez une Pull Request

## Licence

Ce projet est sous licence MIT. Voir le fichier [LICENSE](LICENSE) pour plus de détails.
