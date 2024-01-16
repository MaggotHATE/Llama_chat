#include <atomic>
#include <condition_variable>
#include <cassert>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

template <typename R>
class thread_pool {
    using func = R (int);
    static constexpr size_t max_tasks = 1024;
    using task = std::packaged_task<func>;
    std::atomic<bool>        m_abort = {}, m_flush = {};
    std::vector<std::thread> m_threads;
    std::queue<task>         m_tasks;
    std::condition_variable  m_ready, m_not_full;
    std::mutex               m_mutex;

public:
    explicit thread_pool(size_t nthreads);
    ~thread_pool();

    std::future<R> push(std::function<func> f);
    void wait();

    thread_pool(thread_pool &)  = delete;
    thread_pool(thread_pool &&) = delete;
    void operator=(thread_pool &)  = delete;
    void operator=(thread_pool &&) = delete;

protected:
    void process_tasks(int tid);
};

template <typename R>
thread_pool<R>::thread_pool(size_t nthreads) {
    m_threads.reserve(nthreads);
    for (size_t i = 0; i < nthreads; ++i) {
        m_threads.emplace_back([this, i]() { process_tasks(i); });
    }
}

template <typename R>
thread_pool<R>::~thread_pool() {
    if (m_flush) { return; }
    m_abort = true;
    m_ready.notify_all();
    for (auto & thread : m_threads) { thread.join(); }
}

template <typename R>
std::future<R> thread_pool<R>::push(std::function<func> f) {
    assert(!m_flush);
    auto t = task(std::move(f));
    auto r = t.get_future();
    std::unique_lock<std::mutex> lock(m_mutex);
    m_not_full.wait(lock, [this]() { return m_tasks.size() < max_tasks; });
    m_tasks.emplace(std::move(t));
    lock.unlock();
    m_ready.notify_one();
    return r;
}

template <typename R>
void thread_pool<R>::wait() {
    auto flush = m_flush.exchange(true);
    if (flush) { return; /* already flushing */ }
    m_ready.notify_all();
    for (auto & thread : m_threads) { thread.join(); }
}

template <typename R>
void thread_pool<R>::process_tasks(int tid) {
    task t;
    for (;;) {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (;;) {
            if (m_abort) { return; }
            if (!m_tasks.empty()) { break; }
            if (m_flush) { return; /* no more tasks */ }
            m_ready.wait(lock);
        }
        t = std::move(m_tasks.front());
        m_tasks.pop();
        lock.unlock();
        m_not_full.notify_one();

        t(tid);
    }
}
