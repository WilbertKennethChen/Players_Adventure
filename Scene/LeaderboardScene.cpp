// LeaderboardScene.cpp
#include "LeaderboardScene.hpp"

#include <allegro5/allegro_audio.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <filesystem>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

// ————————————— File I/O & Sorting —————————————

void LeaderboardScene::ReadLeaderboardFile(const std::string& filename) {
    leaderboardEntries.clear();
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string name, scoreStr, datetime,endStr;
        std::getline(iss, name, ';');
        std::getline(iss, scoreStr, ';');
        std::getline(iss, datetime, ';');
        std::getline(iss, endStr);
        // trim whitespace (left as exercise)

        try {
            int score = std::stoi(scoreStr);
            leaderboardEntries.emplace_back(name, score, datetime,endStr);
        } catch (...) {
            continue;
        }
    }
    file.close();
    SortLeaderboard();
}

void LeaderboardScene::WriteLeaderboardFile(const std::string& filename) {
    std::ofstream out(filename);
    for (auto& e : leaderboardEntries) {
        out << std::get<0>(e) << ";"
            << std::get<1>(e) << ";"
            << std::get<2>(e) << ";"
            <<  std::get<3>(e) << "\n";
    }
}

void LeaderboardScene::SortLeaderboard() {
    std::sort(
      leaderboardEntries.begin(),
      leaderboardEntries.end(),
      [](auto& a, auto& b) {
          if (std::get<1>(a) != std::get<1>(b))
              return std::get<1>(a) > std::get<1>(b);
          return std::get<2>(a) > std::get<2>(b);
      }
    );
}

// ————————————— Initialization & Tear-down —————————————

void LeaderboardScene::Initialize() {
    // Play BGM once
    bgmInstance = AudioHelper::PlaySample("leaderboard.mp3", true, AudioHelper::BGMVolume);

    // Load entries
    // for (auto& path : { "Resource/scoreboard.txt", "scoreboard.txt" }) {
    std::string path = (std::filesystem::path(PROJECT_SOURCE_DIR) / "Resource/scoreboard.txt").string();
    ReadLeaderboardFile(path);
        // if (!leaderboardEntries.empty()) break;
    // }

    SetupStaticUI();
    RefreshPageDisplay();
}

void LeaderboardScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance.reset();

    rankLabels.clear();
    nameLabels.clear();
    dateLabels.clear();
    scoreLabels.clear();
    pageLabel = nullptr;
    prevBtn = nullptr;
    nextBtn = nullptr;
    prevLabel = nullptr;
    nextLabel = nullptr;

    IScene::Terminate();
}

// ————————————— UI Construction —————————————

void LeaderboardScene::SetupStaticUI() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;

    // Background + title
    AddNewObject(new Engine::Image("menu/awarding_bg.png", 0, 0, w, h, 0, 0));
    AddNewObject(new Engine::Label("Leaderboard", "undertale.ttf", 60,
                                   halfW, 80, 255,255,255,255, 0.5,0.5));

    // Column headers
    const int headerY = 160;
    AddNewObject(new Engine::Label("Rank", "undertale.ttf", 32, w*0.25f,  headerY,255,255,0,255,0.5,0.5));
    AddNewObject(new Engine::Label("Name", "undertale.ttf", 32, w*0.42f,  headerY,255,255,0,255,0.5,0.5));
    AddNewObject(new Engine::Label("Date", "undertale.ttf", 32, w*0.59f, headerY,200,200,200,255,0.5,0.5));
    AddNewObject(new Engine::Label("Score","undertale.ttf", 32, w*0.76f, headerY,255,255,0,255,0.5,0.5));

    // Persistent rows
    const int startY = 230, spacing = 80;
    for (int i = 0; i < maxEntriesPerPage; ++i) {
        int y = startY + i * spacing;
        auto* r = new Engine::Label("", "undertale.ttf",32, w*0.25f, y,255,255,255,255,0.5,0.5);
        AddNewObject(r); rankLabels.push_back(r);

        auto* n = new Engine::Label("", "undertale.ttf",32, w*0.42f, y,255,255,255,255,0.5,0.5);
        AddNewObject(n); nameLabels.push_back(n);

        auto* d = new Engine::Label("", "undertale.ttf",24, w*0.59f, y,200,200,200,255,0.5,0.5);
        AddNewObject(d); dateLabels.push_back(d);

        auto* s = new Engine::Label("", "undertale.ttf",32, w*0.76f, y,255,255,255,255,0.5,0.5);
        AddNewObject(s); scoreLabels.push_back(s);
    }

    // Page number
    pageLabel = new Engine::Label("", "undertale.ttf",36, halfW, h-170,255,255,255,255,0.5,0.5);
    AddNewObject(pageLabel);

    // Buttons (all share same Y for perfect alignment)
    const int btnY = h - 90, lblY = h - 50;
    prevBtn = new Engine::ImageButton("stage-select/ui_button.png","stage-select/ui_button_hovered.png",
                                      halfW - 320, btnY - 50, 200, 80);
    prevBtn->SetOnClickCallback(std::bind(&LeaderboardScene::PrevPage, this));
    AddNewControlObject(prevBtn);
    prevLabel = new Engine::Label("Prev","undertale.ttf",36, halfW - 220, lblY - 50,0,0,0,255,0.5,0.5);
    AddNewObject(prevLabel);

    nextBtn = new Engine::ImageButton("stage-select/ui_button.png","stage-select/ui_button_hovered.png",
                                      halfW + 120, btnY - 50, 200, 80);
    nextBtn->SetOnClickCallback(std::bind(&LeaderboardScene::NextPage, this));
    AddNewControlObject(nextBtn);
    nextLabel = new Engine::Label("Next","undertale.ttf",36, halfW + 220, lblY - 50,0,0,0,255,0.5,0.5);
    AddNewObject(nextLabel);

    auto* backBtn = new Engine::ImageButton("stage-select/ui_button.png","stage-select/ui_button_hovered.png",
                                            halfW - 100, btnY - 50, 200, 80);
    backBtn->SetOnClickCallback(std::bind(&LeaderboardScene::BackOnClick, this, 0));
    AddNewControlObject(backBtn);
    AddNewObject(new Engine::Label("Back","undertale.ttf",36, halfW, lblY - 50,0,0,0,255,0.5,0.5));
}

// ————————————— Page Refresh & Navigation —————————————

void LeaderboardScene::RefreshPageDisplay() {
    int total   = (int)leaderboardEntries.size();
    int pages   = (total + maxEntriesPerPage - 1) / maxEntriesPerPage;
    pages = std::min(pages, maxPages);

    int start = currentPage * maxEntriesPerPage;
    for (int i = 0; i < maxEntriesPerPage; ++i) {
        int idx = start + i;
        bool has = idx < total;
        rankLabels[i]->Visible = has;
        nameLabels[i]->Visible = has;
        dateLabels[i]->Visible = has;
        scoreLabels[i]->Visible = has;
        if (!has) continue;

        auto& e = leaderboardEntries[idx];
        rankLabels[i]->Text  = std::to_string(idx + 1);
        nameLabels[i]->Text  = std::get<0>(e);
        dateLabels[i]->Text  = std::get<2>(e).empty() ? "N/A" : std::get<2>(e);
        scoreLabels[i]->Text = std::to_string(std::get<1>(e));
        if (std::get<3>(e) == "good") {
            scoreLabels[i]->Color = al_map_rgb(0, 255, 0);
        } else if (std::get<3>(e) == "normal") {
            scoreLabels[i]->Color = al_map_rgb(0, 0, 255);
        } else if (std::get<3>(e) == "bad") {
            scoreLabels[i]->Color = al_map_rgb(255, 0, 0);
        }
    }

    pageLabel->Text      = "Page " + std::to_string(currentPage+1)
                           + " Of " + std::to_string(std::max(1, pages));
    prevBtn->Visible     = (currentPage > 0);
    prevLabel->Visible   = (currentPage > 0);
    nextBtn->Visible     = (currentPage < pages - 1);
    nextLabel->Visible   = (currentPage < pages - 1);
}

void LeaderboardScene::NextPage() {
    int total = (int)leaderboardEntries.size();
    int pages = (total + maxEntriesPerPage - 1) / maxEntriesPerPage;
    pages = std::min(pages, maxPages);
    if (currentPage < pages - 1) {
        ++currentPage;
        RefreshPageDisplay();
    }
}

void LeaderboardScene::PrevPage() {
    if (currentPage > 0) {
        --currentPage;
        RefreshPageDisplay();
    }
}

void LeaderboardScene::BackOnClick(int) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}

void LeaderboardScene::AddScoreToLeaderboard(const std::string& playerName,
                                             int score,
                                             const std::string& filePath,
                                             const std::string& end) {
    std::vector<std::tuple<std::string, int,std::string, std::string>> entries;

    // Read from file
    std::ifstream file(filePath);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string name, scoreStr, datetime, endStr;
        std::getline(iss, name, ';');
        std::getline(iss, scoreStr, ';');
        std::getline(iss, datetime, ';');
        std::getline(iss, endStr);
        try {
            int parsedScore = std::stoi(scoreStr);
            entries.emplace_back(name, parsedScore, datetime,endStr);
        } catch (...) {}
    }
    file.close();

    // Append new entry
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    entries.emplace_back(playerName, score, buf,end);

    // Sort entries
    std::sort(entries.begin(), entries.end(),
              [](auto& a, auto& b) {
                  if (std::get<1>(a) != std::get<1>(b))
                      return std::get<1>(a) > std::get<1>(b);
                  return std::get<2>(a) > std::get<2>(b);
              });

    // Write back
    std::ofstream out(filePath);
    for (auto& e : entries)
        out << std::get<0>(e) << ";" << std::get<1>(e) << ";" << std::get<2>(e) << ";" << std::get<3>(e) << "\n";
}

