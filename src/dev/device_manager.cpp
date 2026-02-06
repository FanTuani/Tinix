#include "dev/device_manager.h"

#include <algorithm>
#include <iostream>

namespace {
constexpr uint32_t kDiskDeviceId = 0;
}

DeviceManager::DeviceManager() {
    register_device(kDiskDeviceId, "disk");
}

void DeviceManager::register_device(uint32_t dev_id, std::string name) {
    auto& dev = devices_[dev_id];
    dev.name = std::move(name);
}

bool DeviceManager::has_device(uint32_t dev_id) const {
    return devices_.find(dev_id) != devices_.end();
}

bool DeviceManager::request(int pid, uint32_t dev_id) {
    auto it = devices_.find(dev_id);
    if (it == devices_.end()) {
        std::cerr << "[Dev] Invalid device id=" << dev_id << " (pid=" << pid
                  << ")\n";
        return false;
    }

    Device& dev = it->second;

    if (dev.owner_pid == -1) { // 成功分配：设备未被占用
        dev.owner_pid = pid;
        std::cerr << "[Dev] Granted dev=" << dev_id << " (" << dev.name
                  << ") to pid=" << pid << "\n";
        return true;
    }

    if (dev.owner_pid == pid) { // 成功分配：设备已被该进程占用
        std::cerr << "[Dev] Request dev=" << dev_id << " (" << dev.name
                  << ") ignored: pid=" << pid << " already owns it\n";
        return true;
    }

    // 运行至此，表示设备被其他进程占用，需排队等待

    const bool already_waiting =
        std::find(dev.wait_queue.begin(), dev.wait_queue.end(), pid) !=
        dev.wait_queue.end(); // 该进程是否已在等待此设备
    if (!already_waiting) {
        dev.wait_queue.push_back(pid);
        std::cerr << "[Dev] Queued pid=" << pid << " for dev=" << dev_id
                  << " (" << dev.name << "), owner=" << dev.owner_pid
                  << ", qlen=" << dev.wait_queue.size() << "\n";
    } else {
        std::cerr << "[Dev] Request dev=" << dev_id << " (" << dev.name
                  << ") ignored: pid=" << pid << " already queued\n";
    }

    return false;
}

std::optional<int> DeviceManager::release(int pid, uint32_t dev_id) {
    auto it = devices_.find(dev_id);
    if (it == devices_.end()) {
        std::cerr << "[Dev] Invalid device id=" << dev_id << " (release by pid="
                  << pid << ")\n";
        return std::nullopt;
    }

    Device& dev = it->second;
    if (dev.owner_pid != pid) {
        std::cerr << "[Dev] Release dev=" << dev_id << " (" << dev.name
                  << ") denied: owner=" << dev.owner_pid << ", pid=" << pid
                  << "\n";
        return std::nullopt;
    }

    dev.owner_pid = -1;

    if (dev.wait_queue.empty()) {
        std::cerr << "[Dev] Released dev=" << dev_id << " (" << dev.name
                  << ") by pid=" << pid << "\n";
        return std::nullopt;
    }

    const int next_pid = dev.wait_queue.front();
    dev.wait_queue.pop_front();
    dev.owner_pid = next_pid;

    std::cerr << "[Dev] Released dev=" << dev_id << " (" << dev.name
              << ") by pid=" << pid << ", reassigned to pid=" << next_pid
              << ", qlen=" << dev.wait_queue.size() << "\n";

    return next_pid;
}

size_t DeviceManager::cancel_wait(int pid) {
    size_t removed = 0;
    for (auto& [dev_id, dev] : devices_) {
        const size_t before = dev.wait_queue.size();
        dev.wait_queue.erase(std::remove(dev.wait_queue.begin(),
                                         dev.wait_queue.end(),
                                         pid),
                             dev.wait_queue.end());
                             // remove 仅为移动元素至末尾，需配合 erase 使用
        const size_t after = dev.wait_queue.size();
        if (after != before) {
            removed += (before - after);
            std::cerr << "[Dev] Removed pid=" << pid << " from dev=" << dev_id
                      << " (" << dev.name << ") wait queue\n";
        }
    }
    return removed;
}

std::vector<std::pair<uint32_t, std::optional<int>>> DeviceManager::release_all(
    int pid) {
    std::vector<std::pair<uint32_t, std::optional<int>>> events;

    for (auto& [dev_id, dev] : devices_) {
        if (dev.owner_pid != pid) {
            continue;
        }
        events.emplace_back(dev_id, release(pid, dev_id));
    }

    cancel_wait(pid);
    return events;
}

std::vector<DeviceManager::DeviceSnapshot> DeviceManager::snapshot() const {
    std::vector<DeviceSnapshot> out;
    out.reserve(devices_.size());

    for (const auto& [dev_id, dev] : devices_) {
        DeviceSnapshot snap;
        snap.dev_id = dev_id;
        snap.name = dev.name;
        snap.owner_pid = dev.owner_pid;
        snap.wait_queue.assign(dev.wait_queue.begin(), dev.wait_queue.end());
        out.push_back(std::move(snap));
    }

    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        return a.dev_id < b.dev_id;
    });

    return out;
}
