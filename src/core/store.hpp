#pragma once

#include "reactive.hpp"
#include "composable.hpp"
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>

namespace cppvue {

// Base class pour les stores
class Store {
public:
    virtual ~Store() = default;
    virtual void reset() = 0;
};

// Template pour créer des stores typés
template<typename State, typename Getters = void, typename Actions = void>
class TypedStore : public Store {
public:
    using StateType = State;
    using GettersType = Getters;
    using ActionsType = Actions;

    explicit TypedStore(const State& initialState) : state_(initialState) {}

    // Accès à l'état
    const Reactive<State>& state() const { return state_; }
    Reactive<State>& state() { return state_; }

    // Patch partiel de l'état
    template<typename Patch>
    void patch(const Patch& patch) {
        auto current = *state_;
        // Applique le patch récursivement
        applyPatch(current, patch);
        state_ = current;
    }

    // Reset l'état
    void reset() override {
        state_ = initialState_;
    }

    // Définition des getters
    template<typename G>
    void defineGetter(const std::string& name, G&& getter) {
        getters_[name] = [this, getter = std::forward<G>(getter)]() {
            return getter(*state_);
        };
    }

    // Définition des actions
    template<typename A>
    void defineAction(const std::string& name, A&& action) {
        actions_[name] = [this, action = std::forward<A>(action)](auto&&... args) {
            return action(*this, std::forward<decltype(args)>(args)...);
        };
    }

private:
    Reactive<State> state_;
    State initialState_;
    std::unordered_map<std::string, std::function<std::any()>> getters_;
    std::unordered_map<std::string, std::function<std::any(auto&&...)>> actions_;

    template<typename T, typename P>
    void applyPatch(T& target, const P& patch) {
        for (const auto& [key, value] : patch) {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::unordered_map<std::string, std::any>>) {
                applyPatch(target[key], value);
            } else {
                target[key] = value;
            }
        }
    }
};

// Helper pour créer un store
template<typename State>
auto defineStore(const std::string& id, const State& initialState) {
    return std::make_shared<TypedStore<State>>(initialState);
}

// Exemple d'utilisation:
/*
auto useCounter = defineStore("counter", State{
    .count = 0,
    .name = "Counter"
});

useCounter->defineGetter("doubleCount", [](const auto& state) {
    return state.count * 2;
});

useCounter->defineAction("increment", [](auto& store) {
    store.state()->count++;
});
*/

} // namespace cppvue
