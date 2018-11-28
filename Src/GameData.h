#pragma once
/**
* @file GameData.h
*/
#include "Sprite.h"
#include "Font.h"

const int windowWidth = 800;				// ウィンドウの描画領域の幅.
const int windowHeight = 600;				// ウィンドウの描画領域の高さ.

extern SpriteRenderer renderer;		// スプライト描画用変数.
extern FontRenderer fontRenderer;	// フォント描画用変数.

const int gamestateTitle = 0;				// タイトル画面の場面ID.
const int gamestateMain = 1;				// ゲーム画面の場面ID.
const int gamestateGameOver = 2;			// ゲームオーバー画面の場面ID.
const int gamestateGameClear = 3;			// ゲームクリア画面の場面ID.
extern int gamestate;						// ゲームの状態.

// 先行宣言.
struct TitleScene;
struct GameOverScene;
struct GameClearScene;
struct MainScene;


extern TitleScene titleScene;
extern MainScene mainScene;
extern GameOverScene gameOverScene;
extern GameClearScene gameClearScene;