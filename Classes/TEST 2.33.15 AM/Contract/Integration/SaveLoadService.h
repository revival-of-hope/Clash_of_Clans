// Source: Classes/Integration/Docs/SaveLoad and Determinism API.md (SaveLoadService)
#ifndef Contract_INTEGRATION_SAVELOADSERVICE_H_
#define Contract_INTEGRATION_SAVELOADSERVICE_H_

#include <cstdint>
#include <string>

namespace Integration{
    
    class SaveLoadService {
    public:
        static SaveLoadService* GetInstance();

        std::string SaveSnapshot() const;
        bool LoadSnapshot(const std::string& blob);

        uint64_t GetDeterminismTick() const;
        uint64_t GetStateHash() const;
    };
}

#endif  // Contract_INTEGRATION_SAVELOADSERVICE_H_
