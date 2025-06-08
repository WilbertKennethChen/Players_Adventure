//
// Created by willy on 2025/6/7.
//

#include "LeaderboardScene.hpp"

#include <algorithm>
#include <allegro5/allegro_audio.h>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"

// Remove dependency on StageSelectScene since we're now self-contained
// #include "StageSelectScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

void LeaderboardScene::SortLeaderboard() {
    std::sort(leaderboardEntries.begin(), leaderboardEntries.end(),
        [](const auto& a, const auto& b) {
            // First sort by score descending
            if (std::get<1>(a) != std::get<1>(b)) {
                return std::get<1>(a) > std::get<1>(b);
            }
            // For equal scores, sort by datetime (newest first)
            return std::get<2>(a) > std::get<2>(b);
        });
}

void LeaderboardScene::ReadLeaderboardFile(const std::string& filename) {
    leaderboardEntries.clear();
    std::ifstream file(filename);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string name, scoreStr, datetime;

            // Get name (may contain spaces)
            std::getline(iss, name, ';'); // Use semicolon as delimiter
            std::getline(iss, scoreStr, ';');
            std::getline(iss, datetime);

            // Trim whitespace
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t") + 1);
            scoreStr.erase(0, scoreStr.find_first_not_of(" \t"));
            scoreStr.erase(scoreStr.find_last_not_of(" \t") + 1);
            datetime.erase(0, datetime.find_first_not_of(" \t"));
            datetime.erase(datetime.find_last_not_of(" \t") + 1);

            try {
                int score = std::stoi(scoreStr);
                leaderboardEntries.emplace_back(name, score, datetime);
            } catch (const std::invalid_argument&) {
                // Skip malformed entries
                continue;
            } catch (const std::out_of_range&) {
                // Skip entries with out of range scores
                continue;
            }
        }
        SortLeaderboard();
    }
}


void LeaderboardScene::WriteLeaderboardFile(const std::string& filename) {
    std::ofstream file(filename);
    for (const auto& entry : leaderboardEntries) {
        file << std::get<0>(entry) << ";"
             << std::get<1>(entry) << ";"
             << std::get<2>(entry) << "\n";
    }
}

void LeaderboardScene::AddScoreToLeaderboard(const std::string& playerName, int score, const std::string& filePath) {
    // Get current datetime
    time_t now = time(0);
    tm* localtm = localtime(&now);
    char datetime[30];
    strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", localtm);

    // Read existing entries
    std::vector<std::tuple<std::string, int, std::string>> entries;
    std::ifstream inFile(filePath);
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string name, scoreStr, datetime;

            std::getline(iss, name, ';');
            std::getline(iss, scoreStr, ';');
            std::getline(iss, datetime);

            // Trim whitespace
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t") + 1);
            scoreStr.erase(0, scoreStr.find_first_not_of(" \t"));
            scoreStr.erase(scoreStr.find_last_not_of(" \t") + 1);
            datetime.erase(0, datetime.find_first_not_of(" \t"));
            datetime.erase(datetime.find_last_not_of(" \t") + 1);

            try {
                int existingScore = std::stoi(scoreStr);
                entries.emplace_back(name, existingScore, datetime);
            } catch (...) {
                continue; // Skip malformed entries
            }
        }
        inFile.close();
    }

    // Add new entry
    entries.emplace_back(playerName, score, std::string(datetime));

    // Sort entries
    std::sort(entries.begin(), entries.end(),
        [](const auto& a, const auto& b) {
            if (std::get<1>(a) != std::get<1>(b)) {
                return std::get<1>(a) > std::get<1>(b);
            }
            return std::get<2>(a) > std::get<2>(b);
        });

    // Remove duplicates and keep top 100
    std::vector<std::tuple<std::string, int, std::string>> uniqueEntries;
    for (const auto& entry : entries) {
        bool exists = false;
        for (const auto& uniqueEntry : uniqueEntries) {
            if (std::get<0>(uniqueEntry) == std::get<0>(entry) &&
                std::get<1>(uniqueEntry) == std::get<1>(entry)) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            uniqueEntries.push_back(entry);
        }
    }

    if (uniqueEntries.size() > 100) {
        uniqueEntries.resize(100);
    }

    // Write to file using semicolon delimiters
    std::ofstream outFile(filePath);
    if (outFile.is_open()) {
        for (const auto& entry : uniqueEntries) {
            outFile << std::get<0>(entry) << ";"
                    << std::get<1>(entry) << ";"
                    << std::get<2>(entry) << "\n";
        }
        outFile.close();
    }
}

void LeaderboardScene::UpdateScoreboardDisplay() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;

    Clear();
    AddNewObject(new Engine::Label("LEADERBOARD", "pirulen.ttf", 60, halfW, 80, 255, 255, 255, 255, 0.5, 0.5));

    int totalEntries = leaderboardEntries.size();
    int totalPages = (totalEntries + maxEntriesPerPage - 1) / maxEntriesPerPage;
    totalPages = std::min(totalPages, maxPages);

    if (!leaderboardEntries.empty()) {
        const int entryStartY = 180;
        const int entrySpacing = 80;
        const int rankX = w * 0.20f;
        const int nameX = w * 0.40f;
        const int dateX = w * 0.65f;
        const int scoreX = w * 0.85f;

        int startIdx = currentPage * maxEntriesPerPage;
        int endIdx = std::min(startIdx + maxEntriesPerPage, (int)leaderboardEntries.size());

        AddNewObject(new Engine::Label("RANK", "pirulen.ttf", 32, rankX, 140, 255, 255, 0, 255, 0.5, 0.5));
        AddNewObject(new Engine::Label("PLAYERS", "pirulen.ttf", 32, nameX, 140, 255, 255, 0, 255, 0.5, 0.5));
        AddNewObject(new Engine::Label("DATE", "pirulen.ttf", 32, dateX, 140, 255, 255, 0, 255, 0.5, 0.5));
        AddNewObject(new Engine::Label("SCORE", "pirulen.ttf", 32, scoreX, 140, 255, 255, 0, 255, 0.5, 0.5));

        for (int i = startIdx; i < endIdx; i++) {
            const auto& entry = leaderboardEntries[i];
            int yPos = entryStartY + (i - startIdx) * entrySpacing;

            AddNewObject(new Engine::Label(std::to_string(i + 1) + ".", "pirulen.ttf", 32, rankX, yPos, 255, 255, 255, 255, 0.5, 0.5));
            AddNewObject(new Engine::Label(std::get<0>(entry), "pirulen.ttf", 32, nameX, yPos, 255, 255, 255, 255, 0.5, 0.5));

            std::string dateStr = std::get<2>(entry);
            if (dateStr.empty() || dateStr == " ") {
                dateStr = "N/A";
            }
            AddNewObject(new Engine::Label(dateStr, "pirulen.ttf", 24, dateX, yPos, 200, 200, 200, 255, 0.5, 0.5));
            AddNewObject(new Engine::Label(std::to_string(std::get<1>(entry)), "pirulen.ttf", 32, scoreX, yPos, 255, 255, 255, 255, 0.5, 0.5));
        }

        std::string pageText = "PAGE " + std::to_string(currentPage + 1) + " OF " + std::to_string(totalPages);
        AddNewObject(new Engine::Label(pageText, "pirulen.ttf", 36, halfW, h - 200, 255, 255, 255, 255, 0.5, 0.5));

        if (totalPages > 1) {
            if (currentPage > 0) {
                Engine::ImageButton* prevBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 300, h - 170, 200, 80);
                prevBtn->SetOnClickCallback(std::bind(&LeaderboardScene::PrevPage, this));
                AddNewControlObject(prevBtn);
                AddNewObject(new Engine::Label("PREV", "pirulen.ttf", 36, halfW - 200, h - 130, 0, 0, 0, 255, 0.5, 0.5));
            }

            if (currentPage < totalPages - 1) {
                Engine::ImageButton* nextBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW + 100, h - 170, 200, 80);
                nextBtn->SetOnClickCallback(std::bind(&LeaderboardScene::NextPage, this));
                AddNewControlObject(nextBtn);
                AddNewObject(new Engine::Label("NEXT", "pirulen.ttf", 36, halfW + 200, h - 130, 0, 0, 0, 255, 0.5, 0.5));
            }
        }
    } else {
        AddNewObject(new Engine::Label("No leaderboard entries found", "pirulen.ttf", 36, halfW, h / 2 - 40, 255, 255, 255, 255, 0.5, 0.5));
        AddNewObject(new Engine::Label("Play the game to add scores!", "pirulen.ttf", 28, halfW, h / 2 + 20, 255, 255, 255, 255, 0.5, 0.5));
    }

    Engine::ImageButton* backBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 100, h - 90, 200, 80);
    backBtn->SetOnClickCallback(std::bind(&LeaderboardScene::BackOnClick, this, 0));
    AddNewControlObject(backBtn);
    AddNewObject(new Engine::Label("BACK", "pirulen.ttf", 36, halfW, h - 50, 0, 0, 0, 255, 0.5, 0.5));
}

void LeaderboardScene::Initialize() {
    // Start playing the BGM for this scene. "select.ogg" is a good choice for menus.
    bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);

    std::vector<std::string> possiblePaths = {
        "Resource/scoreboard.txt",
        "./Resource/scoreboard.txt",
        "scoreboard.txt"
    };

    for (const auto& path : possiblePaths) {
        leaderboardEntries.clear();
        ReadLeaderboardFile(path);
        if (!leaderboardEntries.empty()) {
            break;
        }
    }

    UpdateScoreboardDisplay();
}

static bool changingPages = false;

void LeaderboardScene::NextPage() {
    int totalEntries = leaderboardEntries.size();
    int totalPages = (totalEntries + maxEntriesPerPage - 1) / maxEntriesPerPage;
    totalPages = std::min(totalPages, maxPages);

    if (currentPage < totalPages - 1) {
        currentPage++;
        changingPages = true;
        Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
    }
}

void LeaderboardScene::PrevPage() {
    if (currentPage > 0) {
        currentPage--;
        changingPages = true;
        Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
    }
}

void LeaderboardScene::Terminate() {
    // Stop the BGM when the scene is terminated.
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = nullptr; // Good practice to clear the shared pointer.
    IScene::Terminate();
}

void LeaderboardScene::BackOnClick(int stage) {
    // When the back button is clicked, return to the start scene.
    Engine::GameEngine::GetInstance().ChangeScene("start");
}