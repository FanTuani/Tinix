#pragma once
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class DeviceManager {
public:
    struct DeviceSnapshot {
        uint32_t dev_id = 0;
        std::string name;
        int owner_pid = -1;  // -1 表示空闲
        std::vector<int> wait_queue;
    };

    DeviceManager();

    void register_device(uint32_t dev_id, std::string name);
    bool has_device(uint32_t dev_id) const;

    // 申请设备：若空闲则立即分配；否则进入等待队列。
    // 返回：立即分配返回 true；进入等待队列/无效设备返回 false。
    bool request(int pid, uint32_t dev_id);

    // 释放设备：仅 owner 可释放；如有等待者则转交给队首。
    // 返回：若发生转交，返回新的 owner pid。
    std::optional<int> release(int pid, uint32_t dev_id);

    // 从所有等待队列中移除 pid，返回移除次数。
    size_t cancel_wait(int pid);

    // 释放 pid 持有的所有设备，并从等待队列中移除 pid。
    // 返回：发生释放的设备列表 (dev_id, new_owner_pid?)。
    std::vector<std::pair<uint32_t, std::optional<int>>> release_all(int pid);

    std::vector<DeviceSnapshot> snapshot() const;

private:
    struct Device {
        std::string name;
        int owner_pid = -1;  // -1 表示空闲
        std::deque<int> wait_queue;
    };

    std::unordered_map<uint32_t, Device> devices_;
};
