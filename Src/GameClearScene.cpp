/**
* @file GameClearScene.cpp
*/
#include "GameClearScene.h"
#include "TitleScene.h"
#include "GameData.h"
#include "Audio.h"

// ��������p�ϐ�.
Audio::SoundPtr gameclearbgm;

/*
* �Q�[���N���A��ʂ̏����ݒ���s��.
*
* @param scene			�Q�[���N���A��ʗp�\���̂̃|�C���^.
*
* @retval true			����������.
* @retval false			���������s.
*/
bool initialize(GameClearScene* scene) {
	scene->bg = Sprite("Res/clearbg.png");
	scene->timer = 1.0f;			// ���͂��󂯕t���Ȃ�����(�b).
									// gameoverBGM�����[�v�Đ�����.
	gameclearbgm = Audio::Engine::Instance().Prepare("Res/Audio/ji_039.mp3");
	gameclearbgm->Play(Audio::Flag_Loop);
	return true;
}

/**
* �Q�[���N���A��ʂ̏I���������s��.
*
* @param scene			�Q�[���N���A��ʗp�\���̂̃|�C���^.
*/
void finalize(GameClearScene* scene) {
	scene->bg = Sprite();
	gameclearbgm->Stop();
}

/**
* �Q�[���N���A��ʂ̃v���C���[���͂���������.
*
* @param window			�Q�[�����Ǘ�����E�B���h�E.
* @param scene			�Q�[���N���A��ʗp�\���̂̃|�C���^.
*/
void processInput(GLFWEW::WindowRef window, GameClearScene* scene) {
	window.Update();
	if (scene->timer > 0) {
		return;
	}
	// A�܂���START�{�^���������ꂽ��A�^�C�g����ʂɖ߂�.
	const GamePad gamepad = window.GetGamePad();
	if (gamepad.buttonDown & (GamePad::A | GamePad::START)) {
		finalize(scene);	// �Q�[���N���A��ʂ̌�n��.
		gamestate = gamestateTitle;
		initialize(&titleScene);
	}
}

/*
* �Q�[���N���A���X�V����.
*
* @param window			�Q�[�����Ǘ�����E�B���h�E.
* @param scene			�Q�[���N���A��ʗp�\���̂̃|�C���^.
*/
void update(GLFWEW::WindowRef window, GameClearScene* scene) {
	const float deltaTime = window.DeltaTime();

	// �^�C�}�[��0�ȉ��ɂȂ�܂ŃJ�E���g�_�E�����Ĕw�i�X�v���C�g�̍X�V.
	if (scene->timer > 0) {
		scene->timer -= deltaTime;
	}
	scene->bg.Update(deltaTime);
}

/*
* �Q�[���N���A��ʂ�`�悷��.
*
* @param window			�Q�[�����Ǘ�����E�B���h�E.
* @param scene			�^�C�g����ʗp�\���̂̃|�C���^.
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