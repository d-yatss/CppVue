#include "reactive.hpp"

namespace cppvue {

void DependencyTracker::track(std::shared_ptr<Dependency> dep) {
    if (activeEffect_) {
        dep->addSubscriber(activeEffect_);
        activeEffect_->dependencies_.insert(dep);
    }
}

void DependencyTracker::untrack() {
    activeEffect_ = nullptr;
}

void Dependency::addSubscriber(std::shared_ptr<Effect> effect) {
    subscribers_.insert(effect);
}

void Dependency::removeSubscriber(std::shared_ptr<Effect> effect) {
    subscribers_.erase(effect);
}

void Dependency::notify() {
    // Copie des subscribers pour éviter les problèmes de modification pendant l'itération
    auto subscribersCopy = subscribers_;
    for (const auto& effect : subscribersCopy) {
        effect->run();
    }
}

void Effect::run() {
    // Nettoyage des anciennes dépendances
    cleanup();

    // Configuration du contexte d'exécution
    auto& tracker = DependencyTracker::instance();
    auto previousEffect = tracker.currentEffect();
    tracker.activeEffect_ = shared_from_this();

    try {
        // Exécution de l'effet
        fn_();
    } catch (...) {
        // Restauration du contexte en cas d'erreur
        tracker.activeEffect_ = previousEffect;
        throw;
    }

    // Restauration du contexte
    tracker.activeEffect_ = previousEffect;
}

void Effect::cleanup() {
    // Suppression de cet effet de toutes ses dépendances
    for (const auto& dep : dependencies_) {
        dep->removeSubscriber(shared_from_this());
    }
    dependencies_.clear();
}

} // namespace cppvue
