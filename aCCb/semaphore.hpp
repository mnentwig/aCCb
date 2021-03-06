#include <condition_variable>
#include <mutex>

namespace aCCb {
class semaphore {
   public:
    semaphore(size_t count = 0) : count(count) {
    }

    inline void notify() {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }
    inline void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        while (count == 0) {
            cv.wait(lock);
        }
        count--;
    }

   private:
    std::mutex mtx;
    std::condition_variable cv;
    size_t count;
};
}  // namespace aCCb