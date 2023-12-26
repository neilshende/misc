#include <condition_variable>
#include <mutex>
#include <queue>
#include <type_traits>

template <typename T>
class Channel {
public:
    Channel(int capacity = 0) : capacity_(capacity) {}

    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this] { return queue_.size() < capacity_ || closed_; });

        if (closed_) {
            throw std::runtime_error("Channel closed");
        }

        queue_.push(value);
        not_empty_.notify_one();
    }

    T Recv() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this] { return !queue_.empty() || closed_; });

        if (closed_ && queue_.empty()) {
            throw std::runtime_error("Channel closed");
        }

        T value = queue_.front();
        queue_.pop();
        not_full_.notify_one();
        return value;
    }

    void Close() {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
        not_empty_.notify_all();
        not_full_.notify_all();
    }

private:
    std::queue<T> queue_;
    int capacity_;
    bool closed_ = false;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
};

int main() {
    Channel<int> channel(5);
    std::vector<int> values = {1, 2, 3, 4, 5};

    std::thread sender([&channel, &values] {
        for (int value : values) {
            channel.Send(value);
        }
    });

    std::vector<int> received_values;
    std::thread receiver([&channel, &received_values] {
        for (int i = 0; i < values.size(); ++i) {
            received_values.push_back(channel.Recv());
        }
    });

    sender.join();
    receiver.join();

    Channel<int> channel2;
    std::thread sender2([&channel2] {
        channel2.Send(42);
    });

    int value = channel2.Recv();
    sender2.join();
    return 0;
}
