#pragma once

#include <functional>
#include <memory>
#include <unordered_set>
#include <any>
#include <optional>

namespace cppvue {

class Dependency;
class Effect;

// Gestionnaire des dépendances actives
class DependencyTracker {
public:
    static DependencyTracker& instance() {
        static DependencyTracker tracker;
        return tracker;
    }

    void track(std::shared_ptr<Dependency> dep);
    void untrack();
    std::shared_ptr<Effect> currentEffect() const { return activeEffect_; }

private:
    std::shared_ptr<Effect> activeEffect_;
};

// Base class pour les dépendances réactives
class Dependency : public std::enable_shared_from_this<Dependency> {
public:
    virtual ~Dependency() = default;
    void addSubscriber(std::shared_ptr<Effect> effect);
    void removeSubscriber(std::shared_ptr<Effect> effect);
    void notify();

private:
    std::unordered_set<std::shared_ptr<Effect>> subscribers_;
};

// Effet réactif qui s'exécute quand les dépendances changent
class Effect : public std::enable_shared_from_this<Effect> {
public:
    explicit Effect(std::function<void()> fn) : fn_(std::move(fn)) {}
    void run();
    void cleanup();

private:
    std::function<void()> fn_;
    std::unordered_set<std::shared_ptr<Dependency>> dependencies_;
    friend class DependencyTracker;
};

// Classe template pour les valeurs réactives
template<typename T>
class Reactive : public Dependency {
public:
    Reactive() = default;
    explicit Reactive(T value) : value_(std::move(value)) {}

    // Opérateur de déréférencement pour accéder à la valeur
    const T& operator*() const {
        DependencyTracker::instance().track(this->shared_from_this());
        return value_;
    }

    // Opérateur d'affectation pour modifier la valeur
    Reactive& operator=(const T& newValue) {
        if (value_ != newValue) {
            value_ = newValue;
            this->notify();
        }
        return *this;
    }

    // Accès aux membres pour les types complexes
    const T* operator->() const {
        DependencyTracker::instance().track(this->shared_from_this());
        return &value_;
    }

private:
    T value_;
};

// Fonction utilitaire pour créer un effet
template<typename F>
std::shared_ptr<Effect> createEffect(F&& fn) {
    auto effect = std::make_shared<Effect>(std::forward<F>(fn));
    effect->run();
    return effect;
}

} // namespace cppvue
