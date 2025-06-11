// This is the entry point of your game.
// You can register your scenes here, and start the game.
#include "Engine/GameEngine.hpp"
#include "Engine/LOG.hpp"
#include "Scene/LoseScene.hpp"
#include "Scene/PlayScene.hpp"
#include "Scene/StageSelectScene.hpp"
#include "Scene/WinScene.hpp"
#include "Scene/StartScene.h"
#include "Scene/SettingsScene.hpp"
#include "Scene/BattleScene.hpp"
#include "Scene/BattleScene.hpp"
#include "Scene/PreStartScene.hpp"
#include "Scene/DoorTransitionScene.hpp"
#include "Scene/LeaderboardScene.hpp"
#include "Scene/LoginScene.hpp"

int main(int argc, char **argv) {
	Engine::LOG::SetConfig(true);
	Engine::GameEngine& game = Engine::GameEngine::GetInstance();

    // Register Scenes here
    game.AddNewScene("prestart", new PreStartScene());
	game.AddNewScene("door-transition", new DoorTransitionScene());
	game.AddNewScene("start", new StartScene());
    game.AddNewScene("stage-select", new StageSelectScene());
	game.AddNewScene("play", new PlayScene());
	game.AddNewScene("lose", new LoseScene());
	game.AddNewScene("win", new WinScene());
    game.AddNewScene("settings", new SettingsScene());
    game.AddNewScene("leaderboard", new LeaderboardScene());
	game.AddNewScene("battle",new BattleScene());
	game.AddNewScene("login",new LoginScene());




    // Change the start scene
	game.Start("start", 60, 1600, 832);
	return 0;
}