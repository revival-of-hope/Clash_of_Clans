#include "Classes/Contracts/Integration/SaveLoadService.h"

namespace {

std::string& SnapshotStorage() {
    static std::string snapshot = "MOCK_SNAPSHOT";
    return snapshot;
}

uint64_t& TickStorage() {
    static uint64_t tick = 0;
    return tick;
}

uint64_t HashBlob(const std::string& blob) {
    uint64_t hash = 1469598103934665603ULL;
    for (unsigned char value : blob) {
        hash ^= value;
        hash *= 1099511628211ULL;
    }
    return hash;
}

}  // namespace

SaveLoadService* SaveLoadService::GetInstance() {
    static SaveLoadService instance;
    return &instance;
}

std::string SaveLoadService::SaveSnapshot() const {
    return SnapshotStorage();
}

bool SaveLoadService::LoadSnapshot(const std::string& blob) {
    if (blob.empty()) {
        return false;
    }
    SnapshotStorage() = blob;
    TickStorage() += 1;
    return true;
}

uint64_t SaveLoadService::GetDeterminismTick() const {
    return TickStorage();
}

uint64_t SaveLoadService::GetStateHash() const {
    return HashBlob(SnapshotStorage());
}
