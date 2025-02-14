#pragma once

#include <functional>
#include <vector>
#include <memory>
#include "reactive.hpp"

namespace cppvue {

// Types de hooks du cycle de vie
enum class LifecycleHook {
    BEFORE_CREATE,    // Avant la création
    CREATED,         // Après la création
    BEFORE_MOUNT,    // Avant le montage
    MOUNTED,         // Après le montage
    BEFORE_UPDATE,   // Avant la mise à jour
    UPDATED,         // Après la mise à jour
    BEFORE_UNMOUNT,  // Avant le démontage
    UNMOUNTED,       // Après le démontage
    ERROR_CAPTURED   // Capture d'erreur
};

// Gestionnaire du cycle de vie
class LifecycleManager {
public:
    using Hook = std::function<void()>;
    using ErrorHook = std::function<void(const std::exception&)>;
    
    // Enregistrement des hooks
    void onBeforeCreate(Hook hook);
    void onCreated(Hook hook);
    void onBeforeMount(Hook hook);
    void onMounted(Hook hook);
    void onBeforeUpdate(Hook hook);
    void onUpdated(Hook hook);
    void onBeforeUnmount(Hook hook);
    void onUnmounted(Hook hook);
    void onErrorCaptured(ErrorHook hook);
    
    // Exécution des hooks
    void callHook(LifecycleHook hook);
    void callErrorHook(const std::exception& error);
    
private:
    std::vector<Hook> beforeCreateHooks_;
    std::vector<Hook> createdHooks_;
    std::vector<Hook> beforeMountHooks_;
    std::vector<Hook> mountedHooks_;
    std::vector<Hook> beforeUpdateHooks_;
    std::vector<Hook> updatedHooks_;
    std::vector<Hook> beforeUnmountHooks_;
    std::vector<Hook> unmountedHooks_;
    std::vector<ErrorHook> errorHooks_;
};

// Composition API hooks
template<typename F>
void onBeforeCreate(F&& hook) {
    getCurrentInstance()->lifecycle().onBeforeCreate(std::forward<F>(hook));
}

template<typename F>
void onCreated(F&& hook) {
    getCurrentInstance()->lifecycle().onCreated(std::forward<F>(hook));
}

template<typename F>
void onBeforeMount(F&& hook) {
    getCurrentInstance()->lifecycle().onBeforeMount(std::forward<F>(hook));
}

template<typename F>
void onMounted(F&& hook) {
    getCurrentInstance()->lifecycle().onMounted(std::forward<F>(hook));
}

template<typename F>
void onBeforeUpdate(F&& hook) {
    getCurrentInstance()->lifecycle().onBeforeUpdate(std::forward<F>(hook));
}

template<typename F>
void onUpdated(F&& hook) {
    getCurrentInstance()->lifecycle().onUpdated(std::forward<F>(hook));
}

template<typename F>
void onBeforeUnmount(F&& hook) {
    getCurrentInstance()->lifecycle().onBeforeUnmount(std::forward<F>(hook));
}

template<typename F>
void onUnmounted(F&& hook) {
    getCurrentInstance()->lifecycle().onUnmounted(std::forward<F>(hook));
}

template<typename F>
void onErrorCaptured(F&& hook) {
    getCurrentInstance()->lifecycle().onErrorCaptured(std::forward<F>(hook));
}

// Watchdog pour la gestion automatique du cycle de vie
class LifecycleWatchdog {
public:
    explicit LifecycleWatchdog(Component* component);
    ~LifecycleWatchdog();
    
private:
    Component* component_;
};

} // namespace cppvue
