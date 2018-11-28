/**
* @file GameOverScene.cpp
*/
#include "GameOverScene.h"
#include "TitleScene.h"
#include "GameData.h"
#include "Audio.h"

// 音声制御用変数.
Audio::SoundPtr gameoverbgm;

/*
* ゲームオーバー画面の初期設定を行う.
*
* @param scene			ゲームオーバー画面用構造体のポインタ.
*
* @retval true			初期化成功.
* @retval false			初期化失敗.
*/
bool initialize(GameOverScene* scene) {
	scene->bg = Sprite("Res/UnknownPlanet.png");
	scene->timer = 1.0f;			// 入力を受け付けない期間(秒).
									// gameoverBGMをループ再生する.
	gameoverbgm = Audio::Engine::Instance().Prepare("Res/Audio/hushuu.mp3");
	gameoverbgm->Play(Audio::Flag_Loop);
	return true;
}

/**
* ゲームオーバー画面の終了処理を行う.
*
* @param scene			ゲームオーバー画面用構造体のポインタ.
*/
void finalize(GameOverScene* scene) {
	scene->bg = Sprite();
	gameoverbgm->Stop();
}

/**
* ゲームオーバー画面のプレイヤー入力を処理する.
*
* @param window			ゲームを管理するウィンドウ.
* @param scene			ゲームオーバー画面用構造体のポインタ.
*/
void processInput(GLFWEW::WindowRef window, GameOverScene* scene) {
	window.Update();
	if (scene->timer > 0) {
		return;
	}
	// AまたはSTARTボタンが押されたら、タイトル画面に戻る.
	const GamePad gamepad = window.GetGamePad();
	if (gamepad.buttonDown & (GamePad::A | GamePad::START)) {
		finalize(scene);	// ゲームオーバー画面の後始末.
		gamestate = gamestateTitle;
		initialize(&titleScene);
	}
}

/*
* ゲームオーバー画面を更新する.
*
* @param window			ゲームを管理するウィンドウ.
* @param scene			ゲームオーバー画面用構造体のポインタ.
*/
void update(GLFWEW::WindowRef window, GameOverScene* scene) {
	const float deltaTime = window.DeltaTime();

	// タイマーが0以下になるまでカウントダウンして背景スプライトの更新.
	if (scene->timer > 0) {
		scene->timer -= deltaTime;
	}
	scene->bg.Update(deltaTime);
}

/*
* ゲームオーバー画面を描画する.
*
* @param window			ゲームを管理するウィンドウ.
* @param scene			タイトル画面用構造体のポインタ.
*/
void render(GLFWEW::WindowRef window, GameOverScene* scene) {
	renderer.BeginUpdate();
	renderer.AddVertices(scene->bg);
	renderer.EndUpdate();
	renderer.Draw(glm::vec2(windowWidth, windowHeight));

	fontRenderer.BeginUpdate();
	fontRenderer.AddString(glm::vec2(-110, 16), "GAME OVER");
	fontRenderer.EndUpdate();
	fontRenderer.Draw();

	window.SwapBuffers();
}