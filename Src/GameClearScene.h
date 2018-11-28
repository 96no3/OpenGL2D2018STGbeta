#pragma once
/**
* @file GameClearScene.h
*
* ゲームクリア画面のための構造体および関数を宣言するヘッダファイル.
*/
#include "GLFWEW.h"
#include "Sprite.h"

/**
* ゲームクリア画面用で使用する構造体.
*/
struct GameClearScene {
	Sprite bg;
	float timer;
};
bool initialize(GameClearScene*);
void finalize(GameClearScene*);
void processInput(GLFWEW::WindowRef, GameClearScene*);
void update(GLFWEW::WindowRef, GameClearScene*);
void render(GLFWEW::WindowRef, GameClearScene*);