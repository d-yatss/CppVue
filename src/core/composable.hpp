#pragma once

#include "reactive.hpp"
#include <functional>
#include <memory>
#include <string>
#include <tuple>

namespace cppvue {

// Système de refs comme dans Vue3
template<typename T>
class Ref : public Reactive<T> {
public:
    using Reactive<T>::Reactive;
    
    // Accès à la valeur brute
    T& value() {
        return this->get();
    }
    
    const T& value() const {
        return this->get();
    }
};

// Création de refs
template<typename T>
auto ref(T&& value) {
    return std::make_shared<Ref<std::decay_t<T>>>(std::forward<T>(value));
}

// System de computed values
template<typename T>
class Computed : public Reactive<T> {
public:
    explicit Computed(std::function<T()> getter)
        : getter_(std::move(getter)) {
        this->effect_ = createEffect([this] {
            this->set(this->getter_());
        });
    }

private:
    std::function<T()> getter_;
    std::shared_ptr<Effect> effect_;
};

template<typename F>
auto computed(F&& getter) {
    using ReturnType = std::invoke_result_t<F>;
    return std::make_shared<Computed<ReturnType>>(std::forward<F>(getter));
}

// Système de watch
template<typename T, typename F>
auto watch(const Reactive<T>& source, F&& callback) {
    return createEffect([&source, callback = std::forward<F>(callback)]() {
        callback(*source);
    });
}

// Système de provide/inject
class InjectionKey {
public:
    explicit InjectionKey(const std::string& name) : name_(name) {}
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

template<typename T>
void provide(const InjectionKey& key, T value) {
    getCurrentInstance()->provide(key, std::move(value));
}

template<typename T>
std::optional<T> inject(const InjectionKey& key) {
    return getCurrentInstance()->inject<T>(key);
}

// Helpers pour les composables
template<typename... Args>
auto defineProps() {
    return std::make_tuple(ref<Args>()...);
}

template<typename... Args>
auto defineEmits() {
    return [](const std::string& event, const Args&... args) {
        getCurrentInstance()->emit(event, args...);
    };
}

// Exemple d'utilisation d'un composable
template<typename T>
auto useCounter(T initialValue = T{}) {
    auto count = ref(initialValue);
    auto doubled = computed([count] { return *count * 2; });
    
    auto increment = [count] { *count += 1; };
    auto decrement = [count] { *count -= 1; };
    
    return std::make_tuple(count, doubled, increment, decrement);
}

} // namespace cppvue
