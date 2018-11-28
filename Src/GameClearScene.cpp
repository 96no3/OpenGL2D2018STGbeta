/**
* @file GameClearScene.cpp
*/
#include "GameClearScene.h"
#include "TitleScene.h"
#include "GameData.h"
#include "Audio.h"

// 音声制御用変数.
Audio::SoundPtr gameclearbgm;

/*
* ゲームクリア画面の初期設定を行う.
*
* @param scene			ゲームクリア画面用構造体のポインタ.
*
* @retval true			初期化成功.
* @retval false			初期化失敗.
*/
bool initialize(GameClearScene* scene) {
	scene->bg = Sprite("Res/clearbg.png");
	scene->timer = 1.0f;			// 入力を受け付けない期間(秒).
									// gameoverBGMをループ再生する.
	gameclearbgm = Audio::Engine::Instance().Prepare("Res/Audio/ji_039.mp3");
	gameclearbgm->Play(Audio::Flag_Loop);
	return true;
}

/**
* ゲームクリア画面の終了処理を行う.
*
* @param scene			ゲームクリア画面用構造体のポインタ.
*/
void finalize(GameClearScene* scene) {
	scene->bg = Sprite();
	gameclearbgm->Stop();
}

/**
* ゲームクリア画面のプレイヤー入力を処理する.
*
* @param window			ゲームを管理するウィンドウ.
* @param scene			ゲームクリア画面用構造体のポインタ.
*/
void processInput(GLFWEW::WindowRef window, GameClearScene* scene) {
	window.Update();
	if (scene->timer > 0) {
		return;
	}
	// AまたはSTARTボタンが押されたら、タイトル画面に戻る.
	const GamePad gamepad = window.GetGamePad();
	if (gamepad.buttonDown & (GamePad::A | GamePad::START)) {
		finalize(scene);	// ゲームクリア画面の後始末.
		gamestate = gamestateTitle;
		initialize(&titleScene);
	}
}

/*
* ゲームクリアを更新する.
*
* @param window			ゲームを管理するウィンドウ.
* @param scene			ゲームクリア画面用構造体のポインタ.
*/
void update(GLFWEW::WindowRef window, GameClearScene* scene) {
	const float deltaTime = window.DeltaTime();

	// タイマーが0以下になるまでカウントダウンして背景スプライトの更新.
	if (scene->timer > 0) {
		scene->timer -= deltaTime;
	}
	scene->bg.Update(deltaTime);
}

/*
* ゲームクリア画面を描画する.
*
* @param window			ゲームを管理するウィンドウ.
* @param scene			タイトル画面用構造体のポインタ.
*/
void render(GLFWEW::WindowRef window, GameClearScene* scene) {
	renderer.BeginUpdate();
	renderer.AddVertices(scene->bg);
	renderer.EndUpdate();
	renderer.Draw(glm::vec2(windowWidth, windowHeight));

	fontRenderer.BeginUpdate();


	fontRenderer.Scale(glm::vec2(1.5f, 1.5f));
	fontRenderer.Color(glm::vec4(1.0f, 0.5f, 0, 1.0f));
	fontRenderer.Thickness(0.4f);
	fontRenderer.AddString(glm::vec2(-200, 80), "STAGE CLEAR!");
	fontRenderer.Scale(glm::vec2(1.0f, 1.0f));
	fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	fontRenderer.Thickness(0.33f);

	fontRenderer.EndUpdate();
	fontRenderer.Draw();

	window.SwapBuffers();
}