#pragma once
/**
* @file GameClearScene.h
*
* �Q�[���N���A��ʂ̂��߂̍\���̂���ъ֐���錾����w�b�_�t�@�C��.
*/
#include "GLFWEW.h"
#include "Sprite.h"

/**
* �Q�[���N���A��ʗp�Ŏg�p����\����.
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