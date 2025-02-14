#include "store.hpp"
#include <stack>
#include <mutex>

namespace cppvue {

namespace {
    // Gestionnaire global des stores
    class StoreManager {
    public:
        static StoreManager& instance() {
            static StoreManager manager;
            return manager;
        }
        
        // Enregistre un store
        void registerStore(const std::string& id, std::shared_ptr<Store> store) {
            std::lock_guard<std::mutex> lock(mutex_);
            stores_[id] = store;
        }
        
        // Récupère un store
        template<typename T>
        std::shared_ptr<T> getStore(const std::string& id) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = stores_.find(id);
            if (it != stores_.end()) {
                return std::dynamic_pointer_cast<T>(it->second);
            }
            return nullptr;
        }
        
        // Reset tous les stores
        void resetAll() {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [_, store] : stores_) {
                store->reset();
            }
        }
        
    private:
        StoreManager() = default;
        
        std::mutex mutex_;
        std::unordered_map<std::string, std::shared_ptr<Store>> stores_;
    };
    
    // Helper pour la gestion des transactions
    class Transaction {
    public:
        explicit Transaction(Store* store) : store_(store) {
            if (activeTransaction_) {
                throw std::runtime_error("Nested transactions are not supported");
            }
            activeTransaction_ = this;
        }
        
        ~Transaction() {
            activeTransaction_ = nullptr;
        }
        
        static Transaction* current() {
            return activeTransaction_;
        }
        
        void addChange(std::function<void()> undoFunction) {
            undoStack_.push(std::move(undoFunction));
        }
        
        void commit() {
            committed_ = true;
        }
        
        void rollback() {
            while (!undoStack_.empty()) {
                undoStack_.top()();
                undoStack_.pop();
            }
        }
        
    private:
        static inline thread_local Transaction* activeTransaction_ = nullptr;
        
        Store* store_;
        std::stack<std::function<void()>> undoStack_;
        bool committed_ = false;
    };
}

// Implémentation des méthodes de TypedStore

template<typename State, typename Getters, typename Actions>
void TypedStore<State, Getters, Actions>::beginTransaction() {
    new Transaction(this);
}

template<typename State, typename Getters, typename Actions>
void TypedStore<State, Getters, Actions>::commitTransaction() {
    auto transaction = Transaction::current();
    if (!transaction) {
        throw std::runtime_error("No active transaction");
    }
    transaction->commit();
    delete transaction;
}

template<typename State, typename Getters, typename Actions>
void TypedStore<State, Getters, Actions>::rollbackTransaction() {
    auto transaction = Transaction::current();
    if (!transaction) {
        throw std::runtime_error("No active transaction");
    }
    transaction->rollback();
    delete transaction;
}

template<typename State, typename Getters, typename Actions>
template<typename Patch>
void TypedStore<State, Getters, Actions>::patch(const Patch& patch) {
    auto oldState = *state_;
    
    // Si une transaction est active, enregistre l'état précédent
    if (auto transaction = Transaction::current()) {
        transaction->addChange([this, oldState]() {
            state_ = oldState;
        });
    }
    
    // Applique le patch
    applyPatch(*state_, patch);
    
    // Notifie les observateurs
    notifyObservers();
}

template<typename State, typename Getters, typename Actions>
void TypedStore<State, Getters, Actions>::subscribe(
    std::function<void(const State&)> callback) {
    observers_.push_back(std::move(callback));
}

template<typename State, typename Getters, typename Actions>
void TypedStore<State, Getters, Actions>::notifyObservers() {
    for (const auto& observer : observers_) {
        observer(*state_);
    }
}

// Helper pour créer et enregistrer un store
template<typename State>
auto createStore(const std::string& id, const State& initialState) {
    auto store = defineStore(id, initialState);
    StoreManager::instance().registerStore(id, store);
    return store;
}

// Helper pour accéder à un store existant
template<typename T>
auto useStore(const std::string& id) {
    return StoreManager::instance().getStore<TypedStore<T>>(id);
}

} // namespace cppvue
