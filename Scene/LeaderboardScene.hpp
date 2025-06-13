// LeaderboardScene.hpp
#ifndef LEADERBOARDSCENE_HPP
#define LEADERBOARDSCENE_HPP

#include <allegro5/allegro_audio.h>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include "Engine/IScene.hpp"

namespace Engine { class Label; class ImageButton; }

class LeaderboardScene final : public Engine::IScene {
private:
    // BGM
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;

    // Data
    std::vector<std::tuple<std::string,int,std::string,std::string>> leaderboardEntries;
    int currentPage = 0;
    static constexpr int maxEntriesPerPage = 5;
    static constexpr int maxPages = 10;

    // UI pointers
    std::vector<Engine::Label*>       rankLabels;
    std::vector<Engine::Label*>       nameLabels;
    std::vector<Engine::Label*>       dateLabels;
    std::vector<Engine::Label*>       scoreLabels;
    Engine::Label*                    pageLabel   = nullptr;
    Engine::ImageButton*              prevBtn     = nullptr;
    Engine::ImageButton*              nextBtn     = nullptr;
    Engine::Label*                    prevLabel   = nullptr;
    Engine::Label*                    nextLabel   = nullptr;

    // File I/O & sorting
    void ReadLeaderboardFile(const std::string& filename);
    void WriteLeaderboardFile(const std::string& filename);
    void SortLeaderboard();

    // UI setup & updates
    void SetupStaticUI();
    void RefreshPageDisplay();

public:
    void Initialize() override;
    void Terminate() override;
    void NextPage();
    void PrevPage();
    void BackOnClick(int stage);
    static void AddScoreToLeaderboard(const std::string& playerName,
                                      int score,
                                      const std::string& filePath,
                                      const std::string& end);
};

#endif // LEADERBOARDSCENE_HPP
