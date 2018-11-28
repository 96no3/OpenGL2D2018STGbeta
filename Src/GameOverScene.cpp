/**
* @file GameOverScene.cpp
*/
#include "GameOverScene.h"
#include "TitleScene.h"
#include "GameData.h"
#include "Audio.h"

// ��������p�ϐ�.
Audio::SoundPtr gameoverbgm;

/*
* �Q�[���I�[�o�[��ʂ̏����ݒ���s��.
*
* @param scene			�Q�[���I�[�o�[��ʗp�\���̂̃|�C���^.
*
* @retval true			����������.
* @retval false			���������s.
*/
bool initialize(GameOverScene* scene) {
	scene->bg = Sprite("Res/UnknownPlanet.png");
	scene->timer = 1.0f;			// ���͂��󂯕t���Ȃ�����(�b).
									// gameoverBGM�����[�v�Đ�����.
	gameoverbgm = Audio::Engine::Instance().Prepare("Res/Audio/hushuu.mp3");
	gameoverbgm->Play(Audio::Flag_Loop);
	return true;
}

/**
* �Q�[���I�[�o�[��ʂ̏I���������s��.
*
* @param scene			�Q�[���I�[�o�[��ʗp�\���̂̃|�C���^.
*/
void finalize(GameOverScene* scene) {
	scene->bg = Sprite();
	gameoverbgm->Stop();
}

/**
* �Q�[���I�[�o�[��ʂ̃v���C���[���͂���������.
*
* @param window			�Q�[�����Ǘ�����E�B���h�E.
* @param scene			�Q�[���I�[�o�[��ʗp�\���̂̃|�C���^.
*/
void processInput(GLFWEW::WindowRef window, GameOverScene* scene) {
	window.Update();
	if (scene->timer > 0) {
		return;
	}
	// A�܂���START�{�^���������ꂽ��A�^�C�g����ʂɖ߂�.
	const GamePad gamepad = window.GetGamePad();
	if (gamepad.buttonDown & (GamePad::A | GamePad::START)) {
		finalize(scene);	// �Q�[���I�[�o�[��ʂ̌�n��.
		gamestate = gamestateTitle;
		initialize(&titleScene);
	}
}

/*
* �Q�[���I�[�o�[��ʂ��X�V����.
*
* @param window			�Q�[�����Ǘ�����E�B���h�E.
* @param scene			�Q�[���I�[�o�[��ʗp�\���̂̃|�C���^.
*/
void update(GLFWEW::WindowRef window, GameOverScene* scene) {
	const float deltaTime = window.DeltaTime();

	// �^�C�}�[��0�ȉ��ɂȂ�܂ŃJ�E���g�_�E�����Ĕw�i�X�v���C�g�̍X�V.
	if (scene->timer > 0) {
		scene->timer -= deltaTime;
	}
	scene->bg.Update(deltaTime);
}

/*
* �Q�[���I�[�o�[��ʂ�`�悷��.
*
* @param window			�Q�[�����Ǘ�����E�B���h�E.
* @param scene			�^�C�g����ʗp�\���̂̃|�C���^.
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