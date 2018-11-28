#pragma once
/**
* @file GameData.h
*/
#include "Sprite.h"
#include "Font.h"

const int windowWidth = 800;				// �E�B���h�E�̕`��̈�̕�.
const int windowHeight = 600;				// �E�B���h�E�̕`��̈�̍���.

extern SpriteRenderer renderer;		// �X�v���C�g�`��p�ϐ�.
extern FontRenderer fontRenderer;	// �t�H���g�`��p�ϐ�.

const int gamestateTitle = 0;				// �^�C�g����ʂ̏��ID.
const int gamestateMain = 1;				// �Q�[����ʂ̏��ID.
const int gamestateGameOver = 2;			// �Q�[���I�[�o�[��ʂ̏��ID.
const int gamestateGameClear = 3;			// �Q�[���N���A��ʂ̏��ID.
extern int gamestate;						// �Q�[���̏��.

// ��s�錾.
struct TitleScene;
struct GameOverScene;
struct GameClearScene;
struct MainScene;


extern TitleScene titleScene;
extern MainScene mainScene;
extern GameOverScene gameOverScene;
extern GameClearScene gameClearScene;