/**
* @file Main.cpp
*/
#include "MainScene.h"
#include "TitleScene.h"
#include "GameOverScene.h"
#include "GameClearScene.h"
#include "GameData.h"
#include "Actor.h"
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"
#include "Font.h"
#include "TiledMap.h"
#include "Audio.h"
#include <glm/gtc/constants.hpp>
#include <random>


const char windowTitle[] = "OpenGL2D 2018";	// �^�C�g���o�[�ɕ\������镶��.


std::mt19937 random;				// �����𔭐�������ϐ�(�����G���W��).
int score;							// �v���C���[�̓��_.
float enemyGenerationTimer;			// ���̓G���o������܂ł̎���(�P��:�b).
float waitTimer;					// GameOverScene�Ɉڍs����܂ł̎���

const int weaponLevelMin = 1;		// ���@�̕��틭���̍Œ�i�K.
const int weaponLevelMax = 7;		// ���@�̕��틭���̍ō��i�K.
int weaponLevel;					// ���@���틭���i�K.

int laserLevel = 5;					// ���[�U�[�g�p�\�i�K.

const int weaponTypeWideShot = 0;	// �L�͈̓V���b�g.
const int weaponTypeLaser = 1;		// ���[�U�[.
int weaponType;						// �I�𒆂̕���.

// �Q�[���̏��.
int gamestate;						// ���s���̏��ID.
Actor* boss;						// �{�XActor�̃|�C���^.
bool isStagePassed;					// �X�e�[�W���N���A���Ă����true.

TitleScene titleScene;
MainScene mainScene;
GameOverScene gameOverScene;
GameClearScene gameClearScene;

// �G�̏o���𐧌䂷�邽�߂̃f�[�^.
TiledMap enemyMap;
float mapCurrentPosX;
float mapProcessedX;
bool isEndOfMap;					// �}�b�v�̏I�[�ɓ��B������true.

// ��������p�ϐ�.
Audio::SoundPtr bgm;
Audio::SoundPtr sePlayerShot;
Audio::SoundPtr sePlayerLaser;
Audio::SoundPtr seBlast;
Audio::SoundPtr seBreak;
Audio::SoundPtr sePowerUp;

const float playerWidth = 64;		// ���@�̕�.
const float playerHeight = 32;		// ���@�̍���.

SpriteRenderer renderer;			// �X�v���C�g��`�悷��I�u�W�F�N�g.
FontRenderer fontRenderer;			// �t�H���g�`��p�ϐ�.
Sprite sprBackground;				// �w�i�p�X�v���C�g.
Actor sprPlayer;					// ���@�pActor�z��.
glm::vec3 playerVelocity;			// ���@�̈ړ����x.
Actor enemyList[128];				// �G�̃��X�g.
Actor playerBulletList[128];		// ���@�̒e�̃��X�g.
Actor playerLaserList[6];			// ���@�̃��[�U�[�̃��X�g.
Actor effectList[128];				// �����Ȃǂ̓�����ʗp�X�v���C�g�̃��X�g.
Actor itemList[64];					// �A�C�e���p�X�v���C�g�̃��X�g.
Actor enemyBulletList[256];			// �G�̒e�̃��X�g.

// ���@�̃A�j���[�V����.
const FrameAnimation::KeyFrame playerKeyFrames[] = {
	{ 0.000f, glm::vec2(0, 0), glm::vec2(64, 32) },
{ 0.125f, glm::vec2(0, 64), glm::vec2(64, 32) },
{ 0.250f, glm::vec2(0, 32), glm::vec2(64, 32) },
};
FrameAnimation::TimelinePtr tlPlayer;

// �����A�j���[�V����.
const FrameAnimation::KeyFrame blastKeyFrames[] = {
	{ 0 / 60.0f, glm::vec2(416, 0), glm::vec2(32, 32) },
	{ 5 / 60.0f, glm::vec2(416, 32), glm::vec2(32, 32) },
	{ 10 / 60.0f, glm::vec2(416, 64), glm::vec2(32, 32) },
	{ 15 / 60.0f, glm::vec2(416, 96), glm::vec2(32, 32) },
	{ 20 / 60.0f, glm::vec2(416, 96), glm::vec2(32, 32) },
};
FrameAnimation::TimelinePtr tlBlast;

/*
* ���C����ʗp�̍\���̂̏����ݒ���s��.
*
* @param scene		���C����ʗp�\���̂̃|�C���^.
*
* @retval true		����������.
* @retval false		���������s.
*/
bool initialize(MainScene* scene) {
	sprBackground = Sprite("Res/UnknownPlanet.png");
	sprPlayer.spr = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));
	// ���@�̗̑�.
	sprPlayer.health = 1;
	
	//enemyGenerationTimer = 2;

	// Animator�̐ݒ�.
	sprPlayer.spr.Animator(FrameAnimation::Animate::Create(tlPlayer));

	// ���@�̓����蔻��.
	sprPlayer.collisionShape = Rect(-24, -8, 48, 16);

	// Actor�z���������.
	initializeActorList(std::begin(enemyList), std::end(enemyList));
	initializeActorList(std::begin(enemyBulletList), std::end(enemyBulletList));
	initializeActorList(std::begin(playerBulletList), std::end(playerBulletList));
	initializeActorList(std::begin(playerLaserList), std::end(playerLaserList));
	initializeActorList(std::begin(effectList), std::end(effectList));
	initializeActorList(std::begin(itemList), std::end(itemList));
	
	if (scene->stageNo == 1) {
		score = 0;
		weaponLevel = weaponLevelMin;
		weaponType = weaponTypeWideShot;
	}
	
	// �G�z�u�}�b�v��ǂݍ���.
	enemyMap.Load("Res/EnemyMap.json");
	mapCurrentPosX = mapProcessedX = windowWidth;
	isEndOfMap = false;
	boss = nullptr;
	isStagePassed = false;

	// ��������������.
	Audio::EngineRef audio = Audio::Engine::Instance();
	seBlast = audio.Prepare("Res/Audio/Blast.xwm");
	sePlayerShot = audio.Prepare("Res/Audio/PlayerShot.xwm");
	sePlayerLaser = audio.Prepare("Res/Audio/Laser.xwm");
	bgm = audio.Prepare("Res/Audio/Neolith.xwm");
	seBreak = audio.Prepare("Res/Audio/shock4.mp3");
	sePowerUp = audio.Prepare("Res/Audio/GetItem.xwm");
	// BGM�����[�v�Đ�����.
	bgm->Play(Audio::Flag_Loop);
	return true;
}

/**
* �v���g�^�C�v�錾.
*/
void processInput(GLFWEW::WindowRef);
void update(GLFWEW::WindowRef);
void render(GLFWEW::WindowRef);
void playerBulletAndEnemyContactHandler(Actor*, Actor*);
void playerLaserAndEnemyContactHandler(Actor*, Actor*);
void playerAndEnemyContactHandler(Actor*, Actor*);
void playerAndEnemyBulletContactHandler(Actor*, Actor*);
void playerAndItemContactHandler(Actor*, Actor*);
void stopPlayerLaser();
void generateObjectFromMap(float);
void updateEnemies(float);
void enemyblastAnimation(Actor*);
void playerblastAnimation(Actor*);

int randomTime(float, float);

/**
* �v���O�����̃G���g���[�|�C���g.
*/
int main()
{
	// �A�v���P�[�V�����̏�����.
	GLFWEW::WindowRef window = GLFWEW::Window::Instance();
	if (!window.Initialize(windowWidth, windowHeight, windowTitle)) {
		return 1;
	}
	// �����Đ��V�X�e���̏�����.
	Audio::EngineRef audio = Audio::Engine::Instance();
	if (!audio.Initialize()) {
		return 1;
	}
	if (!Texture::Initialize()) {
		return 1;
	}
	if (!renderer.Initialize(1024)) {
		return 1;
	}
	if (!fontRenderer.Initialize(1024,glm::vec2(windowWidth, windowHeight))) {
		return 1;
	}
	if (!fontRenderer.LoadFromFile("Res/Font/makinas_scrap.fnt")) {
		return 1;
	}

	random.seed(std::random_device()());			// �����G���W���̏�����.
	// �^�C�����C���̍쐬
	tlPlayer = FrameAnimation::Timeline::Create(playerKeyFrames);	
	tlBlast = FrameAnimation::Timeline::Create(blastKeyFrames);

	initializeMapObjectDataList();

	// �^�C�g����ʂ�����������.
	gamestate = gamestateTitle;
	initialize(&titleScene);

	// �Q�[�����[�v.
	while (!window.ShouldClose()) {
		processInput(window);
		update(window);
		render(window);
		audio.Update();
	}

	Texture::Finalize();
	audio.Destroy();
    return 0;
}

/**
* �v���C���[�̓��͂���������.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
*/
void processInput(GLFWEW::WindowRef window) {
	if (gamestate == gamestateTitle) {
		processInput(window, &titleScene);
		return;
	}
	else if (gamestate == gamestateGameOver) {
		processInput(window, &gameOverScene);
		return;
	}
	else if (gamestate == gamestateGameClear) {
		processInput(window, &gameClearScene);
		return;
	}
	window.Update();

	if (sprPlayer.health > 0) {
		// ���@�̑��x��ݒ肷��.
		const GamePad gamepad = window.GetGamePad();
		if (gamepad.buttons & GamePad::DPAD_UP) {
			playerVelocity.y = 1;
		}
		else if (gamepad.buttons & GamePad::DPAD_DOWN) {
			playerVelocity.y = -1;
		}
		else {
			playerVelocity.y = 0;
		}
		if (gamepad.buttons & GamePad::DPAD_RIGHT) {
			playerVelocity.x = 1;
		}
		else if (gamepad.buttons & GamePad::DPAD_LEFT) {
			playerVelocity.x = -1;
		}
		else {
			playerVelocity.x = 0;
		}
		if (playerVelocity.x || playerVelocity.y) {
			playerVelocity = glm::normalize(playerVelocity)* 400.0f;
		}

		// ����̐؂�ւ�.
		if((weaponLevel >= laserLevel) && (gamepad.buttonDown & GamePad::B)) {
			if (weaponType == weaponTypeWideShot) {
				weaponType = weaponTypeLaser;
			}
			else {
				weaponType = weaponTypeWideShot;
			}
		}

		// �e�̔���.
		if((weaponType == weaponTypeWideShot) && (gamepad.buttonDown & GamePad::A)) {
			for (int i = 0; i < weaponLevel; ++i) {
				// �󂢂Ă���e�̍\���̂�����.
				Actor* bullet = findAvailableActor(std::begin(playerBulletList), std::end(playerBulletList));

				// �󂢂Ă���\���̂�����������A������g���Ēe���o��������.
				if (bullet != nullptr) {
					bullet->spr = Sprite("Res/Objects.png",
						sprPlayer.spr.Position(), Rect(64, 0, 32, 16));

					const float angles[] = { 0,7.5f,-7.5f,15.0f,-15.0f,22.5f,-22.5f };
					const float radian = angles[i] / 180.0f * 3.14f;
					const float c = std::cos(radian);
					const float s = std::sin(radian);

					// �e�̈ړ�(�g�E�B�[�j���O)
					bullet->spr.Tweener(TweenAnimation::Animate::Create(
						TweenAnimation::MoveBy::Create(
							1, glm::vec3(1200 * c, 1200 * s, 0), TweenAnimation::EasingType::Linear)));
					bullet->spr.Rotation(radian);
					// �e�̓����蔻��.
					bullet->collisionShape = Rect(-8, -4, 16, 8);
					// �e�̗̑�.
					bullet->health = 1;
				}
			}
			sePlayerShot->Play();		// �e�̔��ˉ����Đ�.
		}

		// �O�����[�U�[�̔���.
		if ((weaponType == weaponTypeLaser) && (gamepad.buttons & GamePad::A)) {
			if (playerLaserList[0].health <= 0) {
				const glm::vec3 posFiringPoint = sprPlayer.spr.Position();
				playerLaserList[0].spr = Sprite("Res/Objects.png", posFiringPoint, Rect(96, 0, 32, 16));
				// ���[�U�[�̓����蔻��.
				playerLaserList[0].collisionShape = Rect(-8, -4, 16, 8);
				// ���[�U�[�̗̑�.
				playerLaserList[0].health = 1;

				playerLaserList[1].spr = Sprite("Res/Objects.png", posFiringPoint, Rect(112, 0, 32, 16));				
				playerLaserList[1].collisionShape = Rect(-8, -4, 16, 8);
				playerLaserList[1].health = 1;

				playerLaserList[2].spr = Sprite("Res/Objects.png", posFiringPoint, Rect(128, 0, 32, 16));
				playerLaserList[2].collisionShape = Rect(-8, -4, 16, 8);
				playerLaserList[2].health = 1;
				sePlayerLaser->Play(Audio::Flag_Loop);		// ���[�U�[�̔��ˉ����Đ�.
			}
		}
		else {
			// ���[�U�[���~����.
			stopPlayerLaser();
		}
		// ������[�U�[�̔���.
		if ((weaponType == weaponTypeLaser) && (weaponLevel >= weaponLevelMax) && (gamepad.buttons & GamePad::A)) {
			if (playerLaserList[3].health <= 0) {
				const glm::vec3 posFiringPoint2 = -sprPlayer.spr.Position();
				playerLaserList[3].spr = Sprite("Res/Objects.png", posFiringPoint2, Rect(96, 0, 32, 16));
				// ���[�U�[�̓����蔻��.
				playerLaserList[3].collisionShape = Rect(-8, -4, 16, 8);
				// ���[�U�[�̗̑�.
				playerLaserList[3].health = 1;

				playerLaserList[4].spr = Sprite("Res/Objects.png", posFiringPoint2, Rect(112, 0, 32, 16));
				playerLaserList[4].collisionShape = Rect(-8, -4, 16, 8);
				playerLaserList[4].health = 1;

				playerLaserList[5].spr = Sprite("Res/Objects.png", posFiringPoint2, Rect(128, 0, 32, 16));
				playerLaserList[5].collisionShape = Rect(-8, -4, 16, 8);
				playerLaserList[5].health = 1;				
			}
		}
	}
	else {
		playerVelocity = glm::vec3(0, 0, 0);
		stopPlayerLaser();
	}	
}


/**
* �Q�[���̏�Ԃ��X�V����.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
*/
void update(GLFWEW::WindowRef window) {
	if (gamestate == gamestateTitle) {
		update(window, &titleScene);
		return;
	}
	else if (gamestate == gamestateMain) {
		
		// ���@���j�󂳂�Ă�����Q�[���I�[�o�[��ʂ�\������.
		if (sprPlayer.health <= 0) {
			const float deltaTime = window.DeltaTime();	// �O��̍X�V����̌o�ߎ��ԁi�b�j.			
			waitTimer -= deltaTime;
			// �^�C�}�[��0�ȉ��ɂȂ�܂ŃJ�E���g�_�E��.
			if (waitTimer < 0) {				
				bgm->Stop();	// BGM���~����.
				gamestate = gamestateGameOver;
				initialize(&gameOverScene);
				return;
			}
		}
	}
	else if (gamestate == gamestateGameOver) {
		update(window, &gameOverScene);
		return;
	}
	else if (gamestate == gamestateGameClear) {
		update(window, &gameClearScene);
		return;
	}

	// �X�e�[�W�N���A����.
	if (!isStagePassed && boss && (boss->health <= 0)) {
		isStagePassed = true;
		boss = nullptr;
		waitTimer = 2.0f;
	}
	if (isStagePassed) {
		const float deltaTime = window.DeltaTime();	// �O��̍X�V����̌o�ߎ��ԁi�b�j.	
		waitTimer -= deltaTime;
		if (waitTimer <= 0) {
			stopPlayerLaser();
			++mainScene.stageNo;
			bgm->Stop();	// BGM���~����.
			gamestate = gamestateGameClear;
			initialize(&gameClearScene);
			return;
		}
	}

	const float deltaTime = window.DeltaTime();	// �O��̍X�V����̌o�ߎ��ԁi�b�j.	

	if (sprPlayer.health > 0) {
		// ���@�̈ړ�.
		if (playerVelocity.x || playerVelocity.y) {
			glm::vec3 newPos = sprPlayer.spr.Position() + playerVelocity * deltaTime;
			// ���@�̈ړ��͈͂���ʓ��ɐ�������.
			const Rect playerRect = sprPlayer.spr.Rectangle();
			if (newPos.x < -0.5f * (windowWidth - playerRect.size.x)) {
				newPos.x = -0.5f * (windowWidth - playerRect.size.x);
			}
			else if (newPos.x > 0.5f * (windowWidth - playerRect.size.x)) {
				newPos.x = 0.5f * (windowWidth - playerRect.size.x);
			}
			if (newPos.y < -0.5f * (windowHeight - playerRect.size.y)) {
				newPos.y = -0.5f * (windowHeight - playerRect.size.y);
			}
			else if (newPos.y > 0.5f * (windowHeight - playerRect.size.y)) {
				newPos.y = 0.5f * (windowHeight - playerRect.size.y);
			}
			sprPlayer.spr.Position(newPos);
		}
		sprPlayer.spr.Update(deltaTime);
	}
	else {
		stopPlayerLaser();
	}

	//// ���_�ɉ����Ď��@�̕������������.
	//weaponLevel = weaponLevelMin + score / 2000;
	//if (weaponLevel > weaponLevelMax) {
	//	weaponLevel = weaponLevelMax;
	//}

	// �G�̏o��.
	#if 1

	generateObjectFromMap(deltaTime);

	#else

	// �o���܂ł̎��Ԃ�0�ȉ��ɂȂ�����G���o��������.
	enemyGenerationTimer -= deltaTime;
	if (enemyGenerationTimer <= 0) {
		// �󂢂Ă���(�j�󂳂�Ă���)�G�\���̂�����.
		Actor* enemy = nullptr;
		for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
			if (i->health <= 0) {
				enemy = i;
				break;
			}
		}
		// �󂢂Ă���\���̂�����������A������g���ēG���o��������.
		if (enemy != nullptr) {
			// �uy_distribution�v��-0.5f * 600 = -300,�` 0.5f * 600 = 300 �͈̗̔͂��������o���ϐ�.
			const std::uniform_real_distribution<float> 
				y_distribution(-0.5f * windowHeight, 0.5f *  windowHeight);		

			// 09��@�ۑ�3�p�ux_distribution�v��-0.5f * 800 + 400 = 0,�` 0.5f * 800 = 400 �͈̗̔͂��������o���ϐ�.
			const std::uniform_real_distribution<float>
				x_distribution(-0.5f * windowWidth + 400, 0.5f *  windowWidth);

			// �������̓G�o���ʒu
			/*enemy->spr = Sprite("Res/Objects.png",
				glm::vec3(0.5f * windowWidth, y_distribution(random), 0),
				Rect(480, 0, 32, 32));*/

			// 09��@�ۑ�3�p�o���ʒu
			enemy->spr = Sprite("Res/Objects.png",
				glm::vec3(x_distribution(random), 0.5f *  windowHeight, 0),
				Rect(480, 0, 32, 32));
			
			// Animator�̐ݒ�.
			enemy->spr.Animator(FrameAnimation::Animate::Create(tlEnemy));

			// �G�̈ړ�(�g�E�B�[�j���O)

			// ���꒼���̈ړ�
			/*enemy->spr.Tweener(TweenAnimation::Animate::Create(
				TweenAnimation::MoveBy::Create(
					5.0f, glm::vec3(-1000, 0, 0), TweenAnimation::EasingType::Linear)));*/
			namespace TA = TweenAnimation;

			// 09��@1.1
			/*TA::SequencePtr seq = TA::Sequence::Create(2);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-200, 100, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-200, -100, 0), TA::EasingType::Linear));
			enemy->spr.Tweener(TA::Animate::Create(seq));*/

			// 09��@�ۑ�1
			/*TA::SequencePtr seq = TA::Sequence::Create(4);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 100, 0), TA::EasingType::Linear));			
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, -100, 0), TA::EasingType::Linear));			
			enemy->spr.Tweener(TA::Animate::Create(seq));*/				
			
			// 09��@�ۑ�2
			/*TA::SequencePtr seq = TA::Sequence::Create(4);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 0, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 0, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::Linear));
			enemy->spr.Tweener(TA::Animate::Create(seq));*/

			// 09��@1.2
			/*TA::SequencePtr seq = TA::Sequence::Create(2);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::EaseInOut,TA::Target::Y));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
			TA::ParallelizePtr par = TA::Parallelize::Create(1);
			par->Add(seq);
			par->Add(TA::MoveBy::Create(8, glm::vec3(-1000, 0, 0), TA::EasingType::Linear, TA::Target::X));
			enemy->spr.Tweener(TA::Animate::Create(par));*/

			// 09��@�ۑ�3 �W�O�U�O�ړ�
			/*TA::SequencePtr seq = TA::Sequence::Create(2);
			seq->Add(TA::MoveBy::Create(4, glm::vec3(-400, -150, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(4, glm::vec3(400, -150, 0), TA::EasingType::Linear));			
			enemy->spr.Tweener(TA::Animate::Create(seq));*/

			//  09��@�ۑ�3 x���W�����_���o��ver
			TA::SequencePtr seq = TA::Sequence::Create(3);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-400, 0, 0), TA::EasingType::EaseInOut, TA::Target::X));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(400, 0, 0), TA::EasingType::EaseInOut, TA::Target::X));
			TA::ParallelizePtr par = TA::Parallelize::Create(1);
			par->Add(seq);
			par->Add(TA::MoveBy::Create(6, glm::vec3(0, -900, 0), TA::EasingType::Linear, TA::Target::Y));			
			enemy->spr.Tweener(TA::Animate::Create(par));	
			
			// �G�̓����蔻��.
			enemy->collisionShape = Rect(-16, -16, 32, 32);
			// �G�̗̑�.
			enemy->health = 3;

			// �utime_distribution�v�� 0.5�`4.0�͈̗̔͂��������o���ϐ�.(�ۑ�1�̂���)
			const std::uniform_real_distribution<float>time_distribution(0.5f, 4.0f);	// 0.5�`4.0�͈̗̔͂��������.
			// ���̓G���o������܂ł̎��Ԃ�ݒ肷��.
			enemyGenerationTimer = time_distribution(random);	// 0.5�`4.0�͈̗̔͂������~����
			
			// const std::normal_distribution<float>time_distribution(0.125f, 4.0f);
			// enemyGenerationTimer = randomTime(0.5, 4.0);	
		}
	}
	#endif
	// Actor�̍X�V.
	updateEnemies(deltaTime);
	updateActorList(std::begin(enemyList), std::end(enemyList), deltaTime); 
	updateActorList(std::begin(enemyBulletList), std::end(enemyBulletList), deltaTime);
	updateActorList(std::begin(playerBulletList), std::end(playerBulletList), deltaTime);
	updateActorList(std::begin(playerLaserList), std::end(playerLaserList), deltaTime);
	updateActorList(std::begin(effectList), std::end(effectList), deltaTime);
	updateActorList(std::begin(itemList), std::end(itemList), deltaTime);
	
	// �O�����[�U�[�̈ړ�.
	if (playerLaserList[0].health > 0) {
		// ���[�U�[(����)�̈ړ�.
		const float laserSpeed = 1600.0f;
		const glm::vec3 posFiringPoint = sprPlayer.spr.Position();
		glm::vec3 posHead = playerLaserList[2].spr.Position();
		posHead.x += laserSpeed * deltaTime;
		if (posHead.x > windowWidth * 0.5f) {
			posHead.x = windowWidth * 0.5f;
		}
		posHead.y = posFiringPoint.y;
		playerLaserList[2].spr.Position(posHead);

		// ���[�U�[(����)�̈ړ�.
		const float halfHeadSize = playerLaserList[2].spr.Rectangle().size.x * 0.5f;
		const float halfTailSize = playerLaserList[0].spr.Rectangle().size.x * 0.5f;
		const float bodySize = playerLaserList[1].spr.Rectangle().size.x;
		const float bodyLength = posHead.x - posFiringPoint.x - halfHeadSize - halfTailSize;
				
		glm::vec3 posBody = playerLaserList[1].spr.Position();
		posBody.x = posFiringPoint.x + (posHead.x - posFiringPoint.x) * 0.5f;
		posBody.y = posFiringPoint.y;
		playerLaserList[1].spr.Position(posBody);
		playerLaserList[1].spr.Scale(glm::vec2(bodyLength / bodySize, 1));
		playerLaserList[1].collisionShape = Rect(-bodyLength * 0.5f, -4, bodyLength, 8);

		// ���[�U�[(����)�̈ړ�.		
		playerLaserList[0].spr.Position(posFiringPoint);

		// ������[�U�[�̈ړ�.
		if (weaponLevel >= weaponLevelMax) {
			// ���[�U�[(����)�̈ړ�.
			const float laserSpeed2 = -1600.0f;
			const glm::vec3 posFiringPoint = sprPlayer.spr.Position();
			glm::vec3 posHead2 = playerLaserList[5].spr.Position();
			posHead2.x += laserSpeed2 * deltaTime;
			if (posHead2.x < -windowWidth * 0.5f) {
				posHead2.x = -windowWidth * 0.5f;
			}
			posHead2.y = posFiringPoint.y;
			playerLaserList[5].spr.Position(posHead2);

			// ���[�U�[(����)�̈ړ�.
			const float halfHeadSize2 = playerLaserList[5].spr.Rectangle().size.x * 0.5f;
			const float halfTailSize2 = playerLaserList[3].spr.Rectangle().size.x * 0.5f;
			const float bodySize2 = playerLaserList[4].spr.Rectangle().size.x;
			const float bodyLength2 = -posHead2.x + posFiringPoint.x + halfHeadSize2 + halfTailSize2;

			glm::vec3 posBody2 = playerLaserList[3].spr.Position();
			posBody2.x = posFiringPoint.x + (posHead2.x - posFiringPoint.x) * 0.5f;
			posBody2.y = posFiringPoint.y;
			playerLaserList[4].spr.Position(posBody2);
			playerLaserList[4].spr.Scale(glm::vec2(bodyLength2 / bodySize2, 1));
			playerLaserList[4].collisionShape = Rect(-bodyLength2 * 0.5f, -4, bodyLength2, 8);

			// ���[�U�[(����)�̈ړ�.		
			playerLaserList[3].spr.Position(posFiringPoint);
		}
	}


	//// �G�̍X�V.(�E�[���烉���_���o������ꍇ�̒���ver)	
	//for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
	//	glm::vec3 enepos = i->spr.Position();
	//	// �G�@�̏o���͈͂���ʓ��ɐ�������.
	//	const Rect enemyRect = i->spr.Rectangle();
	//	if (enepos.y < -0.5f * (windowHeight - enemyRect.size.y)) {
	//		enepos.y = -0.5f * (windowHeight - enemyRect.size.y);
	//	}
	//	else if (enepos.y > 0.5f * (windowHeight - enemyRect.size.y)) {
	//		enepos.y = 0.5f * (windowHeight - enemyRect.size.y);
	//	}
	//	i->spr.Position(enepos);

	//	if (i->health > 0) {
	//		i->spr.Update(deltaTime);
	//		// �ړ����I�����G��j�󂷂�.
	//		if (i->spr.Tweener()->IsFinished()) {
	//			i->health = 0;
	//		}
	//	}
	//}	
	if (!isStagePassed) {
		// ���@�̒e�ƓG�̏Փ˔���.
		detectCollision(std::begin(playerBulletList), std::end(playerBulletList),
			std::begin(enemyList), std::end(enemyList), playerBulletAndEnemyContactHandler);

		// ���@�̃��[�U�[�ƓG�̏Փ˔���.
		detectCollision(std::begin(playerLaserList), std::end(playerLaserList),
			std::begin(enemyList), std::end(enemyList), playerLaserAndEnemyContactHandler);

		// ���@�ƓG�̏Փ˔���.	
		detectCollision(&sprPlayer, &sprPlayer + 1,
			std::begin(enemyList), std::end(enemyList), playerAndEnemyContactHandler);

		// ���@�ƓG�̒e�̏Փ˔���.	
		detectCollision(&sprPlayer, &sprPlayer + 1,
			std::begin(enemyBulletList), std::end(enemyBulletList), playerAndEnemyBulletContactHandler);

		// ���@�ƃA�C�e���̏Փ˔���.	
		detectCollision(&sprPlayer, &sprPlayer + 1,
			std::begin(itemList), std::end(itemList), playerAndItemContactHandler);
	}	
}

/**
* �Q�[���̏�Ԃ�`�悷��.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
*/
void render(GLFWEW::WindowRef window) {
	if (gamestate == gamestateTitle) {
		render(window, &titleScene);
		return;
	}
	else if (gamestate == gamestateGameOver) {
		render(window, &gameOverScene);
		return;
	}
	else if (gamestate == gamestateGameClear) {
		render(window, &gameClearScene);
		return;
	}

	renderer.BeginUpdate();
	renderer.AddVertices(sprBackground);
	if (sprPlayer.health > 0) {
		renderer.AddVertices(sprPlayer.spr);
	}

	// Actor�̕\��.
	renderActorList(std::begin(enemyList), std::end(enemyList));
	renderActorList(std::begin(enemyBulletList), std::end(enemyBulletList));
	renderActorList(std::begin(playerBulletList), std::end(playerBulletList));
	renderActorList(std::begin(playerLaserList), std::end(playerLaserList));
	renderActorList(std::begin(effectList), std::end(effectList));
	renderActorList(std::begin(itemList), std::end(itemList));

	renderer.EndUpdate();
	renderer.Draw({ windowWidth, windowHeight });

	// �X�R�A�𕶎��ɕϊ����ĕ\��.
	fontRenderer.BeginUpdate();
	char str[64];
	snprintf(str, sizeof(str), "%08d", score);
	fontRenderer.AddString(glm::vec2(-100, 300), str);

	snprintf(str, sizeof(str), "STAGE: %02d", mainScene.stageNo);
	fontRenderer.Scale(glm::vec2(0.5f, 0.5f));
	fontRenderer.Color(glm::vec4(0.75f, 0.75f, 1.0f, 1.0f));
	fontRenderer.Thickness(0);
	fontRenderer.AddString(glm::vec2(-384, 300), str);
	fontRenderer.Scale(glm::vec2(1.0f, 1.0f));
	fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	fontRenderer.Thickness(0.33f);
	
	fontRenderer.EndUpdate();
	fontRenderer.Draw();

	window.SwapBuffers();
}


/**
* �Փ˂����o����.
*
* @param firstA		�Փ˂�����z��A�̐擪�|�C���^.
* @param lastA		�Փ˂�����z��A�̏I�[�|�C���^.
* @param firstB		�Փ˂�����z��B�̐擪�|�C���^.
* @param lastB		�Փ˂�����z��B�̏I�[�|�C���^.
* @param handler	A-B�ԂŏՓ˂����o���ꂽ�Ƃ��Ɏ��s����֐�.
*/
void detectCollision(Actor* firstA, Actor* lastA, Actor* firstB, Actor* lastB, CollisionHandlerType handler) {
	// a��b�̏Փ˔���.
	for (Actor* a = firstA; a != lastA; ++a) {		
		if (a->health <= 0) {
			continue;
		}
		Rect rectA = a->collisionShape;
		rectA.origin += glm::vec2(a->spr.Position());
		for (Actor*b = firstB; b != lastB; ++b) {			
			if (b->health <= 0) {
				continue;
			}
			Rect rectB = b->collisionShape;
			rectB.origin += glm::vec2(b->spr.Position());
			if (detectCollision(&rectA, &rectB)) {
				handler(a, b);				

				if (a->health <= 0) {
					break;
				}				
			}
		}
	}
}

/**
* ���@�̒e�ƓG�̏Փ˂���������.
*
* @param bullet		�Փ˂�����z��A�̐擪�|�C���^.
* @param enemy		�Փ˂�����z��A�̏I�[�|�C���^.
*/
void playerBulletAndEnemyContactHandler(Actor* bullet, Actor* enemy) {
	const int bulletDamage = bullet->health;
	const int enemyDamage = enemy->health;
	bullet->health -= enemyDamage;
	enemy->health -= bulletDamage;
	if (enemy->health <= 0) {
		score += enemy->score;		// �G��j�󂵂��瓾�_�𑝂₷.
		enemyblastAnimation(enemy);
	}
}

/**
* ���@�̃��[�U�[�ƓG�̏Փ˂���������.
*
* @param laser		���@�̃��[�U�[�̃|�C���^.
* @param enemy		�G�̃|�C���^.
*/
void playerLaserAndEnemyContactHandler(Actor* laser, Actor* enemy) {
	
	enemy->health -= laser->health;
	if (enemy->health <= 0) {
		score += enemy->score;		// �G��j�󂵂��瓾�_�𑝂₷.
		enemyblastAnimation(enemy);
	}
}

/**
* ���@�ƓG�̏Փ˂���������.
*
* @param player		���@�̃|�C���^.
* @param enemy		�G�̃|�C���^.
*/
void playerAndEnemyContactHandler(Actor* player, Actor* enemy) {
	
	if (player->health > enemy->health) {
		player->health -= enemy->health;
		enemy->health = 0;
	}
	else {
		enemy->health -= player->health;
		player->health = 0;
	}
	if (enemy->health <= 0) {
		score += enemy->score;
		enemyblastAnimation(enemy);
	}

	if (player->health <= 0) {
		playerblastAnimation(player);
		waitTimer = 2.0f;
	}
}

/**
* ���@�ƓG�̒e�̏Փ˂���������.
*
* @param player		���@�̃|�C���^.
* @param ebullet	�G�̒e�̃|�C���^.
*/
void playerAndEnemyBulletContactHandler(Actor* player, Actor* ebullet) {

	const int ebulletDamage = ebullet->health;
	const int playerDamage = player->health;
	ebullet->health -= playerDamage;
	player->health -= ebulletDamage;
	
	if (player->health <= 0) {
		playerblastAnimation(player);
		waitTimer = 2.0f;
	}
}

/**
* ���@�ƃA�C�e���̏Փ˂���������.
*
* @param player		���@�̃|�C���^.
* @param item		�A�C�e���̃|�C���^.
*/
void playerAndItemContactHandler(Actor* player, Actor* item) {
		
	item->health = 0;
	sePowerUp->Play();

	// ���@�̕������������.
	++weaponLevel;
	if (weaponLevel > weaponLevelMax) {
		weaponLevel = weaponLevelMax;
		score += item->score;
	}
}

/**
* ���@�̃��[�U�[���~����.
*/
void stopPlayerLaser() {
	if (playerLaserList[0].health > 0) {
		for (Actor* i = std::begin(playerLaserList); i != std::end(playerLaserList); ++i) {
			i->spr = Sprite();
			i->health = 0;
		}
		sePlayerLaser->Stop();
	}
}

/**
* �}�b�v�f�[�^�����ēG��A�C�e�����o��������.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���.
*/
void generateObjectFromMap(float deltaTime) {
	if (isEndOfMap) {
		return;
	}
	const TiledMap::Layer& tiledMapLayer = enemyMap.GetLayer(0);
	const glm::vec2 tileSize = enemyMap.GetTileSet(tiledMapLayer.tilesetNo).size;
	// �G�z�u�}�b�v�Q�ƈʒu�̍X�V.
	const float enemyMapScrollSpeed = 100;	// �X�V���x.
	mapCurrentPosX += enemyMapScrollSpeed * deltaTime;
	if (mapCurrentPosX >= tiledMapLayer.size.x * tileSize.x) {
		// �I�[�𒴂������~.
		isEndOfMap = true;
		return;
	}
	// ���̗�ɓ��B������f�[�^��ǂ�.
	if (mapCurrentPosX - mapProcessedX >= tileSize.x) {
		mapProcessedX += tileSize.x;
		const int mapX = static_cast<int>(mapProcessedX / tileSize.x);
		for (int mapY = 0; mapY < tiledMapLayer.size.y; ++mapY) {
			
			const int tileId = tiledMapLayer.At(mapY, mapX);

			const MapObjectData* data = findMapObjectData(tileId);

			if (data != nullptr) {
				// �󂢂Ă���(�j�󂳂�Ă���)�G�\���̂̌���.
				Actor* enemy = findAvailableActor(std::begin(enemyList), std::end(enemyList));

				// �󂢂Ă���\���̂�����������A������g���ēG���o��������.
				if (enemy != nullptr) {
					// �������̓G�o���ʒu
					const float y = (windowHeight - data->imageRect.size.y) * 0.5f - static_cast<float>(mapY * tileSize.y);
					const float x = (windowWidth + data->imageRect.size.x) * 0.5f;

					enemy->spr = Sprite("Res/Objects.png", glm::vec3(x, y, 0), data->imageRect);
					// Animator�̐ݒ�.
					enemy->spr.Animator(FrameAnimation::Animate::Create(data->tl));
					// �G�̈ړ�(�g�E�B�[�j���O)					
					enemy->spr.Tweener(TweenAnimation::Animate::Create(data->createTween()));
					// �G�̓����蔻��.
					enemy->collisionShape = data->collisionShape;
					// �G�̗̑�.
					enemy->health = data->health;
					// �G�̓��_.
					enemy->score = data->score;
					// �^�C�}�[�̐ݒ�.
					enemy->timer = 0;
					// ID�̐ݒ�
					enemy->id = data->id;
					// �{�X�̃|�C���^��ݒ肷��.
					if (data->id == tileId_Boss && boss == nullptr) {
						boss = enemy;
					}
				}
			}			
			else if (tileId == tileId_PowerUpItem) {
				// �󂢂Ă���A�C�e���\���̂̌���.
				Actor* item = findAvailableActor(std::begin(itemList), std::end(itemList));

				// �󂢂Ă���\���̂�����������A������g���ăA�C�e�����o��������.
				if (item != nullptr) {
					// �������̃A�C�e���o���ʒu
					const float y = windowHeight * 0.5f - static_cast<float>(mapY * tileSize.x);
					item->spr = Sprite("Res/Objects.png", glm::vec3(0.5f * windowWidth, y, 0), Rect(160, 32, 32, 32));

					// �A�C�e���̈ړ�(�g�E�B�[�j���O)
					namespace TA = TweenAnimation;
					item->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(16, glm::vec3(-1000, 0, 0))));
					// �A�C�e���̓����蔻��.
					item->collisionShape = Rect(-16, -16, 32, 32);
					// �A�C�e���̗̑�.
					item->health = 1;
					// �A�C�e���̓��_.
					item->score = 1000;
				}
			}

		}
	}
}

/**
* �G�̏�Ԃ��X�V����.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���.
*/
void updateEnemies(float deltaTime) {
	const glm::vec3 playerPos = sprPlayer.spr.Position();
	for (Actor* i = std::begin(enemyList); i != std::end(enemyList); i++) {
		if (i->health <= 0) {
			continue;
		}

		// ��莞�Ԃ��Ƃɒe�𔭎�.
		const float shotInterval = 2.0f;
		i->timer += deltaTime;
		if (i->timer < shotInterval) {
			continue;
		}
		i->timer -= shotInterval;

		// ��ʊO�ɂ���Ƃ��͔��˂��Ȃ�.
		const glm::vec3 enemyPos = i->spr.Position();
		if (enemyPos.x <= windowWidth * -0.5f || enemyPos.x > windowWidth * 0.5f) {
			continue;
		}
		if (enemyPos.y <= windowHeight * -0.5f || enemyPos.y > windowHeight * 0.5f) {
			continue;
		}

		// ���@�̕������v�Z.
		const glm::vec3 distance = playerPos - i->spr.Position();
		const float radian = std::atan2(distance.y, distance.x);
		const float c = std::cos(radian);
		const float s = std::sin(radian);

		// ���^�̓G�̍U��.
		if (i->id == tileId_SmallEnemy) {
			// �󂢂Ă���G�̒e������.
			Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
			if (bullet == nullptr) {
				continue;
			}
			// ���@�Ɍ������Ēe�𔭎�.
			bullet->spr = Sprite("Res/Objects.png", i->spr.Position(), Rect(464, 0, 16, 16));
			namespace TA = TweenAnimation;
			bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(8, glm::vec3(1200.0f*c, 1200.0f*s, 0))));
			bullet->spr.Rotation(radian + 3.14f);
			bullet->collisionShape = Rect(-4, -4, 8, 8);
			bullet->health = 1;
		}

		// ���^�̓G�̍U��.
		if (i->id == tileId_MediumEnemy) {
			Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
			if (bullet == nullptr) {
				continue;
			}
			glm::vec3 pos = i->spr.Position();
			bullet->spr = Sprite("Res/Objects.png", pos, Rect(464, 0, 16, 16));
			namespace TA = TweenAnimation;
			bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(6, glm::vec3(1200.0f*c, 1200.0f*s, 0))));
			bullet->spr.Rotation(radian + 3.14f);
			bullet->spr.Scale(glm::vec2(2, 1));
			bullet->collisionShape = Rect(-8, -4, 16, 8);
			bullet->health = 1;
		}

		// ��^�̓G�̍U��.
		if (i->id == tileId_LargeEnemy) {
			for (int n = 0; n < 2; ++n) {
				Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
				if (bullet == nullptr) {
					continue;
				}
				glm::vec3 pos = i->spr.Position();
				pos.y += (float)(16 - n * 32);
				bullet->spr = Sprite("Res/Objects.png", pos, Rect(464, 0, 16, 16));
				namespace TA = TweenAnimation;
				bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(4, glm::vec3(-1200, 0, 0))));
				bullet->spr.Scale(glm::vec2(3, 1));
				bullet->collisionShape = Rect(-12, -4, 24, 8);
				bullet->health = 1;
			}
			continue;
		}

		// �{�X�̍U��.
		if (i->id == tileId_Boss) {
			for (int l = 0; l < 4; ++l) {
				Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
				if (bullet == nullptr) {
					continue;
				}
				glm::vec3 pos = i->spr.Position();
				pos.y += (float)(112 - l * 32);
				// ���@�Ɍ������Ēe�𔭎�.
				bullet->spr = Sprite("Res/Objects.png", pos, Rect(448, 0, 16, 16));
				namespace TA = TweenAnimation;
				bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(8, glm::vec3(1400.0f*c, 1400.0f*s, 0))));
				bullet->spr.Rotation(radian + 3.14f);
				bullet->collisionShape = Rect(-4, -4, 8, 8);
				bullet->health = 1;
			}
			for (int m = 0; m < 4; ++m) {
				Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
				if (bullet == nullptr) {
					continue;
				}
				glm::vec3 pos = i->spr.Position();
				pos.y += (float)(-48 - m * 32);
				// ���@�Ɍ������Ēe�𔭎�.
				bullet->spr = Sprite("Res/Objects.png", pos, Rect(448, 0, 16, 16));
				namespace TA = TweenAnimation;
				bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(8, glm::vec3(1400.0f*c, 1400.0f*s, 0))));
				bullet->spr.Rotation(radian + 3.14f);
				bullet->collisionShape = Rect(-4, -4, 8, 8);
				bullet->health = 1;
			}
			for (int n = 0; n < 2; ++n) {				
				Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
				if (bullet == nullptr) {
					continue;
				}
				glm::vec3 pos = i->spr.Position();
				pos.y += (float)(16 - n * 32);
				bullet->spr = Sprite("Res/Objects.png", pos, Rect(448, 48, 32, 16));
				namespace TA = TweenAnimation;
				bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(4, glm::vec3(-1200, 0, 0))));
				bullet->spr.Scale(glm::vec2(10, 2));
				bullet->collisionShape = Rect(-80, -8, 160, 16);
				bullet->health = 3;
			}
			continue;
		}
	}
}

/**
* enemy�����A�j���[�V�����֐�.
*/
void enemyblastAnimation(Actor* enemy) {
	Actor* blast = findAvailableActor(std::begin(effectList), std::end(effectList));
	if (blast != nullptr) {
		blast->spr = Sprite("Res/Objects.png", enemy->spr.Position());
		blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
		namespace TA = TweenAnimation;
		blast->spr.Tweener(TA::Animate::Create(
			TA::Rotation::Create(20 / 60.0f, glm::pi<float>() * 0.5f)));
		blast->health = 1;
		seBlast->Play();		// ���������Đ�.
	}
}

/**
* player�����A�j���[�V�����֐�.
*/
void playerblastAnimation(Actor* player) {
	Actor* blast = findAvailableActor(std::begin(effectList), std::end(effectList));
	if (blast != nullptr) {
		blast->spr = Sprite("Res/Objects.png", player->spr.Position());
		blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
		namespace TA = TweenAnimation;
		blast->spr.Tweener(TA::Animate::Create(
			TA::Rotation::Create(20 / 60.0f, glm::pi<float>() * 0.5f)));
		blast->spr.Scale(glm::vec2(2, 2));			// ���@�̔����͏����傫�߂�
		blast->health = 1;
		seBreak->Play();		// ���������Đ�.
	}
}

// �ݒ肵���ŏ��l�ƍő�l���痐���𓾂�randomTime�֐�.
int randomTime(float min, float max)
{
	return std::uniform_real_distribution<float>(min, max)(random);
}


