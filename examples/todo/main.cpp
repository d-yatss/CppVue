#include <cppvue/app.hpp>
#include "App.cvue"
#include "store/todo_store.hpp"

int main() {
    auto& app = cppvue::App::instance();
    
    // Enregistrer le store
    app.use<TodoStore>();
    
    // Monter l'application
    app.mount("#app");
    
    return 0;
}
