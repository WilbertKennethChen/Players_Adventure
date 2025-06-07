//
// Created by willy on 2025/6/7.
//

#ifndef LEADERBOARDSCENE_HPP
#define LEADERBOARDSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include <tuple>

#include "Engine/IScene.hpp"

class LeaderboardScene final : public Engine::IScene {
private:
    // This scene will now manage its own BGM
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    std::vector<std::tuple<std::string, int, std::string>> leaderboardEntries; // name, score, datetime
    int currentPage = 0;
    int maxEntriesPerPage = 5;
    const int maxPages = 5;
    void ReadLeaderboardFile(const std::string& filename);
    void WriteLeaderboardFile(const std::string& filename);
    void SortLeaderboard();
    void UpdateScoreboardDisplay();

public:
    explicit LeaderboardScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
    void NextPage();
    void PrevPage();
    static void AddScoreToLeaderboard(const std::string& playerName, int score, const std::string& filePath);
};
#endif //LEADERBOARDSCENE_HPP