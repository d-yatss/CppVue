#pragma once

#include <cppvue/app.hpp>
#include <string>
#include <vector>

struct Todo {
    int id;
    std::string text;
    bool completed;
};

struct TodoState {
    std::vector<Todo> todos;
    std::string filter; // "all", "active", "completed"
};

class TodoStore : public cppvue::Store<TodoState> {
public:
    TodoStore() {
        state.filter = "all";
        
        // Charger les todos depuis le localStorage
        auto stored = localStorage.getItem("todos");
        if (!stored.empty()) {
            state.todos = json::parse(stored);
        }
    }
    
    void addTodo(const std::string& text) {
        static int nextId = 1;
        state.todos.push_back({nextId++, text, false});
        saveTodos();
    }
    
    void removeTodo(int id) {
        auto it = std::find_if(state.todos.begin(), state.todos.end(),
            [id](const Todo& todo) { return todo.id == id; });
        if (it != state.todos.end()) {
            state.todos.erase(it);
            saveTodos();
        }
    }
    
    void toggleTodo(int id) {
        auto it = std::find_if(state.todos.begin(), state.todos.end(),
            [id](const Todo& todo) { return todo.id == id; });
        if (it != state.todos.end()) {
            it->completed = !it->completed;
            saveTodos();
        }
    }
    
    void editTodo(int id, const std::string& text) {
        auto it = std::find_if(state.todos.begin(), state.todos.end(),
            [id](const Todo& todo) { return todo.id == id; });
        if (it != state.todos.end()) {
            it->text = text;
            saveTodos();
        }
    }
    
    void clearCompleted() {
        state.todos.erase(
            std::remove_if(state.todos.begin(), state.todos.end(),
                [](const Todo& todo) { return todo.completed; }),
            state.todos.end());
        saveTodos();
    }
    
    void setFilter(const std::string& filter) {
        state.filter = filter;
    }
    
    std::vector<Todo> filteredTodos() const {
        if (state.filter == "active") {
            return filtered([](const Todo& todo) { return !todo.completed; });
        } else if (state.filter == "completed") {
            return filtered([](const Todo& todo) { return todo.completed; });
        }
        return state.todos;
    }
    
    int activeCount() const {
        return std::count_if(state.todos.begin(), state.todos.end(),
            [](const Todo& todo) { return !todo.completed; });
    }
    
private:
    void saveTodos() {
        localStorage.setItem("todos", json::stringify(state.todos));
    }
    
    template<typename Pred>
    std::vector<Todo> filtered(Pred pred) const {
        std::vector<Todo> result;
        std::copy_if(state.todos.begin(), state.todos.end(),
            std::back_inserter(result), pred);
        return result;
    }
};
